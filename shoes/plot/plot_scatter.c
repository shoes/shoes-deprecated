// scatter chart
#include "shoes/plot/plot.h"

// forward declares in this file:
void shoes_plot_scatter_nub(cairo_t *, double, double, int);


void shoes_plot_draw_scatter_pts(cairo_t *cr, shoes_plot *plot)
{
  // first series (x) controls graphical settings. 
  if (plot->seriescnt !=  2)
    return; // we can only use two series 
  int i, num_series;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  VALUE rbxary = rb_ary_entry(plot->values, 0);
  VALUE rbyary = rb_ary_entry(plot->values, 1);
  
  VALUE rbxmax = rb_ary_entry(plot->maxvs, 0);
  VALUE rbymax = rb_ary_entry(plot->maxvs, 1);
  VALUE rbxmin = rb_ary_entry(plot->minvs, 0);
  VALUE rbymin = rb_ary_entry(plot->minvs, 1);
  double xmax = NUM2DBL(rbxmax);
  double ymax = NUM2DBL(rbymax);
  double xmin = NUM2DBL(rbxmin);
  double ymin = NUM2DBL(rbymin);
  VALUE rbnubs = rb_ary_entry(plot->nubs, 0);
  VALUE shcolor = rb_ary_entry(plot->color, 0);
  VALUE rbstroke = rb_ary_entry(plot->strokes, 0);
  int strokew = NUM2INT(rbstroke);
  if (strokew < 1) strokew = 1;
  shoes_color *color;
  Data_Get_Struct(shcolor, shoes_color, color);
  // scale x and y to 
  //printf("scale x to %f, %f\n", xmin, xmax);
  //printf("scale y to %f, %f\n", ymin, ymax);
  VALUE rbobs = rb_ary_entry(plot->sizes, 0);
  int obvs = NUM2INT(rbobs);
  double yScale;
  double xScale;
  for (i = 0; i < obvs; i++) {
    double xval, yval;
    xval = NUM2DBL(rb_ary_entry(rbxary, i));
    yval = NUM2DBL(rb_ary_entry(rbyary, i));
    //printf("scatter x: %f, y: %f\n", xval, yval);
  }
  return;   
  for (i = 0; i < plot->seriescnt; i + 2) {
    VALUE rbvalues = rb_ary_entry(plot->values, i);
    VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    VALUE rbminv = rb_ary_entry(plot->minvs, i);
    VALUE rbsize = rb_ary_entry(plot->sizes, i);
    VALUE rbstroke = rb_ary_entry(plot->strokes, i);
    VALUE rbnubs = rb_ary_entry(plot->nubs, i);
    VALUE shcolor = rb_ary_entry(plot->color, i);
    shoes_color *color;
    Data_Get_Struct(shcolor, shoes_color, color);
    double maximum = NUM2DBL(rbmaxv);
    double minimum = NUM2DBL(rbminv);
    int strokew = NUM2INT(rbstroke);
    if (strokew < 1) strokew = 1;
    cairo_set_line_width(cr, strokew);
    // Shoes: Remember - we use ints for x, y, w, h and for drawing lines and points
    int height = bottom - top;
    int width = right - left; 
    int range = plot->end_idx - plot->beg_idx; // zooming adj
    float vScale = height / (maximum - minimum);
    float hScale = width / (double) (range - 1);
    int nubs = (width / range > 10) ? NUM2INT(rbnubs) : 0;  
  
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
       color->b / 255.0, color->a / 255.0); 

    int j;
    int brk = 0; // for missing value control
    for (j = 0; j < range; j++) {
      VALUE rbdp = rb_ary_entry(rbvalues, j + plot->beg_idx);
      if (NIL_P(rbdp)) {
        if (plot->missing == MISSING_MIN) {
          rbdp = rbminv;
        } else if (plot->missing == MISSING_MAX) {
          rbdp = rbmaxv;
        } else {
          brk = 1;
          continue;
        }
      }
      double v = NUM2DBL(rbdp);
      long x = roundl(j * hScale);
      long y = height - roundl((v - minimum) *vScale);
      x += left;
      y += top;
      //printf("draw i: %i, x: %i, y: %i %f \n", j, (int) x, (int) y, hScale);
      
      cairo_move_to(cr, x, y);
      cairo_line_to(cr, x, y);
      
      if (nubs) 
        shoes_plot_draw_nub(cr, plot, x, y, nubs, strokew + 2);
    }
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1.0); // reset between series
  } // end of drawing one series
  // tell cairo to draw all lines (and points)
  cairo_stroke(cr); 
  shoes_plot_set_cairo_default(cr, plot);
}

static void shoes_plot_scatter_ticks_and_labels(cairo_t *cr, shoes_plot *plot)
{
}

static void shoes_plot_scatter_adornments(cairo_t *cr, shoes_plot *plot)
{
}

// called at draw time. Call many other functions
void shoes_plot_scatter_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
  shoes_plot_set_cairo_default(cr, self_t);
  shoes_plot_draw_fill(cr, self_t);
  shoes_plot_draw_title(cr, self_t);
  shoes_plot_draw_caption(cr, self_t);
  if (self_t->boundbox) 
    shoes_plot_draw_boundbox(cr, self_t);
  self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
  self_t->graph_y = self_t->title_h + 3;
  self_t->yaxis_offset = 50; // TODO:  run TOTO, run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    // draw  box, ticks and x,y labels.
    shoes_plot_scatter_adornments(cr, self_t);
    shoes_plot_scatter_ticks_and_labels(cr, self_t);
    shoes_plot_draw_legend(cr, self_t); 
    shoes_plot_draw_scatter_pts(cr, self_t);
  }
}
