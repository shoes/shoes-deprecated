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
static void shoes_plot_radar_draw_radials(cairo_t *, shoes_plot *, radar_chart_t *);
static void shoes_plot_radar_draw_ticks(cairo_t *, shoes_plot *, radar_chart_t *);
static void shoes_plot_radar_draw_rings(cairo_t *, shoes_plot *, radar_chart_t *);
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
  int numcols = RARRAY_LEN(plot->column_opts); 
  int i;
  if (plot->seriescnt == 1) {
    // one init time
    rdrchart = malloc(sizeof(radar_chart_t));
    plot->c_things = (void *)rdrchart;
    VALUE outer_ary = plot->column_opts;
    rdrchart->count = RARRAY_LEN(outer_ary);
    rdrchart->colmax = malloc(sizeof(double) * numcols);
    rdrchart->colmin = malloc(sizeof(double) * numcols);
    rdrchart->labels = malloc(sizeof(char *) * numcols);
    rdrchart->fmt_strs = malloc(sizeof(char *) * numcols);
    rdrchart->lw = malloc(sizeof(double) * numcols);
    rdrchart->lh = malloc(sizeof(double) * numcols);
    rdrchart->lx = malloc(sizeof(double) * numcols);
    rdrchart->ly = malloc(sizeof(double) * numcols);
    rdrchart->layouts = malloc(sizeof(PangoLayout *) * numcols); 
    for (i = 0; i < numcols; i++) {
      // unwrap the Ruby array of arrays
      VALUE inner_ary = rb_ary_entry(outer_ary, i);
      int j, cnt;
      VALUE rbval;
      char *str;
      cnt = RARRAY_LEN(inner_ary);
      rbval = rb_ary_entry(inner_ary, RADAR_LABEL); 
      rdrchart->labels[i] = RSTRING_PTR(rbval); // should be safe from gc
      rbval = rb_ary_entry(inner_ary, RADAR_MIN); 
      rdrchart->colmin[i] = NUM2DBL(rbval);
      rbval = rb_ary_entry(inner_ary, RADAR_MAX); 
      rdrchart->colmax[i] = NUM2DBL(rbval); 
      rbval = rb_ary_entry(inner_ary, RADAR_EXTRA); // format [optional]
      if (NIL_P(rbval)) {
        rdrchart->fmt_strs[i] = strdup("%4.2f");
      } else {
        rdrchart->fmt_strs[i] = strdup(RSTRING_PTR(rbval));
      }
      // layout fun for outer labels
      rdrchart->lw[i] = 0.0;
      rdrchart->lh[i] = 0.0;
      rdrchart->lx[i] = 0.0;
      rdrchart->ly[i] = 0.0;
    }
  }
}

// called when it needs to go away
void shoes_plot_radar_dealloc(shoes_plot *plot) {
  if (plot->c_things) {
   radar_chart_t *rdrchart = (radar_chart_t *) plot->c_things;
   free(rdrchart->colmax);
   free(rdrchart->lw);
   free(rdrchart->lh);
   free(rdrchart->lx);
   free(rdrchart->ly);
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
  
  // TODO: incomplete or done poorly ?
  shoes_plot_radar_draw_radials(cr, plot, chart);
  shoes_plot_radar_draw_ticks(cr, plot, chart);
  shoes_plot_radar_draw_rings(cr, plot, chart);
  
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
        // move_to (save opening pos)
        close_x = xpos;
        close_y = ypos;
        cairo_move_to(cr, xpos, ypos);
      } else {
        // line to
        cairo_line_to(cr, xpos, ypos);
      }
    }
    // line to first pos
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
    double r_offset = 1.0;
    double x_offset = cx;
    double y_offset = cy;
    double rad_pos = deg2rad(angle);
    double rx = x_offset + (radius * r_offset * sin(deg2rad(angle)));
    double ry = y_offset - (radius * r_offset * cos(deg2rad(angle)));
    //printf("num label ang: %4.2f - > %4.2f\n", angle, rad_pos);
    cairo_move_to(cr, rx, ry);
    PangoRectangle logical;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->tiny_pfd);
    pango_layout_set_text (layout, vlbl, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    pango_cairo_show_layout(cr, layout);
    // TODO unref the layout? 
    g_object_unref(layout);
}

