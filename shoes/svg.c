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
  // rb_gc_mark_maybe(handle->handle); // handle->handle is not a Ruby object
  // we don't have any Ruby objects to mark.
}

static void
shoes_svghandle_free(shoes_svghandle *handle)
{
  if (handle->handle != NULL)
    g_object_unref(handle->handle);  // do the g_unref thing
  RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE
shoes_svghandle_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svghandle *handle = SHOE_ALLOC(shoes_svghandle);
  SHOE_MEMZERO(handle, shoes_svghandle, 1);
  obj = Data_Wrap_Struct(klass, NULL, shoes_svghandle_free, handle);
  handle->handle = NULL;
  handle->subid = NULL;
  return obj;
}

VALUE
shoes_svghandle_new(int argc, VALUE *argv, VALUE parent)
{
  // parse args for :content or :filename (load file if needed)
  // parse arg for subid. 
  VALUE filename = shoes_hash_get(argv[0], rb_intern("filename"));
  VALUE fromstring = shoes_hash_get(argv[0], rb_intern("content"));
  VALUE subidObj = shoes_hash_get(argv[0], rb_intern("group"));
  VALUE aspectObj = shoes_hash_get(argv[0], rb_intern("aspect"));
  
  VALUE obj = shoes_svghandle_alloc(cSvgHandle);
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
  
  if (NIL_P(aspectObj) || (aspectObj == Qtrue))
  { // :aspect => true or not specified, Keep aspect ratio
    self_t->aspect = 0.0;
  } 
  else if (aspectObj == Qfalse)
  { // :aspect => false, Don't keep aspect ratio
    self_t->aspect = -1.0;
  }
  else if (TYPE(aspectObj) == T_FLOAT) 
  { // :aspect => a double (ie 1.33), Don't keep aspect ratio
    self_t->aspect = NUM2DBL(aspectObj);
  }
  else
  { // fallback on keep aspect ratio
    self_t->aspect = 0.0;
  }
/*
  printf("sub x: %i, y: %i, w: %i, h: %i)\n", 
    self_t->svghpos.x, self_t->svghpos.y, 
    self_t->svghdim.width, self_t->svghdim.height);
*/  
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

VALUE shoes_svghandle_has_group(VALUE self, VALUE group)
{
  shoes_svghandle *handle;
  int result = 0;
  Data_Get_Struct(self, shoes_svghandle, handle);
  if (!NIL_P(group) && (TYPE(group) == T_STRING)) {
    char *grp = RSTRING_PTR(group);
    int has = rsvg_handle_has_sub(handle->handle, grp);
    if (has)
      return Qtrue;
    else
      return Qnil;
  }
  else {
    // raise error - must be string
  }
}

// ------- svg widget -----
// forward declares in this file
static void
shoes_svg_draw_surface(cairo_t *, shoes_svg *, shoes_place *, /*cairo_surface_t *,*/ int, int);

// alloc some memory for a shoes_svg; We'll protect it from gc
// out of caution. fingers crossed.
void
shoes_svg_mark(shoes_svg *svg)
{
  rb_gc_mark_maybe(svg->parent);
  rb_gc_mark_maybe(svg->attr);
  rb_gc_mark_maybe(svg->svghandle);
}

static void
shoes_svg_free(shoes_svg *svg)
{
  RUBY_CRITICAL(SHOE_FREE(svg));
}

VALUE
shoes_svg_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svg *svg = SHOE_ALLOC(shoes_svg);
  SHOE_MEMZERO(svg, shoes_svg, 1);
  obj = Data_Wrap_Struct(klass, shoes_svg_mark, shoes_svg_free, svg);
  svg->svghandle = Qnil;
  svg->parent = Qnil;
  return obj;
}

void
svg_aspect_ratio(int imw, int imh, shoes_svg *self_t, shoes_svghandle *svghan)
{
  double outw = imw * 1.0; // width given to svg() 
  double outh = imh * 1.0; // height given to svg() 

  self_t->scalew = outw / svghan->svghdim.width;   // don't keep aspect ratio, Adapt to parent canvas
  self_t->scaleh = outh / svghan->svghdim.height;  // 

  if (svghan->aspect == 0.0) {                    // keep aspect ratio
    self_t->scalew = self_t->scaleh = MIN(outw / svghan->svghdim.width, outh / svghan->svghdim.height);

  } else if (svghan->aspect > 0.0) {              // don't keep aspect ratio, User aspect ratio

    double new_svgdim_height = svghan->svghdim.width / svghan->aspect;
    double new_svgdim_width = svghan->svghdim.height * svghan->aspect;

    if (outw / new_svgdim_width < outh / new_svgdim_height)
      self_t->scaleh = self_t->scalew * new_svgdim_height / svghan->svghdim.height;
    else
      self_t->scalew = self_t->scaleh * new_svgdim_width / svghan->svghdim.width;
  }
}

