/*
 * plot - draws graphs from annotated arrays
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


/*  ------- Plot widget -----
 *  several methods are defined in ruby.c Macros (CLASS_COMMON2, TRANS_COMMON)
 */

// forward declares in this file
static int
shoes_plot_draw_surface(cairo_t *, shoes_plot *, shoes_place *, int, int);

// alloc some memory for a shoes_plot; We'll protect it from gc
// out of caution. fingers crossed.
void
shoes_plot_mark(shoes_plot *plot)
{
  rb_gc_mark_maybe(plot->parent);
  rb_gc_mark_maybe(plot->attr);
  rb_gc_mark_maybe(plot->values);
  rb_gc_mark_maybe(plot->minvs);
  rb_gc_mark_maybe(plot->maxvs);
  rb_gc_mark_maybe(plot->names);
}

static void
shoes_plot_free(shoes_plot *plot)
{
  shoes_transform_release(plot->st);
  RUBY_CRITICAL(SHOE_FREE(plot));
}

VALUE
shoes_plot_alloc(VALUE klass)
{
  VALUE obj;
  shoes_plot *plot = SHOE_ALLOC(shoes_plot);
  SHOE_MEMZERO(plot, shoes_plot, 1);
  obj = Data_Wrap_Struct(klass, shoes_plot_mark, shoes_plot_free, plot);
  plot->values = Qnil;
  plot->minvs = Qnil;
  plot->maxvs = Qnil;
  plot->names = Qnil;
  plot->parent = Qnil;
  plot->st = NULL;
  return obj;
}

#if 0
void
plot_aspect_ratio(int imw, int imh, shoes_plot *self_t, shoes_plothandle *plothan)
{
  double outw = imw * 1.0; // width given to plot() 
  double outh = imh * 1.0; // height given to plot() 

  self_t->scalew = outw / plothan->plothdim.width;   // don't keep aspect ratio, Fill provided or deduced dimensions
  self_t->scaleh = outh / plothan->plothdim.height;  // 

  if (plothan->aspect == 0.0) {                    // keep aspect ratio
    self_t->scalew = self_t->scaleh = MIN(outw / plothan->plothdim.width, outh / plothan->plothdim.height);

  } else if (plothan->aspect > 0.0) {              // don't keep aspect ratio, User aspect ratio

    double new_plotdim_height = plothan->plothdim.width / plothan->aspect;
    double new_plotdim_width = plothan->plothdim.height * plothan->aspect;

    if (outw / new_plotdim_width < outh / new_plotdim_height)
      self_t->scaleh = self_t->scalew * new_plotdim_height / plothan->plothdim.height;
    else
      self_t->scalew = self_t->scaleh * new_plotdim_width / plothan->plothdim.width;
  }
}
#endif

VALUE
shoes_plot_new(int argc, VALUE *argv, VALUE parent)
{
  VALUE attr = Qnil, widthObj, heightObj;
  shoes_canvas *canvas;
  Data_Get_Struct(parent, shoes_canvas, canvas);
  
  rb_arg_list args;
  // TODO parse args
  switch (rb_parse_args(argc, argv, "ii", &args))
  {
    case 1: 
     widthObj  = args.a[0];
     heightObj = args.a[1];
    break;
  }

  VALUE obj = shoes_plot_alloc(cPlot);
  shoes_plot *self_t;
  Data_Get_Struct(obj, shoes_plot, self_t);
  
  
  self_t->place.w = NUM2INT(widthObj);
  self_t->place.h = NUM2INT(heightObj);
  self_t->parent = parent;
  self_t->attr = Qnil;
  // initialize cairo matrice used in transform methods (rotate, scale, skew, translate)
  self_t->st = shoes_transform_touch(canvas->st);
  
  return obj;
}

