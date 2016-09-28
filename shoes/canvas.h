//
// shoes/canvas.h
// Ruby methods for all the drawing ops.
//
#ifndef SHOES_CANVAS_H
#define SHOES_CANVAS_H

#include <cairo.h>
#include <cairo-svg.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>

#include <pango/pangocairo.h>
#include <ruby.h>

#include "shoes/config.h"
#include "shoes/code.h"
#include <rsvg.h>


struct _shoes_app;

typedef unsigned int PIXEL;

extern const double RAD2PI, PIM2, PI;
extern const char *dialog_title, *dialog_title_says;

#define REL_WINDOW  1
#define REL_CANVAS  2
#define REL_CURSOR  3
#define REL_TILE    4
#define REL_STICKY  5
#define REL_SCALE   8

#define REL_COORDS(x) (x & 0x07)
#define REL_FLAGS(x)  (x & 0xF8)

#define FLAG_POSITION 0x0F
#define FLAG_ABSX     0x10
#define FLAG_ABSY     0x20
#define FLAG_ORIGIN   0x40

#define HOVER_MOTION  0x01
#define HOVER_CLICK   0x02

//
// affine transforms, to avoid littering these structs everywhere
//
typedef struct {
  cairo_matrix_t tf;
  ID mode;
  int refs;
} shoes_transform;

//
// place struct
// (outlines the area where a control has been placed)
//
typedef struct {
  int x, y, w, h, dx, dy;
  int ix, iy, iw, ih;
  unsigned char flags;
} shoes_place;

#define SETUP_BASIC() \
  shoes_basic *basic; \
  Data_Get_Struct(self, shoes_basic, basic);
#define COPY_PENS(attr1, attr2) \
  if (NIL_P(ATTR(attr1, stroke))) ATTRSET(attr1, stroke, ATTR(attr2, stroke)); \
  if (NIL_P(ATTR(attr1, fill)))   ATTRSET(attr1, fill, ATTR(attr2, fill)); \
  if (NIL_P(ATTR(attr1, strokewidth))) ATTRSET(attr1, strokewidth, ATTR(attr2, strokewidth)); \
  if (NIL_P(ATTR(attr1, cap))) ATTRSET(attr1, cap, ATTR(attr2, cap));
#define DRAW(c, app, blk) \
  { \
    rb_ary_push(app->nesting, c); \
    blk; \
    rb_ary_pop(app->nesting); \
  }
#define PATTERN_DIM(self_t, x) (self_t->cached != NULL ? self_t->cached->x : 1)
#define PATTERN(self_t) (self_t->cached != NULL ? self_t->cached->pattern : self_t->pattern)
#define ABSX(place)   ((place).flags & FLAG_ABSX)
#define ABSY(place)   ((place).flags & FLAG_ABSY)
#define POS(place)    ((place).flags & FLAG_POSITION)
#define ORIGIN(place) ((place).flags & FLAG_ORIGIN)
#define CPX(c)  (c != NULL && (c->place.flags & FLAG_ORIGIN) ? 0 : c->place.ix)
#define CPY(c)  (c != NULL && (c->place.flags & FLAG_ORIGIN) ? 0 : c->place.iy)
#define CPB(c)  ((c->place.h - c->place.ih) - (c->place.iy - c->place.y))
#define CPH(c)  (ORIGIN(c->place) ? c->height : (c->fully - CPB(c)) - CPY(c))
#define CPW(c)  (c->place.iw)
#define CCR(c)  (c->cr == NULL ? c->app->scratch : c->cr)
#define SWPOS(x) ((int)sw % 2 == 0 ? x * 1. : x + .5)

//
// color struct
//
typedef struct {
  unsigned char r, g, b, a, on;
} shoes_color;

#define SHOES_COLOR_OPAQUE 0xFF
#define SHOES_COLOR_TRANSPARENT 0x0
#define SHOES_COLOR_DARK   (0x55 * 3)
#define SHOES_COLOR_LIGHT  (0xAA * 3)

//
// basic struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
} shoes_basic;

typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
} shoes_element;

//
// shape struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  ID name;
  char hover;
  cairo_path_t *line;
  shoes_transform *st;
} shoes_shape;

//
// flow struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  VALUE contents;
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
// text cursor
//
typedef struct {
  int pos, x, y, hi;
} shoes_textcursor;

//
// text block struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  VALUE texts;
  VALUE links;
  shoes_textcursor *cursor;
  PangoLayout *layout;
  PangoAttrList *pattr;
  GString *text;
  guint len;
  char cached, hover;
  shoes_transform *st;
} shoes_textblock;

//
// text struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  VALUE texts;
  char hover;
} shoes_text;

//
// cached image
//
typedef enum {
  SHOES_IMAGE_NONE,
  SHOES_IMAGE_PNG,
  SHOES_IMAGE_JPEG,
  SHOES_IMAGE_GIF
} shoes_image_format;

typedef struct {
  cairo_surface_t *surface;
  cairo_pattern_t *pattern;
  int width, height, mtime;
  shoes_image_format format;
} shoes_cached_image;

#define SHOES_CACHE_FILE  0
#define SHOES_CACHE_ALIAS 1
#define SHOES_CACHE_MEM   2

typedef struct {
  unsigned char type;
  shoes_cached_image *image;
} shoes_cache_entry;

//
// image struct
//
#define SHOES_IMAGE_EXPIRE (60 * 60)
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  unsigned char type;
  shoes_cached_image *cached;
  shoes_transform *st;
  cairo_t *cr;
  VALUE path;
  char hover;
} shoes_image;


//
// pattern struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  VALUE source;
  char hover;
  shoes_cached_image *cached;
  cairo_pattern_t *pattern;
} shoes_pattern;

//
// native controls struct
//
#define CONTROL_NORMAL   0
#define CONTROL_READONLY 1
#define CONTROL_DISABLED 2

typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  SHOES_CONTROL_REF ref;
} shoes_control;

#define ANIM_NADA    0
#define ANIM_STARTED 1
#define ANIM_PAUSED  2
#define ANIM_STOPPED 3

//
// animation struct
//
typedef struct {
  VALUE parent;
  VALUE block;
  unsigned int rate, frame;
  char started;
  SHOES_TIMER_REF ref;
} shoes_timer;

typedef void (*shoes_effect_filter)(cairo_t *, VALUE attr, shoes_place *);

typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  shoes_effect_filter filter;
} shoes_effect;

typedef struct {
  VALUE parent;
  VALUE attr;
  VALUE response;
  unsigned char state;
  unsigned LONG_LONG total;
  unsigned LONG_LONG transferred;
  unsigned long percent;
} shoes_http_klass;

#define CANVAS_NADA    0
#define CANVAS_STARTED 1
#define CANVAS_PAINT   2
#define CANVAS_EMPTY   3
#define CANVAS_REMOVED 4

// SvgHandle struct - not a graphical widget
// new in 3.3.0
typedef struct _svghandle {
  RsvgHandle *handle;
  RsvgDimensionData svghdim;
  RsvgPositionData svghpos;
  char *path;
  char *data;
  char *subid;
  double aspect;
} shoes_svghandle;

//
// SVG struct
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  double scalew;
  double scaleh;
  VALUE svghandle;
  char hover;
  shoes_transform *st;
} shoes_svg;


