
#include <ruby.h>
#ifdef HAVE_RUBY_ST_H
#include <ruby/st.h>
#else
#include <st.h>
#endif
#ifdef HAVE_RUBY_IO_H
#include <ruby/io.h>
#else
#include <rubyio.h>
#endif
#include <stdlib.h>

#define swap(x, a, b) { char *t = x[a]; x[a] = x[b]; x[b] = t; }
#define i2c(x, i, depth) x[i][depth]
#define MIN(a,b) ((a < b) ? a : b)

#define MAX_QUERY_SIZE 256

static void 
vecswap(int i, int j, int n, char *x[])
{
  while(n-- > 0) {
      swap(x, i, j);
      i++;
      j++;
  }
}

static void
insertion_sort(char *x[], int n, int depth)
{
  char **p, **q;

  for(p = x; --n > 0; p++) {
      for(q = p; q > x; q--) {
          char *s = *(q-1) + depth;
          char *t = *q + depth;
          for(; *s && *t && *s == *t; s++, t++)  ;
          if (!*s || (*t && *s <= *t)) {
              break;
          }
          char *tmp = *q;
          *q = *(q-1);
          *(q-1) = tmp;
      }
  }
}

static void 
ssort(char *x[], int n, int depth)
{
  int a, b, c, d, r, v;

  if(depth > MAX_QUERY_SIZE) {
      return;
  }
  if(n <= 5) {
      insertion_sort(x, n, depth);
      return;
  }
  a = rand() % n;
  swap(x, 0, a);
  v = i2c(x, 0, depth);
  a = b = 1;
  c = d = n-1;
  for(;;) {
      while(b <= c && (r = i2c(x, b, depth) - v) <= 0) {
          if(r == 0) { 
              swap(x, a, b); 
              a++;
          }
          b++;
      }
      while(b <= c && (r = i2c(x, c, depth) - v) >= 0) {
          if(r == 0) {
              swap(x, c, d);
              d--;
          }
          c--;
      }
      if(b > c) break;
      swap(x, b, c);
      b++;
      c--;
  }
  r = MIN(a, b-a); 
  vecswap(0, b-r, r, x);
  
  r = MIN(d-c, n-d-1);
  vecswap(b, n-r, r, x);
  
  r = b-a;
  ssort(x, r, depth);
  
  if(i2c(x, r, depth) != 0) {
      ssort(x+r, a + n-d-1, depth + 1);
  }
  r = d-c;
  ssort(x + n - r, r, depth);
}

VALUE
sort_bang(VALUE self, VALUE text)
{
  char *fulltext;
  char **array;
  VALUE suffixes;
  int i;

  fulltext = StringValuePtr(text);
  suffixes = rb_ivar_get(self, rb_intern("@suffixes"));
  suffixes = rb_funcall(suffixes, rb_intern("to_a"), 0);
  if(TYPE(suffixes) != T_ARRAY) {
      rb_raise(rb_eRuntimeError, "@suffixes must be an array");
  }
  array   = (char **)RARRAY_PTR(suffixes);
  for(i = 0; i < RARRAY_LEN(suffixes); i++) {
      array[i] = fulltext + FIX2INT(array[i]);
  }
  ssort(array, RARRAY_LEN(suffixes), 0);
  for(i = 0; i < RARRAY_LEN(suffixes); i++) {
      RARRAY_PTR(suffixes)[i] = INT2NUM(array[i] - fulltext);
  }
}

// TODO: 64-bit 
static void
value_ary_to_native_ary(VALUE arr)
{
  VALUE *vptr;
  int i;

  for(i = 0, vptr = RARRAY_PTR(arr); i < RARRAY_LEN(arr); i++) {
      *vptr = (VALUE)(NUM2INT(*vptr));
      vptr++;
  }
}


// TODO: 64-bit 
static void
native_ary_to_value_ary(VALUE arr)
{
  VALUE *vptr;
  int i;

  for(i = 0, vptr = RARRAY_PTR(arr); i < RARRAY_LEN(arr); i++) {
      *vptr = INT2NUM((int)*vptr);
      vptr++;
  }
}

#if HAVE_RUBY_RUBY_H
#define HEAP_PTR(ptr, field) ptr->as.heap.field
#else
#define HEAP_PTR(ptr, field) ptr->field
#endif

#ifdef GetWriteFile
#define IO_FD(ptr) fileno(GetWriteFile(ptr))
#else
#define IO_FD(ptr) ptr->fd
#endif

