
#include <ruby.h>
#ifdef RUBY_1_8
#include <st.h>
#include <rubyio.h>
#else       /* ifdef RUBY_1_9 */
#include <ruby/st.h>
#include <ruby/io.h>
#endif
#include <stdlib.h>
#include <zlib.h>
#include <sys/stat.h>

#include <hfs/hfsplus.h>
#include <dirent.h>
#include <hfs/hfslib.h>
#include "abstractfile.h"
#include "pe.h"

char *pe_pad = "PADDINGXXPADDING";
char endianness;

void TestByteOrder()
{
	short int word = 0x0001;
	char *byte = (char *) &word;
	endianness = byte[0] ? IS_LITTLE_ENDIAN : IS_BIG_ENDIAN;
}

unsigned short int swap16(unsigned short int x)
{
  return (x >> 8) | (x << 8);
}

unsigned int swap32(unsigned int x)
{
  return (x >> 24) | (x << 24) |
    ((x << 8) & 0x00FF0000) | 
    ((x >> 8) & 0x0000FF00);
}

#if 0
unsigned long long swap64(unsigned long long x)
{
  return (x >> 56) | (x << 56) |
    ((x << 40) & 0x00FF000000000000) | 
    ((x >> 40) & 0x000000000000FF00) |
    ((x << 24) & 0x0000FF0000000000) | 
    ((x >> 24) & 0x0000000000FF0000) |
    ((x << 8)  & 0x000000FF00000000) | 
    ((x >> 8)  & 0x00000000FF000000);
}
#endif

typedef struct {
  struct dos_header_t dos_header;
  struct image_file_header_t image_file_header;
  struct image_optional_header_t image_optional_header;
  struct section_header_t section_header;
  struct resource_dir_t resource_dir;
  struct resource_dir_entry_t resource_dir_entry;
  struct resource_data_t resource_data;
  int resource_type[16];
  struct resource_data_t resources[16];
  int resource_count;
  int ids, resids;
  unsigned int namestart, datastart, datapos, dataend, vdelta;
  char signature[4];
  VALUE adds, proc;
  FILE *file, *out;
} binject_exe_t;

#define BUFSIZE 16384

static void
binject_exe_mark(binject_exe_t *binj)
{
  rb_gc_mark_maybe(binj->adds);
}

static void
binject_exe_free(binject_exe_t *binj)
{
  if (binj->file != NULL)
    fclose(binj->file);
  free(binj);
}

VALUE
binject_exe_alloc(VALUE klass)
{
  binject_exe_t *binj = ALLOC(binject_exe_t);
  MEMZERO(binj, binject_exe_t, 1);
  binj->adds = rb_ary_new();
  binj->proc = Qnil;
  return Data_Wrap_Struct(klass, binject_exe_mark, binject_exe_free, binj);
}

#define BINJ_READ(binj, stct) \
  fread(&(stct), sizeof(stct), 1, binj->file)
#define BINJ_READ_POS(binj, stct, pos) \
  { \
    int mark = ftell(binj->file); \
    fseek(binj->file, pos, SEEK_SET); \
    BINJ_READ(binj, stct); \
    fseek(binj->file, mark, SEEK_SET); \
  }
#define BINJ_WRITE(binj, stct) \
  fwrite(&(stct), sizeof(stct), 1, binj->out)

void
binject_exe_resources(binject_exe_t *binj, int offset, int level, int res_type)
{
  int count = 0, i = 0;
  BINJ_READ_POS(binj, binj->resource_dir, binj->section_header.PointerToRawData + offset);
  FLIPENDIANLE(binj->resource_dir.NumberOfNamedEntries);
  FLIPENDIANLE(binj->resource_dir.NumberOfIdEntries);
  if (level == 1)
    binj->ids++;
  if (level == 2)
    binj->resids++;

  offset += 16;
  count = binj->resource_dir.NumberOfNamedEntries + binj->resource_dir.NumberOfIdEntries;
  for (i = 0; i < count; i++)
  {
    BINJ_READ_POS(binj, binj->resource_dir_entry, binj->section_header.PointerToRawData + offset);
    FLIPENDIANLE(binj->resource_dir_entry.OffsetToData);

    if ((binj->resource_dir_entry.OffsetToData & 0x80000000) == 0)
    {
      struct resource_data_t *rdata = &binj->resource_data;
      if (res_type == 0 || res_type == 10 || res_type == 6)
      {
        binj->resource_type[binj->resource_count] = res_type;
        rdata = &binj->resources[binj->resource_count];
        binj->resource_count++;
      }

      BINJ_READ_POS(binj, *rdata, binj->section_header.PointerToRawData + 
        binj->resource_dir_entry.OffsetToData);
    }
    else
    {
      if (level == 0)
      {
        // print_resource_type(binj->resource_dir_entry.Name, level);
        binject_exe_resources(binj, 
          (binj->resource_dir_entry.OffsetToData & 0x7fffffff), level + 1,
          binj->resource_dir_entry.Name);
      }
      else
      {
        binject_exe_resources(binj, 
          (binj->resource_dir_entry.OffsetToData & 0x7fffffff), level + 1,
          res_type);
      }
    }
    offset += 8;
  }
}

