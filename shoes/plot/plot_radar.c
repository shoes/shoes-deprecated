/* 
 * radar chart
 * 
 * uses radar_chart_t and radar_pole_t for now. MAY NOT BE NEEDED
 * It's more like a bar chart than a pie chart.
*/
#include "shoes/plot/plot.h"


/* borrows heavily from https://github.com/topfunky/gruff
 * also a hat tip to python code https://bitbucket.org/lgs/pycha/src/
*/
// Forward declares in this file
VALUE shoes_plot_radar_color(int);
void shoes_plot_radar_draw_axes(cairo_t *, shoes_plot *, radar_chart_t *);
static double deg2rad(double);

/* 
 * called when each data series is added to the chart, after it's added to
 * shoes_plot->series (array of) chart_series class and shoes_plot->seriescnt
 * is accurate (1, 2, ..) Create a more gruff like struct to play with and
 * convert to c values (normalized?)- don't forget to free things
*/
void shoes_plot_radar_init(shoes_plot *plot) {
  radar_chart_t *rdrchart;
  
  VALUE cs = rb_ary_entry(plot->series, plot->seriescnt -1);
  shoes_chart_series *ser;
  Data_Get_Struct(cs, shoes_chart_series, ser);
  //int numcols = RARRAY_LEN(ser->labels);
  int numcols = RARRAY_LEN(plot->column_opts); 
  radar_pole_t *slices; 
  int i;
  if (plot->seriescnt == 1) {
    // one init time
    rdrchart = malloc(sizeof(radar_chart_t));
    plot->c_things = (void *)rdrchart;
    VALUE outer_ary = plot->column_opts;
    rdrchart->count = RARRAY_LEN(outer_ary);
    rdrchart->slices = slices = (radar_pole_t *)malloc(sizeof(radar_pole_t) * numcols);
    rdrchart->colmax = malloc(sizeof(double) * numcols);
    rdrchart->colmin = malloc(sizeof(double) * numcols);
    rdrchart->labels = malloc(sizeof(char *) * numcols);
    rdrchart->fmt_strs = malloc(sizeof(char *) * numcols);
    for (i = 0; i < numcols; i++) {
      // unwrap the Ruby array of arrays
      VALUE inner_ary = rb_ary_entry(outer_ary, i);
      int j, cnt;
      VALUE rbval;
      char *str;
      cnt = RARRAY_LEN(inner_ary);
      rbval = rb_ary_entry(inner_ary, 0); // label
      rdrchart->labels[i] = RSTRING_PTR(rbval); // should be safe from gc
      rbval = rb_ary_entry(inner_ary, 1); // min
      rdrchart->colmin[i] = NUM2DBL(rbval);
      rbval = rb_ary_entry(inner_ary, 2); // max
      rdrchart->colmax[i] = NUM2DBL(rbval); 
      rbval = rb_ary_entry(inner_ary, 3); // format [optional]
      if (NIL_P(rbval)) {
        rdrchart->fmt_strs[i] = strdup("%4.2f");
      } else {
        rdrchart->fmt_strs[i] = strdup(RSTRING_PTR(rbval));
      }
    }
  } else { // as long as we need radar_pole_t
    rdrchart = (radar_chart_t *)plot->c_things;
    slices = (radar_pole_t *)rdrchart->slices;
  }
#if 0
  // ignore chart_series - computte min max settings use the values in the column
  // in each row (chart_series)
  for (i = 0; i < plot->seriescnt; i++) {
    VALUE cs = rb_ary_entry(plot->series, i);
    shoes_chart_series *ser;
    Data_Get_Struct(cs, shoes_chart_series, ser);
    int j;   // column in each row
    for (j = 0; j < numcols; j++) {
      VALUE rbv  = rb_ary_entry(ser->values, j);
      double val = NUM2DBL(rbv);
      rdrchart->colmax[j] = max(val, rdrchart->colmax[j]);
      rdrchart->colmin[j] = min(val, rdrchart->colmin[j]);
    }
  }
#endif
  
  
  // following is pie chart stuff - not needed for radar? 
  rdrchart->maxv = 0.0; 
  rdrchart->minv = 100000000.0; // TODO: use a max double constant
  // sum the values
  for (i = 0; i <numcols; i++) {
    radar_pole_t *slice = &slices[i];
    VALUE rbv = rb_ary_entry(ser->values, i);
    double v = NUM2DBL(rbv);
    slice->value = v;
    if (v < rdrchart->minv) rdrchart->minv = v;
    rdrchart->maxv += v;
  }
  double fraction = 0.0;
  double angle = 0.0;
  for (i = 0; i < numcols; i++) {
    radar_pole_t *slice = &slices[i];
    angle += fraction;
    double v = slice->value;
    fraction = v / rdrchart->maxv;
    slice->startAngle = 2 * angle * SHOES_PI;
    slice->endAngle = 2 * (angle + fraction) * SHOES_PI;
    VALUE wedge_color = rb_ary_entry(plot->default_colors, i);
    Data_Get_Struct(wedge_color, shoes_color, slice->color);
  }
}

