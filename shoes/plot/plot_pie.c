// pie chart
#include "shoes/plot/plot.h"

// REMEMBER - there is only one data series in a PIE CHART. 

/* borrows heavily from this python code
 * https://bitbucket.org/lgs/pycha/src/e3e270a0e7ae4896052d4cc34fe7f1e532ece914/pycha/pie.py?at=default&fileviewer=file-view-default
 * because its kind of Ruby/Shoes friendly and short-ish.
*/
// Forward declares in this file

// called when the data series is added to the chart.
// Trying very hard to not pollute Shoes C name space and h files with plot stuff
void shoes_plot_pie_init(shoes_plot *plot) {
  pie_chart_t *piechart = malloc(sizeof(pie_chart_t));
  plot->c_things = (void *)piechart;
  VALUE cs = rb_ary_entry(plot->series, 0);
  shoes_chart_series *ser;
  Data_Get_Struct(cs, shoes_chart_series, ser);
  int numobs = RARRAY_LEN(ser->values);
  piechart->count = numobs;
  pie_slice_t *slices = (pie_slice_t *)malloc(sizeof(pie_slice_t) * numobs);
  piechart->slices = slices;
  int i;
  piechart->maxv = 0.0; 
  piechart->minv = 100000000.0; // TODO: use a max double constant
  
  // sum the values
  for (i = 0; i <numobs; i++) {
    pie_slice_t *slice = &slices[i];
    VALUE rbv = rb_ary_entry(ser->values, i);
    double v = NUM2DBL(rbv);
    slice->value = v;
    if (v < piechart->minv) piechart->minv = v;
    piechart->maxv += v;
  }
  double fraction = 0.0;
  double angle = 0.0;
  for (i = 0; i < numobs; i++) {
    pie_slice_t *slice = &slices[i];
    angle += fraction;
    double v = slice->value;
    fraction = v / piechart->maxv;
    slice->startAngle = 2 * angle * SHOES_PI;
    slice->endAngle = 2 * (angle + fraction) * SHOES_PI;
    //VALUE wedge_color = shoes_plot_pie_color(i);
    VALUE wedge_color = rb_ary_entry(plot->default_colors, i);
    Data_Get_Struct(wedge_color, shoes_color, slice->color);
  }
}

// called when it needs to go away
void shoes_plot_pie_dealloc(shoes_plot *plot) {
  if (plot->c_things) {
   pie_chart_t *piechart = (pie_chart_t *) plot->c_things;
   free(piechart->slices);
   free(piechart);
  }
}

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
  pie_chart_t *chart = (pie_chart_t *) plot->c_things;

  chart->centerx = left + roundl(width * 0.5);
  chart->centery = top + roundl(height * 0.5);
  chart->radius = min(width / 2.0, height / 2.0);
  chart->top = top; chart->left = left; chart->bottom = bottom;
  chart->right = right; chart->width = width; chart->height = height;
  chart->percent = plot->missing; // Bait `n switch
  
#if 0 // OR attr(drop_shadow) 
  // draw something circular which might be the drop shadow of the pie.
  // Which we probably don't need since Shoes doesn't have a drop shadow option
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0, 0, 0, 0.15);
  cairo_new_path(cr);
  cairo_move_to(cr, centerx, centery);
  cairo_arc(cr, centerx + 1, centery + 2, chart->radius + 1, 0, SHOES_PIM2);
  cairo_line_to(cr, centerx, centery);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_restore(cr);
#endif    

  for (i = 0; i < chart->count; i++) {
    pie_slice_t *slice = &chart->slices[i];
    if (fabs(slice->startAngle - slice->endAngle) > 0.001) { // bigEnough?
      shoes_color *color = slice->color;
      cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0, 
          color->b / 255.0, color->a / 255.0);
      //printf("pie color for %i: r:%i g:%i b:%i a:%i\n", i, color->r, color->g,
      //    color->b, color->a);
      cairo_new_path(cr);
      cairo_move_to(cr, chart->centerx, chart->centery);
      cairo_arc(cr, chart->centerx, chart->centery, chart->radius, -(slice->endAngle), -(slice->startAngle));
      cairo_close_path(cr);
      cairo_fill(cr);
    }
  }
  shoes_plot_set_cairo_default(cr, plot);
}

// just draws a box
void shoes_plot_pie_legend_box(cairo_t *cr, shoes_plot *self_t, 
  int t, int l, int b, int r)
{
  shoes_plot_set_cairo_default(cr, self_t);
  cairo_move_to(cr, l, t);
  cairo_line_to(cr, r, t);  // across top
  cairo_line_to(cr, r, b);  // down right side
  cairo_line_to(cr, l, b);  // across bottom
  cairo_line_to(cr, l, t);  // up left
  cairo_stroke(cr);
}