#define BINJ_COPY(dest, src, stct, off, off2) \
  src = (stct *)(buf + off); \
  dest = (stct *)(out + off2); \
  MEMCPY(dest, src, stct, 1)

VALUE
binject_exe_get_key(binject_exe_t *binj, VALUE klass, int index)
{
  long i;
  for (i = 0; i < RARRAY_LEN(binj->adds); i++)
  {
    VALUE obj = rb_ary_entry(binj->adds, i);
    if (rb_obj_is_kind_of(rb_ary_entry(obj, 1), klass))
    {
      if (index == 0) return rb_ary_entry(obj, 0);
      index--;
    }
  }
  return Qnil;
}

unsigned int
binject_exe_write_name(binject_exe_t *binj, char *out, VALUE klass, int index)
{
  long i;
  char *str;
  unsigned short int *datlen;
  VALUE key = binject_exe_get_key(binj, klass, index);
  datlen = (unsigned short int *)(out + binj->namestart);
  *datlen = (unsigned short int)RSTRING_LEN(key);
  FLIPENDIANLE(*datlen);
  str = RSTRING_PTR(key);
  for (i = 0; i < RSTRING_LEN(key); i++)
  {
    datlen = (unsigned short int *)(out + binj->namestart + 2 + (i * 2));
    *datlen = (unsigned short int)str[i];
    FLIPENDIANLE(*datlen);
  }
  return 2 + (RSTRING_LEN(key) * 2);
}

VALUE
binject_exe_get_type(binject_exe_t *binj, VALUE klass, int index)
{
  long i;
  for (i = 0; i < RARRAY_LEN(binj->adds); i++)
  {
    VALUE obj = rb_ary_entry(rb_ary_entry(binj->adds, i), 1);
    if (rb_obj_is_kind_of(obj, klass))
    {
      if (index == 0) return obj;
      index--;
    }
  }
  return Qnil;
}

int
binject_exe_count_type(binject_exe_t *binj, VALUE klass)
{
  long i, count = 0;
  for (i = 0; i < RARRAY_LEN(binj->adds); i++)
  {
    VALUE obj = rb_ary_entry(binj->adds, i);
    if (rb_obj_is_kind_of(rb_ary_entry(obj, 1), klass))
      count++;
  }
  return count;
}

int
binject_exe_new_ids(binject_exe_t *binj)
{
  long i = 0;
  if (binject_exe_count_type(binj, rb_cString)) i++;
  if (binject_exe_count_type(binj, rb_cFile)) i++;
  return i;
}

int
binject_exe_names_len(binject_exe_t *binj)
{
  int i, len = 0;
  for (i = 0; i < RARRAY_LEN(binj->adds); i++)
  {
    VALUE obj = rb_ary_entry(rb_ary_entry(binj->adds, i), 0);
    len += 2 + (RSTRING_LEN(obj) * 2);
  }
  return len;
}

long AnotherGetFileSize ( FILE * hFile )
{
  long lCurPos, lEndPos;
  if ( hFile == NULL )
    return 0;
  lCurPos = ftell ( hFile );
  fseek ( hFile, 0, 2 );
  lEndPos = ftell ( hFile );
  fseek ( hFile, lCurPos, 0 );
  return lEndPos;
}

unsigned int
binject_exe_file_size(VALUE obj)
{
  struct stat st;
  rb_io_t *fptr;
  FILE *fres;
  GetOpenFile(obj, fptr);
  rb_io_check_readable(fptr);
#ifndef RUBY_1_8
  fres = rb_io_stdio_file(fptr);
  st.st_size = AnotherGetFileSize(fres);
#else
  fres = GetReadFile(fptr);
  fstat(fileno(fres), &st);
#endif
  return (unsigned int)st.st_size;
}

