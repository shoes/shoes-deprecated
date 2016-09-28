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
  double range = maxv - minv; // needed? 
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
  // compute some trig stuff for each wedge. Called slices in Pycha
  double startAngle[numobs];
  double endAngle[numobs];
  double fraction = 0.0;
  double angle = 0.0;
  for (i = 0; i < numobs; i++) {
    angle += fraction;
    double v = values[i];
    fraction = v / maxv;
    startAngle[i] = 2 * angle * SHOES_PI;
    endAngle[i] = 2 * (angle + fraction) * SHOES_PI;

  }
#if 0 // Or attr(drop_shadow) 
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
  // problems below, somewhere
  for (i = 0; i < numobs; i++) {
    //TODO: !!! if (abs(startAngle[i] - endAngle[i]) > 0.001) { // bigEnough?
    {
      cairo_save(cr);
      VALUE wedge_color = shoes_plot_pie_color(cr, plot, i);
      shoes_color *color;
      Data_Get_Struct(wedge_color, shoes_color, color);
      cairo_set_source_rgba(cr, color->r / 255.0, color->b / 255.0, 
          color->g / 255.0, color->a / 255.0);
      printf("pie color for %i: r:%i g:%i b:%i a:%i\n", i, color->r, color->g,
          color->b, color->a);
      cairo_new_path(cr);
      cairo_move_to(cr, centerx, centery);
      cairo_arc(cr, centerx, centery, radius, -endAngle[i], -startAngle[i]);
      cairo_close_path(cr);
      cairo_fill(cr);
      cairo_restore(cr);
    }
  }
  shoes_plot_set_cairo_default(cr, plot);
}
;
// Yes this could be precomputed. It isn't. TODO:
VALUE shoes_plot_pie_color(cairo_t *cr, shoes_plot *plot, int ser)
{
  VALUE color_wrapped = Qnil;
      switch (ser) {
      case 0: 
        color_wrapped = shoes_hash_get(cColors, rb_intern("blue"));
        break;
      case 1:
        color_wrapped = shoes_hash_get(cColors, rb_intern("red"));
        break;
      case 2:
        color_wrapped = shoes_hash_get(cColors, rb_intern("green"));
        break;
      case 3:
        color_wrapped = shoes_hash_get(cColors, rb_intern("coral"));
        break;
      case 4:
        color_wrapped = shoes_hash_get(cColors, rb_intern("purple"));
        break;
      case 5:
        color_wrapped = shoes_hash_get(cColors, rb_intern("orange"));
        break;
      case 6:
        color_wrapped = shoes_hash_get(cColors, rb_intern("aqua"));
        break;
      case 7:
        color_wrapped = shoes_hash_get(cColors, rb_intern("brown"));
        break;
      case 8:
        color_wrapped = shoes_hash_get(cColors, rb_intern("darkolivegreen"));
        break;
      case 9:
        color_wrapped = shoes_hash_get(cColors, rb_intern("hotpink"));
        break;
      case 10:
        color_wrapped = shoes_hash_get(cColors, rb_intern("lightskyblue"));
        break;
      case 11:
        color_wrapped = shoes_hash_get(cColors, rb_intern("greenyellow"));
        break;
      default:
        // too many wedges. 
        color_wrapped = shoes_hash_get(cColors, rb_intern("gray"));
      }
    return color_wrapped;
}

void shoes_plot_draw_pie_legend(cairo_t *cr, shoes_plot *self_t) {
  /* draw xobs values (string) in the legend area in color, on the right 
   * hand side. need to compute the max height and width of all strings
   * attempt to get the most data on the chart.  Ugly. TODO: it would
   * be nice to have shoes class/struct. 
  */
  int i; 
  if (self_t->seriescnt != 1) return;
  VALUE rbsz = rb_ary_entry(self_t->sizes, 0);
  VALUE rbobs = rb_ary_entry(self_t->xobs, 0);
  int numstrs = NUM2INT(rbsz);
  PangoLayout *layouts[numstrs];
  char *strary[numstrs];
  char strh[numstrs];
  int boxh = 0, boxw = 0;
  // compute layouts for each string. Don't forget to g_unref them!
  for (i = 0; i < numstrs; i++) {
    strary[i] = RSTRING_PTR(rb_ary_entry(rbobs, i));
    layouts[i] = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layouts[i], self_t->caption_pfd);
    pango_layout_set_text (layouts[i], strary[i], -1);
    PangoRectangle logical;
    pango_layout_get_pixel_extents (layouts[i], NULL, &logical);
    strh[i] = logical.height;
    boxh += logical.height;
    boxw = max(boxw, logical.width);
  }
  int box_x = (self_t->place.iw - boxw) - 5; // everything we can get on the right
  int box_y = self_t->graph_y + 2;
  printf("legend box x: %i, y: %i, h: %i, w: %i\n", box_x, box_y, boxh, boxw);
  // Draw (layout)
  for (i = 0; i < numstrs; i++) {
    cairo_move_to(cr, box_x, box_y);
    shoes_color *color;
    VALUE rbcolor = shoes_plot_pie_color(cr, self_t, i);
    Data_Get_Struct(rbcolor, shoes_color, color);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
       color->b / 255.0, color->a / 255.0);
    pango_cairo_show_layout(cr, layouts[i]);
    box_y += strh[i];
    g_object_unref(layouts[i]);
  }
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