#if 0
static int
shoes_plot_draw_surface(cairo_t *cr, shoes_plot *self_t, shoes_place *place, int imw, int imh)
{
  //shoes_plothandle *plothan;
  //(self_t->plothandle, shoes_plothandle, plothan);
  
  // calculate aspect ratio only once at initialization
  //if (self_t->scalew == 0.0 && self_t->scaleh == 0.0) {
  //  plot_aspect_ratio(imw, imh, self_t, plothan);
  //}
  
  /* drawing on plot parent's (canvas) surface */
  // applying any transform : translate, rotate, scale, skew
  shoes_apply_transformation(cr, self_t->st, place, 0);  // cairo_save(cr) inside

  cairo_translate(cr, place->ix + place->dx, place->iy + place->dy);

  if (plothan->subid == NULL) {
    cairo_scale(cr, self_t->scalew, self_t->scaleh);
  } else {
    cairo_scale(cr, self_t->scalew, self_t->scaleh);          // order of scaling + translate matters !!!
    cairo_translate(cr, -plothan->plothpos.x, -plothan->plothpos.y);
  }

  int result = rplot_handle_render_cairo_sub(plothan->handle, cr, plothan->subid);

  shoes_undo_transformation(cr, self_t->st, place, 0); // doing cairo_restore(cr)
  
  self_t->place = *place;
  
  return result;
}
#endif 
// This gets called very often by Shoes. May be slow for large plot?
VALUE shoes_plot_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_plot *self_t; 
  shoes_place place; 
  shoes_canvas *canvas; 
  Data_Get_Struct(self, shoes_plot, self_t); 
  Data_Get_Struct(c, shoes_canvas, canvas); 
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; 
  int rel =(REL_CANVAS | REL_SCALE);
  shoes_place_decide(&place, c, self_t->attr, self_t->place.w, self_t->place.h, rel, REL_COORDS(rel) == REL_CANVAS);
  
  //if (RTEST(actual)) 
  //  shoes_plot_draw_surface( CCR(canvas), self_t, &place, place.w, place.h);
  
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
  // TODO lots of code to be added
  printf("leaving shoes_plot_draw\n");
  return self;
}

VALUE shoes_plot_add(VALUE self, VALUE newseries) 
{
  if (TYPE(newseries) == T_HASH) {
    printf("shoes_plot_add using hash\n");
  }
  if (rb_obj_class(newseries) == cTimeSeries) {
    printf("shoes_plot_add using TimeSeries\n");
  } else {
    printf("shoes_plot_add: assumes timeseries\n");
  }
  return self;
}

VALUE shoes_plot_delete(VALUE self, VALUE series) 
{
  printf("shoes_plot_add called\n");
  return self;
}

VALUE shoes_plot_redraw(VALUE self) 
{
  printf("shoes_plot_redraw called\n");
  return self;
}

#if 0
typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

static cairo_surface_function_t *
get_vector_surface(char *format)
{
  if (strstr(format, "pdf") != NULL) return & cairo_pdf_surface_create;
  if (strstr(format, "ps") != NULL)  return & cairo_ps_surface_create;
  if (strstr(format, "svg") != NULL) return & cairo_svg_surface_create;
  return NULL;
}

static cairo_surface_t* 
buid_surface(VALUE self, VALUE docanvas, double scale, int *result, char *filename, char *format) 
{
  shoes_plot *self_t;
  Data_Get_Struct(self, shoes_plot, self_t);
  shoes_canvas *canvas;
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_place place = self_t->place;
  cairo_surface_t *surf;
  cairo_t *cr;
  
  if (docanvas == Qtrue) {
    if (format != NULL)
      surf = get_vector_surface(format)(filename, canvas->width*scale, canvas->height*scale);
    else
      surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, (int)(canvas->width*scale), (int)(canvas->height*scale));
    cr = cairo_create(surf);
    
    if (scale != 1.0) cairo_scale(cr, scale, scale);
// TODO:   *result = shoes_plot_draw_surface(cr, self_t, &place, (int)(place.w*scale), (int)(place.h*scale));
    place.w = (int)(place.w*scale); place.h = (int)(place.h*scale);
    cairo_t *waz_cr = canvas->cr;
    canvas->cr = cr;
    shoes_canvas_draw(self_t->parent, self_t->parent, Qtrue);
    canvas->cr = waz_cr;
    *result = 1; //TODO
  } else {
    int w = (int)(NUM2INT(shoes_plot_get_actual_width(self))*scale);
    int h = (int)(NUM2INT(shoes_plot_get_actual_height(self))*scale);
    if (format != NULL)
      surf = get_vector_surface(format)(filename, w, h);
    else
      surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cr = cairo_create(surf);
    
    if (scale != 1.0) cairo_scale(cr, scale, scale);
    cairo_translate(cr, -(place.ix + place.dx), -(place.iy + place.dy));
    // TODO *result = shoes_plot_draw_surface(cr, self_t, &place, w, h);
  }
  if (format != NULL) cairo_show_page(cr);
  cairo_destroy(cr);
  
  return surf;
}