int
binject_exe_data_len(binject_exe_t *binj)
{
  unsigned int i, len = 0;
  for (i = 0; i < RARRAY_LEN(binj->adds); i++)
  {
    VALUE obj = rb_ary_entry(rb_ary_entry(binj->adds, i), 1);
    if (rb_obj_is_kind_of(obj, rb_cFile))
      len += binject_exe_file_size(obj);
    else if (rb_obj_is_kind_of(obj, rb_cFile))
      len += RSTRING_LEN(obj);
  }
  return len;
}

void
binject_exe_string_copy(binject_exe_t *binj, char *str, unsigned int size, unsigned int pos, VALUE proc)
{
  int mark = ftell(binj->out);
  fseek(binj->out, pos, SEEK_SET);
  fwrite(str, sizeof(char), size, binj->out);
  if (RTEST(proc))
    rb_funcall(proc, rb_intern("call"), 1, INT2NUM(size));
  fseek(binj->out, mark, SEEK_SET);
}

void
binject_exe_file_copy(FILE *file, FILE *out, unsigned int size, unsigned int pos1, unsigned int pos2, VALUE proc)
{
  char buf[BUFSIZE];
  int mark1 = ftell(file), mark2 = ftell(out);
  fseek(file, pos1, SEEK_SET);
  fseek(out, pos2, SEEK_SET);
  while (size > 0)
  {
    unsigned int len = size > BUFSIZE ? BUFSIZE : size;
    fread(buf, sizeof(char), len, file);
    fwrite(buf, sizeof(char), len, out);
    if (RTEST(proc))
      rb_funcall(proc, rb_intern("call"), 1, INT2NUM(len));
    size -= len;
  }
  fseek(file, mark1, SEEK_SET);
  fseek(out, mark2, SEEK_SET);
}

#ifndef RUBY_1_8
void
binject_exe_file_copy1(rb_io_t *fptr, FILE *out, unsigned int size, unsigned int pos1, unsigned int pos2, VALUE proc)
{
  char buf[BUFSIZE];
  FILE *file;
  file = rb_io_stdio_file(fptr);
  
  int mark1 = ftell(file), mark2 = ftell(out);
  fseek(file, pos1, SEEK_SET);
  fseek(out, pos2, SEEK_SET);
  while (size > 0)
  {
    unsigned int len = size > BUFSIZE ? BUFSIZE : size;
    //fread(buf, sizeof(char), len, file);
    read(fptr->fd, buf, sizeof(char) * len);
    fwrite(buf, sizeof(char), len, out);
    if (RTEST(proc))
      rb_funcall(proc, rb_intern("call"), 1, INT2NUM(len));
    size -= len;
  }
  fseek(file, mark1, SEEK_SET);
  fseek(out, mark2, SEEK_SET);
}
#endif 

int
binject_exe_offset(binject_exe_t *binj, int level, int res_type)
{
  unsigned int offset = binject_exe_new_ids(binj) * 8;
  if (level > 0 || (level == 0 && res_type > 10))
    offset += (RARRAY_LEN(binj->adds) * 8) + (binject_exe_new_ids(binj) * 16);
  if (level > 1 || (level == 1 && res_type > 10))
    offset += (RARRAY_LEN(binj->adds) * 24);
  if (level == 2 && res_type > 10)
    offset += RARRAY_LEN(binj->adds) * 16;
  return offset;
}

#define BINJ_PAD(num, base) \
  ((((num - 1) / base) + 1) * base)

