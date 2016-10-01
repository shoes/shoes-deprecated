// pie chart
#include "shoes/plot/plot.h"

// REMEMBER - this is only one data series in a PIE CHART. 

/* borrows heavily from this python code
 * https://bitbucket.org/lgs/pycha/src/e3e270a0e7ae4896052d4cc34fe7f1e532ece914/pycha/pie.py?at=default&fileviewer=file-view-default
 * because its kind of Ruby/Shoes friendly and short-ish.
*/
// Forward declares in this file
VALUE shoes_plot_pie_color(int);

// called when the data series is added to the chart.
// Trying very hard to not pollute Shoes C name space and h files with plot stuff
void shoes_plot_pie_init(shoes_plot *plot) {
  pie_chart_t *piechart = malloc(sizeof(pie_chart_t));
  plot->pie_things = (void *)piechart;
  VALUE rbsz = rb_ary_entry(plot->sizes, 0);
  int numobs = NUM2INT(rbsz);
  piechart->count = numobs;
  pie_slice_t *slices = (pie_slice_t *)malloc(sizeof(pie_slice_t) * numobs);
  piechart->slices = slices;
  int i;
  piechart->maxv = 0.0; 
  piechart->minv = 100000000.0; // TODO: use a max double constant
  VALUE rbvals = rb_ary_entry(plot->values, 0);
  // sum the values
  for (i = 0; i <numobs; i++) {
    pie_slice_t *slice = &slices[i];
    VALUE rbv = rb_ary_entry(rbvals, i);
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
    VALUE wedge_color = shoes_plot_pie_color(i);
    Data_Get_Struct(wedge_color, shoes_color, slice->color);
  }
}

// called when it needs to go away
void shoes_plot_pie_dealloc(shoes_plot *plot) {
  if (plot->pie_things) {
   pie_chart_t *piechart = (pie_chart_t *) plot->pie_things;
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
  pie_chart_t *chart = (pie_chart_t *) plot->pie_things;

  chart->centerx = left + roundl(width * 0.5);
  chart->centery = top + roundl(height * 0.5);
  chart->radius = min(width / 2.0, height / 2.0);
  chart->top = top; chart->left = left; chart->bottom = bottom;
  chart->right = right; chart->width = width; chart->height = height;
  
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
;
// Yes this could be precomputed. It isn't. TODO:
VALUE shoes_plot_pie_color(int ser)
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
  //printf("legend box x: %i, y: %i, h: %i, w: %i\n", box_x, box_y, boxh, boxw);
  // Draw using layout
  for (i = 0; i < numstrs; i++) {
    cairo_move_to(cr, box_x, box_y);
    shoes_color *color;
    VALUE rbcolor = shoes_plot_pie_color(i);
    Data_Get_Struct(rbcolor, shoes_color, color);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
       color->b / 255.0, color->a / 255.0);
    pango_cairo_show_layout(cr, layouts[i]);
    box_y += strh[i];
    g_object_unref(layouts[i]);
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
  int half_width = slice->lw / 2.0;
  int half_height = slice->lh / 2.0;
  double k1, k2, j1, j2;
  printf("tick: value: %s, radius: %f, angle: %f\n", slice->label, chart->radius, angle);
  if (0 <= angle < 0.5 * SHOES_PI) {
    // first quadrant
    k1 = j1 = k2 = 1;
    j2 = -1;
    printf("first quad \n");
  }
#ifdef PYTHON
       if 0 <= angle < 0.5 * math.pi:
            # first quadrant
            k1 = j1 = k2 = 1
            j2 = -1
        elif 0.5 * math.pi <= angle < math.pi:
            # second quadrant
            k1 = k2 = -1
            j1 = j2 = 1
        elif math.pi <= angle < 1.5 * math.pi:
            # third quadrant
            k1 = j1 = k2 = -1
            j2 = 1
        elif 1.5 * math.pi <= angle < 2 * math.pi:
            # fourth quadrant
            k1 = k2 = 1
            j1 = j2 = -1

        cx = radius * math.cos(angle) + k1 * half_width
        cy = radius * math.sin(angle) + j1 * half_height

        radius2 = math.sqrt(cx * cx + cy * cy)

        tan = math.tan(angle)
        x = math.sqrt((radius2 * radius2) / (1 + tan * tan))
        y = tan * x

        x = centerx + k2 * x
        y = centery + j2 * y

        return x - half_width, y - half_height, text_width, text_height
#endif
}

void shoes_plot_draw_pie_ticks(cairo_t *cr, shoes_plot *plot) 
{
  if (plot->seriescnt != 1) 
    return; //  just in case
  pie_chart_t *chart = (pie_chart_t *) plot->pie_things;
  int i;
  PangoRectangle logical;
  for (i = 0; i < chart->count; i++) {
    pie_slice_t *slice = &chart->slices[i];
    char vstr[10];
    // TODO - option for % 
    sprintf(vstr, "%i", (int)slice->value);
    //sprintf(vstr, "%i%%", (int)((slice->value / (chart->maxv - chart->minv))*100.0));
    // gets ugly quick
    slice->label = malloc(strlen(vstr));
    strcpy(slice->label, vstr);
    slice->layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (slice->layout, plot->label_pfd);
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
