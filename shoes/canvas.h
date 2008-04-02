//
// shoes/canvas.h
// Ruby methods for all the drawing ops.
//
#ifndef SHOES_CANVAS_H
#define SHOES_CANVAS_H

#include <cairo.h>
#include <pango/pangocairo.h>
#ifdef VIDEO
#include <vlc/libvlc.h>
#endif
#include <ruby.h>

#include "shoes/config.h"

struct _shoes_app;

typedef unsigned int PIXEL;

extern const double RAD2PI, PIM2, PI;

#define REL_WINDOW  1
#define REL_CANVAS  2
#define REL_CURSOR  3
#define REL_TILE    4
#define REL_STICKY  5

#define FLAG_POSITION 0x0F
#define FLAG_ABSX     0x10
#define FLAG_ABSY     0x20
#define FLAG_ORIGIN   0x40

#define HOVER_MOTION  0x01
#define HOVER_CLICK   0x02
//
// place struct
// (outlines the area where a control has been placed)
//
typedef struct {
  int x, y, w, h;
  int ix, iy, iw, ih;
  unsigned char flags;
} shoes_place;

#define ABSX(place)   ((place).flags & FLAG_ABSX)
#define ABSY(place)   ((place).flags & FLAG_ABSY)
#define POS(place)    ((place).flags & FLAG_POSITION)
#define ORIGIN(place) ((place).flags & FLAG_ORIGIN)
#define CPX(c)  (c->place.flags & FLAG_ORIGIN ? 0 : c->place.ix)
#define CPY(c)  (c->place.flags & FLAG_ORIGIN ? 0 : c->place.iy)
#define CPB(c)  ((c->place.h - c->place.ih) - (c->place.iy - c->place.y))

//
// color struct
//
typedef struct {
  unsigned char r, g, b, a, on;
} shoes_color;

#define SHOES_COLOR_OPAQUE 0xFF

//
// shape struct
//
typedef struct {
  cairo_path_t *line;
  int width, height;
  double sw;
  VALUE fg;
  VALUE bg;
  VALUE attr;
  VALUE parent;
  shoes_place place;
  char hover;
} shoes_shape;

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
  VALUE ele;
} shoes_link;

//
// text block struct
//
typedef struct {
  VALUE string;
  VALUE texts;
  VALUE links;
  VALUE attr;
  VALUE parent;
  VALUE cursor;
  PangoLayout *layout;
  shoes_place place;
  char hover;
} shoes_textblock;

//
// text struct
//
typedef struct {
  VALUE texts;
  VALUE attr;
  VALUE parent;
  char hover;
} shoes_text;

//
// image struct
//
typedef struct {
  cairo_surface_t *surface;
  cairo_matrix_t *tf;
  VALUE mode;
  shoes_place place;
  VALUE path;
  VALUE attr;
  VALUE parent;
  char hover;
} shoes_image;

#ifdef VIDEO
//
// video struct
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
  libvlc_exception_t excp;
  libvlc_instance_t *vlc;
  shoes_place place;
  int init;
  VALUE path;
  VALUE attr;
  VALUE parent;
  SHOES_SLOT_OS slot;
} shoes_video;
#endif

//
// pattern struct
//
typedef struct {
  cairo_pattern_t *pattern;
  int width, height;     // dimensions of the underlying surface
  VALUE source;
  VALUE attr;
  VALUE parent;
  char hover;
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
  VALUE attr;
  VALUE parent;
  shoes_place place;
} shoes_control;

#define ANIM_NADA    0
#define ANIM_STARTED 1
#define ANIM_PAUSED  2
#define ANIM_STOPPED 3

//
// animation struct
//
typedef struct {
  VALUE block;
  VALUE parent;
  unsigned int rate, frame;
  char started;
#ifdef SHOES_GTK
  guint tag;
#endif
#ifdef SHOES_QUARTZ
  EventLoopTimerRef ref;
#endif
} shoes_timer;

#define CANVAS_NADA    0
#define CANVAS_STARTED 1
#define CANVAS_REMOVED 2