//
// This rewrite assumes the .rsrc section doesn't include any strings or user data
// and that the other data sections include only items without names and with one
// locale leaf each.
//
int
binject_exe_rewrite(binject_exe_t *binj, char *buf, char *out, int offset, int offset2, int level, int res_type)
{
  int count = 0, i = 0, ins = 0;
  unsigned int newoff;
  struct resource_dir_t *rd, *rd2;
  struct resource_dir_entry_t *rde, *rde2;
  struct resource_data_t *rdat, *rdat2;
  // printf("DIR[%d]: %x TO %x\n", level, offset, offset2);
  BINJ_COPY(rd2, rd, struct resource_dir_t, offset, offset2);
  FLIPENDIANLE(rd->NumberOfIdEntries);
  FLIPENDIANLE(rd->NumberOfNamedEntries);
  FLIPENDIANLE(rd2->NumberOfIdEntries);
  FLIPENDIANLE(rd2->NumberOfNamedEntries);
  if (level == 0)
  {
    ins = binject_exe_new_ids(binj);
    rd2->NumberOfIdEntries += binject_exe_new_ids(binj);
    binj->ids = rd2->NumberOfIdEntries;
    binj->namestart = 16 +
      (rd2->NumberOfIdEntries * (8 + sizeof(struct resource_data_t))) + 
      ((binj->resids + RARRAY_LEN(binj->adds)) * ((2 * 8) + (2 * sizeof(struct resource_dir_t))));
    binj->datastart = BINJ_PAD(binj->namestart + binject_exe_names_len(binj), 4);
    binj->datapos = binj->section_header.PointerToRawData + binj->datastart;
    binj->vdelta = binj->section_header.VirtualAddress - binj->section_header.PointerToRawData;
    // printf("DATAPOS: %x\n", binj->datapos);
  }
  else
  {
    ins = RARRAY_LEN(binj->adds);
  }
  FLIPENDIANLE(rd2->NumberOfIdEntries);
  FLIPENDIANLE(rd2->NumberOfNamedEntries);

  offset += 16;
  offset2 += 16;
  count = rd->NumberOfNamedEntries + rd->NumberOfIdEntries;
  for (i = 0; i < count; i++)
  {
    rde = (struct resource_dir_entry_t *)(buf + offset);
    FLIPENDIANLE(rde->Name);
    FLIPENDIANLE(rde->OffsetToData);
    if (level == 0)
      res_type = rde->Name;
    if (level == 0 && res_type > 10 && ins > 0)
    {
      VALUE obj, key, ctype;
      unsigned int ti = 0, i2 = 0, doff = 0, doff2 = 0, doff3 = 0, doff4 = 0,
                   btype = 0, oc = 0, padlen = 0, oo = 0;
      for (ti = 0; ti < 2; ti++)
      {
        ctype = (ti == 0 ? rb_cString : rb_cFile);
        btype = (ti == 0 ? 6 : 10);
        // printf("\nNEW ID\n");
        if (binject_exe_count_type(binj, ctype))
        {
          rde = (struct resource_dir_entry_t *)(buf + offset);
          rde2 = (struct resource_dir_entry_t *)(out + offset2);

          rde2->Name = btype;
          rde2->OffsetToData = rde->OffsetToData + binject_exe_offset(binj, 0, btype) + (ti * 16) + (oc * 8);
          // printf("STRING ENTRY[0] @ %x (%u, %x)\n", (char *)rde2 - out, rde2->Name, rde2->OffsetToData);
          oo = rde->OffsetToData & 0x7fffffff;
          doff = rde2->OffsetToData & 0x7fffffff;
          FLIPENDIANLE(rde2->Name);
          FLIPENDIANLE(rde2->OffsetToData);

          rd2 = (struct resource_dir_t *)(out + doff);
          rd2->NumberOfNamedEntries = binject_exe_count_type(binj, ctype);
          FLIPENDIANLE(rd2->NumberOfNamedEntries);
          // printf("STRING DIR[1]: %x\n", doff);

          for (i2 = 0; i2 < binject_exe_count_type(binj, ctype); i2++)
          {
            rde = (struct resource_dir_entry_t *)(buf + oo + 16);
            doff3 = rde->OffsetToData;
            FLIPENDIANLE(doff3);
            rde2 = (struct resource_dir_entry_t *)(out + doff + 16 + (i2 * 8));
            // printf("STRING ENTRY[1] @ %x / NAME(%x)\n", (char *)rde2 - out, binj->namestart);
            rde2->Name = 0x80000000 | binj->namestart;
            rde2->OffsetToData = doff3 + binject_exe_offset(binj, 1, btype) + (oc * 24);
            binj->namestart += binject_exe_write_name(binj, out, ctype, i2);

            doff2 = rde2->OffsetToData & 0x7fffffff;
            FLIPENDIANLE(rde2->Name);
            FLIPENDIANLE(rde2->OffsetToData);

            rd2 = (struct resource_dir_t *)(out + doff2);
            rd2->NumberOfIdEntries = 1;
            FLIPENDIANLE(rd2->NumberOfIdEntries);
            // printf("STRING DIR[2]: %x / %x (%x)\n", rde2->Name, rde2->OffsetToData, doff3);
            rde = (struct resource_dir_entry_t *)(buf + (doff3 & 0x7fffffff) + 16);
            doff4 = rde->OffsetToData;
            FLIPENDIANLE(doff4);

            rde2 = (struct resource_dir_entry_t *)(out + doff2 + 16);
            // printf("STRING ENTRY[2] @ %x\n", (char *)rde2 - out);
            rde2->OffsetToData = doff4 + binject_exe_offset(binj, 2, btype) + (oc * 16);
            // printf("RESDATA: %x / %x\n", doff4, rde2->OffsetToData);
            obj = binject_exe_get_type(binj, ctype, i2);
            // printf("DATA: %x\n", binj->datapos);
            rdat = (struct resource_data_t *)(out + (rde2->OffsetToData));
            rdat->OffsetToData = binj->datapos + binj->vdelta;
            if (ctype == rb_cString)
            {
              rdat->Size = RSTRING_LEN(obj);
              binject_exe_string_copy(binj, RSTRING_PTR(obj), RSTRING_LEN(obj), binj->datapos, binj->proc);
            }
            else
            {
              rb_io_t *fptr;
              rdat->Size = binject_exe_file_size(obj);
              GetOpenFile(obj, fptr);
#ifndef RUBY_1_8
#ifdef SHOES_MINGW32
              binject_exe_file_copy1(fptr, binj->out, rdat->Size, 0, binj->datapos, binj->proc);
#else
              binject_exe_file_copy(rb_io_stdio_file(fptr), binj->out, rdat->Size, 0, binj->datapos, binj->proc);
#endif
#else
              binject_exe_file_copy(GetReadFile(fptr), binj->out, rdat->Size, 0, binj->datapos, binj->proc);
#endif
            }
            binj->datapos += rdat->Size;
            padlen = BINJ_PAD(rdat->Size, 4) - rdat->Size;
            if (padlen > 0)
            {
              binject_exe_string_copy(binj, pe_pad, padlen, binj->datapos, binj->proc);
              binj->datapos += padlen;
            }

            FLIPENDIANLE(rdat->Size);
            FLIPENDIANLE(rdat->OffsetToData);
            FLIPENDIANLE(rde2->OffsetToData);
            oc++;
          }
          offset2 += 8;
        }
      }
    }

    // printf("ENTRY[%d]: %x TO %x\n", level, offset, offset2);
    BINJ_COPY(rde2, rde, struct resource_dir_entry_t, offset, offset2);

    // offset every entry to leave a hole for new stuff
    rde2->OffsetToData += binject_exe_offset(binj, level, res_type);
    if ((rde->OffsetToData & 0x80000000) == 0)
    {
      unsigned int dataoff = offset2 + rde2->OffsetToData;
      BINJ_COPY(rdat2, rdat, struct resource_data_t, rde->OffsetToData, rde2->OffsetToData);
      FLIPENDIANLE(rdat->Size);
      FLIPENDIANLE(rdat->OffsetToData);
      FLIPENDIANLE(rdat2->Size);
      FLIPENDIANLE(rdat2->OffsetToData);
      rdat2->OffsetToData = binj->datapos + binj->vdelta;
      // printf("RESDATA: %x TO %x AT %x / %x\n", rde->OffsetToData, rde2->OffsetToData, 
      //   binj->namestart, binj->datastart);
      binject_exe_file_copy(binj->file, binj->out, rdat->Size, 
        rdat->OffsetToData - binj->vdelta, binj->datapos, binj->proc);
      // printf("DATA: %x TO %x\n", rdat->OffsetToData, binj->datapos);
      binj->datapos += rdat->Size;
      binj->dataend = (rdat->OffsetToData - binj->vdelta) + rdat->Size;
      FLIPENDIANLE(rdat2->Size);
      FLIPENDIANLE(rdat2->OffsetToData);
    }
    else
    {
      if (level == 0)
      {
        binject_exe_rewrite(binj, buf, out,
          rde->OffsetToData & 0x7fffffff, rde2->OffsetToData & 0x7fffffff, level + 1,
          res_type);
      }
      else
      {
        binject_exe_rewrite(binj, buf, out,
          rde->OffsetToData & 0x7fffffff, rde2->OffsetToData & 0x7fffffff, level + 1,
          res_type);
      }
    }

    FLIPENDIANLE(rde2->Name);
    FLIPENDIANLE(rde2->OffsetToData);
    offset += 8;
    offset2 += 8;
  }
  return offset;
}

