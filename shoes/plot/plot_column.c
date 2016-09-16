// column chart

/*
 * Note: this draws very differently from plot_line.c - assumes a small
 * number of observations and draws across the x
 */

#include "shoes/plot/plot.h"

void shoes_plot_draw_column_top(cairo_t *cr, int x, int y)
{
}

void shoes_plot_draw_columns(cairo_t *cr, shoes_plot *plot)
{
  int i, num_series;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h;
  int width = right - left;  
  int height = bottom - top;
  // need to compute x advance based on number of series and stroke width
  int colsw = 0;       // combined stroke width
  int numobs = 0;      // aka series.size(s)
  num_series = plot->seriescnt;
  int strokesw[num_series];
  for (i = 0; i < plot->seriescnt; i++) {
    VALUE rbstroke = rb_ary_entry(plot->strokes, i);
    int sw = NUM2INT(rbstroke);
    if (sw < 4) sw = 4;
    strokesw[i] = sw;
    colsw += sw;
    VALUE rbsize = rb_ary_entry(plot->sizes, i);
    numobs = max(numobs, NUM2INT(rbsize));
  }
  int ncolsw = (width - (2 * colsw)) / numobs; // TODO: float and round? 
  int colpos_xoffset = ncolsw / 2;
  printf("ncolsw: %i offset %i\n", ncolsw, colpos_xoffset);
  int xpos = colpos_xoffset;
  for (i = 0; i < numobs; i++) {
    // move to center pos of o
    printf("move to obvs %i, at x:%i\n", i, xpos);
    xpos += colpos_xoffset;
    // draw x label there.
    // move left or right from that sopt and draw the bar
  }
  for (i = 0; i < plot->seriescnt; i++) {
    VALUE rbvalues = rb_ary_entry(plot->values, i);
    VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    VALUE rbminv = rb_ary_entry(plot->minvs, i);
    VALUE rbstroke = rb_ary_entry(plot->strokes, i);
    VALUE rbnubs = rb_ary_entry(plot->nubs, i);
    VALUE shcolor = rb_ary_entry(plot->color, i);
    shoes_color *color;
    Data_Get_Struct(shcolor, shoes_color, color);
    double maximum = NUM2DBL(rbmaxv);
    double minimum = NUM2DBL(rbminv);

    cairo_set_line_width(cr, strokesw[i]);
    // Shoes: Remember - we use ints for x, y, w, h and for drawing lines and points
    int range = plot->end_idx - plot->beg_idx; // zooming adj
    float vScale = height / (maximum - minimum);
    float hScale = width / (double) (range - 1);
    int nubs = (width / range > 10) ? RTEST(rbnubs) : 0; 
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
          rbdp = rbminv;
          brk = 0;
        }
      }
      double v = NUM2DBL(rbdp);
      long x = roundl(j * hScale);
      long y = height - roundl((v - minimum) *vScale);
      x += left;
      y += top;
      {
        cairo_move_to(cr, x, bottom);
        cairo_line_to(cr, x, y);
        if (nubs) 
          shoes_plot_draw_column_top(cr, x, y);
      }
      brk = 0;
    }
    cairo_stroke(cr);
    cairo_set_line_width(cr, 1.0); // reset between series
  } // end of drawing one series
  // tell cairo to draw all lines (and points)
  cairo_stroke(cr); 
  // set color back to dark gray and stroke to 1
  cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0);
  cairo_set_line_width(cr, 1.0);  
}

void shoes_plot_column_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
  shoes_plot_draw_fill(cr, self_t);
  shoes_plot_draw_title(cr, self_t);
  shoes_plot_draw_caption(cr, self_t);
  self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
  self_t->graph_y = self_t->title_h + 3;
  self_t->yaxis_offset = 50; // TODO:  run TOTO, run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    // draw  box, ticks and x,y labels.
    shoes_plot_draw_adornments(cr, self_t);
    // draw data
    shoes_plot_draw_columns(cr, self_t);
  }
}
