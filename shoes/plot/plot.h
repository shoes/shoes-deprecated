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
  PIE_CHART
};
typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

extern void shoes_plot_line_draw(cairo_t *, shoes_place *, shoes_plot *);
extern void shoes_plot_column_draw(cairo_t *, shoes_place *, shoes_plot *);

// plot utility functions (in plot_line.c for now), called at draw time

extern void shoes_plot_draw_title(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_caption(cairo_t *,shoes_plot *);
extern void shoes_plot_draw_fill(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_adornments(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_datapts(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_ticks_and_labels(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_legend(cairo_t *, shoes_plot *);
extern void shoes_plot_draw_tick(cairo_t *, shoes_plot *, int, int, int);
extern void shoes_plot_draw_label(cairo_t *, shoes_plot *, int, int , char*, int);
//extern void shoes_plot_draw_nub(cairo_t *, int, int);
//extern void shoes_plot_draw_columns(cairo_t *, shoes_plot *);
//extern int shoes_plot_save_vector(VALUE, char *, char *);
//extern void shoes_plot_draw_everything(cairo_t *, shoes_place *, shoes_plot *);

#endif