VALUE
binject_exe_load(VALUE self, VALUE file)
{
  int i, lfanew;
  binject_exe_t *binj;
  Data_Get_Struct(self, binject_exe_t, binj);
#ifndef RUBY_1_8
  binj->file = fopen(RSTRING_PTR(file), "rb");
#else
  binj->file = rb_fopen(RSTRING_PTR(file), "rb");
#endif
  BINJ_READ(binj, binj->dos_header);
  FLIPENDIANLE(binj->dos_header.e_lfanew);
  fseek(binj->file, binj->dos_header.e_lfanew, SEEK_SET);
  BINJ_READ(binj, binj->signature);
  BINJ_READ(binj, binj->image_file_header);
  FLIPENDIANLE(binj->image_file_header.SizeOfOptionalHeader);
  FLIPENDIANLE(binj->image_file_header.NumberOfSections);
  if (binj->image_file_header.SizeOfOptionalHeader != 0)
  {
    fread(&binj->image_optional_header, sizeof(char),
      binj->image_file_header.SizeOfOptionalHeader, binj->file);
  }

  for (i = 0; i < binj->image_file_header.NumberOfSections; i++)
  {
    BINJ_READ(binj, binj->section_header);
    FLIPENDIANLE(binj->section_header.VirtualAddress);
    FLIPENDIANLE(binj->section_header.PointerToRawData);

    if (strcmp(binj->section_header.Name, ".rsrc") == 0)
      binject_exe_resources(binj, 0, 0, 0);
  }
}