// called when it needs to go away
void shoes_plot_radar_dealloc(shoes_plot *plot) {
  if (plot->c_things) {
   radar_chart_t *rdrchart = (radar_chart_t *) plot->c_things;
   free(rdrchart->colmax);
   free(rdrchart->colmin);
   free(rdrchart->slices);
   free(rdrchart->labels); // an array of RSTRING_PTRs
   int i;
   for (i = 0; i < rdrchart->count; i++) 
     free(rdrchart->fmt_strs[i]);
   free(rdrchart->fmt_strs);
   free(rdrchart);
  }
}

void shoes_plot_draw_radar_chart(cairo_t *cr, shoes_plot *plot)
{
  // first series controls labels and such. 
  if (plot->seriescnt <= 0)
    return; 
  int i;
  int top,left,bottom,right, height, width;
  left = plot->graph_x;
  top = plot->graph_y + 20;
  right = plot->graph_w;
  bottom = plot->graph_h -20; 
  width = right - left;
  height = bottom - top; 
  
  radar_chart_t *chart = (radar_chart_t *) plot->c_things;
   

  chart->centerx = left + roundl(width * 0.5);
  chart->centery = top + roundl(height * 0.5);
  chart->radius = min(width / 2.0, height / 2.0);
  chart->top = top; chart->left = left; chart->bottom = bottom;
  chart->right = right; chart->width = width; chart->height = height;

  int count = chart->count;
  chart->angle = (2 * SHOES_PI) / count;
  chart->rotation = 0.0;
  
  // TODO: draw the axes and labels at the edge - incomplete or done poorly
  shoes_plot_radar_draw_axes(cr, plot, chart);
  
  // draw the data points - 
  for (i = 0; i < plot->seriescnt; i++) {
    // for each row (aka chart_series) 
    shoes_chart_series *cs;
    VALUE rbcs = rb_ary_entry(plot->series, i);
    Data_Get_Struct(rbcs, shoes_chart_series, cs);
    shoes_color *color;
    Data_Get_Struct(cs->color, shoes_color, color);
    int strokew = NUM2INT(cs->strokes);
    cairo_set_source_rgba(cr, color->r / 255.0 ,color->g / 255.0 ,
        color->b / 255.0 , color->a / 255.0);
    cairo_set_line_width(cr, strokew);
    
    double close_x;
    double close_y;
    int j;
    for (j = 0; j < count; j++) {
      // scale the value (aka normalize) for the column min/max
      VALUE rbval = rb_ary_entry(cs->values, j);
      double val = NUM2DBL(rbval);
      double minv = chart->colmin[j];
      double maxv = chart->colmax[j];
      double spread = maxv - minv;
      spread > 0 ? spread: 1.0;
      double sv = (val - minv) / spread;
      // compute position on the axis to move/draw to
      //printf("scaleval: %f\n", sv);
      double rad_pos = j * SHOES_PI * 2 / count;
      double point_distance = sv * chart->radius;
      double xpos = chart->centerx + sin(rad_pos) * point_distance;
      double ypos = chart->centery - cos(rad_pos) * point_distance;
      if (j == 0) {
        // move_to
        close_x = xpos;
        close_y = ypos;
        cairo_move_to(cr, xpos, ypos);
      } else {
        cairo_line_to(cr, xpos, ypos);
      }
    }
    cairo_line_to(cr, close_x, close_y);
    cairo_stroke(cr); 
  }
  
  shoes_plot_set_cairo_default(cr, plot);
}


