#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SHAPE_H
#define SHOES_SHAPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

VALUE cShape;

SYMBOL_ID(shape);

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    ID name;
    char hover;
    cairo_path_t *line;
    shoes_transform *st;
} shoes_shape;

// native forward declarations
/* extern SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
extern void shoes_native_spinner_start(SHOES_CONTROL_REF ref);
extern void shoes_native_spinner_stop(SHOES_CONTROL_REF ref);
extern gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref); */

/* each widget should have its own init function */
void shoes_shape_init();

// ruby
VALUE shoes_shape_draw(VALUE self, VALUE c, VALUE actual);
void shoes_shape_mark(shoes_shape *path);
void shoes_shape_free(shoes_shape *path);
VALUE shoes_shape_attr(int argc, VALUE *argv, int syms, ...);
unsigned char shoes_shape_check(cairo_t *cr, shoes_place *place);
void shoes_shape_sketch(cairo_t *cr, ID name, shoes_place *place, shoes_transform *st, VALUE attr, cairo_path_t* line, unsigned char draw);
VALUE shoes_shape_new(VALUE parent, ID name, VALUE attr, shoes_transform *st, cairo_path_t *line);
VALUE shoes_shape_alloc(VALUE klass);
VALUE shoes_shape_motion(VALUE self, int x, int y, char *touch);
VALUE shoes_shape_send_click(VALUE self, int button, int x, int y);
void shoes_shape_send_release(VALUE self, int button, int x, int y);

// canvas
VALUE shoes_canvas_shape(int argc, VALUE *argv, VALUE self);
VALUE shoes_add_shape(VALUE self, ID name, VALUE attr, cairo_path_t *line);
VALUE shoes_canvas_arc(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_rect(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_oval(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_line(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_arrow(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_star(int argc, VALUE *argv, VALUE self);

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
  
#endif