VALUE
binject_exe_inject(VALUE self, VALUE key, VALUE obj)
{
  binject_exe_t *binj;
  Data_Get_Struct(self, binject_exe_t, binj);
  rb_ary_push(binj->adds, rb_ary_new3(2, key, obj));
}

VALUE
binject_exe_save(VALUE self, VALUE file)
{
  int i;
  size_t len, pos;
  binject_exe_t *binj;
  char buf[BUFSIZE];
  char buf2[BUFSIZE];
  Data_Get_Struct(self, binject_exe_t, binj);
#ifndef RUBY_1_8
  binj->out = fopen(RSTRING_PTR(file), "wb");
#else
  binj->out = rb_fopen(RSTRING_PTR(file), "wb");
#endif
  binj->ids = 0;
  binj->namestart = 0;
  binj->datastart = 0;
  binj->datapos = 0;
  binj->proc = rb_block_proc();
  fseek(binj->file, 0, SEEK_SET);

  pos = 0;
  while (!feof(binj->file))
  {
    int rlen = BUFSIZE;
    if (pos < binj->section_header.PointerToRawData && pos + rlen > binj->section_header.PointerToRawData)
      rlen = binj->section_header.PointerToRawData - pos;

    len = fread(buf, sizeof(char), rlen, binj->file);
    if (pos == binj->section_header.PointerToRawData)
    {
      MEMZERO(buf2, char, BUFSIZE);
      len = binject_exe_rewrite(binj, buf, buf2, 0, 0, 0, 0);
      fwrite(buf2, sizeof(char), binj->datastart, binj->out);
      // printf("FINISHING AT: %x / %x\n", binj->dataend, binj->datapos);
      fseek(binj->out, binj->datapos, SEEK_SET);
      fseek(binj->file, 0, SEEK_END);
    }
    else
    {
      fwrite(buf, sizeof(char), len, binj->out);
    }
    pos += len;
  }
  
  unsigned int posend = BINJ_PAD(binj->datapos, 0x1000);
  unsigned int grow = posend - ftell(binj->file);
  unsigned int actual = binj->datapos - binj->section_header.PointerToRawData;
  // printf("GROW: %x / ACTUAL: %x (%x / %x)\n", grow, actual, binj->datapos, binj->dataend);
  fseek(binj->file, 0, SEEK_SET);
  fseek(binj->out, 0, SEEK_SET);
  fread(buf, sizeof(char), 1024, binj->file);
  unsigned int *uninit = (unsigned int *)(buf + (binj->dos_header.e_lfanew + 32));
  FLIPENDIANLE(*uninit);
  *uninit += grow - binj->vdelta;
  FLIPENDIANLE(*uninit);
  uninit = (unsigned int *)(buf + (binj->dos_header.e_lfanew + 80));
  FLIPENDIANLE(*uninit);
  *uninit += grow;
  FLIPENDIANLE(*uninit);
  int *resd = (int *)(buf + (binj->dos_header.e_lfanew + 140));
  *resd = actual;
  FLIPENDIANLE(*resd);
  resd = (int *)(buf + (binj->dos_header.e_lfanew + 376));
  *resd = actual;
  FLIPENDIANLE(*resd);
  uninit = (unsigned int *)(buf + (binj->dos_header.e_lfanew + 384));
  FLIPENDIANLE(*uninit);
  *uninit += grow;
  FLIPENDIANLE(*uninit);
  fwrite(buf, sizeof(char), 1024, binj->out);

  fseek(binj->out, binj->datapos, SEEK_SET);
  while (binj->datapos < posend)
  {
    int len = posend - binj->datapos;
    if (len > 16) len = 16;
    fwrite(pe_pad, sizeof(char), len, binj->out);
    binj->datapos += len;
  }
  // printf("SIZE: %x\n", ftell(binj->out));
  fclose(binj->out);
  binj->out = NULL;
}

