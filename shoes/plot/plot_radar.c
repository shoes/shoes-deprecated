/* 
 * radar chart
 * 
 * uses radar_chart_t and radar_slice_t for now. MAY NOT BE NEEDED
 * It's more like a bar chart than a pie chart.
*/
#include "shoes/plot/plot.h"


/* borrows heavily from this python code
 * https://bitbucket.org/lgs/pycha/src/
 * because its kind of Ruby/Shoes friendly and short-ish.
*/
// Forward declares in this file
VALUE shoes_plot_radar_color(int);

// called when a data series is added to the chart.
// USES the chart_series class. Beware.
void shoes_plot_radar_init(shoes_plot *plot) {
  radar_chart_t *rdrchart = malloc(sizeof(radar_chart_t));
  plot->c_things = (void *)rdrchart;
  VALUE cs = rb_ary_entry(plot->series, 0);
  shoes_chart_series *ser;
  Data_Get_Struct(cs, shoes_chart_series, ser);
  int numobs = RARRAY_LEN(ser->values);
  rdrchart->count = numobs;
  radar_slice_t *slices = (radar_slice_t *)malloc(sizeof(radar_slice_t) * numobs);
  rdrchart->slices = slices;
  int i;
  rdrchart->maxv = 0.0; 
  rdrchart->minv = 100000000.0; // TODO: use a max double constant
  // sum the values
  for (i = 0; i <numobs; i++) {
    radar_slice_t *slice = &slices[i];
    VALUE rbv = rb_ary_entry(ser->values, i);
    double v = NUM2DBL(rbv);
    slice->value = v;
    if (v < rdrchart->minv) rdrchart->minv = v;
    rdrchart->maxv += v;
  }
  double fraction = 0.0;
  double angle = 0.0;
  for (i = 0; i < numobs; i++) {
    radar_slice_t *slice = &slices[i];
    angle += fraction;
    double v = slice->value;
    fraction = v / rdrchart->maxv;
    slice->startAngle = 2 * angle * SHOES_PI;
    slice->endAngle = 2 * (angle + fraction) * SHOES_PI;
    VALUE wedge_color = shoes_plot_radar_color(i);
    Data_Get_Struct(wedge_color, shoes_color, slice->color);
  }
}

// called when it needs to go away
void shoes_plot_radar_dealloc(shoes_plot *plot) {
  if (plot->c_things) {
   radar_chart_t *rdrchart = (radar_chart_t *) plot->c_things;
   free(rdrchart->slices);
   free(rdrchart);
  }
}

// draws the lines/nubs connecting the data points of each series
void shoes_plot_draw_radar_chart(cairo_t *cr, shoes_plot *plot)
{
  // first series (x) controls graphical settings. 
  if (plot->seriescnt <= 0)
    return; 
  int i;
  int top,left,bottom,right, height, width;
  left = plot->graph_x; top = plot->graph_y;
  right = plot->graph_w; bottom = plot->graph_h; 
  width = right - left;
  height = bottom - top; 
  
  radar_chart_t *chart = (radar_chart_t *) plot->c_things;
   

  chart->centerx = left + roundl(width * 0.5);
  chart->centery = top + roundl(height * 0.5);
  chart->radius = min(width / 2.0, height / 2.0);
  chart->top = top; chart->left = left; chart->bottom = bottom;
  chart->right = right; chart->width = width; chart->height = height;
  printf("radius %4.2f\n", chart->radius);
  
  int j;
  cairo_save(cr);
  for (i = 0; i < plot->seriescnt; i++) {
    VALUE cs = rb_ary_entry(plot->series, i);
    shoes_chart_series *ser;
    Data_Get_Struct(cs, shoes_chart_series, ser);
    //VALUE rbvary = rb_ary_entry(plot->values, i);
    //int count = RARRAY_LEN(rbvary);
    int count = RARRAY_LEN(ser->values);
    //VALUE rbminv = rb_ary_entry(plot->minvs, i);
    //double minv = NUM2DBL(rbminv);
    double minv = NUM2DBL(ser->minv);
    //VALUE rbmaxv = rb_ary_entry(plot->maxvs, i);
    //double maxv = NUM2DBL(rbmaxv);
    double maxv = NUM2DBL(ser->maxv);
    int first = TRUE;
    cairo_new_path(cr);
    for (j = 0; j < count; j++) {
      int k = j + 1;
      //VALUE rbv = rb_ary_entry(rbvary, j);
      VALUE rbv = rb_ary_entry(ser->values, j);
      double value =  NUM2DBL(rbv);
      double offset1 = k * 2 * SHOES_PI / k;
      double offset = SHOES_PI / 2 - offset1;
      //double rad = (height / 2) * (1 - value);
      double rad = (height / 2) / (value);
      int x = chart->centerx - cos(offset) * rad;
      int y = chart->centery - sin(offset) * rad;
      if (first) {
        printf("move to x %i, y: %i v: %4.2f offset: %4.2f rad: %4.2f\n", x, y, value, offset, rad);
        cairo_move_to(cr, x, y);
        first = FALSE;
        continue;
      } else {
        printf("line to x %i, y: %i v: %4.2f offset: %4.2f rad: %4.2f\n", x, y, value, offset, rad);
        cairo_line_to(cr, x, y);
      }
    }
    cairo_stroke(cr);
  }
  cairo_restore(cr);
  shoes_plot_set_cairo_default(cr, plot);
}
;
// Yes this could be precomputed. It isn't. TODO:
VALUE shoes_plot_radar_color(int ser)
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

