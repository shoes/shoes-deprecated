// column chart

/*
 * Note: this draws very differently from plot_line.c - assumes a small
 * number of observations and draws across (left to right)
 */

#include "shoes/plot/plot.h"

void shoes_plot_draw_column_top(cairo_t *cr, int x, int y)
{
}

void shoes_plot_column_xaxis(cairo_t *cr, shoes_plot *plot, int x, VALUE obs)
{
  char *rawstr = RSTRING_PTR(obs);
  int y;
  y = plot->graph_h;
  cairo_stroke(cr); // doesn't help, doesn't hurt
  //cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0); // Why doesn't this work?
  cairo_set_line_width(cr, 1.0);  
  shoes_plot_draw_label(cr, plot, x, y, rawstr, BELOW);
  cairo_stroke(cr);
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
  int colsw = 0;       // combined stroke width for one set of bars
  num_series = plot->seriescnt;
  int strokesw[num_series];
  double maximums[num_series];
  double minimums[num_series];
  double vScales[num_series];
  shoes_color *colors[num_series];
  int nubs[num_series];
  VALUE values[num_series];
  int range = plot->end_idx - plot->beg_idx; // zooming adj
  VALUE rbobs = rb_ary_entry(plot->xobs, 0);
  
  for (i = 0; i < plot->seriescnt; i++) {
    values[i] = rb_ary_entry(plot->values, i);
    VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    VALUE rbminv = rb_ary_entry(plot->minvs, i);
    maximums[i] = NUM2DBL(rbmaxv);
    minimums[i] = NUM2DBL(rbminv);
    VALUE rbnubs = rb_ary_entry(plot->nubs, i);
    nubs[i] = (width / range > 10) ? RTEST(rbnubs) : 0; 
    VALUE rbcolor = rb_ary_entry(plot->color, i);
    VALUE rbstroke = rb_ary_entry(plot->strokes, i);
    int sw = NUM2INT(rbstroke);
    if (sw < 4) sw = 4;
    strokesw[i] = sw;
    colsw += sw;
    vScales[i] = (height / (maximums[i] - minimums[i]));
    shoes_color *color;
    Data_Get_Struct(rbcolor, shoes_color, colors[i]);
  }
  int ncolsw = width / (range) ; // 
  int xinset = left + (ncolsw / 2); // start inside the box, half a width
  int xpos = xinset; 
  //printf("ncolsw: w: %i, advw: %i, start inset %i\n", width, ncolsw, xpos);
  for (i = plot->beg_idx; i < plot->end_idx; i++) {
    int j;
    for (j = 0; j < plot->seriescnt; j++) {
      VALUE rbdp  = rb_ary_entry(values[j], i + plot->beg_idx);
      if (NIL_P(rbdp)) {
          printf("skipping nil at %i\n", i + plot->beg_idx);
          continue;
      }
      double v = NUM2DBL(rbdp);
      cairo_set_line_width(cr, strokesw[j]);
      cairo_set_source_rgba(cr, colors[j]->r / 255.0, colors[j]->g / 255.0,
        colors[j]->b / 255.0, colors[j]->a / 255.0); 
       
      long y = height - roundl((v - minimums[j]) * vScales[j]);
      //printf("move i: %i, j: %i, x: %i, v: %f -> y: %i,%i \n", i, j, xpos, v, y, y+top);
      y += top;
      cairo_move_to(cr, xpos, bottom);
      cairo_line_to(cr, xpos, y);
      cairo_stroke(cr);
      xpos += strokesw[j];
    }
    VALUE obs = rb_ary_entry(rbobs, i + plot->beg_idx);
    shoes_plot_column_xaxis(cr, plot, xpos-(colsw / 2), obs);
    xpos = (ncolsw * (i + 1)) + xinset;
  }
  // tell cairo to draw all lines (and points) not already drawn.
  cairo_stroke(cr); 
  // set color back to dark gray and stroke to 1
  cairo_set_source_rgba(cr, 0.9, 0.9, 0.9, 1.0);
  cairo_set_line_width(cr, 1.0);  
}

// called by the draw event - draw everything. 
void shoes_plot_column_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
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

    shoes_plot_draw_ticks_and_labels(cr, self_t);
    shoes_plot_draw_legend(cr, self_t);
    // shoes_plot_draw_adornments(cr, self_t);
    // draw data
    shoes_plot_draw_columns(cr, self_t);
  }
}