#if 0
// draw xaxis (radial) label value 
// This is the gruff code - we might need it
void shoes_plot_radar_draw_label(cairo_t *cr, shoes_plot *plot,  double cx, double cy, double angle, double radius, char *vlbl)
{
    double r_offset = 1.2;
    double x_offset = cx;
    double y_offset = cy;
    double rx = x_offset + (radius * r_offset * sin(deg2rad(angle)));
    double ry = y_offset - (radius * r_offset * cos(deg2rad(angle)));
  
    cairo_move_to(cr, rx, ry);
    PangoRectangle logical;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->label_pfd);
    pango_layout_set_text (layout, vlbl, -1);
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    pango_cairo_show_layout(cr, layout);
}
#endif

// determines how many rings can be drawn without getting too busy. 
// user setting can override
static int shoes_plot_radar_ring_count(shoes_plot *plot, double radius) {
  if (plot->x_ticks == 1) {
    // magic heuristic
    return max(1, ceill(radius / 75));
  } else
    return plot->x_ticks;
}

// checks if all colmin[*] are the same AND all colmax[*] are the same
static int shoes_plot_radar_same_range(radar_chart_t *chart) {
  double same_min = chart->colmin[0];
  double same_max = chart->colmax[0];
  int sz = chart->count;
  int i;
  for (i = 0; i < sz; i++) {
    if (chart->colmin[i] != same_min) return 0;
    if (chart->colmax[i] != same_max) return 0;
  }
  return 1;
}

// Just draw the x_axis lines from center to outer
static void shoes_plot_radar_draw_radials(cairo_t *cr, shoes_plot *plot, radar_chart_t *chart)
{
  int i;
  int sz = chart->count;
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0.0, 0.0 ,0.0, 0.6); // black, 60%
  // how many rings/labels for each axis?
  int rings = shoes_plot_radar_ring_count(plot, chart->radius);
  int only_one = shoes_plot_radar_same_range(chart);
 
    for (i = 0; i < sz ; i++) {
      double rad_pos = (i * SHOES_PI * 2) / sz;
      int x = chart->centerx;
      int y = chart->centery;
      int rx = chart->centerx + sin(rad_pos) * chart->radius;
      int ry = chart->centery - cos(rad_pos) * chart->radius;
      cairo_move_to(cr, x, y);
      cairo_line_to(cr, rx, ry);
      cairo_stroke(cr);
     }

  cairo_restore(cr);
}

// draws the xaxis (radial) numeric labels
static void shoes_plot_radar_draw_ticks(cairo_t *cr, shoes_plot *plot, radar_chart_t *chart)
{
  int i;
  int sz = chart->count;
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0.0, 0.0 ,0.0, 0.6); // black, 60%
  // how many rings/labels for each axis?
  int rings = shoes_plot_radar_ring_count(plot, chart->radius);
  int only_one = shoes_plot_radar_same_range(chart);
  int j;
  for (j = 0; j < rings; j++) {
    double radius = (chart->radius / rings) * (j +1 ); // drawing pos
    double scale_pos = (1.0 / rings) * (j + 1);
    // printf("radius: %4.2f scale_pos %4.2f\n", radius, scale_pos);
    for (i = 0; i < sz ; i++) {
      double rad_pos = (i * SHOES_PI * 2) / sz;
      int x = chart->centerx;
      int y = chart->centery;
      int rx = chart->centerx + sin(rad_pos) * radius;
      int ry = chart->centery - cos(rad_pos) * radius;
      cairo_move_to(cr, x, y);
      //cairo_line_to(cr, rx, ry);
      //cairo_stroke(cr);
      if (i == 0 || !only_one) {
        char vlbl[16];
        double range = chart->colmax[i] - chart->colmin[i];
        double lblv = range * scale_pos;
        sprintf(vlbl, chart->fmt_strs[i], lblv);
        shoes_plot_radar_draw_mark(cr, plot, chart->centerx, chart->centery, 
            rad_pos * 360 / (2 * SHOES_PI), radius, vlbl);
       }   
     }
  }
  cairo_restore(cr);
}

