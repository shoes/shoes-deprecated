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
 * Only plot_*.c includes plot.h so the enums don't blead into Shoes.
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
  COLUMN_CHART,
  SCATTER_CHART,
  PIE_CHART
};

enum {
  NUB_DOT,
  NUB_RECT,
  NUB_CIRCLE,
  NUB_DIAMOND,
  NUM_STAR
};

typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

extern void shoes_plot_line_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_column_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_scatter_draw(cairo_t *, shoes_place *, shoes_plot *);

// plot utility functions (in plot_line.c for now), called at draw time
extern void shoes_plot_set_cairo_default(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_title(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_caption(cairo_t *,shoes_plot *);
extern void shoes_plot_draw_fill(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_boundbox(cairo_t *, shoes_plot *);
//extern void shoes_plot_draw_datapts(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_ticks_and_labels(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_legend(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_tick(cairo_t *, shoes_plot *, int, int, int);
extern void shoes_plot_draw_label(cairo_t *, shoes_plot *, int, int , char*, int);


#endif