static VALUE
shared_string(void *ptr, int len)
{
  VALUE ret;
#if HAVE_RB_DEFINE_ALLOC_FUNC
  ret = rb_obj_alloc(rb_cString);
#else
  ret = rb_str_new2("");
  free(RSTRING_PTR(ret));
#endif
  HEAP_PTR(RSTRING(ret), ptr) = ptr;
  HEAP_PTR(RSTRING(ret), len) = len;
#if HAVE_RB_DEFINE_ALLOC_FUNC
  HEAP_PTR(RSTRING(ret), aux.shared) = ret;
  FL_SET(ret, ELTS_SHARED);
#else
  RSTRING(ret)->orig = ret;
#endif

  return ret;
}

static VALUE
dump_inline_suffixes(VALUE self, VALUE io, VALUE fulltext)
{
  char *data;
  rb_io_t *fptr;
  VALUE block_size, suffixes, new_suffixes, inline_suffix_size;
  int i;

  data               = StringValuePtr(fulltext);
  suffixes           = rb_ivar_get(self, rb_intern("@suffixes"));
  new_suffixes       = rb_funcall(suffixes, rb_intern("to_a"), 0);
  block_size         = rb_ivar_get(self, rb_intern("@block_size"));
  inline_suffix_size = rb_ivar_get(self, rb_intern("@inline_suffix_size"));

  rb_funcall(io, rb_intern("flush"), 0);
  if(TYPE(io) == T_FILE) {
      GetOpenFile(io, fptr);
      for(i = 0; i < RARRAY_LEN(new_suffixes); i += NUM2INT(block_size)) {
          write(IO_FD(fptr), data + NUM2INT(RARRAY_PTR(new_suffixes)[i]), 
                NUM2INT(inline_suffix_size));
      }
  } else {
      ID write = rb_intern("write");
      for(i = 0; i < RARRAY_LEN(new_suffixes); i += NUM2INT(block_size)) {
          rb_funcall(io, write, 1,
                     shared_string(data + NUM2INT(RARRAY_PTR(new_suffixes)[i]),
                                   NUM2INT(inline_suffix_size)));
      }
  }

  return Qnil;
}


static VALUE
dump_suffix_array(VALUE self, VALUE io)
{
  VALUE suffixes, new_suffixes;
  int i;
  VALUE *vptr;
  rb_io_t *fptr;

  suffixes     = rb_ivar_get(self, rb_intern("@suffixes"));
  new_suffixes = rb_funcall(suffixes, rb_intern("to_a"), 0);
  if(TYPE(suffixes) != T_ARRAY) {
      rb_raise(rb_eRuntimeError, "@suffixes must be an array");
  }
  
  // TODO: fixes for 64bit archs
  rb_funcall(io, rb_intern("flush"), 0);

  value_ary_to_native_ary(new_suffixes);
  
  if(TYPE(io) == T_FILE) {
      GetOpenFile(io, fptr);
      write(IO_FD(fptr), RARRAY_PTR(new_suffixes), 
            sizeof(VALUE) * RARRAY_LEN(new_suffixes));
  } else {
      ID write;
      /* a GC run at this time would be catastrophic since the
       * items in the array would be bogus pointers */
      rb_gc_disable();
      write = rb_intern("write");
      rb_funcall(io, write, 1,
                 shared_string(RARRAY_PTR(new_suffixes),
                               sizeof(VALUE) * RARRAY_LEN(new_suffixes)));
      rb_gc_enable();
  }
  
  /* only restore if actually needed */
  if(new_suffixes == suffixes) {
      native_ary_to_value_ary(new_suffixes);
  }

  return Qnil;
}


static VALUE
whitespace_analyzer_append_suffixes(VALUE self, VALUE array, VALUE text, VALUE offset)
{
 VALUE str  = StringValue(text);
 char *ptr  = RSTRING_PTR(str);
 char *base = ptr;
 char *eof  = ptr + RSTRING_LEN(str);
 int off    = NUM2INT(offset);

 while(ptr != eof) {
     if(!isspace(*ptr)) {
         rb_ary_push(array, INT2NUM(ptr - base + off));
         while(ptr != eof && !isspace(*ptr)) ptr++;
     }
     while(ptr != eof && isspace(*ptr)) ptr++;
 }

 return array;
}


static VALUE
si_analyzer_append_suffixes(VALUE self, VALUE array, VALUE text, VALUE offset)
{
 VALUE str  = StringValue(text);
 char *ptr  = RSTRING_PTR(str);
 char *base = ptr;
 char *eof  = ptr + RSTRING_LEN(str);
 int off    = NUM2INT(offset);

 while(ptr != eof) {
     if(isalpha(*ptr) || *ptr == '_') {
         rb_ary_push(array, INT2NUM(ptr - base + off));
         while(ptr != eof && (isalnum(*ptr) || *ptr == '_')) ptr++;
     }
     while(ptr != eof && !(isalpha(*ptr) || *ptr == '_')) ptr++;
 }

 return array;
}