//
// temporary canvas (used internally for painting)
//
typedef struct {
  cairo_t *cr;
  VALUE fg;
  VALUE bg;
  cairo_matrix_t *tf;
  cairo_matrix_t *gr;
  int grl;
  int grt;
  ID mode;
  VALUE contents;
  VALUE parent;
  VALUE attr;
  VALUE click, release,     // canvas-level event handlers
    motion, keypress,
    start, finish;
  unsigned char stage;
  double sw;                // current stroke-width
  int cx, cy;               // cursor x and y (stored in absolute coords)
  int endx, endy;           // jump points if the cursor spills over
  int topy, fully;          // since we often stack vertically
  int width, height;        // the full height and width used by this box
  shoes_place place;        // temporary storage of box placement
  struct _shoes_app *app;
  SHOES_SLOT_OS slot;
#ifdef SHOES_GTK
  GtkWidget *layout;
  GtkWidget *radios;
#endif
} shoes_canvas;

void shoes_slot_init(VALUE, SHOES_SLOT_OS *, int, int, int, int, int);
cairo_t *shoes_cairo_create(SHOES_SLOT_OS *, int, int, int);
void shoes_control_hide_ref(SHOES_CONTROL_REF);
void shoes_control_show_ref(SHOES_CONTROL_REF);

VALUE shoes_app_main(int, VALUE *, VALUE);
VALUE shoes_app_window(int, VALUE *, VALUE, VALUE);
VALUE shoes_app_contents(VALUE);

VALUE shoes_canvas_info (VALUE, VALUE);
VALUE shoes_canvas_debug(VALUE, VALUE);
VALUE shoes_canvas_warn (VALUE, VALUE);
VALUE shoes_canvas_error(VALUE, VALUE);
void shoes_info (const char *fmt, ...);
void shoes_debug(const char *fmt, ...);
void shoes_warn (const char *fmt, ...);
void shoes_error(const char *fmt, ...);

VALUE shoes_canvas_alloc(VALUE);
VALUE shoes_canvas_new(VALUE, struct _shoes_app *);
void shoes_canvas_clear(VALUE);
shoes_canvas *shoes_canvas_init(VALUE, SHOES_SLOT_OS, VALUE, int, int);
void shoes_canvas_paint(VALUE);
void shoes_apply_transformation(shoes_canvas *, cairo_matrix_t *, 
  double, double, double, double, VALUE);
