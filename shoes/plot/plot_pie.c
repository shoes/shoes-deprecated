// pie chart
#include "shoes/plot/plot.h"

/* borrows heavily from this python code
 * https://bitbucket.org/lgs/pycha/src/e3e270a0e7ae4896052d4cc34fe7f1e532ece914/pycha/pie.py?at=default&fileviewer=file-view-default
 * because its kind of Ruby/Shoes friendly and short.
*/
// Forward declares in this file
VALUE shoes_plot_pie_color(cairo_t *, shoes_plot *, int);

void shoes_plot_draw_pie_chart(cairo_t *cr, shoes_plot *plot)
{
  // first series (x) controls graphical settings. 
  if (plot->seriescnt !=  1)
    return; // we can only use one series 
  int i;
  int top,left,bottom,right, height, width;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  width = right - left;
  height = bottom - top; 
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
#if 0
  // debug
  for (i =0; i < numobs; i++) {
    pcts[i] = values[i] / range;
    printf("pie obs %i: [%f], pct: %f in range %f, %f\n", i, values[i], pcts[i], minv, maxv);
  }
#endif
  // much is "unclean" and "ignorant". Help!
  int centerx = left + roundl(width * 0.5);
  int centery = top + roundl(height * 0.5);
  double radius = min(width, height) / 2.2; // TODO: emperical or heuristic?

#if 1 // Or attr(drop_shadow) 
  // draw something circular which might be the drop shadow of the pie.
  // Which we probably don't need since Shoes doesn't have a drop shadow option
  // Pixel counting still sucks
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 0.15);
  cairo_new_path(cr);
  cairo_move_to(cr, centerx, centery);
  cairo_arc(cr, centerx +1, centery + 2, radius +1, 0, SHOES_PIM2);
  cairo_line_to(cr, centerx, centery);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_restore(cr);
#endif    
  cairo_save(cr);
  for (i = 0; i < numobs; i++) {
    VALUE wedge_color = shoes_plot_pie_color(cr, plot, i);
    
  }
  cairo_restore(cr);
  shoes_plot_set_cairo_default(cr, plot);
}

VALUE shoes_plot_pie_color(cairo_t *cr, shoes_plot *plot, int ser)
{
}

void shoes_plot_draw_pie_legend(cairo_t *cr, shoes_plot *self_t) {
  // draw xobs values (string) in the legend area in color, on the right 
  // hand side? left hand side?  Legend Area?
}

// called at draw time. Calls many other functions. 
//      Whole lotta drawing going on
void shoes_plot_pie_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
  shoes_plot_set_cairo_default(cr, self_t);
  shoes_plot_draw_fill(cr, self_t);
  shoes_plot_draw_title(cr, self_t);
  shoes_plot_draw_caption(cr, self_t);
  //if (self_t->boundbox) 
  //  shoes_plot_draw_boundbox(cr, self_t); // not helpful for pie charts. IMHO.
  self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
  self_t->graph_y = self_t->title_h + 3;
  self_t->yaxis_offset = 70; // TODO:  run TOTO! run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    shoes_plot_draw_pie_chart(cr, self_t);
    shoes_plot_draw_pie_legend(cr, self_t);
  }
}
