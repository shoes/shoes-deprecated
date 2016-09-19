// line chart
#include "shoes/plot/plot.h"

// forward declares in this file:
void shoes_plot_draw_scatter_nub(cairo_t *, int, int, int);


void shoes_plot_draw_scatter_pts(cairo_t *cr, shoes_plot *plot)
{
  int i, num_series;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h;    
  for (i = 0; i < plot->seriescnt; i++) {
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
    int nubs = (width / range > 10) ? RTEST(rbnubs) : 0;  // could be done if asked
  
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
        shoes_plot_scatter_nub(cr, NUB_DOT, x, y);
    }
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1.0); // reset between series
  } // end of drawing one series
  // tell cairo to draw all lines (and points)
  cairo_stroke(cr); 
  shoes_plot_set_cairo_default(cr, plot);
}

// * fun below  - port to plot_line.c ?
void shoes_plot_scatter_nub(cairo_t *cr, int nubt, int x, int y)
{
  switch (nubt) {
    case NUB_DOT:
    default: {
      int sz = 2; 
      cairo_move_to(cr, x - sz, y - sz);
      cairo_line_to(cr, x + sz, y - sz);
      cairo_move_to(cr, x - sz, y - sz);
      cairo_line_to(cr, x - sz, y + sz);
      cairo_move_to(cr, x + sz, y + sz);
      cairo_line_to(cr, x + sz, y - sz);
      cairo_move_to(cr, x + sz, y + sz);
      cairo_line_to(cr, x - sz, y + sz);
      cairo_move_to(cr, x, y); // back to center point.
    }
  }
}

// called at draw time.
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
    //shoes_plot_draw_adornments(cr, self_t);
    shoes_plot_draw_ticks_and_labels(cr, self_t);
    shoes_plot_draw_legend(cr, self_t);    // draw data
    shoes_plot_draw_scatter_pts(cr, self_t);
  }
}