void shoes_canvas_shape_do(shoes_canvas *, double, double, double, double, unsigned char);
VALUE shoes_canvas_style(int, VALUE *, VALUE);
VALUE shoes_canvas_owner(VALUE);
VALUE shoes_canvas_close(VALUE);
VALUE shoes_canvas_get_top(VALUE);
VALUE shoes_canvas_get_left(VALUE);
VALUE shoes_canvas_get_width(VALUE);
VALUE shoes_canvas_get_height(VALUE);
VALUE shoes_canvas_get_scroll_height(VALUE);
VALUE shoes_canvas_get_scroll_max(VALUE);
VALUE shoes_canvas_get_scroll_top(VALUE);
VALUE shoes_canvas_set_scroll_top(VALUE, VALUE);
VALUE shoes_canvas_get_gutter_width(VALUE);
VALUE shoes_canvas_nostroke(VALUE);
VALUE shoes_canvas_stroke(int, VALUE *, VALUE);
VALUE shoes_canvas_strokewidth(VALUE, VALUE);
VALUE shoes_canvas_nofill(VALUE);
VALUE shoes_canvas_fill(int, VALUE *, VALUE);
VALUE shoes_canvas_rgb(int, VALUE *, VALUE);
VALUE shoes_canvas_gray(int, VALUE *, VALUE);
VALUE shoes_canvas_rect(int, VALUE *, VALUE);
VALUE shoes_canvas_oval(int, VALUE *, VALUE);
VALUE shoes_canvas_line(VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_arrow(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_star(int, VALUE *, VALUE);
VALUE shoes_canvas_para(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_banner(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_title(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_subtitle(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_tagline(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_caption(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_inscription(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_code(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_del(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_em(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_ins(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_link(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_span(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_strong(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_sub(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_sup(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_background(int, VALUE *, VALUE);
VALUE shoes_canvas_border(int, VALUE *, VALUE);
VALUE shoes_canvas_video(int, VALUE *, VALUE);
VALUE shoes_canvas_image(int, VALUE *, VALUE);
VALUE shoes_canvas_animate(int, VALUE *, VALUE);
VALUE shoes_canvas_every(int, VALUE *, VALUE);
VALUE shoes_canvas_timer(int, VALUE *, VALUE);
VALUE shoes_canvas_imagesize(VALUE, VALUE);
VALUE shoes_canvas_shape(int, VALUE *, VALUE);
void shoes_canvas_remove_item(VALUE, VALUE, char, char);
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
VALUE shoes_canvas_check(int, VALUE *, VALUE);
VALUE shoes_canvas_radio(int, VALUE *, VALUE);
VALUE shoes_canvas_contents(VALUE);
void shoes_canvas_size(VALUE, int, int);
VALUE shoes_canvas_clear_contents(int, VALUE *, VALUE);
VALUE shoes_canvas_remove(VALUE);
VALUE shoes_canvas_draw(VALUE, VALUE, VALUE);
VALUE shoes_canvas_after(int, VALUE *, VALUE);
VALUE shoes_canvas_before(int, VALUE *, VALUE);
VALUE shoes_canvas_append(int, VALUE *, VALUE);
VALUE shoes_canvas_prepend(int, VALUE *, VALUE);
VALUE shoes_canvas_flow(int, VALUE *, VALUE);
VALUE shoes_canvas_stack(int, VALUE *, VALUE);
VALUE shoes_canvas_mask(int, VALUE *, VALUE);
VALUE shoes_canvas_widget(int, VALUE *, VALUE);
VALUE shoes_canvas_hide(VALUE);
VALUE shoes_canvas_show(VALUE);
VALUE shoes_canvas_toggle(VALUE);
VALUE shoes_canvas_mouse(VALUE);
VALUE shoes_canvas_start(int, VALUE *, VALUE);
VALUE shoes_canvas_finish(int, VALUE *, VALUE);
VALUE shoes_canvas_click(int, VALUE *, VALUE);
VALUE shoes_canvas_release(int, VALUE *, VALUE);
VALUE shoes_canvas_motion(int, VALUE *, VALUE);
VALUE shoes_canvas_keypress(int, VALUE *, VALUE);
int shoes_canvas_independent(shoes_canvas *);
void shoes_canvas_repaint_all(VALUE);
void shoes_canvas_compute(VALUE);
VALUE shoes_canvas_goto(VALUE, VALUE);
VALUE shoes_canvas_send_click(VALUE, int, int, int);
void shoes_canvas_send_release(VALUE, int, int, int);
VALUE shoes_canvas_send_motion(VALUE, int, int, VALUE);
void shoes_canvas_send_keypress(VALUE, VALUE);
VALUE shoes_canvas_get_clipboard(VALUE);
VALUE shoes_canvas_set_clipboard(VALUE, VALUE);
VALUE shoes_canvas_window(int, VALUE *, VALUE);
VALUE shoes_canvas_dialog(int, VALUE *, VALUE);
VALUE shoes_canvas_window_plain(VALUE);
VALUE shoes_canvas_dialog_plain(VALUE);

VALUE shoes_slot_new(VALUE, VALUE, VALUE);
VALUE shoes_flow_new(VALUE, VALUE);
VALUE shoes_stack_new(VALUE, VALUE);
VALUE shoes_mask_new(VALUE, VALUE);
VALUE shoes_widget_new(VALUE, VALUE, VALUE);

VALUE shoes_control_new(VALUE, VALUE, VALUE);
VALUE shoes_control_alloc(VALUE);
void shoes_control_send(VALUE, ID);
VALUE shoes_control_get_top(VALUE);
VALUE shoes_control_get_left(VALUE);
VALUE shoes_control_get_width(VALUE);
VALUE shoes_control_get_height(VALUE);
VALUE shoes_control_remove(VALUE);
VALUE shoes_control_show(VALUE);
VALUE shoes_control_hide(VALUE);
VALUE shoes_control_focus(VALUE);

VALUE shoes_button_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_line_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_line_get_text(VALUE);
VALUE shoes_edit_line_set_text(VALUE, VALUE);
VALUE shoes_edit_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_box_get_text(VALUE);
VALUE shoes_edit_box_set_text(VALUE, VALUE);
VALUE shoes_list_box_text(VALUE);
VALUE shoes_list_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_progress_draw(VALUE, VALUE, VALUE);

VALUE shoes_shape_new(cairo_path_t *, VALUE, VALUE, VALUE, int, int);
VALUE shoes_shape_alloc(VALUE);
VALUE shoes_shape_draw(VALUE, VALUE, VALUE);
VALUE shoes_shape_move(VALUE, VALUE, VALUE);
VALUE shoes_shape_get_top(VALUE);
VALUE shoes_shape_get_left(VALUE);
VALUE shoes_shape_get_width(VALUE);
VALUE shoes_shape_get_height(VALUE);
VALUE shoes_shape_remove(VALUE);
VALUE shoes_shape_motion(VALUE, int, int, int *);
VALUE shoes_shape_send_click(VALUE, int, int, int);
void shoes_shape_send_release(VALUE, int, int, int);

VALUE shoes_image_new(VALUE, VALUE, VALUE, VALUE, cairo_matrix_t *, VALUE);
VALUE shoes_image_alloc(VALUE);
VALUE shoes_image_draw(VALUE, VALUE, VALUE);
VALUE shoes_image_get_top(VALUE);
VALUE shoes_image_get_left(VALUE);
VALUE shoes_image_get_width(VALUE);
VALUE shoes_image_get_height(VALUE);
VALUE shoes_image_remove(VALUE);
VALUE shoes_image_motion(VALUE, int, int, int *);
VALUE shoes_image_send_click(VALUE, int, int, int);
void shoes_image_send_release(VALUE, int, int, int);

#ifdef VIDEO
VALUE shoes_video_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_video_alloc(VALUE);
VALUE shoes_video_draw(VALUE, VALUE, VALUE);
VALUE shoes_video_show(VALUE);
VALUE shoes_video_hide(VALUE);
VALUE shoes_video_get_top(VALUE);
VALUE shoes_video_get_left(VALUE);
VALUE shoes_video_get_width(VALUE);
VALUE shoes_video_get_height(VALUE);
VALUE shoes_video_remove(VALUE);
#endif

VALUE shoes_pattern_self(VALUE);
VALUE shoes_pattern_method(VALUE, VALUE);
VALUE shoes_pattern_args(int, VALUE *, VALUE);
VALUE shoes_pattern_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_pattern_alloc(VALUE);
VALUE shoes_pattern_remove(VALUE);
VALUE shoes_pattern_motion(VALUE, int, int, int *);
VALUE shoes_background_draw(VALUE, VALUE, VALUE);
VALUE shoes_border_draw(VALUE, VALUE, VALUE);
VALUE shoes_subpattern_new(VALUE, VALUE, VALUE);

VALUE shoes_timer_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_timer_alloc(VALUE);
VALUE shoes_timer_init(VALUE, VALUE);
VALUE shoes_timer_remove(VALUE);
VALUE shoes_timer_start(VALUE);
VALUE shoes_timer_stop(VALUE);
void shoes_timer_call(VALUE);

VALUE shoes_color_new(int, int, int, int);
VALUE shoes_color_alloc(VALUE);
VALUE shoes_color_rgb(int, VALUE *, VALUE);
VALUE shoes_color_gray(int, VALUE *, VALUE);
cairo_pattern_t *shoes_color_pattern(VALUE);
void shoes_color_grad_stop(cairo_pattern_t *, double, VALUE);
VALUE shoes_color_args(int, VALUE *, VALUE);
VALUE shoes_color_parse(VALUE, VALUE);
VALUE shoes_color_to_s(VALUE);
VALUE shoes_color_to_pattern(VALUE);
VALUE shoes_color_gradient(VALUE, VALUE, VALUE);

VALUE shoes_link_new(VALUE, int, int);
VALUE shoes_link_alloc(VALUE);
VALUE shoes_text_new(VALUE, VALUE, VALUE);
VALUE shoes_text_alloc(VALUE);

VALUE shoes_textblock_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_textblock_alloc(VALUE);
VALUE shoes_textblock_get_top(VALUE);
VALUE shoes_textblock_get_left(VALUE);
VALUE shoes_textblock_get_width(VALUE);
VALUE shoes_textblock_get_height(VALUE);
VALUE shoes_textblock_remove(VALUE);
VALUE shoes_textblock_set_cursor(VALUE, VALUE);
VALUE shoes_textblock_get_cursor(VALUE);
VALUE shoes_textblock_draw(VALUE, VALUE, VALUE);
VALUE shoes_textblock_motion(VALUE, int, int, int *);
VALUE shoes_textblock_send_click(VALUE, int, int, int, VALUE *);
void shoes_textblock_send_release(VALUE, int, int, int);

VALUE shoes_p(VALUE, VALUE);

//
// shoes/image.c
//
cairo_surface_t *shoes_load_image(VALUE);

#endif
