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
  ID  s_subid = rb_intern("group");
  ID  s_aspect = rb_intern("aspect");
  VALUE filename = shoes_hash_get(argv[0], s_filename);
  VALUE fromstring = shoes_hash_get(argv[0], s_content);
  VALUE subidObj = shoes_hash_get(argv[0], s_subid);
  VALUE aspectObj = shoes_hash_get(argv[0], s_aspect);
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
  if (NIL_P(aspectObj) || (strcmp("yes", RSTRING_PTR(aspectObj)) == 0))
    self_t->aspect = (double) self_t->svghdim.width / (double) self_t->svghdim.height;
  else 
  {
    self_t->aspect = 1.0;
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
// forward declares in this file
static void
shoes_svg_draw_surface(cairo_t *, shoes_svg *, shoes_place *, cairo_surface_t *, int, int);

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
  RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE
shoes_svg_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svg *handle = SHOE_ALLOC(shoes_svg);
  SHOE_MEMZERO(handle, shoes_svg, 1);
  obj = Data_Wrap_Struct(klass, shoes_svg_mark, shoes_svg_free, handle);
  handle->svghandle = Qnil;
  handle->parent = Qnil;
  return obj;
}


VALUE
shoes_svg_new(int argc, VALUE *argv, VALUE parent)
{
  rb_arg_list args;
  VALUE klass = cSvg;
  VALUE svghanObj;
  VALUE widthObj = argv[0];
  VALUE heightObj = argv[1];
  if (TYPE(argv[2]) == T_HASH)
    svghanObj = shoes_svghandle_new(1, &argv[2], parent);
  else
    svghanObj = argv[2];
  
  shoes_svghandle *shandle;
  Data_Get_Struct(svghanObj, shoes_svghandle, shandle);
  VALUE obj;
  shoes_canvas *canvas;

  Data_Get_Struct(parent, shoes_canvas, canvas);
  obj = shoes_svg_alloc(klass);

  shoes_svg *self_t;
  int width, height;
  Data_Get_Struct(obj, shoes_svg, self_t);
  self_t->svghandle = svghanObj;
  self_t->place.w = width = NUM2INT(widthObj);
  self_t->place.h = height = NUM2INT(heightObj);
  self_t->parent = shoes_find_canvas(parent);;
  self_t->svghandle = svghanObj;
  shoes_place place;
  self_t->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, place.iw, place.ih);
  self_t->cr = cairo_create(self_t->surface);
  shoes_place_exact(&place, self_t->attr, 0, 0);
  if (place.iw < 1) place.w = place.iw = width;
  if (place.ih < 1) place.h = place.ih = height;
  shoes_svg_draw_surface(self_t->cr, self_t, &place, self_t->surface, place.w, place.h);
  return obj;
}

static void
shoes_svg_draw_surface(cairo_t *cr, shoes_svg *self_t, shoes_place *place, cairo_surface_t *surf, int imw, int imh)
{
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  double outw = imw * 1.0; // slot width
  double outh = imh * 1.0; // slot height
  double scalew = outw / svghan->svghdim.width;
  double scaleh = outh / svghan->svghdim.height;

  double ratio = MIN(outw / svghan->svghdim.width, outh / svghan->svghdim.height);

  if (svghan->aspect != 1.0)
  {
    scalew = scaleh = ratio;
  }

  if (svghan->subid == NULL)
  {
    // Full svg contents
//    if (place->iw != imw || place->ih != imh)                           // What's the use of this ?
//      cairo_scale(cr, (place->iw * 1.) / imw, (place->ih * 1.) / imh);  //
    cairo_scale(cr, scalew, scaleh);
    cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);
    cairo_set_source_surface(cr, surf, 0., 0.);
    rsvg_handle_render_cairo_sub(svghan->handle, cr, svghan->subid);
    self_t->place = *place;
  }
  else 
  {
    cairo_scale(cr, scalew, scaleh);
    cairo_translate(cr, place->ix + place->dx - svghan->svghpos.x, place->iy + place->dy - svghan->svghpos.y);
    cairo_set_source_surface(cr, surf, 0., 0.);
    rsvg_handle_render_cairo_sub(svghan->handle, cr, svghan->subid);
    self_t->place = *place;
  }
  printf("surface\n");
}