//
// Plot struct - It's HUGE!
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  int chart_type;
  int seriescnt;
  int auto_grid; 
  int boundbox;
  int missing; 
  VALUE background;
  VALUE maxvs;  // these will be Ruby arrays of things (0..seriescnt)
  VALUE minvs;
  VALUE values;
  VALUE names;  
  VALUE long_names; // for y axis display?? Someday
  VALUE xobs; 
  VALUE sizes;
  VALUE strokes;
  VALUE nubs;
  VALUE color;
  // now the singles for the plot
  VALUE title;  
  VALUE legend; 
  VALUE caption;
  int x_ticks;   // number of x_axis (which means a vertical grid line draw)
  int y_ticks;   // number of (left side) y axis horizontial grid lines)
  char  *fontname; // not a Shoes name, cairo "toy" name - might be the same
  int beg_idx;  //used for zooming in
  int end_idx;  // and zooming out
  int title_h;
  PangoFontDescription *title_pfd; 
  int caption_h;
  PangoFontDescription *caption_pfd; 
  int legend_h; 
  PangoFontDescription *legend_pfd;
  PangoFontDescription *label_pfd; 
  int yaxis_offset; // don't like
  int graph_h;  // to where the dots are drawn
  int graph_w;
  int graph_x;
  int graph_y;
  char hover;   
  shoes_transform *st;
} shoes_plot;

//
// not very temporary canvas (used internally for painting)
//
typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  cairo_t *cr, *shape;
  shoes_transform *st, **sts;
  int stl, stt;
  VALUE contents;
  unsigned char stage;
  long insertion;
  int cx, cy;               // cursor x and y (stored in absolute coords)
  int endx, endy;           // jump points if the cursor spills over
  int topy, fully;          // since we often stack vertically
  int width, height;        // the full height and width used by this box
  char hover;
  struct _shoes_app *app;
  SHOES_SLOT_OS *slot;
  SHOES_GROUP_OS group;
} shoes_canvas;

void shoes_control_hide_ref(SHOES_CONTROL_REF);
void shoes_control_show_ref(SHOES_CONTROL_REF);

VALUE shoes_app_main(int, VALUE *, VALUE);
VALUE shoes_app_window(int, VALUE *, VALUE, VALUE);
VALUE shoes_app_contents(VALUE);
VALUE shoes_app_get_width(VALUE);
VALUE shoes_app_get_height(VALUE);

VALUE shoes_basic_remove(VALUE);

shoes_transform *shoes_transform_new(shoes_transform *);
shoes_transform *shoes_transform_touch(shoes_transform *);
shoes_transform *shoes_transform_detach(shoes_transform *);
void shoes_transform_release(shoes_transform *);

VALUE shoes_canvas_info (VALUE, VALUE);
VALUE shoes_canvas_debug(VALUE, VALUE);
VALUE shoes_canvas_warn (VALUE, VALUE);
VALUE shoes_canvas_error(VALUE, VALUE);
void shoes_info (const char *fmt, ...);
void shoes_debug(const char *fmt, ...);
void shoes_warn (const char *fmt, ...);
void shoes_error(const char *fmt, ...);