VALUE
shoes_svg_new(int argc, VALUE *argv, VALUE parent)
{
  VALUE path = Qnil, attr = Qnil, widthObj, heightObj, svg_string;
  VALUE svghanObj = Qnil;
  
  ID  s_filename = rb_intern ("filename");
  ID  s_content = rb_intern ("content");
  ID  s_subid = rb_intern("group");
  ID  s_aspect = rb_intern("aspect");
  
  shoes_canvas *canvas;
  Data_Get_Struct(parent, shoes_canvas, canvas);
  
  rb_arg_list args;
//  switch (rb_parse_args(argc, argv, "s|h", &args))
  switch (rb_parse_args(argc, argv, "s|h,o|h", &args))
  {
    case 1:
      svg_string = args.a[0];
      attr = args.a[1];
    break;
    case 2: 
      //  if first arg is an svghandle 
      if (rb_obj_is_kind_of(args.a[0], cSvgHandle)) {
        svghanObj = args.a[0];
        attr = args.a[1];
      } else 
        printf("crash ahead\n");
    break;
  }
  
  // get width and height out of hash/attr arg
  if (RTEST(ATTR(attr, width))) {
    widthObj = rb_hash_delete(attr, ID2SYM(s_width));
  } else
    widthObj = RTEST(ATTR(canvas->attr, width)) ? ATTR(canvas->attr, width) : Qnil;
      
  if ( RTEST(ATTR(attr, height))) {
    heightObj = rb_hash_delete(attr, ID2SYM(s_height));
  } else
    heightObj = RTEST(ATTR(canvas->attr, height)) ? ATTR(canvas->attr, height) : Qnil;
  
  if (NIL_P(svghanObj)) {  // likely case
    if (strstr(RSTRING_PTR(svg_string), "</svg>") != NULL) //TODO
      ATTRSET(attr, content, svg_string);
    else
      ATTRSET(attr, filename, svg_string);
    svghanObj = shoes_svghandle_new(1, &attr, parent);
  }
  
  shoes_svghandle *shandle;
  Data_Get_Struct(svghanObj, shoes_svghandle, shandle);
  
  // we couldn't find the width/height of the parent canvas, now that we have a rsvg handle,
  // fallback to original size as defined in the svg file but no more than Shoes.app size
  if (widthObj == Qnil) {
    widthObj = INT2NUM(shandle->svghdim.width);
    widthObj = (shandle->svghdim.width >= canvas->app->width) ? 
                              INT2NUM(canvas->app->width) : widthObj;
  }
  if (heightObj == Qnil) {
    heightObj = INT2NUM(shandle->svghdim.height);
    heightObj = (shandle->svghdim.height >= canvas->app->height) ? 
                              INT2NUM(canvas->app->height) : heightObj;
  }
  //printf("app.width, app->height = %i, %i\n",canvas->app->width, canvas->app->height);
  
  VALUE klass = cSvg, obj;
  obj = shoes_svg_alloc(klass);
  shoes_svg *self_t;
  Data_Get_Struct(obj, shoes_svg, self_t);
  
  self_t->svghandle = svghanObj;
  self_t->place.w = self_t->out_width = NUM2INT(widthObj);
  self_t->place.h = self_t->out_height = NUM2INT(heightObj);
  self_t->parent = parent;
  self_t->scalew = 0.0;
  self_t->scaleh = 0.0;
  self_t->balance_margins_w = 0.0;
  self_t->balance_margins_h = 0.0;
  self_t->attr = attr;
  
  // useless !!?? needs to be confirmed
//  shoes_place place;
//  shoes_place_exact(&place, self_t->attr, 0, 0);
//  if (place.iw < 1) place.w = place.iw = width;
//  if (place.ih < 1) place.h = place.ih = height;
//  
//  shoes_svg_draw_surface(self_t->cr, self_t, &place, self_t->surface, place.w, place.h);
  
  return obj;
}