VALUE shoes_plot_export(VALUE self, VALUE attr) 
{
  VALUE _filename, _dpi, _docanvas;
  _filename = shoes_hash_get(attr, rb_intern("filename"));
  _dpi = shoes_hash_get(attr, rb_intern("dpi"));
  _docanvas = shoes_hash_get(attr, rb_intern("canvas"));
  double scale = 1.0;
  int result;
  
  if (NIL_P(_filename)) {
    rb_raise(rb_eArgError, "wrong arguments for plot export ({:filename=>'...', "
                            "[:dpi=>90, :canvas=>true|false] })\n:filename is mandatory\n");
  }
  
  if (!NIL_P(_dpi)) scale = NUM2INT(_dpi)/90.0;
  
  cairo_surface_t *surf = buid_surface(self, _docanvas, scale, &result, NULL, NULL);
  
  cairo_status_t r = cairo_surface_write_to_png(surf, RSTRING_PTR(_filename));
  cairo_surface_destroy(surf);
  
  return r == CAIRO_STATUS_SUCCESS ? Qtrue : Qfalse;
}
  
VALUE shoes_plot_save(VALUE self, VALUE attr)
{
  VALUE _filename, _format, _docanvas;
  _filename = shoes_hash_get(attr, rb_intern("filename"));
  _format = shoes_hash_get(attr, rb_intern("format"));
  _docanvas = shoes_hash_get(attr, rb_intern("canvas"));
  int result;
  
  if (NIL_P(_filename) || NIL_P(_format)) {
    rb_raise(rb_eArgError, "wrong arguments for plot save ({:filename=>'...', "
      ":format=>'pdf'|'ps'|'plot' [, :canvas=>true|false] })\n:filename and :format are mandatory");
  }
  
  char *filename = RSTRING_PTR(_filename);
  char *format = RSTRING_PTR(_format);

  cairo_surface_t *surf = buid_surface(self, _docanvas, 1.0, &result, filename, format);
  cairo_surface_destroy(surf);
  
  return result == 0 ? Qfalse : Qtrue;
}
#endif

/*  Not using PLACE_COMMMON Macro in ruby.c, as we do the plot rendering a bit differently
 *  than other widgets [parent, left, top, width, height ruby methods]
 */
VALUE
shoes_plot_get_parent(VALUE self)
{
  GET_STRUCT(plot, self_t);
  return self_t->parent;
}



VALUE shoes_plot_remove(VALUE self)
{
  //printf("remove\n");
  shoes_plot *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_plot, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  
  rb_ary_delete(canvas->contents, self);    // shoes_basic_remove does it this way
  shoes_canvas_repaint_all(self_t->parent); //
  
  // let ruby gc collect handle (it may be shared) just remove this ref
  // TODO self_t->plothandle = Qnil;
  self_t = NULL;
  self = Qnil;
  
  return Qtrue;
}

//called by shoes_plot_send_click and shoes_canvas_send_motion
VALUE
shoes_plot_motion(VALUE self, int x, int y, char *touch)
{
  char h = 0;
  VALUE click;
  GET_STRUCT(plot, self_t);

  click = ATTR(self_t->attr, click);

  if (IS_INSIDE(self_t, x, y)) {
    if (!NIL_P(click)) {
      shoes_canvas *canvas;
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
      shoes_app_cursor(canvas->app, s_link);
    }
    h = 1;
  }
  
  /* Checks if element is hovered, clicked, released, leaved
   * and eventually calls hover and/or leave callbacks
   *   if hovered:  self_t->hover == 1
   *   if leaved:   self_t->hover == 0
   *   if clicked and not yet released:
   *     if hovered + clicked: self_t->hover == 3 
   *     if leaved + clicked:  self_t->hover == 2
   */
  CHECK_HOVER(self_t, h, touch);

  return h ? click : Qnil;
}

// called by shoes_canvas_send_click --> shoes_canvas_send_click2
VALUE
shoes_plot_send_click(VALUE self, int button, int x, int y)
{
  VALUE v = Qnil;

  if (button == 1) {
    GET_STRUCT(plot, self_t);
    v = shoes_plot_motion(self, x, y, NULL);
    if (self_t->hover & HOVER_MOTION)             // ok, cursor is over the element, proceed
      self_t->hover = HOVER_MOTION | HOVER_CLICK; // we have been clicked, but not yet released
  }
  
  // if we found a click callback send it back to shoes_canvas_send_click method
  // where it will be processed
  return v;
}

// called by shoes_canvas_send_release
void
shoes_plot_send_release(VALUE self, int button, int x, int y)
{
  GET_STRUCT(plot, self_t);
  if (button == 1 && (self_t->hover & HOVER_CLICK)) {
    VALUE proc = ATTR(self_t->attr, release);
    self_t->hover ^= HOVER_CLICK; // we have been clicked and released
    if (!NIL_P(proc))
      shoes_safe_block(self, proc, rb_ary_new3(1, self));
  }
}