typedef struct {
  AbstractFile* in;
	io_func* in_func;
	Volume* in_vol;
  VALUE tmpname;
} binject_dmg_t;

VALUE
binject_dmg_uncompress(VALUE filename, VALUE volname)
{
  FILE *hfs;
  int len;
  gzFile file;
  VALUE filename2 = rb_funcall(filename, rb_intern("gsub"), 2,
    rb_eval_string("/\\.\\w+$/"), rb_str_new2(".hfs"));

  char *vname = RSTRING_PTR(volname);
  char *fname = RSTRING_PTR(filename);
  unsigned char buf[BUFSIZE], sig[8];
  int pos = 0;

  file = (gzFile)gzopen(fname, "rb");
#ifndef RUBY_1_8
  hfs = fopen(RSTRING_PTR(filename2), "wb");
#else
  hfs = rb_fopen(RSTRING_PTR(filename2), "wb");
#endif
  while ((len = gzread(file, buf, BUFSIZE)) > 0)
  {
    if (pos == 0)
      MEMCPY(sig, buf + 0x414, char, 4);

    if (pos <= 0xB000 && pos + len > 0xB000)
    {
      int i;
      char *meta = buf + (0xB000 - pos);
      unsigned char o = (RSTRING_LEN(volname) * 2) + 6;
      MEMZERO(meta + 0x17, char, 0xFF0 - 0x18);
      // printf("POS: %d\n", pos);
      meta[0x0F] = o;
      meta[0x15] = meta[0x79 + o] = (unsigned char)RSTRING_LEN(volname);
      for (i = 0; i < RSTRING_LEN(volname); i++)
      {
        meta[0x17 + (i * 2)] = meta[0x7B + o + (i * 2)] = vname[i];
      }
      meta[0x17 + (i * 2)] = 1; meta[0x1B + o] = 2;
      MEMCPY(meta + 0x1C + o, sig, char, 4);
      MEMCPY(meta + 0x20 + o, sig, char, 4);
      meta[0x69 + o] = 6; meta[0x6D + o] = 2;
      meta[0x71 + o] = 3; meta[0x77 + o] = 1;
      meta[0xffd] = 0x68 + o; meta[0xffb] = 0x7A + o + (i * 2);
    }
    fwrite(buf, sizeof(char), len, hfs);
    pos += len;
  }
  // printf("LEN: %d / POS: %d\n", len, pos);
  gzclose(file);
  fclose(hfs);

  return filename2;
}

static void
binject_dmg_mark(binject_dmg_t *binj)
{
  rb_gc_mark_maybe(binj->tmpname);
}

static void
binject_dmg_free(binject_dmg_t *binj)
{
  free(binj);
}

VALUE
binject_dmg_load(VALUE self, VALUE filename, VALUE volname)
{
  binject_dmg_t *binj;
  Data_Get_Struct(self, binject_dmg_t, binj);
  binj->tmpname = binject_dmg_uncompress(filename, volname);
	binj->in = createAbstractFileFromFile(fopen(RSTRING_PTR(binj->tmpname), "rb+"));
	if(binj->in == NULL) {
		fprintf(stderr, "error: Cannot open image-file.\n");
		return Qnil;
	}

	binj->in_func = IOFuncFromAbstractFile(binj->in);
	binj->in_vol = openVolume(binj->in_func); 
	if(binj->in_vol == NULL) {
		fprintf(stderr, "error: Cannot open volume.\n");
		CLOSE(binj->in_func);
		return 1;
	}
}

