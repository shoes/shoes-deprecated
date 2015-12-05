/*
 * svghandle -  experimental for 3.3.0
 * svg (the widget - two different Shoes/Ruby object
*/
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/version.h"
#include "shoes/http.h"
#include "shoes/effects.h"
#include <math.h>

// ------svghandle --------
// see app.h 

void
shoes_svghandle_mark(shoes_svghandle *handle)
{
  //rb_gc_mark_maybe(handle->handle);
  //rb_gc_mark_maybe(handle->subid);
}

static void
shoes_svghandle_free(shoes_svghandle *handle)
{
  if (handle->handle != NULL)
    ;  // do the g_unref thing
  RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE
shoes_svghandle_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svghandle *handle = SHOE_ALLOC(shoes_svghandle);
  SHOE_MEMZERO(handle, shoes_svghandle, 1);
  obj = Data_Wrap_Struct(klass, shoes_svghandle_mark, shoes_svghandle_free, handle);
  handle->handle = NULL;
  handle->subid = NULL;
  return obj;
}

VALUE
shoes_svghandle_new(int argc, VALUE *argv, VALUE parent)
{
  // parse args for :content or :filename (load file if needed)
  // parse arg for subid. 
  VALUE klass = cSvgHandle;
  ID  s_filename = rb_intern ("filename");
  ID  s_content = rb_intern ("content");
  ID  s_subid = rb_intern("subid");
  VALUE filename = shoes_hash_get(argv[0], s_filename);
  VALUE fromstring = shoes_hash_get(argv[0], s_content);
  VALUE subidObj = shoes_hash_get(argv[0], s_subid);
  VALUE obj = shoes_svghandle_alloc(klass);
  shoes_svghandle *self_t;
  Data_Get_Struct(obj, shoes_svghandle, self_t);
  GError *gerror = NULL;
  if (!NIL_P(filename)) {
    // load it from a file
    char *path = RSTRING_PTR(filename);
    self_t->handle = rsvg_handle_new_from_file (path, &gerror);
    if (self_t->handle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else if (!NIL_P(fromstring)) {
    // load it from a string
    char *data = RSTRING_PTR(fromstring);
    int len = RSTRING_LEN(fromstring);
    self_t->handle = rsvg_handle_new_from_data (data, len, &gerror);
    if (self_t->handle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else {
    // raise an exception
  }
  if (!NIL_P(subidObj) && (RSTRING_LEN(subidObj) > 0))
  {
    self_t->subid = RSTRING_PTR(subidObj);
    if (!rsvg_handle_has_sub(self_t->handle, self_t->subid))
      printf("not a id %s\n",self_t->subid);
    if (!rsvg_handle_get_dimensions_sub(self_t->handle, &self_t->svghdim, self_t->subid))
      printf("no dim for %s\n", self_t->subid);
    if (!rsvg_handle_get_position_sub(self_t->handle, &self_t->svghpos, self_t->subid))
      printf("no pos for %s\n",self_t->subid);
  }
  else 
  {
    rsvg_handle_get_dimensions(self_t->handle, &self_t->svghdim);
    self_t->svghpos.x = self_t->svghpos.y = 0;
    self_t->subid = NULL;
  }
  printf("sub x: %i, y: %i, w: %i, h: %i)\n", 
  self_t->svghpos.x, self_t->svghpos.y, 
  self_t->svghdim.width, self_t->svghdim.height);
  return obj;
}

VALUE
shoes_svghandle_get_width(VALUE self) 
{
  GET_STRUCT(svghandle, self_t);
  return INT2NUM(self_t->svghdim.width);
}

VALUE
shoes_svghandle_get_height(VALUE self)
{
  GET_STRUCT(svghandle, self_t);
  return INT2NUM(self_t->svghdim.height);
}


// ------- svg widget -----

// alloc some memory for a shoes_svg; We'll protect it from gc
// out of caution. fingers crossed.
void
shoes_svg_mark(shoes_svg *handle)
{
  rb_gc_mark_maybe(handle->parent);
  rb_gc_mark_maybe(handle->attr);
}

static void
shoes_svg_free(shoes_svg *handle)
{
  if (handle->handle != NULL)
    ;  // do the g_unref thing
  RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE
shoes_svg_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svg *handle = SHOE_ALLOC(shoes_svg);
  SHOE_MEMZERO(handle, shoes_svg, 1);
  obj = Data_Wrap_Struct(klass, shoes_svg_mark, shoes_svg_free, handle);
  handle->handle = NULL;
  handle->parent = Qnil;
  return obj;
}


VALUE
shoes_svg_new(int argc, VALUE *argv, VALUE parent)
{
  rb_arg_list args;
  VALUE klass = cSvg;
 
  VALUE widthObj = shoes_hash_get(argv[0], s_width);
  VALUE heightObj = shoes_hash_get(argv[0], s_height);
  int width = 300;  //default
  int height = 300;
  if (!NIL_P(widthObj))
    width = NUM2INT(widthObj);
  if (!NIL_P(heightObj))
    height = NUM2INT(heightObj);
  RsvgHandle *rhandle;
  GError *gerror = NULL;
  VALUE obj;
  shoes_canvas *canvas;

  Data_Get_Struct(parent, shoes_canvas, canvas);
  obj = shoes_svg_alloc(klass);
  // get a ptr to the struct inside obj
  shoes_svg *svghan;
  Data_Get_Struct(obj, shoes_svg, svghan);
  svghan->place.w = width;
  svghan->place.h = height;
  svghan->parent = parent;
  svghan->handle = rhandle;
  rsvg_handle_set_dpi(rhandle, 90.0);

  svghan->init = FALSE;
  return obj;
}

// This gets called very often. May be slow for large SVG?
VALUE shoes_svg_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_svg *self_t;
  shoes_canvas *canvas;
  shoes_place place;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);
  if (RTEST(actual))
  {
    if (HAS_DRAWABLE(canvas->slot))
    {
      if (self_t->init == 0)
      {
        // still need to finish the new/init
        int w = self_t->place.w; // script requested size
        int h = self_t->place.h;
        self_t->ref = shoes_native_svg(canvas, self, &canvas->place);
        shoes_place_decide(&place, c, self_t->attr, w, h, REL_CANVAS, TRUE);
        shoes_native_surface_position(self_t->ref, &self_t->place, self, canvas, &place);
        self_t->init = 1;
        shoes_native_svg_paint(self_t->ref, canvas->cr, self);
      }
    }
    printf("svg_draw\n");
  }
  return self;
}

// sets the sub id string or nil for all. Causes a repaint.
VALUE shoes_svg_render(int argc, VALUE *argv, VALUE self)
{
  shoes_canvas *canvas;
  shoes_svg *self_t;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  if (argc == 0 || NIL_P(argv[0]))
    self_t->subid = NULL;
  else
  {
    char *sub = RSTRING_PTR(argv[0]);
    if (RSTRING_LEN(argv[0]) == 0)
      self_t->subid = NULL;
    else
      self_t->subid = sub;
  }
  shoes_native_svg_paint(self_t->ref, canvas->cr, self);
  return Qnil;
}

VALUE shoes_svg_show(VALUE self)
{
  printf("show\n");
}

VALUE shoes_svg_hide(VALUE self)
{
  printf("hide\n");
  GET_STRUCT(svg, self_t);
  ATTRSET(self_t->attr, hidden, Qtrue);
  shoes_native_surface_hide(self_t->ref); // shoes_native_svg_hide ??
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE shoes_svg_get_top(VALUE self)
{
  printf("get_top\n");
}

VALUE shoes_svg_get_left(VALUE self)
{
  printf("get_left\n");
}

VALUE shoes_svg_get_width(VALUE self)
{
  printf("width\n");
}

VALUE shoes_svg_get_height(VALUE self)
{
  printf("height\n");
}

VALUE shoes_svg_get_full_width(VALUE self)
{
  int w;
  //shoes_svg *self_t;
  GET_STRUCT(svg, self_t);
  w = self_t->svgdim.width;
  return INT2NUM(w);
}

VALUE shoes_svg_get_full_height(VALUE self)
{
  GET_STRUCT(svg, self_t);
  return INT2NUM(self_t->svgdim.height);
}

VALUE shoes_svg_remove(VALUE self)
{
  printf("remove\n");
  shoes_svg *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_native_svg_remove(canvas, self_t->ref);
  return Qtrue;
}

/*  This scales and renders the svg . Called from shoes_native_svg_paint
 *  so those are just tiny functions in cocoa.m and gktsvg.c 
*/
void
shoes_svg_paint_svg(cairo_t *cr, VALUE svg)
{
  shoes_svg *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(svg, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  if (self_t->subid == NULL)
  {
    // Full svg
    cairo_scale(cr, (self_t->place.w * 1.0) / self_t->svgdim.width, 
      (self_t->place.h * 1.0) / self_t->svgdim.height);
    rsvg_handle_render_cairo_sub(self_t->handle, cr, self_t->subid);
  }
  else
  {
    // Partial svg 
    cairo_matrix_t matrix;

    double outw = self_t->place.w * 1.0;
    double outh = self_t->place.h * 1.0;
    double scalew = outw / self_t->subdim.width;
    double scaleh = outh / self_t->subdim.height;
    //cairo_scale(cr, round(outw), round(outh));
    //cairo_translate(cr, self_t->place.x, 
    //  self_t->place.y);
    cairo_matrix_init_identity (&matrix);
    cairo_matrix_scale (&matrix, scalew, scaleh);
    // note hack suggesting it's the wrong surface. 
    cairo_matrix_translate (&matrix, self_t->subpos.x * -1.0 , (self_t->subpos.y *-1.0) + 27.0);
    cairo_set_matrix (cr, &matrix);


    rsvg_handle_render_cairo_sub(self_t->handle, cr, self_t->subid);

    //cairo_restore(cr);

  }
  printf("paint\n");
}