// This gets called very often by Shoes. May be slow for large SVG?
VALUE shoes_svg_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_svg *self_t; 
  shoes_place place; 
  shoes_canvas *canvas; 
  Data_Get_Struct(self, shoes_svg, self_t); 
  Data_Get_Struct(c, shoes_canvas, canvas); 
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; 
  int rel =(REL_CANVAS | REL_SCALE);
  shoes_place_decide(&place, c, self_t->attr, self_t->place.w, self_t->place.h, rel, REL_COORDS(rel) == REL_CANVAS);
  VALUE ck = rb_obj_class(c);
  if (RTEST(actual)) 
    shoes_svg_draw_surface( CCR(canvas), self_t, &place, self_t->surface, place.w, place.h);
  if (!ABSY(place)) { 
    canvas->cx += place.w; 
    canvas->cy = place.y; 
    canvas->endx = canvas->cx; 
    canvas->endy = max(canvas->endy, place.y + place.h); 
  } 
  if(ck == cStack) { 
    canvas->cx = CPX(canvas); 
    canvas->cy = canvas->endy; 
  }
  printf("svg draw\n");
  return self;

}

/*  This scales and renders the svg . Called from shoes_native_svg_paint
 *  so those are just tiny functions in cocoa.m and gktsvg.c 
*/
void
shoes_svg_paint_svg(cairo_t *cr, VALUE svg, shoes_canvas *canvas)
{
  shoes_svg *self_t;
  shoes_svghandle *svghan;
  Data_Get_Struct(svg, shoes_svg, self_t);
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);

  double outw = self_t->place.w * 1.0;
  double outh = self_t->place.h * 1.0;
  double scalew = outw / svghan->svghdim.width;
  double scaleh = outh / svghan->svghdim.height;
  double aspect = (double)svghan->svghdim.width / (double)svghan->svghdim.height;
  if (svghan->aspect != 1.0)
  {
    // work with pixels.
    if (svghan->svghdim.width > outw && svghan->svghdim.height > outh)
    {
      // shrink svg  to fit
      scalew = outw / svghan->svghdim.width;
      scaleh = (outh / svghan->svghdim.height) * (1.0 / aspect);
    } 
    if (svghan->svghdim.width < outw && svghan->svghdim.height < outh)
    {
      // expand svg to fit
      scalew = (outw / svghan->svghdim.width) * aspect;
      scaleh = outh /svghan->svghdim.height;
    }
  }
  printf("scalew: %f, scaleh, %f, aspect %f\n", scalew, scaleh, aspect);
  if (svghan->subid == NULL)
  {
    // Full svg
    cairo_scale(cr, scalew , scaleh);
    rsvg_handle_render_cairo_sub(svghan->handle, cr, svghan->subid);
  }
  else
  {
    // a partial svg - fun fact:
    // 0,0 in Shoes (or gdk/gtk ) is left,top. In Cairo and svg, 0,0 is left,bottom)
    cairo_matrix_t matrix;
    cairo_matrix_init_identity (&matrix);
    cairo_matrix_scale (&matrix, scalew, scaleh);
    //cairo_matrix_translate (&matrix, self_t->place.x * -1.0 , ((self_t->place.y + svghan->svghpos.y) *-1.0));
    cairo_matrix_translate (&matrix, (svghan->svghpos.x  * -1.0 ), ((canvas->place.y ) *-1.0));
    cairo_set_matrix (cr, &matrix);


    //cairo_translate(cr, svghan->svghpos.x * -1, 0.0 - svghan->svghpos.y);
    //cairo_translate(cr, self_t->place.ix + self_t->place.dx, (self_t->place.iy + self_t->place.dy));
    //cairo_scale(cr, round(outw), round(outh));
    rsvg_handle_render_cairo_sub(svghan->handle, cr, svghan->subid);

  }
  printf("paint\n");
}


VALUE 
shoes_svg_get_handle(VALUE self)
{
  shoes_svg *self_t;
  Data_Get_Struct(self, shoes_svg, self_t);
  return self_t->svghandle;
}


VALUE
shoes_svg_set_handle(VALUE self, VALUE han)
{
  shoes_canvas *canvas;
  shoes_svg *self_t;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  if ( !NIL_P(han) ) // should test if han/obj is a svghandle
    self_t->svghandle = han;
  else
  {
    // should raise an error
    printf("not a handle\n");
  }
  //shoes_svg_paint_svg(self_t->ref, canvas->cr, self);
  return han;
}

VALUE
shoes_svg_get_dpi(VALUE self)
{
  return INT2NUM(75);
}

VALUE 
shoes_svg_set_dpi(VALUE dpi, VALUE self)
{
  return Qnil;
}
  
// nobody knows what goes in here. 
VALUE shoes_svg_save(VALUE self, VALUE path, VALUE block)
{
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

VALUE shoes_svg_preferred_width(VALUE self)
{
  int w;
  GET_STRUCT(svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  w = svghan->svghdim.width;
  return INT2NUM(w);
}

VALUE shoes_svg_preferred_height(VALUE self)
{
  int h;
  GET_STRUCT(svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  h = svghan->svghdim.height;
  return INT2NUM(h);
}

VALUE shoes_svg_remove(VALUE self)
{
  printf("remove\n");
  shoes_svg *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  //shoes_native_svg_remove(canvas, self_t->ref);
  return Qtrue;
}

