//
// shoes/image.c
// Loading image formats in Cairo.  I've already tried gdk-pixbuf and imlib2, but 
// the idea here is to cut down dependencies, since I only really need reading of 
// the basics: GIF and JPEG.
//
#include "shoes/internal.h"
#include "shoes/canvas.h"
#include <jpeglib.h>
#include <gif_lib.h>

#define JPEG_LINES 16

static cairo_surface_t *
shoes_surface_create_from_pixels(PIXEL *pixels, int width, int height)
{
  guchar *cairo_pixels;
  cairo_surface_t *surface;
  static const cairo_user_data_key_t key;
  int j;
 
  cairo_pixels = g_malloc(4 * width * height);
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

cairo_surface_t *
shoes_surface_create_from_jpeg(char *filename)
{
  int w, h;
  cairo_surface_t *surface = NULL;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jpgerr;
  FILE *f = fopen(filename, "rb");
  if (!f) return NULL;

  // TODO: error handling
  cinfo.err = jpeg_std_error(&jpgerr);
  // jpgerr.error_exit = shoes_jpeg_fatal;
  // jpgerr.emit_message = shoes_jpeg_error;
  // jpgerr.output_message = shoes_jpeg_error2;
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, f);
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

cairo_surface_t *
shoes_load_image(VALUE filename)
{
  char *fname = RSTRING_PTR(filename);
  int len = RSTRING_LEN(filename);
  if (shoes_has_ext(fname, len, ".png"))
  {
    return cairo_image_surface_create_from_png(fname);
  }
  else if (shoes_has_ext(fname, len, ".jpg") || shoes_has_ext(fname, len, ".jpg"))
  {
    return shoes_surface_create_from_jpeg(fname);
  }
  else if (shoes_has_ext(fname, len, ".gif"))
  {
    return shoes_surface_create_from_gif(fname);
  }
  return NULL;
}