static double deg2rad(double angle) 
{
  return angle * (SHOES_PI / 180.0);
}

// draw xaxis (radial) numeric value. 
void shoes_plot_radar_draw_mark(cairo_t *cr, shoes_plot *plot,  double cx, double cy, double angle, double radius, char *vlbl)
{
    double r_offset = 1.1;
    double x_offset = cx;
    double y_offset = cy;
    double rx = x_offset + (radius * r_offset * sin(deg2rad(angle)));
    double ry = y_offset - (radius * r_offset * cos(deg2rad(angle)));
  
    cairo_move_to(cr, rx, ry);
    PangoRectangle logical;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->tiny_pfd);
    pango_layout_set_text (layout, vlbl, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    pango_cairo_show_layout(cr, layout);
}

void shoes_plot_radar_draw_axes(cairo_t *cr, shoes_plot *plot, radar_chart_t *chart)
{
  int i;
  // int sz = RARRAY_LEN(cs->labels);
  int sz = chart->count;
  cairo_save(cr);
  for (i = 0; i < sz; i++) {
    //VALUE rblbl = rb_ary_entry(cs->labels, i);
    //char *lbl = RSTRING_PTR(rblbl);  // might not need here
    radar_pole_t *axis =  &chart->slices[i]; // might not need color here
    shoes_color *color = axis->color; 
    double rad_pos = (i * SHOES_PI * 2) / sz;
    int x = chart->centerx;
    int y = chart->centery;
    int rx = chart->centerx + sin(rad_pos) * chart->radius;
    int ry = chart->centery - cos(rad_pos) * chart->radius;
    cairo_move_to(cr, x, y);
    cairo_line_to(cr, rx, ry);
    cairo_stroke(cr);
    
    char vlbl[16];
    sprintf(vlbl, chart->fmt_strs[i], chart->colmax[i]);
    //char *disp_label = vlbl;
    char *disp_label = chart->labels[i];
    //printf("%10.10s x: %i, y: %i, to rx: %i, ry: %i, %s\n", lbl, x, y, rx, ry, vlbl);
    // draw the max value at the end of radial.
    shoes_plot_radar_draw_mark(cr, plot, chart->centerx, chart->centery, 
        rad_pos * 360 / (2 * SHOES_PI), chart->radius, disp_label);
#if 0
    double r_offset = 1.1;
    double x_offset = chart->centerx;
    double y_offset = chart->centery;
    rx = x_offset + (chart->radius * r_offset * sin(deg2rad(chart->angle)));
    ry = y_offset - (chart->radius * r_offset * cos(deg2rad(chart->angle)));
  
    cairo_move_to(cr, rx, ry);
    PangoRectangle logical;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->tiny_pfd);
    pango_layout_set_text (layout, vlbl, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    pango_cairo_show_layout(cr, layout);
#endif      
  }
  cairo_restore(cr);
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
double shoes_plot_radar_getNormalisedAngle(radar_pole_t *self) {
  double normalisedAngle = (self->startAngle + self->endAngle) / 2;
  if (normalisedAngle > SHOES_PI * 2)
    normalisedAngle -= SHOES_PI * 2;
  else if (normalisedAngle < 0)
    normalisedAngle += SHOES_PI * 2;

  return normalisedAngle;
}

shoes_plot_radar_tick_position(cairo_t *cr, radar_chart_t * chart, radar_pole_t *slice, double angle)
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
    radar_pole_t *slice = &chart->slices[i];
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
    radar_pole_t *slice = &chart->slices[i];
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
    shoes_plot_draw_legend(cr, self_t);
  }
}
