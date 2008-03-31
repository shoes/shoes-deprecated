//
// shoes/image.c
// Loading image formats in Cairo.  I've already tried gdk-pixbuf and imlib2, but 
// the idea here is to cut down dependencies, since I only really need reading of 
// the basics: GIF and JPEG.
//
#include <stdio.h>
#include <setjmp.h>
#include "shoes/internal.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include <jpeglib.h>
#include <jerror.h>

#ifndef SHOES_GTK
#ifdef DrawText
#undef DrawText
#endif
#endif
#define DrawText gif_DrawText
#include <gif_lib.h>

#define JPEG_LINES 16

static cairo_surface_t *
shoes_surface_create_from_pixels(PIXEL *pixels, int width, int height)
{
  guchar *cairo_pixels;
  cairo_surface_t *surface;
  static const cairo_user_data_key_t key;
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

cairo_surface_t *
shoes_surface_create_from_gif(char *filename)
{
  cairo_surface_t *surface = NULL;
  GifFileType *gif;
  PIXEL *ptr = NULL, *pixels = NULL;
  GifPixelType **rows = NULL;
  GifRecordType rec;
  ColorMapObject *cmap;
  int i, j, bg, r, g, b, w = 0, h = 0, done = 0, transp = -1;
  float per = 0.0, per_inc;
  int last_per = 0, last_y = 0;
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
      h = gif->Image.Height;
      if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
        goto done;

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
  per_inc = 100.0 / (((float)w) * h);
  for (i = 0; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      if (rows[i][j] == transp)
      {
        r = cmap->Colors[bg].Red;
        g = cmap->Colors[bg].Green;
        b = cmap->Colors[bg].Blue;
        *ptr++ = 0x00ffffff & ((r << 16) | (g << 8) | b);
      }
      else
      {
        r = cmap->Colors[rows[i][j]].Red;
        g = cmap->Colors[rows[i][j]].Green;
        b = cmap->Colors[rows[i][j]].Blue;
        *ptr++ = (0xff << 24) | (r << 16) | (g << 8) | b;
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

  FILE *infile;         /* source stream */
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

static void
shoes_jpeg_term_source(j_decompress_ptr cinfo)
{
}

static void
shoes_jpeg_init_source(j_decompress_ptr cinfo)
{
  shoes_jpeg_file src = (shoes_jpeg_file) cinfo->src;
  src->start_of_file = 1;
}


static boolean
shoes_jpeg_fill_input_buffer(j_decompress_ptr cinfo)
{
  shoes_jpeg_file src = (shoes_jpeg_file)cinfo->src;
  size_t nbytes;

  nbytes = fread(src->buffer, 1, JPEG_INPUT_BUF_SIZE, src->infile);

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

static void
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

static void
jpeg_file_src(j_decompress_ptr cinfo, FILE *infile)
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
 
static void
shoes_jpeg_fatal(j_common_ptr cinfo)
{
  shoes_jpeg_err jpgerr = (shoes_jpeg_err)cinfo->err;
  longjmp(jpgerr->setjmp_buffer, 1);
}

cairo_surface_t *
shoes_surface_create_from_jpeg(char *filename)
{
  int w, h;
  cairo_surface_t *surface = NULL;
  struct jpeg_decompress_struct cinfo;
  struct shoes_jpeg_error_mgr jerr;
  FILE *f = fopen(filename, "rb");
  if (!f) return NULL;

  // TODO: error handling
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = shoes_jpeg_fatal;
  // jpgerr.emit_message = shoes_jpeg_error;
  // jpgerr.output_message = shoes_jpeg_error2;
  if (setjmp(jerr.setjmp_buffer)) {
    // Tcl_AppendResult(interp, "couldn't read JPEG string: ", (char *) NULL);
    // append_jpeg_message(interp, (j_common_ptr) &cinfo);
    jpeg_destroy_decompress(&cinfo);
    return NULL;
  }

  jpeg_create_decompress(&cinfo);
  jpeg_file_src(&cinfo, f);
  jpeg_read_header(&cinfo, TRUE);
  cinfo.do_fancy_upsampling = FALSE;
  cinfo.do_block_smoothing = FALSE;

  jpeg_start_decompress(&cinfo);
  w = cinfo.output_width;
  h = cinfo.output_height;

  if ((w < 1) || (h < 1) || (w > 8192) || (h > 8192))
    goto done;

  int x, y, l, i, scans, count, prevy;
  unsigned char *ptr, *rgb, *line[JPEG_LINES];
  PIXEL *pixels, *ptr2;
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
          ptr += c;
          ptr2++;
        }
      }
    }
  }

  surface = shoes_surface_create_from_pixels(pixels, w, h);
  jpeg_finish_decompress(&cinfo);
done:
  if (pixels != NULL) free(pixels);
  if (rgb != NULL) free(rgb);
  jpeg_destroy_decompress(&cinfo);
  fclose(f);
  return surface;
}

static char
shoes_has_ext(char *fname, int len, const char *ext)
{
  return strncmp(fname + (len - strlen(ext)), ext, strlen(ext)) == 0;
}

static unsigned char
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

static void
shoes_unsupported_image(VALUE path)
{
  VALUE ext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
  StringValue(path);
  StringValue(ext);
  shoes_error("Couldn't load %s. Shoes does not support images with the %s extension.", 
    RSTRING_PTR(path), RSTRING_PTR(ext)); 
}

static void
shoes_failed_image(VALUE path)
{
  VALUE ext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
  StringValue(path);
  StringValue(ext);
  shoes_error("Couldn't load %s. Is the file a valid %s?", 
    RSTRING_PTR(path), RSTRING_PTR(ext)); 
}

#define NO_IMAGE (cairo_surface_t *)-1

cairo_surface_t *
shoes_load_image(VALUE imgpath)
{
  cairo_surface_t *img = NULL;
  VALUE filename = rb_funcall(imgpath, s_downcase, 0);
  char *fname = RSTRING_PTR(filename);
  int len = RSTRING_LEN(filename);

  if (!shoes_check_file_exists(imgpath))
    img = NO_IMAGE;
  else if (shoes_has_ext(fname, len, ".png"))
  {
    img = cairo_image_surface_create_from_png(RSTRING_PTR(imgpath));
    if (cairo_surface_status(img) != CAIRO_STATUS_SUCCESS)
      img = NULL;
  }
  else if (shoes_has_ext(fname, len, ".jpg") || shoes_has_ext(fname, len, ".jpg"))
    img = shoes_surface_create_from_jpeg(RSTRING_PTR(imgpath));
  else if (shoes_has_ext(fname, len, ".gif"))
    img = shoes_surface_create_from_gif(RSTRING_PTR(imgpath));
  else
  {
    shoes_unsupported_image(imgpath);
    img = NO_IMAGE;
  }

  if (img == NULL)
    shoes_failed_image(imgpath);
  else if (img == NO_IMAGE)
    img == NULL;

  return img;
}