// just draws a box
void shoes_plot_radar_legend_box(cairo_t *cr, shoes_plot *self_t, 
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

/* 
 * NOTE: here, "ticks" are values (or %?) drawn around the slices.  
 * Expect confusion and many helper functions.
 * This called after drawing the wedges in a shoes draw event so we know
 * we actually have a cairo_t that is real and the slices are OK. 
 * We depend on that. 
*/
double shoes_plot_radar_getNormalisedAngle(radar_slice_t *self) {
  double normalisedAngle = (self->startAngle + self->endAngle) / 2;
  if (normalisedAngle > SHOES_PI * 2)
    normalisedAngle -= SHOES_PI * 2;
  else if (normalisedAngle < 0)
    normalisedAngle += SHOES_PI * 2;

  return normalisedAngle;
}

shoes_plot_radar_tick_position(cairo_t *cr, radar_chart_t * chart, radar_slice_t *slice, double angle)
{
  int text_height = slice->lh;
  int text_width = slice->lw;
  int half_width = text_width / 2.0;
  int half_height = text_height / 2.0;
  int k1, k2, j1, j2;
  //printf("tick: value: %s, radius: %f, angle: %f\n", slice->label, chart->radius, angle);
  if ((0 <= angle) && (angle < 0.5 * SHOES_PI)) {
    // first quadrant
    k1 = j1 = k2 = 1;
    j2 = -1;
  } else if ((0.5 * SHOES_PI <= angle) && (angle < SHOES_PI)) {
    // second quadrant
    k1 = k2 = -1;
    j1 = j2 = 1;
  } else if ((SHOES_PI <= angle) && (angle < 1.5 * SHOES_PI)) {
    // third quadrant
    k1 = j1 = k2 = -1;
    j2 = 1;
  } else if ((1.5 * SHOES_PI <= angle) && (angle < 2 * SHOES_PI)) {
    // fourth quadrant
    k1 = k2 = 1;
    j1 = j2 = -1;
  } else {
    fprintf(stderr, "plot_radar.c - bad news\n");
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

void shoes_plot_draw_radar_ticks(cairo_t *cr, shoes_plot *plot) 
{
  if (plot->seriescnt != 1) 
    return; //  just in case
  radar_chart_t *chart = (radar_chart_t *) plot->c_things;
  int i;
  PangoRectangle logical;
  for (i = 0; i < chart->count; i++) {
    radar_slice_t *slice = &chart->slices[i];
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
    //double angle = shoes_plot_radar_getNormalisedAngle(slice);
    //double radius = get_min_radius(angle, chart->centerx, chart->centery,
    //                                      logical->width, logical->height)
  }
  
  // pass through the slices again, drawing, free the string, unref the layouts ?
  for (i = 0; i < chart->count; i++) {
    radar_slice_t *slice = &chart->slices[i];
    double angle = shoes_plot_radar_getNormalisedAngle(slice);
    shoes_plot_radar_tick_position(cr, chart, slice, angle);
    cairo_move_to(cr, slice->lx, slice->ly);
    // set color?
    pango_cairo_show_layout(cr, slice->layout);
    free(slice->label);
    g_object_unref(slice->layout);
  }
}

// called at Shoes draw time. Calls many other functions. 
//      Whole lotta drawing going on
void shoes_plot_radar_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
  shoes_plot_set_cairo_default(cr, self_t);
  shoes_plot_draw_fill(cr, self_t);
  shoes_plot_draw_title(cr, self_t);
  shoes_plot_draw_caption(cr, self_t);
  self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
  self_t->graph_y = self_t->title_h + 3;
  self_t->yaxis_offset = 20; // TODO:  run TOTO! run!
  self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
  self_t->graph_x = self_t->yaxis_offset;
  if (self_t->seriescnt) {
    shoes_plot_draw_radar_chart(cr, self_t);
    // shoes_plot_draw_radar_ticks(cr, self_t);
    shoes_plot_draw_cslegend(cr, self_t);
  }
}