void shoes_plot_draw_pie_legend(cairo_t *cr, shoes_plot *self_t) {
  /* draw labels (strings) in the legend area in color, on the right 
   * hand side. Need to compute the max height and width of all strings
   * attempt to get the most data on the chart.  Ugly. TODO: it would
   * be nice to have shoes class/struct. 
  */
  int i; 
  if (self_t->seriescnt != 1) return;
  VALUE cs = rb_ary_entry(self_t->series, 0);
  shoes_chart_series *ser;
  Data_Get_Struct(cs, shoes_chart_series, ser);
  int numstrs = RARRAY_LEN(ser->labels);
  VALUE rbobs = ser->labels;
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
  int box_y = self_t->graph_y + 1;
  int box_t = box_y - 2;
  int box_l = box_x - 3;
  int box_r = self_t->place.iw - 1;
  int box_b = boxh + box_y + 1;
  // Draw strings using layout
  for (i = 0; i < numstrs; i++) {
    cairo_move_to(cr, box_x, box_y);
    shoes_color *color;
    VALUE rbcolor = rb_ary_entry(self_t->default_colors, i);
    Data_Get_Struct(rbcolor, shoes_color, color);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
       color->b / 255.0, color->a / 255.0);
    pango_cairo_show_layout(cr, layouts[i]);
    box_y += strh[i];
    g_object_unref(layouts[i]);
  }
  // Draw a box around the legend (auto grid repurpose?)
  if (self_t->auto_grid) {
    //printf("legend box l: %i, t: %i, b: %i, r: %i\n", box_l, box_t, box_b, box_r);
    shoes_plot_pie_legend_box(cr, self_t, box_t, box_l, box_b, box_r);
  }
}

/* 
 * NOTE: here, "ticks" are values (or %?) drawn around the slices.  
 * Expect confusion and many helper functions.
 * This called after drawing the wedges in a shoes draw event so we know
 * we actually have a cairo_t that is real and the slices are OK. 
 * We depend on that. 
*/
double shoes_plot_pie_getNormalisedAngle(pie_slice_t *self) {
  double normalisedAngle = (self->startAngle + self->endAngle) / 2;
  if (normalisedAngle > SHOES_PI * 2)
    normalisedAngle -= SHOES_PI * 2;
  else if (normalisedAngle < 0)
    normalisedAngle += SHOES_PI * 2;

  return normalisedAngle;
}

shoes_plot_pie_tick_position(cairo_t *cr, pie_chart_t * chart, pie_slice_t *slice, double angle)
{
  int text_height = slice->lh;
  int text_width = slice->lw;
  int half_width = text_width / 2.0;
  int half_height = text_height / 2.0;
  int k1, k2, j1, j2;
  int quad = shoes_plot_util_quadrant(angle);
  switch(quad) {
    case QUAD_ONE:
      k1 = j1 = k2 = 1;
      j2 = -1;
      break;
    case QUAD_TWO:
      k1 = k2 = -1;
      j1 = j2 = 1;
      break;
    case QUAD_THREE:
      k1 = j1 = k2 = -1;
      j2 = 1;
      break;
    case QUAD_FOUR:
      k1 = k2 = 1;
      j1 = j2 = -1;
      break;
    default:
      fprintf(stderr, "plot_pie- bad news\n");
      return;
  }

  double cx = chart->radius * cos(angle) + k1 * half_width;
  double cy = chart->radius * sin(angle) + j1 * half_height;

  double radius2 = sqrt(cx * cx + cy * cy);

  double tang = tan(angle);
  double x = sqrt((radius2 * radius2) / (1 + tang * tang));
  double y = tang * x;

  x = chart->centerx + k2 * x;
  y = chart->centery + j2 * y;
  
  // set 4 variables for return (python puts the list into the tick tuple)
  //return x - half_width, y - half_height, text_width, text_height
  slice->lx = x - half_width;
  slice->ly = y - half_height;
  slice->lw = text_width;
  slice->lh = text_height;
}

void shoes_plot_draw_pie_ticks(cairo_t *cr, shoes_plot *plot) 
{
  if (plot->seriescnt != 1) 
    return; //  just in case
  pie_chart_t *chart = (pie_chart_t *) plot->c_things;
  int i;
  PangoRectangle logical;
  for (i = 0; i < chart->count; i++) {
    pie_slice_t *slice = &chart->slices[i];
    char vstr[10];
    if (chart->percent)
      sprintf(vstr, "%i%%", (int)((slice->value / (chart->maxv - chart->minv))*100.0));
    else 
      sprintf(vstr, "%i", (int)slice->value);

    slice->label = malloc(strlen(vstr));
    strcpy(slice->label, vstr);
    slice->layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (slice->layout, plot->legend_pfd);
    pango_layout_set_text (slice->layout, slice->label, -1);
    pango_layout_get_pixel_extents (slice->layout, NULL, &logical);
    slice->lw = logical.width;
    slice->lh = logical.height;
    //double angle = shoes_plot_pie_getNormalisedAngle(slice);
    //double radius = get_min_radius(angle, chart->centerx, chart->centery,
    //                                      logical->width, logical->height)
  }
  
  // pass through the slices again, drawing, free the string, unref the layouts ?
  for (i = 0; i < chart->count; i++) {
    pie_slice_t *slice = &chart->slices[i];
    double angle = shoes_plot_pie_getNormalisedAngle(slice);
    shoes_plot_pie_tick_position(cr, chart, slice, angle);
    cairo_move_to(cr, slice->lx, slice->ly);
    // set color?
    pango_cairo_show_layout(cr, slice->layout);
    free(slice->label);
    g_object_unref(slice->layout);
  }
}

// called at Shoes draw time. Calls many other functions. 
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
  self_t->yaxis_offset = 20; // TODO:  run TOTO! run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    shoes_plot_draw_pie_chart(cr, self_t);
    shoes_plot_draw_pie_ticks(cr, self_t);
    shoes_plot_draw_pie_legend(cr, self_t);
  }
}