static void
shoes_svg_draw_surface(cairo_t *cr, shoes_svg *self_t, shoes_place *place, /*cairo_surface_t *surf,*/ int imw, int imh)
{
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  
  // calculate aspect ratio only once at initialization ?? needs testing
  if (self_t->scalew == 0.0 && self_t->scaleh == 0.0) {
    svg_aspect_ratio(imw, imh, self_t, svghan);
  }
  
   // drawing on svg parent's (canvas) surface
  cairo_save(cr);
  cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);
  
  if (svghan->subid == NULL) {
    // need to compensate because of margins
    if (place->iw != imw || place->ih != imh) {     // TODO: works, but must be a better way
      if (self_t->balance_margins_w == 0.0)                 // first time calculated scale factors 
        self_t->balance_margins_w = (place->iw * 1.) / imw; // are the good ones, followings are wrong
      if (self_t->balance_margins_h == 0.0)
        self_t->balance_margins_h = (place->ih * 1.) / imh;
      cairo_scale(cr, self_t->balance_margins_w, self_t->balance_margins_h);
    }
    cairo_scale(cr, self_t->scalew, self_t->scaleh);
  } else {
    cairo_scale(cr, self_t->scalew, self_t->scaleh);  // order of scaling + translate matters !!!
    cairo_translate(cr, -svghan->svghpos.x, -svghan->svghpos.y);
  }
  
  rsvg_handle_render_cairo_sub(svghan->handle, cr, svghan->subid);
  cairo_restore(cr);
  
  self_t->place = *place;
  
  //printf("surface\n");
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
  
  if (RTEST(actual)) 
    shoes_svg_draw_surface( CCR(canvas), self_t, &place, /*self_t->surface,*/ place.w, place.h);
  
  if (!ABSY(place)) { 
    canvas->cx += place.w; 
    canvas->cy = place.y; 
    canvas->endx = canvas->cx; 
    canvas->endy = max(canvas->endy, place.y + place.h); 
  } 
  if(rb_obj_class(c) == cStack) { 
    canvas->cx = CPX(canvas); 
    canvas->cy = canvas->endy; 
  }
  //printf("svg draw\n");
  return self;

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
  
  if ( !NIL_P(han) && (rb_obj_is_kind_of(han, cSvgHandle)) ) {
    self_t->svghandle = han;
    shoes_svghandle *svghan;
    Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
    svg_aspect_ratio(self_t->out_width, self_t->out_height, self_t, svghan);
  } else {
    rb_raise(rb_eArgError, "bad arguments, expecting a cSvgHandle \n");
  }
  
  shoes_canvas_repaint_all(self_t->parent);
  return han;
}

VALUE
shoes_svg_get_dpi(VALUE self)
{
  shoes_svg *self_t;
  Data_Get_Struct(self, shoes_svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  double dpix, dpiy;
  g_object_get(svghan->handle, "dpi-x", &dpix, NULL);
  g_object_get(svghan->handle, "dpi-y", &dpiy, NULL);
  
  return DBL2NUM(dpix); //TODO dpi-x, dpi-y ?
}

VALUE
shoes_svg_set_dpi(VALUE self, VALUE dpi)
{
  shoes_svg *self_t;
  Data_Get_Struct(self, shoes_svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  
  /* We have to handle the change in dpi ourselves as nothing in Shoes has a clue of the meaning of dpi 
   * Default is 90 as per rsvg specification, so if dpi is set to 180 we have to 
   * provide twice the number of pixels in x, twice in y,
   * IF at any moment it has a meaning inside the Shoes environment !!!
   */
  rsvg_handle_set_dpi(svghan->handle, NUM2DBL(dpi));
  //shoes_canvas_repaint_all(self_t->parent); // no meaning at the moment !
  
  return Qnil;
}
  
// nobody knows what goes in here. 
VALUE shoes_svg_save(VALUE self, VALUE path, VALUE block)
{
  return Qnil;
}

/* MACROS in ruby.c
VALUE shoes_svg_show(VALUE self) {} // Done
VALUE shoes_svg_hide(VALUE self) {} // Done

VALUE shoes_svg_get_top(VALUE self) { printf("get_top\n"); }

VALUE shoes_svg_get_left(VALUE self) { printf("get_left\n"); }

VALUE shoes_svg_get_width(VALUE self) { printf("width\n"); }

VALUE shoes_svg_get_height(VALUE self) { printf("height\n"); }
*/

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

VALUE shoes_svg_get_offsetX(VALUE self)
{
  GET_STRUCT(svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  int x = svghan->svghpos.x;
  return INT2NUM(x);
}

VALUE shoes_svg_get_offsetY(VALUE self)
{
  GET_STRUCT(svg, self_t);
  shoes_svghandle *svghan;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, svghan);
  int y = svghan->svghpos.y;
  return INT2NUM(y);
}

VALUE shoes_svg_has_group(VALUE self, VALUE group)
{
  shoes_svg *self_t;
  shoes_svghandle *handle;
  int result = 0;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, handle);
  if (!NIL_P(group) && (TYPE(group) == T_STRING)) {
    char *grp = RSTRING_PTR(group);
    result = rsvg_handle_has_sub(handle->handle, grp);
  }
  else {
    // raise error - must be string
  }
  return (result ? Qtrue : Qnil);
}

VALUE shoes_svg_remove(VALUE self)
{
  printf("remove\n");
  shoes_svg *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_svg, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_svghandle *handle;
  Data_Get_Struct(self_t->svghandle, shoes_svghandle, handle);
  
  rb_ary_delete(canvas->contents, self);
  // let ruby gc collect handle (it may be shared) just remove this ref
  self_t->svghandle = Qnil;
  self_t = NULL;
  self = Qnil;
  
  return Qtrue;
}



