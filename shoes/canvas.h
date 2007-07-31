//
// shoes/canvas.h
// Ruby methods for all the drawing ops.
//
#ifndef SHOES_CANVAS_H
#define SHOES_CANVAS_H

#include <cairo.h>
#include <pango/pangocairo.h>
#include <ruby.h>

#include "shoes/config.h"

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

//
// color struct
//
typedef struct {
  double r, g, b, a;
  unsigned char on;
} shoes_color;

//
// path struct
//
typedef struct {
  cairo_path_t *line;
  shoes_color fg;
  shoes_color bg;
  VALUE attr;
  VALUE parent;
} shoes_path;

//
// flow struct
//
typedef struct {
  VALUE attr;
  VALUE contents;
  VALUE parent;
} shoes_flow;

//
// link struct
//
typedef struct {
  int start;
  int end;
  VALUE url;
} shoes_link;

//
// text struct
//
typedef struct {
  VALUE markup;
  VALUE links;
  VALUE attr;
  VALUE parent;
  PangoLayout *layout;
  int x, y, w, h, i;

  GString *tmp;
  int linki;
  VALUE linku;
} shoes_text;

//
// image struct
//
typedef struct {
  cairo_surface_t *surface;
  double width, height;
  VALUE path;
  VALUE attr;
  VALUE parent;
} shoes_image;

//
// pattern struct
//
typedef struct {
  cairo_pattern_t *pattern;
  cairo_surface_t *surface;
  VALUE source;
  VALUE attr;
  VALUE parent;
} shoes_pattern;

//
// native controls struct
//
typedef struct {
#ifdef SHOES_GTK
  GtkWidget *ref;
#endif
#ifdef SHOES_QUARTZ
  ControlRef ref;
#endif
#ifdef SHOES_WIN32
  HWND ref;
#endif
  VALUE text;
  VALUE attr;
  VALUE parent;
  int x, y, w, h;
} shoes_control;

//
// temporary canvas (used internally for painting)
//
typedef struct {
  cairo_t *cr;
  shoes_color fg;
  shoes_color bg;
  cairo_matrix_t *tf;
  cairo_matrix_t *gr;
  int grl;
  int grt;
  ID mode;
  VALUE contents;
  VALUE parent;
  VALUE attr;
  VALUE click, release, motion, keypress;
  double x, y, cx, cy, endx, endy;
  int width, height;
  APPSLOT slot;
#ifdef SHOES_GTK
  GtkWidget *layout;
#endif
} shoes_canvas;

void shoes_slot_init(VALUE, APPSLOT *, int, int);
cairo_t *shoes_cairo_create(APPSLOT *, int, int, int);

VALUE shoes_app_main(int, VALUE *, VALUE);
VALUE shoes_canvas_alloc(VALUE);
void shoes_canvas_clear(VALUE);
shoes_canvas *shoes_canvas_init(VALUE, APPSLOT, VALUE, int, int);
void shoes_canvas_paint(VALUE);
void shoes_canvas_shape_do(shoes_canvas *, double, double);
VALUE shoes_canvas_nostroke(VALUE);
VALUE shoes_canvas_stroke(int, VALUE *, VALUE);
VALUE shoes_canvas_strokewidth(VALUE, VALUE);
VALUE shoes_canvas_nofill(VALUE);
VALUE shoes_canvas_fill(int, VALUE *, VALUE);
VALUE shoes_canvas_rect(int, VALUE *, VALUE);
VALUE shoes_canvas_oval(VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_line(VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_arrow(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_star(int, VALUE *, VALUE);
VALUE shoes_canvas_markup(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_background(int, VALUE *, VALUE);
VALUE shoes_canvas_image(int, VALUE *, VALUE);
VALUE shoes_canvas_imagesize(VALUE, VALUE);
VALUE shoes_canvas_path(int, VALUE *, VALUE);
VALUE shoes_canvas_move_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_line_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_curve_to(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_transform(VALUE, VALUE);
VALUE shoes_canvas_translate(VALUE, VALUE, VALUE);
VALUE shoes_canvas_rotate(VALUE, VALUE);
VALUE shoes_canvas_scale(int, VALUE *, VALUE);
VALUE shoes_canvas_skew(int, VALUE *, VALUE);
VALUE shoes_canvas_push(VALUE);
VALUE shoes_canvas_pop(VALUE);
VALUE shoes_canvas_reset(VALUE);
VALUE shoes_canvas_button(int, VALUE *, VALUE);
VALUE shoes_canvas_list_box(int, VALUE *, VALUE);
VALUE shoes_canvas_edit_line(int, VALUE *, VALUE);
VALUE shoes_canvas_edit_box(int, VALUE *, VALUE);
VALUE shoes_canvas_progress(int, VALUE *, VALUE);
VALUE shoes_canvas_contents(VALUE);
void shoes_canvas_size(VALUE, int, int);
VALUE shoes_canvas_draw(VALUE, VALUE, VALUE);
VALUE shoes_canvas_flow(int, VALUE *, VALUE);
VALUE shoes_canvas_stack(int, VALUE *, VALUE);
VALUE shoes_canvas_hide(VALUE);
VALUE shoes_canvas_show(VALUE);
VALUE shoes_canvas_toggle(VALUE);
VALUE shoes_canvas_click(int, VALUE *, VALUE);
VALUE shoes_canvas_release(int, VALUE *, VALUE);
VALUE shoes_canvas_motion(int, VALUE *, VALUE);
VALUE shoes_canvas_keypress(int, VALUE *, VALUE);
void shoes_canvas_repaint_all(VALUE);
VALUE shoes_canvas_goto(VALUE, VALUE);
VALUE shoes_canvas_send_click(VALUE, int, int, int);
void shoes_canvas_send_release(VALUE, int, int, int);
void shoes_canvas_send_motion(VALUE, int, int);
void shoes_canvas_send_keypress(VALUE, VALUE);
VALUE shoes_flow_new(VALUE, VALUE);
VALUE shoes_stack_new(VALUE, VALUE);

VALUE shoes_control_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_control_alloc(VALUE);
void shoes_control_send(VALUE, ID);
VALUE shoes_button_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_line_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_list_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_progress_draw(VALUE, VALUE, VALUE);

VALUE shoes_path_new(cairo_path_t *, VALUE);
VALUE shoes_path_alloc(VALUE);
VALUE shoes_path_draw(VALUE, VALUE, VALUE);
VALUE shoes_path_move(VALUE, VALUE, VALUE);

VALUE shoes_image_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_image_alloc(VALUE);
VALUE shoes_image_draw(VALUE, VALUE, VALUE);

VALUE shoes_pattern_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_pattern_alloc(VALUE);
VALUE shoes_pattern_draw(VALUE, VALUE, VALUE);
VALUE shoes_background_draw(VALUE, VALUE, VALUE);

VALUE shoes_link_new(VALUE, int, int);
VALUE shoes_link_alloc(VALUE);

VALUE shoes_text_new(VALUE, VALUE, VALUE);
VALUE shoes_text_alloc(VALUE);
VALUE shoes_text_draw(VALUE, VALUE, VALUE);
VALUE shoes_text_click(VALUE, int, int, int);

#endif