void shoes_canvas_mark(shoes_canvas *);
VALUE shoes_canvas_alloc(VALUE);
VALUE shoes_canvas_new(VALUE, struct _shoes_app *);
void shoes_canvas_clear(VALUE);
shoes_canvas *shoes_canvas_init(VALUE, SHOES_SLOT_OS *, VALUE, int, int);
void shoes_slot_scroll_to(shoes_canvas *, int, int);
void shoes_canvas_paint(VALUE);
void shoes_apply_transformation(cairo_t *, shoes_transform *, shoes_place *, unsigned char);
void shoes_undo_transformation(cairo_t *, shoes_transform *, shoes_place *, unsigned char);
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
VALUE shoes_canvas_displace(VALUE, VALUE, VALUE);
VALUE shoes_canvas_move(VALUE, VALUE, VALUE);
VALUE shoes_canvas_nostroke(VALUE);
VALUE shoes_canvas_stroke(int, VALUE *, VALUE);
VALUE shoes_canvas_strokewidth(VALUE, VALUE);
VALUE shoes_canvas_dash(VALUE, VALUE);
VALUE shoes_canvas_cap(VALUE, VALUE);
VALUE shoes_canvas_nofill(VALUE);
VALUE shoes_canvas_fill(int, VALUE *, VALUE);
VALUE shoes_canvas_rgb(int, VALUE *, VALUE);
VALUE shoes_canvas_gray(int, VALUE *, VALUE);
VALUE shoes_canvas_rect(int, VALUE *, VALUE);
VALUE shoes_canvas_arc(int, VALUE *, VALUE);
VALUE shoes_canvas_oval(int, VALUE *, VALUE);
VALUE shoes_canvas_line(int, VALUE *, VALUE);
VALUE shoes_canvas_arrow(int, VALUE *, VALUE);
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
VALUE shoes_canvas_blur(int, VALUE *, VALUE);
VALUE shoes_canvas_glow(int, VALUE *, VALUE);
VALUE shoes_canvas_shadow(int, VALUE *, VALUE);
VALUE shoes_canvas_image(int, VALUE *, VALUE);
VALUE shoes_canvas_animate(int, VALUE *, VALUE);
VALUE shoes_canvas_every(int, VALUE *, VALUE);
VALUE shoes_canvas_timer(int, VALUE *, VALUE);
VALUE shoes_canvas_svg(int, VALUE *, VALUE);
VALUE shoes_canvas_plot(int, VALUE *, VALUE);
VALUE shoes_canvas_imagesize(VALUE, VALUE);
VALUE shoes_canvas_shape(int, VALUE *, VALUE);
void shoes_canvas_remove_item(VALUE, VALUE, char, char);
VALUE shoes_canvas_move_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_line_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_curve_to(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_arc_to(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
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
VALUE shoes_canvas_text_edit_box(int, VALUE *, VALUE);
VALUE shoes_canvas_progress(int, VALUE *, VALUE);
VALUE shoes_canvas_slider(int, VALUE *, VALUE);
VALUE shoes_canvas_check(int, VALUE *, VALUE);
VALUE shoes_canvas_radio(int, VALUE *, VALUE);
VALUE shoes_canvas_contents(VALUE);
VALUE shoes_canvas_children(VALUE);
void shoes_canvas_size(VALUE, int, int);
VALUE shoes_canvas_clear_contents(int, VALUE *, VALUE);
VALUE shoes_canvas_remove(VALUE);
VALUE shoes_canvas_refresh_slot(VALUE);  // 3.3.0
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
VALUE shoes_canvas_hover(int, VALUE *, VALUE);
VALUE shoes_canvas_leave(int, VALUE *, VALUE);
VALUE shoes_canvas_click(int, VALUE *, VALUE);
VALUE shoes_canvas_release(int, VALUE *, VALUE);
VALUE shoes_canvas_motion(int, VALUE *, VALUE);
VALUE shoes_canvas_keydown(int, VALUE *, VALUE);
VALUE shoes_canvas_keypress(int, VALUE *, VALUE);
VALUE shoes_canvas_keyup(int, VALUE *, VALUE);
int shoes_canvas_independent(shoes_canvas *);
VALUE shoes_find_canvas(VALUE);
VALUE shoes_canvas_get_app(VALUE);
void shoes_canvas_repaint_all(VALUE);
void shoes_canvas_compute(VALUE);
VALUE shoes_canvas_goto(VALUE, VALUE);
VALUE shoes_canvas_send_click(VALUE, int, int, int);
void shoes_canvas_send_release(VALUE, int, int, int);
VALUE shoes_canvas_send_motion(VALUE, int, int, VALUE);
void shoes_canvas_send_wheel(VALUE, ID, int, int);
void shoes_canvas_wheel_way(shoes_canvas *, ID);
void shoes_canvas_send_keydown(VALUE, VALUE);
void shoes_canvas_send_keypress(VALUE, VALUE);
void shoes_canvas_send_keyup(VALUE, VALUE);
void shoes_canvas_send_finish(VALUE);
VALUE shoes_canvas_get_cursor(VALUE);
VALUE shoes_canvas_set_cursor(VALUE, VALUE);
VALUE shoes_canvas_get_clipboard(VALUE);
VALUE shoes_canvas_set_clipboard(VALUE, VALUE);
VALUE shoes_canvas_window(int, VALUE *, VALUE);
VALUE shoes_canvas_dialog(int, VALUE *, VALUE);
VALUE shoes_canvas_window_plain(VALUE);
VALUE shoes_canvas_dialog_plain(VALUE);
VALUE shoes_canvas_snapshot(int, VALUE *, VALUE);
VALUE shoes_canvas_download(int, VALUE *, VALUE);


SHOES_SLOT_OS *shoes_slot_alloc(shoes_canvas *, SHOES_SLOT_OS *, int);
VALUE shoes_slot_new(VALUE, VALUE, VALUE);
VALUE shoes_flow_new(VALUE, VALUE);
VALUE shoes_stack_new(VALUE, VALUE);
VALUE shoes_mask_new(VALUE, VALUE);
VALUE shoes_widget_new(VALUE, VALUE, VALUE);

VALUE shoes_svghandle_new(int argc, VALUE *argv, VALUE self);
VALUE shoes_svghandle_alloc(VALUE);
VALUE shoes_svghandle_get_width(VALUE);
VALUE shoes_svghandle_get_height(VALUE);
VALUE shoes_svghandle_has_group(VALUE, VALUE);

VALUE shoes_svg_new(int, VALUE *, VALUE);
VALUE shoes_svg_alloc(VALUE);
VALUE shoes_svg_draw(VALUE, VALUE, VALUE);
VALUE shoes_svg_get_handle(VALUE);
VALUE shoes_svg_set_handle(VALUE, VALUE);
VALUE shoes_svg_get_dpi(VALUE);
VALUE shoes_svg_set_dpi(VALUE, VALUE);
VALUE shoes_svg_export(VALUE, VALUE);
VALUE shoes_svg_save(VALUE, VALUE);
VALUE shoes_svg_show(VALUE);
VALUE shoes_svg_hide(VALUE);
VALUE shoes_svg_get_actual_width(VALUE);
VALUE shoes_svg_get_actual_height(VALUE);
VALUE shoes_svg_get_actual_left(VALUE);
VALUE shoes_svg_get_actual_top(VALUE);
VALUE shoes_svg_get_parent(VALUE);
VALUE shoes_svg_get_offsetX(VALUE);
VALUE shoes_svg_get_offsetY(VALUE);
VALUE shoes_svg_preferred_height(VALUE);
VALUE shoes_svg_preferred_width(VALUE);
VALUE shoes_svg_remove(VALUE);
VALUE shoes_svg_has_group(VALUE, VALUE);
VALUE shoes_svg_motion(VALUE, int, int, char *);
VALUE shoes_svg_send_click(VALUE, int, int, int);
void shoes_svg_send_release(VALUE, int, int, int);

VALUE shoes_plot_new(int, VALUE *, VALUE);
VALUE shoes_plot_alloc(VALUE);
VALUE shoes_plot_draw(VALUE, VALUE, VALUE);
VALUE shoes_plot_redraw_to(VALUE, VALUE);
VALUE shoes_plot_add(VALUE, VALUE);
VALUE shoes_plot_delete(VALUE, VALUE);
VALUE shoes_plot_find_name(VALUE, VALUE);
VALUE shoes_plot_get_count(VALUE);
VALUE shoes_plot_get_first(VALUE);
VALUE shoes_plot_set_first(VALUE, VALUE);
VALUE shoes_plot_get_last(VALUE);
VALUE shoes_plot_set_last(VALUE, VALUE);
VALUE shoes_plot_near(VALUE, VALUE);
VALUE shoes_plot_zoom(VALUE, VALUE, VALUE);
VALUE shoes_plot_get_actual_width(VALUE);
VALUE shoes_plot_get_actual_height(VALUE);
VALUE shoes_plot_get_actual_left(VALUE);
VALUE shoes_plot_get_actual_top(VALUE);
VALUE shoes_plot_remove(VALUE);
VALUE shoes_plot_export(VALUE, VALUE);
VALUE shoes_plot_save_as(int, VALUE *, VALUE);
VALUE shoes_plot_show(VALUE);
VALUE shoes_plot_hide(VALUE);
VALUE shoes_plot_get_parent(VALUE);
VALUE shoes_plot_motion(VALUE, int, int, char *);
VALUE shoes_plot_send_click(VALUE, int, int, int);
void shoes_plot_send_release(VALUE, int, int, int);

void shoes_control_mark(shoes_control *);
VALUE shoes_control_new(VALUE, VALUE, VALUE);
VALUE shoes_control_alloc(VALUE);
void shoes_control_send(VALUE, ID);
VALUE shoes_control_get_top(VALUE);
VALUE shoes_control_get_left(VALUE);
VALUE shoes_control_get_width(VALUE);
VALUE shoes_control_get_height(VALUE);
VALUE shoes_control_remove(VALUE);
VALUE shoes_control_show(VALUE);
VALUE shoes_control_temporary_show(VALUE);
VALUE shoes_control_hide(VALUE);
VALUE shoes_control_temporary_hide(VALUE);
VALUE shoes_control_focus(VALUE);
VALUE shoes_control_get_state(VALUE);
VALUE shoes_control_set_state(VALUE, VALUE);

VALUE shoes_button_draw(VALUE, VALUE, VALUE);
void shoes_button_send_click(VALUE);
VALUE shoes_edit_line_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_line_get_text(VALUE);
VALUE shoes_edit_line_set_text(VALUE, VALUE);
VALUE shoes_edit_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_edit_box_get_text(VALUE);
VALUE shoes_edit_box_set_text(VALUE, VALUE);
VALUE shoes_list_box_text(VALUE);
VALUE shoes_list_box_draw(VALUE, VALUE, VALUE);
VALUE shoes_progress_draw(VALUE, VALUE, VALUE);

void shoes_shape_mark(shoes_shape *);
VALUE shoes_shape_attr(int, VALUE *, int, ...);
void shoes_shape_sketch(cairo_t *, ID, shoes_place *, shoes_transform *, VALUE, cairo_path_t *, unsigned char);
VALUE shoes_shape_new(VALUE, ID, VALUE, shoes_transform *, cairo_path_t *);
VALUE shoes_shape_alloc(VALUE);
VALUE shoes_shape_draw(VALUE, VALUE, VALUE);
VALUE shoes_shape_move(VALUE, VALUE, VALUE);
VALUE shoes_shape_get_top(VALUE);
VALUE shoes_shape_get_left(VALUE);
VALUE shoes_shape_get_width(VALUE);
VALUE shoes_shape_get_height(VALUE);
VALUE shoes_shape_motion(VALUE, int, int, char *);
VALUE shoes_shape_send_click(VALUE, int, int, int);
void shoes_shape_send_release(VALUE, int, int, int);

void shoes_image_ensure_dup(shoes_image *);
void shoes_image_mark(shoes_image *);
VALUE shoes_image_new(VALUE, VALUE, VALUE, VALUE, shoes_transform *);
VALUE shoes_image_alloc(VALUE);
void shoes_image_image(VALUE, VALUE, VALUE);
VALUE shoes_image_draw(VALUE, VALUE, VALUE);
VALUE shoes_image_get_top(VALUE);
VALUE shoes_image_get_left(VALUE);
VALUE shoes_image_get_width(VALUE);
VALUE shoes_image_get_height(VALUE);
VALUE shoes_image_motion(VALUE, int, int, char *);
VALUE shoes_image_send_click(VALUE, int, int, int);
void shoes_image_send_release(VALUE, int, int, int);

shoes_effect_filter shoes_effect_for_type(ID);
void shoes_effect_mark(shoes_effect *);
VALUE shoes_effect_new(ID, VALUE, VALUE);
VALUE shoes_effect_alloc(VALUE);
VALUE shoes_effect_draw(VALUE, VALUE, VALUE);

VALUE shoes_pattern_self(VALUE);
VALUE shoes_pattern_method(VALUE, VALUE);
VALUE shoes_pattern_args(int, VALUE *, VALUE);
void shoes_pattern_mark(shoes_pattern *);
VALUE shoes_pattern_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_pattern_alloc(VALUE);
VALUE shoes_pattern_motion(VALUE, int, int, char *);
VALUE shoes_background_draw(VALUE, VALUE, VALUE);
VALUE shoes_border_draw(VALUE, VALUE, VALUE);
VALUE shoes_subpattern_new(VALUE, VALUE, VALUE);

void shoes_timer_mark(shoes_timer *);
VALUE shoes_timer_new(VALUE, VALUE, VALUE, VALUE);
VALUE shoes_timer_alloc(VALUE);
VALUE shoes_timer_init(VALUE, VALUE);
VALUE shoes_timer_remove(VALUE);
VALUE shoes_timer_start(VALUE);
VALUE shoes_timer_stop(VALUE);
void shoes_timer_call(VALUE);

void shoes_color_mark(shoes_color *);
VALUE shoes_color_new(int, int, int, int);
VALUE shoes_color_alloc(VALUE);
VALUE shoes_color_rgb(int, VALUE *, VALUE);
VALUE shoes_color_gray(int, VALUE *, VALUE);
cairo_pattern_t *shoes_color_pattern(VALUE);
void shoes_color_grad_stop(cairo_pattern_t *, double, VALUE);
VALUE shoes_color_args(int, VALUE *, VALUE);
VALUE shoes_color_parse(VALUE, VALUE);
VALUE shoes_color_is_black(VALUE);
VALUE shoes_color_is_dark(VALUE);
VALUE shoes_color_is_light(VALUE);
VALUE shoes_color_is_white(VALUE);
VALUE shoes_color_invert(VALUE);
VALUE shoes_color_to_s(VALUE);
VALUE shoes_color_to_pattern(VALUE);
VALUE shoes_color_gradient(int, VALUE *, VALUE);

void shoes_link_mark(shoes_link *);
VALUE shoes_link_new(VALUE, int, int);
VALUE shoes_link_alloc(VALUE);
VALUE shoes_text_new(VALUE, VALUE, VALUE);
VALUE shoes_text_alloc(VALUE);

void shoes_text_mark(shoes_text *);
void shoes_textblock_mark(shoes_textblock *);
VALUE shoes_textblock_new(VALUE, VALUE, VALUE, VALUE, shoes_transform *);
VALUE shoes_textblock_alloc(VALUE);
VALUE shoes_textblock_get_top(VALUE);
VALUE shoes_textblock_get_left(VALUE);
VALUE shoes_textblock_get_width(VALUE);
VALUE shoes_textblock_get_height(VALUE);
VALUE shoes_textblock_set_cursor(VALUE, VALUE);
VALUE shoes_textblock_get_cursor(VALUE);
VALUE shoes_textblock_draw(VALUE, VALUE, VALUE);
VALUE shoes_textblock_motion(VALUE, int, int, char *);
VALUE shoes_textblock_send_click(VALUE, int, int, int, VALUE *);
void shoes_textblock_send_release(VALUE, int, int, int);

void shoes_http_mark(shoes_http_klass *);
VALUE shoes_http_new(VALUE, VALUE, VALUE);
VALUE shoes_http_alloc(VALUE);
VALUE shoes_http_threaded(VALUE, VALUE, VALUE);
int shoes_message_download(VALUE, void *);
int shoes_catch_message(unsigned int name, VALUE obj, void *data);

VALUE shoes_response_new(VALUE, int);
VALUE shoes_response_body(VALUE);
VALUE shoes_response_headers(VALUE);
VALUE shoes_response_status(VALUE);

VALUE shoes_p(VALUE, VALUE);

extern const double SHOES_PIM2, SHOES_PI, SHOES_RAD2PI, SHOES_HALFPI;

//
// shoes/image.c
//
typedef struct {
  unsigned long status;
  char *cachepath, *filepath, *uripath, *etag;
  char hexdigest[42];
  VALUE slot;
} shoes_image_download_event;

shoes_code shoes_load_imagesize(VALUE, int *, int *);
shoes_cached_image *shoes_cached_image_new(int, int, cairo_surface_t *);
shoes_cached_image *shoes_load_image(VALUE, VALUE);
unsigned char shoes_image_downloaded(shoes_image_download_event *);

#endif
