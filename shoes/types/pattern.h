#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_PATTERN_TYPE_H
#define SHOES_PATTERN_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// native forward declarations
// extern SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
// extern void shoes_native_spinner_start(SHOES_CONTROL_REF ref);
// extern void shoes_native_spinner_stop(SHOES_CONTROL_REF ref);
// extern gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref);

VALUE cPattern, cBorder, cBackground;

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    VALUE source;
    char hover;
    shoes_cached_image *cached;
    cairo_pattern_t *pattern;
} shoes_pattern;

/* each widget should have its own init function */
void shoes_pattern_init();

// ruby
void shoes_pattern_mark(shoes_pattern *pattern);
void shoes_pattern_free(shoes_pattern *pattern);
void shoes_pattern_gradient(shoes_pattern *pattern, VALUE r1, VALUE r2, VALUE attr);
VALUE shoes_pattern_set_fill(VALUE self, VALUE source);
VALUE shoes_pattern_get_fill(VALUE self);
VALUE shoes_pattern_self(VALUE self);
VALUE shoes_pattern_args(int argc, VALUE *argv, VALUE self);
VALUE shoes_pattern_new(VALUE klass, VALUE source, VALUE attr, VALUE parent);
VALUE shoes_pattern_method(VALUE klass, VALUE source);
VALUE shoes_pattern_alloc(VALUE klass);
VALUE shoes_background_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_border_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_subpattern_new(VALUE klass, VALUE pat, VALUE parent);

#define PATH_OUT(cr, attr, place, sw, cap, dash, pen, cfunc) \
{ \
  VALUE p = ATTR(attr, pen); \
  if (!NIL_P(p)) \
  { \
    CAP_SET(cr, cap); \
    DASH_SET(cr, dash); \
    cairo_set_line_width(cr, sw); \
    if (rb_obj_is_kind_of(p, cColor)) \
    { \
      shoes_color *color; \
      Data_Get_Struct(p, shoes_color, color); \
      cairo_set_source_rgba(cr, color->r / 255., color->g / 255., color->b / 255., color->a / 255.); \
      cfunc(cr); \
    } \
    else \
    { \
      if (!rb_obj_is_kind_of(p, cPattern)) \
        ATTRSET(attr, pen, p = shoes_pattern_new(cPattern, p, Qnil, Qnil)); \
      cairo_matrix_t matrix1, matrix2; \
      shoes_pattern *pattern; \
      Data_Get_Struct(p, shoes_pattern, pattern); \
      PATTERN_SCALE(pattern, (place), sw); \
      cairo_set_source(cr, PATTERN(pattern)); \
      cfunc(cr); \
      PATTERN_RESET(pattern); \
    } \
  } \
}

#define CAP_SET(cr, cap) \
  if (cap == s_project) \
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); \
  else if (cap == s_round || cap == s_curve) \
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); \
  else if (cap == s_square || cap == s_rect) \
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT)

#define DASH_SET(cr, dash) \
  if (dash == s_onedot) \
  { \
    double dashes[] = {50.0, 10.0, 10.0, 10.0}; \
    int    ndash  = sizeof (dashes)/sizeof(dashes[0]); \
    double offset = -50.0; \
    cairo_set_dash(cr, dashes, ndash, offset); \
  } \
  else \
  { \
    cairo_set_dash(cr, NULL, 0, 0.0); \
  }

#define PATTERN_DIM(self_t, x) (self_t->cached != NULL ? self_t->cached->x : 1)
#define PATTERN(self_t) (self_t->cached != NULL ? self_t->cached->pattern : self_t->pattern)
#define PATTERN_SCALE(self_t, place, sw) \
  if (self_t->cached == NULL) \
  { \
    double woff = abs(place.iw) + (sw * 2.), hoff = abs(place.ih) + (sw * 2.); \
    cairo_pattern_get_matrix(PATTERN(self_t), &matrix1); \
    cairo_pattern_get_matrix(PATTERN(self_t), &matrix2); \
    if (cairo_pattern_get_type(PATTERN(self_t)) == CAIRO_PATTERN_TYPE_RADIAL) \
      cairo_matrix_translate(&matrix2, (-place.ix * 1.) / woff, (-place.iy * 1.) / hoff); \
    cairo_matrix_scale(&matrix2, 1. / woff, 1. / hoff); \
    if (sw != 0.0) cairo_matrix_translate(&matrix2, sw, sw); \
    cairo_pattern_set_matrix(PATTERN(self_t), &matrix2); \
  }

#define PATTERN_RESET(self_t) \
  if (self_t->cached == NULL) \
  { \
    cairo_pattern_set_matrix(PATTERN(self_t), &matrix1); \
  }
  

// canvas
VALUE shoes_canvas_background(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_border(int argc, VALUE *argv, VALUE self);

#endif
