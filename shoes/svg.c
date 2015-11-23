/*
 * svghandle -  experimental  in 3.3.0
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

// only one of path or str is non-nil
VALUE
shoes_svg_new(int argc, VALUE *argv, VALUE parent)
{
  rb_arg_list args;
  VALUE klass = cSvg;
  ID  s_filename = rb_intern ("filename");
  ID  s_fromstring = rb_intern ("from_string");
  ID  s_width = rb_intern ("width");
  ID  s_height = rb_intern("height");
  VALUE filename = shoes_hash_get(argv[0], s_filename);
  VALUE fromstring = shoes_hash_get(argv[0], s_fromstring);
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
  if (!NIL_P(filename)) {
    // load it from a file
    char *path = RSTRING_PTR(filename);
    rhandle = rsvg_handle_new_from_file (path, &gerror);
    if (rhandle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else if (!NIL_P(fromstring)) {
    // load it from a string
    char *data = RSTRING_PTR(fromstring);
    int len = RSTRING_LEN(fromstring);
    rhandle = rsvg_handle_new_from_data (data, len, &gerror);
    if (rhandle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else {
    // raise an exception
  }
  Data_Get_Struct(parent, shoes_canvas, canvas);
  obj = shoes_svg_alloc(klass);
  // get a ptr to the struct inside obj
  shoes_svg *svghan;
  Data_Get_Struct(obj, shoes_svg, svghan);
  rsvg_handle_get_dimensions (rhandle, &svghan->svgdim);
  svghan->place.w = width;
  svghan->place.h = height;
  svghan->parent = parent;
  svghan->handle = rhandle;
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
        self_t->ref = shoes_native_svg_new(canvas, self, &canvas->place);
        shoes_place_decide(&place, c, self_t->attr, w, h, REL_CANVAS, TRUE);
        shoes_native_surface_position(self_t->ref, &self_t->place, self, canvas, &place);
        self_t->init = 1;
        shoes_native_svg_draw(self_t->ref, canvas->cr, self);
        //shoes_native_svg_draw_event(self_t->ref, canvas->cr, Qnil);
        //rsvg_handle_render_cairo(self_t->handle, canvas->cr);
      }
    }
    printf("svg_draw\n");
  }
  //if (ATTR(selt_t->attr, hidden) == Qtrue) return self;
  //shoes_place_decide(&place, c, self_t->attr, 200, 200, REL_CANVAS, TRUE);
  //    REL_COORDS(REL_CANVAS) == REL_CANVAS);
  //rsvg_handle_render_cairo(svg->handle, canvas->cr);
  //rsvg_handle_render_cairo(selt_t->handle, canvas->slot->drawevent);
  //shoes_native_svg_draw_event(svg->ref, canvas->cr, Qnil);
  return self;
}

VALUE shoes_svg_render(VALUE self, VALUE argc, VALUE argv)
{
  printf("render\n");
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
}