VALUE
binject_dmg_grow(VALUE self, VALUE megs)
{
  uint64_t size = (uint64_t)NUM2INT(megs) * (1024 * 1024);
  binject_dmg_t *binj;
  Data_Get_Struct(self, binject_dmg_t, binj);
	grow_hfs(binj->in_vol, size);
  return self;
}

VALUE
binject_dmg_inject_dir(VALUE self, VALUE key, VALUE dir)
{
  binject_dmg_t *binj;
  Data_Get_Struct(self, binject_dmg_t, binj);
  newFolder(RSTRING_PTR(key), binj->in_vol);
	addall_hfs(binj->in_vol, RSTRING_PTR(dir), RSTRING_PTR(key));
  return self;
}

VALUE
binject_dmg_inject_file(VALUE self, VALUE key, VALUE filename)
{
  binject_dmg_t *binj;
	AbstractFile *inFile;
  Data_Get_Struct(self, binject_dmg_t, binj);
	inFile = createAbstractFileFromFile(fopen(RSTRING_PTR(filename), "rb"));
	if(inFile == NULL) {
    return Qnil;
	}
	add_hfs(binj->in_vol, inFile, RSTRING_PTR(key));
  return self;
}

VALUE
binject_dmg_chmod_file(VALUE self, VALUE mode, VALUE filename)
{
  binject_dmg_t *binj;
  Data_Get_Struct(self, binject_dmg_t, binj);
  chmodFile(RSTRING_PTR(filename), NUM2INT(mode), binj->in_vol);
  return self;
}

void
binject_dmg_loop(void *data, unsigned int percent)
{
  VALUE proc = (VALUE)data;
  if (RTEST(proc))
    rb_funcall(proc, rb_intern("call"), 1, INT2NUM(percent));
}

VALUE
binject_dmg_save(VALUE self, VALUE filename)
{
  binject_dmg_t *binj;
	AbstractFile *out;
  Data_Get_Struct(self, binject_dmg_t, binj);
	out = createAbstractFileFromFile(fopen(RSTRING_PTR(filename), "wb"));
	if(out == NULL)
		return Qnil;

  out->progress = binject_dmg_loop;
  if (rb_block_given_p())
    out->user = (void *)rb_block_proc();
  buildDmg(binj->in, out);
  binject_dmg_loop(out->user, 100);
  if (!NIL_P(binj->tmpname))
    rb_funcall(rb_cFile, rb_intern("delete"), 1, binj->tmpname);
  return Qnil;
}

VALUE
binject_dmg_alloc(VALUE klass)
{
  binject_dmg_t *binj = ALLOC(binject_dmg_t);
  MEMZERO(binj, binject_dmg_t, 1);
  binj->tmpname = Qnil;
  return Data_Wrap_Struct(klass, binject_dmg_mark, binject_dmg_free, binj);
}

void Init_binject()
{
  VALUE cBinject = rb_define_module("Binject");
  VALUE cEXE = rb_define_class_under(cBinject, "EXE", rb_cObject);
  rb_define_alloc_func(cEXE, binject_exe_alloc);
  rb_define_method(cEXE, "initialize", binject_exe_load, 1);
  rb_define_method(cEXE, "inject", binject_exe_inject, 2);
  rb_define_method(cEXE, "save", binject_exe_save, 1);

  VALUE cDMG = rb_define_class_under(cBinject, "DMG", rb_cObject);
  rb_define_alloc_func(cDMG, binject_dmg_alloc);
  rb_define_method(cDMG, "initialize", binject_dmg_load, 2);
  rb_define_method(cDMG, "grow", binject_dmg_grow, 1);
  rb_define_method(cDMG, "inject_dir", binject_dmg_inject_dir, 2);
  rb_define_method(cDMG, "inject_file", binject_dmg_inject_file, 2);
  rb_define_method(cDMG, "chmod_file", binject_dmg_chmod_file, 2);
  rb_define_method(cDMG, "save", binject_dmg_save, 1);

	TestByteOrder();
}
