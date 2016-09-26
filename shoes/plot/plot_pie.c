// pie chart
#include "shoes/plot/plot.h"


void shoes_plot_draw_pie_wedges(cairo_t *cr, shoes_plot *plot)
{
  // first series (x) controls graphical settings. 
  if (plot->seriescnt !=  1)
    return; // we can only use one series 
  int i;
  int top,left,bottom,right;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  VALUE rbsz = rb_ary_entry(plot->sizes, 0);
  int numobs = NUM2INT(rbsz);

  double maxv = 0.0; 
  double minv = 100000000.0; // TODO: 
  double pcts[numobs];
  double values[numobs];
  VALUE rbvals = rb_ary_entry(plot->values, 0);
  
  // sum the values
  for (i = 0; i <numobs; i++) {
    VALUE rbv = rb_ary_entry(rbvals, i);
    double v = NUM2DBL(rbv);
    values[i] = v;
    if (v < minv) minv = v;
    maxv += v;
  }
  double range = maxv - minv;
  // debug
  for (i =0; i < numobs; i++) {
    pcts[i] = values[i] / range;
    printf("pie obs %i: [%f], pct: %f in range %f, %f\n", i, values[i], pcts[i], minv, maxv);
  }

  shoes_plot_set_cairo_default(cr, plot);
}


void shoes_plot_draw_pie_legend(cairo_t *cr, shoes_plot *self_t) {
  // draw xobs values (string) in the legend area in color.
}

// called at draw time. Calls many other functions. 
//      Whole lotta drawing going on
void shoes_plot_pie_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
  shoes_plot_set_cairo_default(cr, self_t);
  shoes_plot_draw_fill(cr, self_t);
  shoes_plot_draw_title(cr, self_t);
  shoes_plot_draw_caption(cr, self_t);
  if (self_t->boundbox) 
    shoes_plot_draw_boundbox(cr, self_t);
  self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
  self_t->graph_y = self_t->title_h + 3;
  self_t->yaxis_offset = 70; // TODO:  run TOTO! run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    shoes_plot_draw_pie_wedges(cr, self_t);
    shoes_plot_draw_pie_legend(cr, self_t);
  }
}
