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

#define SETUP_CANVAS() \
   shoes_canvas *canvas; \
   cairo_t *cr; \
   Data_Get_Struct(self, shoes_canvas, canvas); \
   cr = CCR(canvas)
#define SETUP_IMAGE() \
  shoes_place place; \
  GET_STRUCT(image, image); \
  shoes_image_ensure_dup(image); \
  shoes_place_exact(&place, attr, 0, 0); \
  if (NIL_P(attr)) attr = image->attr; \
  else if (!NIL_P(image->attr)) attr = rb_funcall(image->attr, s_merge, 1, attr);

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
// flow struct
//
typedef struct {
    VALUE parent;
    VALUE attr;
    VALUE contents;
} shoes_flow;

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

// ChartSeries struct
typedef struct {
    VALUE maxv;
    VALUE minv;
    VALUE values;
    VALUE name;
    VALUE desc;
    VALUE labels;
    VALUE strokes;
    VALUE point_type;
    VALUE color;
} shoes_chart_series;

//
// Plot struct - It's HUGE!
//
typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    int chart_type;
    int seriescnt;
    VALUE series;
    int auto_grid;
    int boundbox;
    int missing;   // repurposed in pie_charts so beware
    VALUE background;
    VALUE title;
    VALUE legend;
    VALUE caption;
    VALUE default_colors;
    VALUE column_opts;
    void *c_things;
    int x_ticks;   // number of x_axis (which means a vertical grid line draw)
    int y_ticks;   // number of (left side) y axis horizontial grid lines)
    double radar_label_mult; // radius multipler (1.1 ex)
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
    PangoFontDescription *tiny_pfd;
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
//void shoes_canvas_shape_do(shoes_canvas *, double, double, double, double, unsigned char);
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
VALUE shoes_canvas_rgb(int, VALUE *, VALUE);
VALUE shoes_canvas_gray(int, VALUE *, VALUE);
VALUE shoes_canvas_plot(int, VALUE *, VALUE);
VALUE shoes_canvas_chart_series(int, VALUE *, VALUE);
void shoes_canvas_remove_item(VALUE, VALUE, char, char);
VALUE shoes_canvas_push(VALUE);
VALUE shoes_canvas_pop(VALUE);
VALUE shoes_canvas_reset(VALUE);
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
VALUE shoes_canvas_send_click2(VALUE self, int button, int x, int y, VALUE *clicked);
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

typedef VALUE (*ccallfunc)(VALUE);
typedef void (*ccallfunc2)(SHOES_CONTROL_REF);

void shoes_canvas_ccall(VALUE, ccallfunc, ccallfunc2, unsigned char);

VALUE shoes_add_ele(shoes_canvas *canvas, VALUE ele);

SHOES_SLOT_OS *shoes_slot_alloc(shoes_canvas *, SHOES_SLOT_OS *, int);
VALUE shoes_slot_new(VALUE, VALUE, VALUE);
VALUE shoes_flow_new(VALUE, VALUE);
VALUE shoes_stack_new(VALUE, VALUE);
VALUE shoes_mask_new(VALUE, VALUE);
VALUE shoes_widget_new(VALUE, VALUE, VALUE);

VALUE shoes_chart_series_new(int, VALUE *, VALUE);
VALUE shoes_chart_series_alloc(VALUE);
VALUE shoes_chart_series_values(VALUE);
VALUE shoes_chart_series_labels(VALUE);
VALUE shoes_chart_series_min(VALUE);
VALUE shoes_chart_series_min_set(VALUE, VALUE);
VALUE shoes_chart_series_max(VALUE);
VALUE shoes_chart_series_max_set(VALUE, VALUE);
VALUE shoes_chart_series_name(VALUE);
VALUE shoes_chart_series_desc(VALUE);
VALUE shoes_chart_series_desc_set(VALUE, VALUE);
VALUE shoes_chart_series_color(VALUE);
VALUE shoes_chart_series_color_set(VALUE, VALUE);
VALUE shoes_chart_series_strokewidth(VALUE);
VALUE shoes_chart_series_strokewidth_set(VALUE, VALUE);
VALUE shoes_chart_series_points(VALUE);
VALUE shoes_chart_series_points_set(VALUE, VALUE);

VALUE shoes_chart_series_get(VALUE, VALUE);
VALUE shoes_chart_series_set(VALUE, VALUE, VALUE);

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

// Canvas needs cSvg to create snapshots and send events
extern VALUE cSvg;

extern VALUE shoes_svg_motion(VALUE, int, int, char *);
extern VALUE shoes_svg_send_click(VALUE, int, int, int);
extern void shoes_svg_send_release(VALUE, int, int, int);

// TODO: to be removed during refactoring
extern VALUE shoes_text_new(VALUE klass, VALUE texts, VALUE attr);

extern VALUE cNative, cPlot, cRadio;


// TODO: MARKUP_* macro belongs to either TextBlock or Text?
#define MARKUP_BLOCK(klass) \
  text = shoes_textblock_new(klass, msgs, attr, self, canvas->st); \
  shoes_add_ele(canvas, text)

#define MARKUP_INLINE(klass) \
  text = shoes_text_new(klass, msgs, attr)
  
#define MARKUP_DEF(mname, fname, klass) \
  VALUE \
  shoes_canvas_##mname(int argc, VALUE *argv, VALUE self) \
  { \
    long i; \
    VALUE msgs, attr, text; \
    SETUP_CANVAS(); \
    msgs = rb_ary_new(); \
    attr = Qnil; \
    for (i = 0; i < argc; i++) \
    { \
      if (rb_obj_is_kind_of(argv[i], rb_cHash)) \
        attr = argv[i]; \
      else \
        rb_ary_push(msgs, argv[i]); \
    } \
    MARKUP_##fname(klass); \
    return text; \
  }

#define SETUP_SHAPE() \
  shoes_canvas *canvas = NULL; \
  VALUE c = shoes_find_canvas(self); \
  Data_Get_Struct(c, shoes_canvas, canvas)
  
#endif
