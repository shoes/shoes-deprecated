//
// shoes/image.c
// Loading image formats in Cairo.  I've already tried gdk-pixbuf and imlib2, but 
// the idea here is to cut down dependencies, since I only really need reading of 
// the basics: GIF and JPEG.
//
#include <stdio.h>
#include <setjmp.h>
#include <sys/stat.h>
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/version.h"
#include "shoes/http.h"

#undef HAVE_PROTOTYPES
#undef HAVE_STDLIB_H
#undef EXTERN
#include <jpeglib.h>
#include <jerror.h>

#ifndef SHOES_GTK
#ifdef DrawText
#undef DrawText
#endif
#endif
#define DrawText gif_DrawText
#include <gif_lib.h>

shoes_image_format shoes_image_detect(VALUE, int *, int *);

#define JPEG_LINES 16
#define SIZE_SURFACE ((cairo_surface_t *)1)

typedef struct {
  unsigned int state[5];
  unsigned int count[2];
  unsigned char buffer[64];
} SHA1_CTX;

void SHA1Transform(unsigned int state[5], unsigned char buffer[64]);
void SHA1Init(SHA1_CTX* context);
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len);
void SHA1Final(unsigned char digest[20], SHA1_CTX* context);

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
  |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
  ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1Transform(unsigned int state[5], unsigned char buffer[64])
{
unsigned int a, b, c, d, e;
typedef union {
  unsigned char c[64];
  unsigned int l[16];
} CHAR64LONG16;
CHAR64LONG16* block;
#ifdef SHA1HANDSOFF
static unsigned char workspace[64];
  block = (CHAR64LONG16*)workspace;
  memcpy(block, buffer, 64);
#else
  block = (CHAR64LONG16*)buffer;
#endif
  /* Copy context->state[] to working vars */
  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];
  e = state[4];
  /* 4 rounds of 20 operations each. Loop unrolled. */
  R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
  R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
  R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
  R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
  R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
  R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
  R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
  R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
  R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
  R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
  R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
  R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
  R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
  R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
  R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
  R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
  R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
  R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
  R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
  R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
  /* Add the working vars back into context.state[] */
  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
  state[4] += e;
  /* Wipe variables */
  a = b = c = d = e = 0;
}

/* SHA1Init - Initialize new context */
void SHA1Init(SHA1_CTX* context)
{
  /* SHA1 initialization constants */
  context->state[0] = 0x67452301;
  context->state[1] = 0xEFCDAB89;
  context->state[2] = 0x98BADCFE;
  context->state[3] = 0x10325476;
  context->state[4] = 0xC3D2E1F0;
  context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */
void SHA1Update(SHA1_CTX* context, unsigned char* data, unsigned int len)
{
unsigned int i, j;

  j = (context->count[0] >> 3) & 63;
  if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
  context->count[1] += (len >> 29);
  if ((j + len) > 63) {
    memcpy(&context->buffer[j], data, (i = 64-j));
    SHA1Transform(context->state, context->buffer);
    for ( ; i + 63 < len; i += 64) {
      SHA1Transform(context->state, &data[i]);
    }
    j = 0;
  }
  else i = 0;
  memcpy(&context->buffer[j], &data[i], len - i);
}

/* Add padding and return the message digest. */
void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
unsigned int i, j;
unsigned char finalcount[8];

  for (i = 0; i < 8; i++) {
    finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
     >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
  }
  SHA1Update(context, (unsigned char *)"\200", 1);
  while ((context->count[0] & 504) != 448) {
    SHA1Update(context, (unsigned char *)"\0", 1);
  }
  SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
  for (i = 0; i < 20; i++) {
    digest[i] = (unsigned char)
     ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
  }
  /* Wipe variables */
  i = j = 0;
  memset(context->buffer, 0, 64);
  memset(context->state, 0, 20);
  memset(context->count, 0, 8);
  memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make SHA1Transform overwrite it's own static vars */
  SHA1Transform(context->state, context->buffer);
#endif
}

