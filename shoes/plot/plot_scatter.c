// scatter chart
#include "shoes/plot/plot.h"


void shoes_plot_draw_scatter_pts(cairo_t *cr, shoes_plot *plot) {
    // first series (x) controls graphical settings.
    if (plot->seriescnt !=  2)
        return; // we don't have just two series
    int i;
    int top,left,bottom,right;
    left = plot->graph_x;
    top = plot->graph_y;
    right = plot->graph_w;
    bottom = plot->graph_h;
    int height = bottom - top;
    int width = right - left;
    VALUE rbxser, rbyser;
    shoes_chart_series *serx, *sery;
    rbxser = rb_ary_entry(plot->series, 0);
    Data_Get_Struct(rbxser, shoes_chart_series, serx);
    rbyser = rb_ary_entry(plot->series, 1);
    Data_Get_Struct(rbyser, shoes_chart_series, sery);

    double xmax = NUM2DBL(serx->maxv);
    double ymax = NUM2DBL(sery->maxv);
    double xmin = NUM2DBL(serx->minv);
    double ymin = NUM2DBL(sery->minv);
    int nubs = NUM2INT(serx->point_type);
    VALUE shcolor = serx->color;
    VALUE rbstroke = serx->strokes;
    int strokew = NUM2INT(rbstroke);
    if (strokew < 1) strokew = 1;
    shoes_color *color;
    Data_Get_Struct(shcolor, shoes_color, color);

    int obvs = RARRAY_LEN(serx->values);
    for (i = 0; i < obvs; i++) {
        double xval, yval;
        xval = NUM2DBL(rb_ary_entry(serx->values, i));
        yval = NUM2DBL(rb_ary_entry(sery->values, i));
        //printf("scatter x: %f, y: %f\n", xval, yval);
    }

    double yScale = height / (ymax - ymin);
    double xScale = width / (xmax - xmin);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
                          color->b / 255.0, color->a / 255.0);
    for (i = 0; i < obvs; i++) {
        VALUE rbx = rb_ary_entry(serx->values, i);
        double xval = NUM2DBL(rbx);
        VALUE rby = rb_ary_entry(sery->values, i);
        double yval = NUM2DBL(rby);
        long x = roundl((xval - xmin) * xScale);
        long y = height - roundl((yval - ymin) * yScale);
        x += left;
        y += top;
        //printf("x: %f, y: %f --> x: %i px, y: %i, px\n", xval, yval, x, y);
        // lets draw a nub at x, y
        cairo_move_to(cr, x, y);
        shoes_plot_draw_nub(cr, plot, x, y, nubs, strokew + 2);
    }
    cairo_stroke(cr);
    shoes_plot_set_cairo_default(cr, plot);
}

static void shoes_plot_scatter_ticks_and_labels(cairo_t *cr, shoes_plot *plot) {
    int top, left, bottom, right; // these are cairo abs for plot->graph
    int width, height;   // full plot space so it includes everything
    int range;
    int h_padding = 65;
    int v_padding = 25;
    left = plot->graph_x;
    top = plot->graph_y;
    right = plot->graph_w;
    bottom = plot->graph_h;
    range = plot->end_idx - plot->beg_idx;
    width = right - left;
    height = bottom - top;
    h_padding = width / plot->x_ticks; // TODO: rethink.
    v_padding = height / plot->y_ticks;

    VALUE rbserx, rbsery;
    shoes_chart_series *serx, *sery;
    rbserx = rb_ary_entry(plot->series, 0);
    rbsery = rb_ary_entry(plot->series, 1);
    Data_Get_Struct(rbserx, shoes_chart_series, serx);
    Data_Get_Struct(rbsery, shoes_chart_series, sery);
    double xmax = NUM2DBL(serx->maxv);
    double ymax = NUM2DBL(sery->maxv);
    double xmin = NUM2DBL(serx->minv);
    double ymin = NUM2DBL(sery->minv);
    /*
    VALUE rbxmax = rb_ary_entry(plot->maxvs, 0);
    VALUE rbymax = rb_ary_entry(plot->maxvs, 1);
    VALUE rbxmin = rb_ary_entry(plot->minvs, 0);
    VALUE rbymin = rb_ary_entry(plot->minvs, 1);
    double xmax = NUM2DBL(rbxmax);
    double ymax = NUM2DBL(rbymax);
    double xmin = NUM2DBL(rbxmin);
    double ymin = NUM2DBL(rbymin);
    */
    double h_scale;
    int h_interval;
    //h_scale = width / (double) (range -1);
    h_scale = width / (double) (range );
    h_interval = (int) ceil(h_padding / h_scale);
    char tstr[10];

    // draw x axis - labels and tick marks generated between xmin-->xmax
    int i;
    for (i = 0 ; i < range; i++ ) {
        int x = (int) roundl(i * h_scale);
        x += left;
        long y = bottom;
        if ((i % h_interval) == 0) {
            char rawstr[10];
            // convert i to number in the range of xmin->xmax
            sprintf(rawstr, "%4.2f", xmin + ((xmax - xmin) / range) * i); // Do not trust!!
            shoes_plot_draw_tick(cr, plot, x, y, VERTICALLY);
            shoes_plot_draw_label(cr, plot, x, y, rawstr, BELOW);
        }
    }
    // draw the last label on x
    sprintf(tstr, "%4.2f", xmax);
    shoes_plot_draw_label(cr, plot, right, bottom, tstr, BELOW);


    // draw y axis - there is only one in a Shoes scatter plot
    double v_scale = height / (ymax - ymin);
    double j;
    int v_interval = (int) ceil(v_padding / v_scale);
    //printf("v_scale: %f, v_interval: %i\n", v_scale, v_interval);
    for (j = ymin ; j < ymax; j += v_interval) {
        int y = (int) (bottom - roundl((j - ymin) * v_scale));
        int x = left;
        sprintf(tstr, "%4.2f", j);
        //printf("hoz left %i, %i, %s\n", (int)x, (int)y, tstr);
        shoes_plot_draw_tick(cr, plot, x, y, HORIZONTALLY);
        shoes_plot_draw_label(cr, plot, x, y, tstr, LEFT);
    }
    // print top label
    sprintf(tstr, "%4.2f", ymax);
    shoes_plot_draw_label(cr, plot, left, top, tstr, LEFT);

}

