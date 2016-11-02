/* 
 * plot.h
*/
#ifndef PLOT_H
#define PLOT_H
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

/* 
 * NOTE: functions that changes the cairo state (aka ontext, cairo_t) for
 * color or line width and the like should call shoes_plot_set_cairo_default()
 * in plot_util.c to restore the default Shoes plot drawing state.
 * 
 * Only plot_*.c includes plot.h so the enums don't bleed into Shoes.
*/

enum {
  VERTICALLY,
  HORIZONTALLY 
};

enum {
  LEFT,
  BELOW,
  RIGHT
};
// missing value or observation handling
enum {
  MISSING_SKIP,
  MISSING_MIN,
  MISSING_MAX
};

// chart type - line is default
enum  {
  LINE_CHART,
  TIMESERIES_CHART,
  COLUMN_CHART,
  SCATTER_CHART,
  PIE_CHART,
  RADAR_CHART
};

enum {
  NUB_NONE,
  NUB_DOT,     //filled circle
  NUB_CIRCLE,  //unfilled circle
  NUB_BOX,     //filled rect
  NUB_RECT,    //unfilled rect
};

// Pie charts are sufficently complex that we need some structs for the 
// internal stuff. Probably should be done for the other types to
typedef struct {
  double value; 
  double startAngle;
  double endAngle;
  shoes_color *color;
  char *label; 
  int lh; // label height and width and placement
  int lw;
  int lx, ly; 
  PangoLayout *layout; 
} pie_slice_t;

typedef struct {
  int percent;  // true when display % instead of value
  double radius;
  int centerx;
  int centery;
  int count;
  int top, left, bottom, right, height, width;
  double maxv;
  double minv;
  pie_slice_t *slices; // treated as an array because it is.
} pie_chart_t;

// Radar charts are even more complex;
typedef struct {
  double value; 
  double startAngle;
  double endAngle;
  double maxv; // ugly
  double minv; // twice as ugly
  shoes_color *color;
  char *label; 
  int lh; // label height and width and placement
  int lw;
  int lx, ly; 
  PangoLayout *layout; 
} radar_pole_t;

typedef struct {
  int percent;  // true when display % instead of value
  double radius;
  double rotation;
  double additive_angle;
  int centerx;
  int centery;
  int count;
  int top, left, bottom, right, height, width;
  double maxv;
  double minv;
  double *colmax;
  double *colmin;
  radar_pole_t *slices; // treated as an array because it is.
} radar_chart_t;

typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

extern void shoes_plot_line_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_column_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_scatter_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_pie_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_pie_init(shoes_plot *);
extern void shoes_plot_pie_dealloc(shoes_plot *);
extern void shoes_chart_series_Cinit(shoes_chart_series *, VALUE, VALUE,
    VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
// plot utility functions (in plot_util.c)
extern void shoes_plot_set_cairo_default(cairo_t *, shoes_plot *);
extern void shoes_plot_util_default_colors(shoes_plot *);
extern void shoes_plot_draw_title(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_caption(cairo_t *,shoes_plot *);
extern void shoes_plot_draw_fill(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_boundbox(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_ticks_and_labels(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_legend(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_tick(cairo_t *, shoes_plot *, int, int, int);
extern void shoes_plot_draw_label(cairo_t *, shoes_plot *, int, int , char*, int);
extern void shoes_plot_draw_nub(cairo_t *, shoes_plot *, double, double, int, int);



#endif