cairo_surface_t *
shoes_surface_create_from_pixels(PIXEL *pixels, int width, int height)
{
  guchar *cairo_pixels;
  cairo_surface_t *surface;
  cairo_user_data_key_t key;
  int j;
 
  cairo_pixels = (guchar *)g_malloc(4 * width * height);
  surface = cairo_image_surface_create_for_data((unsigned char *)cairo_pixels,
    CAIRO_FORMAT_ARGB32,
    width, height, 4 * width);
  cairo_surface_set_user_data(surface, &key,
    cairo_pixels, (cairo_destroy_func_t)g_free);
 
  for (j = height; j; j--)
  {
    guchar *p = (guchar *)pixels;
    guchar *q = cairo_pixels;
   
    guchar *end = p + 4 * width;
    guint t1,t2,t3;
 
#define MULT(d,c,a,t) G_STMT_START { t = c * a; d = ((t >> 8) + t) >> 8; } G_STMT_END
 
    while (p < end)
    {
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
      MULT(q[2], p[2], p[3], t1);
      MULT(q[1], p[1], p[3], t2);
      MULT(q[0], p[0], p[3], t3);
      q[3] = p[3];
#else
      q[0] = p[3];
      MULT(q[3], p[0], p[3], t1);
      MULT(q[2], p[1], p[3], t2);
      MULT(q[1], p[2], p[3], t3);
#endif
    
      p += 4;
      q += 4;
    }
   
#undef MULT
   
    pixels += width;
    cairo_pixels += 4 * width;
  }
 
  return surface;
}

#define PNG_SIG "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"

cairo_surface_t *
shoes_png_size(char *filename, int *width, int *height)
{
  unsigned char *sig = SHOE_ALLOC_N(unsigned char, 32);
  cairo_surface_t *surface = NULL;
  
#ifdef SHOES_WIN32
  HANDLE hFile;
  hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == INVALID_HANDLE_VALUE ) return NULL;
  DWORD readsize;
	ReadFile( hFile, sig, 32, &readsize, NULL );
  if (readsize < 8)
    goto done; 
#else
  FILE *image = fopen(filename, "rb");
  if (image == NULL) return NULL;

  if (fread(sig, 1, 32, image) < 8)
    goto done; 
#endif
  
  if (memcmp(sig, PNG_SIG, 8) != 0)
    goto done;

  *width = *((int *)(sig + 0x10));
  BE_CPU(*width);
  *height = *((int *)(sig + 0x14));
  BE_CPU(*height);

  surface = SIZE_SURFACE;
done:
#ifdef SHOES_WIN32
  CloseHandle( hFile );
#else
  fclose(image);
#endif
  return surface;
}