void shoes_plot_scatter_legend(cairo_t *cr, shoes_plot *plot) {
    if (plot->seriescnt != 2) return;
    int top, left, bottom, right;
    int width, height;
    left = plot->place.x;
    top = plot->graph_h + 5;
    right = plot->place.w;
    bottom = top + plot->legend_h;
    width = right - left;
    height = bottom - top;
    // scatter has only two series - center the x string [0]
    // try to draw the y string [1] vertically. -fun or groan?
    VALUE rbserx, rbsery;
    shoes_chart_series *serx, *sery;
    rbserx = rb_ary_entry(plot->series, 0);
    rbsery = rb_ary_entry(plot->series, 1);
    Data_Get_Struct(rbserx, shoes_chart_series, serx);
    Data_Get_Struct(rbsery, shoes_chart_series, sery);
    int legend_width = 0;
    int x, y;

    char *xstr = RSTRING_PTR(serx->desc);
    PangoLayout *x_layout  = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (x_layout, plot->legend_pfd);
    pango_layout_set_text (x_layout, xstr, -1);
    PangoRectangle logical;
    pango_layout_get_pixel_extents (x_layout, NULL, &logical);
    legend_width = logical.width;

    int xoffset = (plot->place.w / 2) - (legend_width / 2);
    x = xoffset - (plot->place.dx);
    int yhalf = (plot->legend_h / 2 );
    int yoffset = yhalf;
    y = yoffset;

    int baseline = bottom - 5; //TODO: compute baseline better
    shoes_color *color;
    Data_Get_Struct(serx->color, shoes_color, color);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
                          color->b / 255.0, color->a / 255.0);
    cairo_move_to(cr, x, baseline);
    pango_cairo_show_layout(cr, x_layout);
    /*
     *  Now the y axis label. Rotate and put on the left side
     *  Draw above (in the title area, left or right)??
     *
    */
    char *ystr = RSTRING_PTR(sery->desc);
    cairo_save(cr);
    PangoLayout *y_layout  = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (y_layout, plot->legend_pfd);
    pango_layout_set_text (y_layout, ystr, -1);
    pango_layout_get_pixel_extents (y_layout, NULL, &logical);
    Data_Get_Struct(sery->color, shoes_color, color);
    cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
                          color->b / 255.0, color->a / 255.0);
    // since we're drawing text vertically, compute text placement differently
    // It's very confusing (to me, at least).
    int yoff = ((plot->graph_h - plot->graph_y) - logical.width) / 2;
    cairo_move_to(cr, (plot->graph_x - plot->yaxis_offset) + 5,
                  plot->graph_h - yoff);
    cairo_rotate(cr, -90.0 / (180.0 / G_PI)); // rotate in radians
    pango_cairo_show_layout(cr, y_layout);
    cairo_restore(cr);
    g_object_unref(x_layout);
    g_object_unref(y_layout);
}



// called at draw time. Calls many other functions.
//      Whole lotta drawing going on
// Note: we expand the margins a bit shoe we can draw the label vertically
void shoes_plot_scatter_draw(cairo_t *cr, shoes_place *place, shoes_plot *self_t) {
    shoes_plot_set_cairo_default(cr, self_t);
    shoes_plot_draw_fill(cr, self_t);
    shoes_plot_draw_title(cr, self_t);
    shoes_plot_draw_caption(cr, self_t);
    self_t->graph_h = self_t->place.h - (self_t->title_h + self_t->caption_h);
    self_t->graph_y = self_t->title_h + 3;
    self_t->yaxis_offset = 70; // TODO:  run TOTO! run!
    self_t->graph_w = self_t->place.w - self_t->yaxis_offset;
    self_t->graph_x = self_t->yaxis_offset;
    if (self_t->boundbox)
        shoes_plot_draw_boundbox(cr, self_t);
    if (self_t->seriescnt) {
        // draw  box, ticks and x,y labels.
        shoes_plot_scatter_ticks_and_labels(cr, self_t);
        shoes_plot_scatter_legend(cr, self_t);
        shoes_plot_draw_scatter_pts(cr, self_t);
    }
}