VALUE
dm_reader_binary_search(VALUE self, VALUE ary, VALUE offset, VALUE from, VALUE to)
{
  unsigned long middle, _from, _to, pivot;
  VALUE *ptr;

  _from = NUM2ULONG(from);
  _to   = NUM2ULONG(to);
  /* FIXME: range checking */
  ptr = RARRAY_PTR(ary);
  while(_to - _from > 1) {
      middle = _from + ((_to - _from) >> 1);
      /* FIXME: type checks */
      pivot = RARRAY_PTR(ptr[middle])[0];
      if(offset < pivot) {
          _to = middle;
      } else if(offset > pivot) {
          _from = middle;
      } else {
          return INT2NUM(middle);
      }
  }

  return INT2NUM(_from);
}


VALUE
dm_reader_document_uri(VALUE self, VALUE suffix_idx, VALUE offset)
{
  VALUE idx;
  VALUE field_arr;
  VALUE doc_id;

  field_arr = rb_ivar_get(self, rb_intern("@field_arr"));
  /* FIXME: typecheck*/
  idx    = dm_reader_binary_search(self, field_arr, offset,
                                   INT2FIX(0), INT2NUM(RARRAY_LEN(field_arr)));
  doc_id = RARRAY_PTR(RARRAY_PTR(field_arr)[NUM2INT(idx)])[1];
  return RARRAY_PTR(rb_ivar_get(self, rb_intern("@uri_tbl")))[NUM2INT(doc_id)];
}

int
insert_into_ruby_hash(VALUE doc_id, int score, VALUE hash)
{
  rb_hash_aset(hash, doc_id, INT2NUM(score));
  return ST_CONTINUE;
}


VALUE dm_reader_rank_offsets(VALUE self, VALUE offsets, VALUE weights)
{
  VALUE field_arr;
  long int_weights[256];
  int i;
  st_table *hash;
  VALUE ret;

  weights = rb_funcall(weights, rb_intern("to_a"), 0);
  offsets = rb_funcall(offsets, rb_intern("to_a"), 0);

  for(i = 0; i < (256 < RARRAY_LEN(weights) ? 256 : RARRAY_LEN(weights)); i++) {
      VALUE integer_weight = rb_funcall(RARRAY_PTR(weights)[i], rb_intern("to_i"), 0);
      int_weights[i] = NUM2INT(integer_weight);
  }

  /* TODO: typecheck */
  field_arr = rb_ivar_get(self, rb_intern("@field_arr"));

  hash = st_init_numtable_with_size(RARRAY_LEN(offsets) / 100);
  for(i = 0; i < RARRAY_LEN(offsets); i++) {
      VALUE info, off;
      off = dm_reader_binary_search(self, field_arr, RARRAY_PTR(offsets)[i],
                                    INT2FIX(0), INT2NUM(RARRAY_LEN(field_arr)));
      info = RARRAY_PTR(field_arr)[NUM2INT(off)];
      if(TYPE(info) == T_ARRAY) {
          int val = 0;
          
          st_lookup(hash, RARRAY_PTR(info)[1], (st_data_t *)&val);
          val += int_weights[FIX2INT(RARRAY_PTR(info)[2])] / (NUM2INT(RARRAY_PTR(info)[3]) + 1);
          st_insert(hash, RARRAY_PTR(info)[1], val);
      }
  }

  ret = rb_hash_new();
  st_foreach(hash, insert_into_ruby_hash, ret);

  return rb_funcall(self, rb_intern("sort_score_hash"), 1, ret);
}


VALUE dm_reader_rank_offsets_probabilistic(VALUE self, VALUE offsets, VALUE weights, VALUE iterations)
{
  VALUE hash, field_arr;
  double fl_weights[256];
  int i;

  weights = rb_funcall(weights, rb_intern("to_a"), 0);
  offsets = rb_funcall(offsets, rb_intern("to_a"), 0);

  for(i = 0; i < (256 < RARRAY_LEN(weights) ? 256 : RARRAY_LEN(weights)); i++) {
      VALUE float_val = rb_funcall(RARRAY_PTR(weights)[i], rb_intern("to_f"), 0);
      /* TODO: check type */
      fl_weights[i] = RFLOAT_VALUE(float_val);
  }

  /* TODO: typecheck */
  field_arr = rb_ivar_get(self, rb_intern("@field_arr"));

  hash = rb_hash_new();
  for(i = 0; i < NUM2INT(iterations); i++) {
      VALUE info, off;
      int index;

      index = rand() % RARRAY_LEN(offsets);
      off = dm_reader_binary_search(self, field_arr, RARRAY_PTR(offsets)[index],
                                    INT2FIX(0), INT2NUM(RARRAY_LEN(field_arr)));
      info = RARRAY_PTR(field_arr)[NUM2INT(off)];
      if(TYPE(info) == T_ARRAY) {
          double val;
          VALUE cur = rb_hash_aref(hash, RARRAY_PTR(info)[1]);
          if(cur == Qnil) {
              val = 0.0;
          } else {
              /* FIXME: typecheck */
              val = RFLOAT_VALUE(cur);
          }
          val += fl_weights[FIX2INT(RARRAY_PTR(info)[2])] / 
                 NUM2INT(RARRAY_PTR(info)[3]);
          rb_hash_aset(hash, RARRAY_PTR(info)[1], rb_float_new(val));
      }
  }

  return rb_funcall(self, rb_intern("sort_score_hash"), 1, hash);
}