cairo_surface_t *
shoes_surface_create_from_gif(char *filename, int *width, int *height, unsigned char load)
{
  cairo_surface_t *surface = NULL;
  GifFileType *gif;
  PIXEL *ptr = NULL, *pixels = NULL;
  GifPixelType **rows = NULL;
  GifRecordType rec;
  ColorMapObject *cmap;
  int i, j, bg, r, g, b, w = 0, h = 0, done = 0, transp = -1;
  float per = 0.0f, per_inc;
  int intoffset[] = { 0, 4, 2, 1 };
  int intjump[] = { 8, 8, 4, 2 };

  transp = -1;
  gif = DGifOpenFileName(filename);
  if (gif == NULL)
    goto done;

  do
  {
    if (DGifGetRecordType(gif, &rec) == GIF_ERROR)
      rec = TERMINATE_RECORD_TYPE;
    if ((rec == IMAGE_DESC_RECORD_TYPE) && (!done))
    {
      if (DGifGetImageDesc(gif) == GIF_ERROR)
        {
           /* PrintGifError(); */
           rec = TERMINATE_RECORD_TYPE;
        }
      w = gif->Image.Width;
      if (width != NULL) *width = w;
      h = gif->Image.Height;
      if (height != NULL) *height = h;
      if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
        goto done;

      if (!load)
      {
        surface = SIZE_SURFACE;
        goto done;
      }

      rows = SHOE_ALLOC_N(GifPixelType *, h);
      if (rows == NULL)
        goto done;

      SHOE_MEMZERO(rows, GifPixelType *, h);

      for (i = 0; i < h; i++)
      {
        rows[i] = SHOE_ALLOC_N(GifPixelType, w);
        if (rows[i] == NULL)
          goto done;
      }

      if (gif->Image.Interlace)
      {
        for (i = 0; i < 4; i++)
        {
          for (j = intoffset[i]; j < h; j += intjump[i])
            DGifGetLine(gif, rows[j], w);
        }
      }
      else
      {
        for (i = 0; i < h; i++)
          DGifGetLine(gif, rows[i], w);
      }
      done = 1;
    }
    else if (rec == EXTENSION_RECORD_TYPE)
    {
      int ext_code;
      GifByteType *ext = NULL;
      DGifGetExtension(gif, &ext_code, &ext);
      while (ext)
      {
        if ((ext_code == 0xf9) && (ext[1] & 1) && (transp < 0))
          transp = (int)ext[4];
        ext = NULL;
        DGifGetExtensionNext(gif, &ext);
      }
    }
  } while (rec != TERMINATE_RECORD_TYPE);

  bg = gif->SBackGroundColor;
  cmap = (gif->Image.ColorMap ? gif->Image.ColorMap : gif->SColorMap);
  pixels = SHOE_ALLOC_N(PIXEL, w * h);
  if (pixels == NULL)
    goto done;

  ptr = pixels;
  per_inc = 100.0f / (((float)w) * h);
  for (i = 0; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      if (rows[i][j] == transp)
      {
        r = cmap->Colors[bg].Red;
        g = cmap->Colors[bg].Green;
        b = cmap->Colors[bg].Blue;
        *ptr = 0x00ffffff & ((r << 16) | (g << 8) | b);
        LE_CPU(*ptr);
        ptr++;
      }
      else
      {
        r = cmap->Colors[rows[i][j]].Red;
        g = cmap->Colors[rows[i][j]].Green;
        b = cmap->Colors[rows[i][j]].Blue;
        *ptr = (0xff << 24) | (r << 16) | (g << 8) | b;
        LE_CPU(*ptr);
        ptr++;
      }
      per += per_inc;
    }
  }

  if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
    goto done;

  surface = shoes_surface_create_from_pixels(pixels, w, h);

done:
  if (gif != NULL) DGifCloseFile(gif);
  if (pixels != NULL) SHOE_FREE(pixels);
  if (rows != NULL) {
    for (i = 0; i < h; i++)
      if (rows[i] != NULL)
        SHOE_FREE(rows[i]);
    SHOE_FREE(rows);
  }
  return surface;
}

//
// JPEG handling code
//
struct shoes_jpeg_file_src {
  struct jpeg_source_mgr pub;   /* public fields */
  
#ifdef SHOES_WIN32
  HANDLE infile;        /* source stream */
#else
  FILE *infile;         /* source stream */
#endif
  
  JOCTET *buffer;         /* start of buffer */
  boolean start_of_file;    /* have we gotten any data yet? */
};

struct shoes_jpeg_error_mgr {
  struct jpeg_error_mgr pub;             /* public fields */
  jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct shoes_jpeg_file_src *shoes_jpeg_file;
typedef struct shoes_jpeg_error_mgr *shoes_jpeg_err;

#define JPEG_INPUT_BUF_SIZE 4096

void
shoes_jpeg_term_source(j_decompress_ptr cinfo)
{
}

void
shoes_jpeg_init_source(j_decompress_ptr cinfo)
{
  shoes_jpeg_file src = (shoes_jpeg_file) cinfo->src;
  src->start_of_file = 1;
}


boolean
shoes_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
  shoes_jpeg_file src = (shoes_jpeg_file)cinfo->src;
  size_t nbytes;

#ifdef SHOES_WIN32
  DWORD readsize;
	ReadFile( src->infile, src->buffer, JPEG_INPUT_BUF_SIZE, &readsize, NULL );
  nbytes = readsize;
#else
  nbytes = fread(src->buffer, 1, JPEG_INPUT_BUF_SIZE, src->infile);
#endif