static void shoes_plot_radar_draw_rings(cairo_t *cr, shoes_plot *plot, radar_chart_t *chart)
{
 int i;
  int sz = chart->count;
  cairo_save(cr);
  cairo_set_source_rgba(cr, 0.4, 0.4 ,0.4, 1.0); // lt gray.
  // how many rings?
  int rings = shoes_plot_radar_ring_count(plot, chart->radius);
  int j;
  int close_x, close_y;
  for (j = 0; j < rings; j++) {
    double radius = (chart->radius / rings) * (j +1 ); // drawing pos
    for (i = 0; i < sz ; i++) {
      double rad_pos = (i * SHOES_PI * 2) / sz;
      int x = chart->centerx;
      int y = chart->centery;
      int rx = chart->centerx + sin(rad_pos) * radius;
      int ry = chart->centery - cos(rad_pos) * radius;
      if (i == 0) {
        cairo_move_to(cr, rx, ry);
        close_x = rx;
        close_y = ry;
      }
      else 
        cairo_line_to(cr, rx, ry);
     }
     cairo_line_to(cr, close_x, close_y);
     cairo_stroke(cr);
  }
  cairo_stroke(cr);
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

#if 0 // may want this. 
double shoes_plot_radar_getNormalisedAngle(radar_pole_t *self) {
  double normalisedAngle = (self->startAngle + self->endAngle) / 2;
  if (normalisedAngle > SHOES_PI * 2)
    normalisedAngle -= SHOES_PI * 2;
  else if (normalisedAngle < 0)
    normalisedAngle += SHOES_PI * 2;

  return normalisedAngle;
}
#endif 

void
shoes_plot_radar_label_position(cairo_t *cr, radar_chart_t * chart, int idx, double angle)
{
  int text_height = chart->lh[idx]; //slice->lh;
  int text_width = chart->lw[idx];  //slice->lw;
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
      fprintf(stderr, "plot_radar.c - bad news\n");
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
  
  // set 4 variables for the return value - caller has to deal with it.
  chart->lx[idx] = x - half_width;
  chart->ly[idx] = y - half_height;
  chart->lw[idx] = text_width;
  chart->lh[idx] = text_height;
}

void shoes_plot_draw_radar_outer_labels(cairo_t *cr, shoes_plot *plot) 
{
  if (plot->seriescnt < 1) 
    return; //  just in case
  radar_chart_t *chart = (radar_chart_t *) plot->c_things;
  int i;
  PangoRectangle logical;
  for (i = 0; i < chart->count; i++) {
    chart->layouts[i] = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (chart->layouts[i], plot->legend_pfd);
    pango_layout_set_text (chart->layouts[i], chart->labels[i], -1);
    pango_layout_get_pixel_extents (chart->layouts[i], NULL, &logical);
    chart->lw[i] = logical.width;
    chart->lh[i] = logical.height;
  }
  
  // pass through the labels  again, drawing, free the string, unref the layouts ?
  for (i = 0; i < chart->count; i++) {
    //double angle = shoes_plot_radar_getNormalisedAngle(slice);
    double angle = deg2rad(i * (360.0 / chart->count));
    shoes_plot_radar_label_position(cr, chart, i, angle);
    //printf("ring label ang: %4.2f -> %4.2f, %4.2f\n",angle, chart->lx[i], chart->ly[i]); 
    cairo_move_to(cr, chart->lx[i], chart->ly[i]);
    pango_cairo_show_layout(cr, chart->layouts[i]);
    g_object_unref(chart->layouts[i]);
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
    shoes_plot_draw_radar_outer_labels(cr, self_t);
    // shoes_plot_draw_radar_ticks(cr, self_t);
    shoes_plot_draw_legend(cr, self_t);
  }
}