VALUE
sa_reader_lazyhits_to_offsets(VALUE self, VALUE lazyhits)
{
 int from, to;
 VALUE io, base;
 VALUE ret;

 from = NUM2INT(rb_funcall(lazyhits, rb_intern("from_index"), 0));
 to   = NUM2INT(rb_funcall(lazyhits, rb_intern("to_index"), 0));
 io   = rb_ivar_get(self, rb_intern("@io"));
 base = rb_ivar_get(self, rb_intern("@base"));
 rb_funcall(io, rb_intern("pos="), 1, INT2NUM(NUM2INT(base) + 4 * from));

 ret = rb_ary_new2(to - from);
 /* FIXME: arity */
 /* TODO: 64 bits */
 {
     VALUE str;
     int i;
     unsigned long *src, *dst;

     str = rb_funcall(io, rb_intern("read"), 1, INT2NUM(4 * (to - from)));
     /* TODO: check retval */
     if(TYPE(str) != T_STRING) {
         rb_raise(rb_eRuntimeError, "The @io didn't return a String object.");
     }
     for(src = (unsigned long *)RSTRING_PTR(str), 
         dst = (unsigned long *)RARRAY_PTR(ret), i = 0; i < RSTRING_LEN(str) / 4; i++) {
         *dst++ = INT2NUM(*src++);
     }
     HEAP_PTR(RARRAY(ret), len) = RSTRING_LEN(str) / 4;
 }

 return ret;
}

void Init_ftsearchrt()
{
 int status;
 VALUE cSuffixArrayWriter;
 VALUE cWhiteSpaceAnalyzer;
 VALUE cDocumentMapReader;
 VALUE cSuffixArrayReader;
 VALUE cSimpleIdentifierAnalyzer;
 
 cSuffixArrayWriter = rb_eval_string_protect("::FTSearch::SuffixArrayWriter", &status);
 if(!status) {
     rb_define_method(cSuffixArrayWriter, "sort!", sort_bang, 1);
#if SIZEOF_LONG_LONG != SIZEOF_VOIDP
     rb_define_method(cSuffixArrayWriter, "dump_suffix_array", dump_suffix_array, 1);
     rb_define_method(cSuffixArrayWriter, "dump_inline_suffixes", dump_inline_suffixes, 2);
#else
     fprintf(stderr, "Using slower, 64-bit safe SuffixArrayWriter.\n");
#endif
 }
 cWhiteSpaceAnalyzer = rb_eval_string_protect("::FTSearch::Analysis::WhiteSpaceAnalyzer", &status);
 if(!status) {
     rb_define_method(cWhiteSpaceAnalyzer, "append_suffixes", 
                      whitespace_analyzer_append_suffixes, 3);
 }
 cSimpleIdentifierAnalyzer = rb_eval_string_protect("::FTSearch::Analysis::SimpleIdentifierAnalyzer", &status);
 if(!status) {
     rb_define_method(cSimpleIdentifierAnalyzer, "append_suffixes", 
                      si_analyzer_append_suffixes, 3);
 }
 cDocumentMapReader = rb_eval_string_protect("::FTSearch::DocumentMapReader", &status);
 if(!status) {
     rb_define_method(cDocumentMapReader, "binary_search", dm_reader_binary_search, 4);
     rb_define_method(cDocumentMapReader, "document_uri", dm_reader_document_uri, 2);
     rb_define_method(cDocumentMapReader, "rank_offsets", dm_reader_rank_offsets, 2);
     rb_define_method(cDocumentMapReader, "rank_offsets_probabilistic", dm_reader_rank_offsets_probabilistic, 3);
 }

 cSuffixArrayReader = rb_eval_string_protect("::FTSearch::SuffixArrayReader", &status);
 if(!status) {
#if SIZEOF_LONG_LONG != SIZEOF_VOIDP
     rb_define_method(cSuffixArrayReader, "lazyhits_to_offsets", sa_reader_lazyhits_to_offsets, 1);
#else
     fprintf(stderr, "Using slower, 64-bit safe SuffixArrayReader.\n");
#endif
 }
}