  if (nbytes <= 0) {
    if (src->start_of_file) /* Treat empty input file as fatal error */
      ERREXIT(cinfo, JERR_INPUT_EMPTY);
    WARNMS(cinfo, JWRN_JPEG_EOF);
    src->buffer[0] = (JOCTET) 0xFF;
    src->buffer[1] = (JOCTET) JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;
  src->start_of_file = 0;

  return 1;
}

void
shoes_jpeg_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
{
  shoes_jpeg_file src = (shoes_jpeg_file)cinfo->src;

  if (num_bytes > 0)
  {
    while (num_bytes > (long)src->pub.bytes_in_buffer)
    {
      num_bytes -= (long)src->pub.bytes_in_buffer;
      (void)shoes_jpeg_fill_input_buffer(cinfo);
    }
    src->pub.next_input_byte += (size_t)num_bytes;
    src->pub.bytes_in_buffer -= (size_t)num_bytes;
  }
}

void
#ifdef SHOES_WIN32
jpeg_file_src(j_decompress_ptr cinfo, HANDLE infile)
#else
jpeg_file_src(j_decompress_ptr cinfo, FILE *infile)
#endif
{
  shoes_jpeg_file src;

  if (cinfo->src == NULL)
  {
    /* first time for this JPEG object? */
    cinfo->src = (struct jpeg_source_mgr *)
      (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT,
                    sizeof(struct shoes_jpeg_file_src));
    src = (shoes_jpeg_file) cinfo->src;
    src->buffer = (JOCTET *)
      (*cinfo->mem->alloc_small)((j_common_ptr) cinfo, JPOOL_PERMANENT,
                    JPEG_INPUT_BUF_SIZE * sizeof(JOCTET));
  }

  src = (shoes_jpeg_file)cinfo->src;
  src->pub.init_source = shoes_jpeg_init_source;
  src->pub.fill_input_buffer = shoes_jpeg_fill_input_buffer;
  src->pub.skip_input_data = shoes_jpeg_skip_input_data;
  src->pub.resync_to_restart = jpeg_resync_to_restart; /* use default method */
  src->pub.term_source = shoes_jpeg_term_source;
  src->infile = infile;
  src->pub.bytes_in_buffer = 0; /* forces fill_input_buffer on first read */
  src->pub.next_input_byte = NULL; /* until buffer loaded */
}
 
void
shoes_jpeg_fatal(j_common_ptr cinfo)
{
  shoes_jpeg_err jpgerr = (shoes_jpeg_err)cinfo->err;
  longjmp(jpgerr->setjmp_buffer, 1);
}

cairo_surface_t *
shoes_surface_create_from_jpeg(char *filename, int *width, int *height, unsigned char load)
{
  int x, y, w, h, l, i, scans, count, prevy;
  unsigned char *ptr, *rgb = NULL, **line = NULL;
  PIXEL *pixels = NULL, *ptr2;
  cairo_surface_t *surface = NULL;
  struct jpeg_decompress_struct cinfo;
  struct shoes_jpeg_error_mgr jerr;
  
#ifdef SHOES_WIN32
  HANDLE hFile;
  hFile = CreateFile( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
  if ( hFile == INVALID_HANDLE_VALUE ) return NULL;
#else
  FILE *f = fopen(filename, "rb");
  if (!f) return NULL;
#endif
  
  // TODO: error handling
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = shoes_jpeg_fatal;
  // jerr.pub.emit_message = shoes_jpeg_error;
  // jerr.pub.output_message = shoes_jpeg_error2;
  if (setjmp(jerr.setjmp_buffer)) {
    // append_jpeg_message(interp, (j_common_ptr) &cinfo);
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }

  jpeg_create_decompress(&cinfo);
#ifdef SHOES_WIN32
  jpeg_file_src(&cinfo, hFile);
#else
  jpeg_file_src(&cinfo, f);
#endif
  jpeg_read_header(&cinfo, TRUE);
  cinfo.do_fancy_upsampling = FALSE;
  cinfo.do_block_smoothing = FALSE;

  line = SHOE_ALLOC_N(unsigned char *, JPEG_LINES);
  jpeg_start_decompress(&cinfo);
  w = cinfo.output_width;
  if (width != NULL) *width = w;
  h = cinfo.output_height;
  if (height != NULL) *height = h;

  if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
    goto done;

  if (!load)
  {
    surface = SIZE_SURFACE;
    goto done;
  }

  if (cinfo.rec_outbuf_height > JPEG_LINES)
    goto done;

  rgb = SHOE_ALLOC_N(unsigned char, w * JPEG_LINES * 3);
  ptr2 = pixels = SHOE_ALLOC_N(PIXEL, w * h);
  if (rgb == NULL || pixels == NULL)
    goto done;

  count = 0;
  prevy = 0;
  if (cinfo.output_components == 3 || cinfo.output_components == 1)
  {
    int c = cinfo.output_components;
    for (i = 0; i < cinfo.rec_outbuf_height; i++)
      line[i] = rgb + (i * w * c);
    for (l = 0; l < h; l += cinfo.rec_outbuf_height)
    {
      jpeg_read_scanlines(&cinfo, line, cinfo.rec_outbuf_height);
      scans = cinfo.rec_outbuf_height;
      if ((h - l) < scans) scans = h - l;
      ptr = rgb;
      for (y = 0; y < scans; y++)
      {
        for (x = 0; x < w; x++)
        {
          if (c == 3)
            *ptr2 = (0xff000000) | ((ptr[0]) << 16) | ((ptr[1]) << 8) | (ptr[2]);
          else if (c == 1)
            *ptr2 = (0xff000000) | ((ptr[0]) << 16) | ((ptr[0]) << 8) | (ptr[0]);
          LE_CPU(*ptr2);
          ptr += c;
          ptr2++;
        }
      }
    }
  }

  surface = shoes_surface_create_from_pixels(pixels, w, h);
  jpeg_finish_decompress(&cinfo);
done:
  free(line);
  if (pixels != NULL) free(pixels);
  if (rgb != NULL) free(rgb);
  jpeg_destroy_decompress(&cinfo);
#ifdef SHOES_WIN32
  CloseHandle( hFile );
#else
  fclose(f);
#endif
  return surface;
}

char
shoes_has_ext(char *fname, int len, const char *ext)
{
  return strncmp(fname + (len - strlen(ext)), ext, strlen(ext)) == 0;
}

unsigned char
shoes_check_file_exists(VALUE path)
{
  if (!RTEST(rb_funcall(rb_cFile, rb_intern("exists?"), 1, path)))
  {
    StringValue(path);
    shoes_error("Shoes could not find the file %s.", RSTRING_PTR(path)); 
    return FALSE;
  }
  return TRUE;
}

void
shoes_unsupported_image(VALUE path)
{
  VALUE ext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
  StringValue(path);
  StringValue(ext);
  shoes_error("Couldn't load %s. Shoes does not support images with the %s extension.", 
    RSTRING_PTR(path), RSTRING_PTR(ext)); 
}

void
shoes_failed_image(VALUE path)
{
  VALUE ext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
  StringValue(path);
  StringValue(ext);
  shoes_error("Couldn't load %s. Is the file a valid %s?", 
    RSTRING_PTR(path), RSTRING_PTR(ext)); 
}

cairo_surface_t *
shoes_surface_create_from_file(VALUE imgpath, int *width, int *height)
{
  cairo_surface_t *img = NULL;
  shoes_image_format format = shoes_image_detect(imgpath, width, height);
  if (format == SHOES_IMAGE_PNG)
  {
    img = cairo_image_surface_create_from_png(RSTRING_PTR(imgpath));
    if (cairo_surface_status(img) != CAIRO_STATUS_SUCCESS)
      img = NULL;
  }
  else if (format == SHOES_IMAGE_JPEG)
    img = shoes_surface_create_from_jpeg(RSTRING_PTR(imgpath), width, height, TRUE);
  else if (format == SHOES_IMAGE_GIF)
    img = shoes_surface_create_from_gif(RSTRING_PTR(imgpath), width, height, TRUE);
  else
    return shoes_world->blank_image;

  if (img == NULL)
  {
    shoes_failed_image(imgpath);
    img = shoes_world->blank_image;
  }

  return img;
}

int
shoes_file_mtime(char *path)
{
  int mtime = 0;
  struct stat *s = SHOE_ALLOC(struct stat);
  stat(path, s);
  mtime = (int)s->st_mtime;
  SHOE_FREE(s);
  return mtime;
}

shoes_cached_image *
shoes_cached_image_new(int width, int height, cairo_surface_t *surface)
{
  shoes_cached_image *cached = SHOE_ALLOC(shoes_cached_image);
  if (surface == NULL)
    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cached->surface = surface;
  cached->pattern = NULL;
  cached->width = width;
  cached->height = height;
  cached->mtime = 0;
  return cached;
}

int
shoes_cache_lookup(char *imgpath, shoes_cached_image **image)
{
  shoes_cache_entry *cached = NULL;
  int ret = st_lookup(shoes_world->image_cache, (st_data_t)imgpath, (st_data_t *)&cached);
  // if (ret && shoes_file_mtime(imgpath) == cached->image->mtime) {
  //   st_delete(shoes_world->image_cache, (st_data_t *)&imgpath, 0);
  //   // TODO: memory leak if an image's mtime changes (it's overwritten)
  //   // need to free this struct, but only after the surface is out of play
  //   // shoes_world_free_image_cache(imgpath, cached, NULL);
  //   ret = 0;
  // }
  if (ret) *image = cached->image;
  return ret;
}

void
shoes_cache_insert(unsigned char type, VALUE imgpath, shoes_cached_image *image)
{
  char *path = strdup(RSTRING_PTR(imgpath));
  shoes_cache_entry *cached = SHOE_ALLOC(shoes_cache_entry);
  cached->type = type;
  cached->image = image;
  if (type == SHOES_CACHE_FILE) image->mtime = shoes_file_mtime(path);
  st_insert(shoes_world->image_cache, (st_data_t)path, (st_data_t)cached);
}

shoes_image_format
shoes_image_detect(VALUE imgpath, int *width, int *height)
{
  shoes_image_format format = SHOES_IMAGE_NONE;
  shoes_cached_image *cached = NULL;
  cairo_surface_t *img = NULL;
  VALUE filename = rb_funcall(imgpath, s_downcase, 0);
  char *fname = RSTRING_PTR(filename);
  int len = (int)RSTRING_LEN(filename);

  if (shoes_cache_lookup(RSTRING_PTR(imgpath), &cached))
  {
    *width = cached->width;
    *height = cached->height;
    return cached->format;
  }

  if (!shoes_check_file_exists(imgpath))
    return SHOES_IMAGE_NONE;
  else if (shoes_has_ext(fname, len, ".png"))
  {
    img = shoes_png_size(RSTRING_PTR(imgpath), width, height);
    format = SHOES_IMAGE_PNG;
  }
  else if (shoes_has_ext(fname, len, ".jpg") || shoes_has_ext(fname, len, ".jpeg"))
  {
    img = shoes_surface_create_from_jpeg(RSTRING_PTR(imgpath), width, height, FALSE);
    format = SHOES_IMAGE_JPEG;
  }
  else if (shoes_has_ext(fname, len, ".gif"))
  {
    img = shoes_surface_create_from_gif(RSTRING_PTR(imgpath), width, height, FALSE);
    format = SHOES_IMAGE_GIF;
  }

  if (img != SIZE_SURFACE)
  {
    if (format != SHOES_IMAGE_PNG && 
      (img = shoes_png_size(RSTRING_PTR(imgpath), width, height)) == SIZE_SURFACE)
      format = SHOES_IMAGE_PNG;
    else if (format != SHOES_IMAGE_JPEG && 
      (img = shoes_surface_create_from_jpeg(RSTRING_PTR(imgpath), width, height, FALSE)) == SIZE_SURFACE)
      format = SHOES_IMAGE_JPEG;
    else if (format != SHOES_IMAGE_GIF &&
      (img = shoes_surface_create_from_gif(RSTRING_PTR(imgpath), width, height, FALSE)) == SIZE_SURFACE)
      format = SHOES_IMAGE_GIF;
  }

  if (format == SHOES_IMAGE_NONE)
  {
    shoes_unsupported_image(imgpath);
    return format;
  }

  if (img != SIZE_SURFACE)
    shoes_failed_image(imgpath);

  return format;
}

shoes_code
shoes_load_imagesize(VALUE imgpath, int *width, int *height)
{
  if (shoes_image_detect(imgpath, width, height) == SHOES_IMAGE_NONE)
    return SHOES_FAIL;
  return SHOES_OK;
}

unsigned char
shoes_image_downloaded(shoes_image_download_event *idat)
{
  int i, j, width, height;
  SHA1_CTX context;
  unsigned char *digest = SHOE_ALLOC_N(unsigned char, 20), 
                *buffer = SHOE_ALLOC_N(unsigned char, 16384);

  if (idat->status == 304)
  {
    idat->filepath = idat->cachepath;
    idat->cachepath = NULL;
  }
  else if (idat->status != 200)
  {
    shoes_error("Shoes could not load the file at %s. [%lu]", idat->uripath, idat->status); 
    return 0;
  }

  cairo_surface_t *img = shoes_surface_create_from_file(rb_str_new2(idat->filepath), &width, &height);
  if (img != NULL)
  {
    shoes_cached_image *cached;
    if (shoes_cache_lookup(idat->uripath, &cached) && cached->surface == shoes_world->blank_image)
    {
      cached->surface = img;
      cached->width = width;
      cached->height = height;
      cached->mtime = shoes_file_mtime(idat->filepath);

      if (idat->status != 304)
      {
#ifdef SHOES_WIN32
        HANDLE hFile;
        hFile = CreateFile( idat->filepath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
        SHA1Init(&context);
        DWORD readsize;
        while (1)
        {
          ReadFile( hFile, buffer, 16384, &readsize, NULL );
          if (readsize != 16384) break;
          SHA1Update(&context, buffer, readsize);
        }
        SHA1Final(digest, &context);
        CloseHandle( hFile );
#else
        FILE* fp = fopen(idat->filepath, "rb");
        if (fp == NULL)
        {
          shoes_error("Shoes was unable to open the cached file at %s.", idat->filepath);
          return 0;
        }
        SHA1Init(&context);
        while (!feof(fp)) 
        {
          i = (int)fread(buffer, 1, 16384, fp);
          SHA1Update(&context, buffer, i);
        }
        SHA1Final(digest, &context);
        fclose(fp);
#endif
        
        for (i = 0; i < 5; i++)
          for (j = 0; j < 4; j++)
            sprintf(&idat->hexdigest[(i*8)+(j*2)], "%02x", digest[i*4+j]);
        idat->hexdigest[41] = '\0';
      }
    }
  }

  SHOE_FREE(digest);
  SHOE_FREE(buffer);
  return 1;
}

int
shoes_http_image_handler(shoes_http_event *de, void *data)
{
  shoes_image_download_event *idat = (shoes_image_download_event *)data;
  if (de->stage == SHOES_HTTP_STATUS)
  {
    idat->status = de->status;
  }
  else if (de->stage == SHOES_HTTP_HEADER)
  {
    if (de->hkeylen == 4 && strncmp(de->hkey, "ETag", 4) == 0 && de->hvallen > 0)
    {
      idat->etag = SHOE_ALLOC_N(char, de->hvallen + 1);
      SHOE_MEMCPY(idat->etag, de->hval, char, de->hvallen);
      idat->etag[de->hvallen] = '\0';
    }
  }
  else if (de->stage == SHOES_HTTP_COMPLETED)
  {
    shoes_image_download_event *side = SHOE_ALLOC(shoes_image_download_event);
    SHOE_MEMCPY(side, idat, shoes_image_download_event, 1);
    return shoes_throw_message(SHOES_IMAGE_DOWNLOAD, idat->slot, side);
  }
  return SHOES_DOWNLOAD_CONTINUE;
}

shoes_cached_image *
shoes_load_image(VALUE slot, VALUE imgpath)
{
  shoes_cached_image *cached = NULL;
  cairo_surface_t *img = NULL;
  VALUE filename = rb_funcall(imgpath, s_downcase, 0);
  StringValue(filename);
  char *fname = RSTRING_PTR(filename);
  int width = 1, height = 1;

  if (shoes_cache_lookup(RSTRING_PTR(imgpath), &cached))
    goto done;

  if (strlen(fname) > 7 && (strncmp(fname, "http://", 7) == 0 || strncmp(fname, "https://", 8) == 0))
  {
    struct timeval tv;
    VALUE cache, uext, hdrs, tmppath, uri, scheme, host, port, requ, path, cachepath = Qnil, hash = Qnil;
    rb_require("shoes/data");
    uri = rb_funcall(cShoes, rb_intern("uri"), 1, imgpath);
    scheme = rb_funcall(uri, s_scheme, 0);
    host = rb_funcall(uri, s_host, 0);
    port = rb_funcall(uri, s_port, 0);
    requ = rb_funcall(uri, s_request_uri, 0);
    path = rb_funcall(uri, s_path, 0);
    path = rb_funcall(path, s_downcase, 0);

    cache = rb_funcall(rb_const_get(rb_cObject, rb_intern("DATABASE")), rb_intern("check_cache_for"), 1, imgpath);
    uext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
    hdrs = Qnil;
    if (!NIL_P(cache)) 
    {
      VALUE etag = rb_hash_aref(cache, ID2SYM(rb_intern("etag")));
      hash = rb_hash_aref(cache, ID2SYM(rb_intern("hash")));
      if (!NIL_P(hash)) cachepath = rb_funcall(cShoes, rb_intern("image_cache_path"), 2, hash, uext);
      int saved = NUM2INT(rb_hash_aref(cache, ID2SYM(rb_intern("saved"))));
      gettimeofday(&tv, 0);
      if (tv.tv_sec - saved < SHOES_IMAGE_EXPIRE)
      {
        cached = shoes_load_image(slot, cachepath);
        if (cached != NULL)
        {
          shoes_cache_insert(SHOES_CACHE_ALIAS, imgpath, cached);
          goto done;
        }
      }
      else if (!NIL_P(etag))
        rb_hash_aset(hdrs = rb_hash_new(), rb_str_new2("If-None-Match"), etag);
    }

    cached = shoes_cached_image_new(1, 1, shoes_world->blank_image);
    shoes_cache_insert(SHOES_CACHE_FILE, imgpath, cached);
    tmppath = rb_funcall(cShoes, rb_intern("image_temp_path"), 2, uri, uext);

    shoes_http_request *req = SHOE_ALLOC(shoes_http_request);
    SHOE_MEMZERO(req, shoes_http_request, 1);
    shoes_image_download_event *idat = SHOE_ALLOC(shoes_image_download_event);
    SHOE_MEMZERO(idat, shoes_image_download_event, 1);
    req->url = strdup(RSTRING_PTR(imgpath));
    req->scheme = strdup(RSTRING_PTR(scheme));
    req->host = strdup(RSTRING_PTR(host));
    req->port = NUM2INT(port);
    req->path = strdup(RSTRING_PTR(requ));
    req->handler = shoes_http_image_handler;
    req->filepath = strdup(RSTRING_PTR(tmppath));
    idat->filepath = strdup(RSTRING_PTR(tmppath));
    idat->uripath = strdup(RSTRING_PTR(imgpath));
    idat->slot = slot;
    if (!NIL_P(cachepath)) idat->cachepath = strdup(RSTRING_PTR(cachepath));
    if (!NIL_P(hash)) SHOE_MEMCPY(idat->hexdigest, RSTRING_PTR(hash), char, min(42, RSTRING_LEN(hash)));
    if (!NIL_P(hdrs)) req->headers = shoes_http_headers(hdrs);
    req->data = idat;
    shoes_queue_download(req);
    goto done;
  }

  img = shoes_surface_create_from_file(imgpath, &width, &height);
  if (img != shoes_world->blank_image)
  {
    cached = shoes_cached_image_new(width, height, img);
    shoes_cache_insert(SHOES_CACHE_FILE, imgpath, cached);
  }

done:
  if (cached == NULL)
    cached = shoes_world->blank_cache;
  return cached;
}

