/*
 * plot_util.c
*/
#include "shoes/types/color.h"
#include "shoes/plot/plot.h"

/*
 *  default drawing state is very dark gray(aka black), line width = 1
 *  If your function changes line_width or color you should probably
 *  call this to ensure others functions can have a consistent state.
*/
void shoes_plot_set_cairo_default(cairo_t *cr, shoes_plot *plot) {
    cairo_set_source_rgba(cr, 0.01, 0.01, 0.01, 1.0);
    cairo_set_line_width(cr, 1);
}

// fill graph area with background color
void shoes_plot_draw_fill(cairo_t *cr, shoes_plot *plot) {
    if (NIL_P(plot->background)) {
        cairo_set_source_rgba(cr, 0.99, 0.99, 0.99, 0.99);
    } else {
        shoes_color *color;
        Data_Get_Struct(plot->background, shoes_color, color);
        cairo_set_source_rgba(cr,
                              color->r / 255.0,
                              color->g / 255.0,
                              color->b / 255.0,
                              color->a / 255.0
                             );
    }
    cairo_set_line_width(cr, 1);
    cairo_rectangle(cr, 0, 0, plot->place.w, plot->place.h);
    cairo_fill(cr);
    shoes_plot_set_cairo_default(cr, plot);
}

void shoes_plot_draw_boundbox(cairo_t *cr, shoes_plot *plot) {
    // draw box around data area (plot->graph_?)
    shoes_plot_set_cairo_default(cr, plot);
    int t,l,b,r;
    l = plot->graph_x;
    t = plot->graph_y;
    r = plot->graph_w;
    b = plot->graph_h;
    cairo_move_to(cr, l, t);
    cairo_line_to(cr, r, t);  // across top
    cairo_line_to(cr, r, b);  // down right side
    cairo_line_to(cr, l, b);  // across bottom
    cairo_line_to(cr, l, t);  // up left
    cairo_stroke(cr);
}


void shoes_plot_draw_ticks_and_labels(cairo_t *cr, shoes_plot *plot) {
    int top, left, bottom, right; // these are cairo abs for plot->graph
    int width, height;   // full plot space so it includes everything
    int range;
    int h_padding = 65; // default width of horizontal tick cell TODO: an option in plot->
    int v_padding = 25; // default height of tick TODO: an option in plot->
    left = plot->graph_x;
    top = plot->graph_y;
    right = plot->graph_w;
    bottom = plot->graph_h;
    range = plot->end_idx - plot->beg_idx;
    width = right - left;
    height = bottom - top;
    h_padding = width / plot->x_ticks;
    v_padding = height / plot->y_ticks;

    double h_scale;
    int h_interval;
    h_scale = width / (double) (range -1);
    h_interval = (int) ceil(h_padding / h_scale);

    // draw x axis - labels and tick mark uses series[0]->labels[*] - assumes it's strings
    // in the array -- TODO: allow a proc to be called to create the string. at 'i'
    int i;
    VALUE rbxser;
    shoes_chart_series *serx;
    rbxser = rb_ary_entry(plot->series, 0);
    Data_Get_Struct(rbxser, shoes_chart_series, serx);
    VALUE xobs = serx->labels;
    if (NIL_P(xobs) || TYPE(xobs) != T_ARRAY) rb_raise (rb_eArgError, "xobs must be an array");

    for (i = 0 ; i < range; i++ ) {
        int x = (int) roundl(i * h_scale);
        x += left;
        long y = bottom;
        if ((i % h_interval) == 0) {
            char *rawstr;
            VALUE rbstr = rb_ary_entry(xobs, i + plot->beg_idx);
            if (NIL_P(rbstr)) {
                rawstr = " ";
            } else {
                rawstr = RSTRING_PTR(rbstr);
            }
            //printf("x label i: %i, x: %i, y: %i, \"%s\" %i %f \n", i, (int) x, (int) y, rawstr, h_interval, h_scale);
            shoes_plot_draw_tick(cr, plot, x, y, VERTICALLY);
            if (plot->chart_type == LINE_CHART || plot->chart_type == TIMESERIES_CHART)
                shoes_plot_draw_label(cr, plot, x, y, rawstr, BELOW);
        }
    }

    int j;
    for (j = 0; j < min(2, plot->seriescnt); j++) {
        VALUE rbser = rb_ary_entry(plot->series, j);
        shoes_chart_series *cs;
        Data_Get_Struct(rbser, shoes_chart_series, cs);
        double maximum = NUM2DBL(cs->maxv);
        double minimum = NUM2DBL(cs->minv);
        double v_scale = height / (maximum - minimum);
        int v_interval = (int) ceil(v_padding / v_scale);
        char tstr[16];
        long i;
        for (i = ((long) minimum) + 1 ; i < ((long) roundl(maximum)); i = i + roundl(v_interval)) {
            int y = (int) (bottom - roundl((i - minimum) * v_scale));
            int x = 0;
            sprintf(tstr, "%i",  (int)i); // TODO user specificed format?
            if (j == 0) { // left side y presentation
                x = left;
                //printf("hoz left %i, %i, %s\n", (int)x, (int)y,tstr);
                shoes_plot_draw_tick(cr, plot, x, y, HORIZONTALLY);
                shoes_plot_draw_label(cr, plot, x, y, tstr, LEFT);
            } else {        // right side y presentation
                x = right;
                shoes_plot_draw_tick(cr, plot, x, y, HORIZONTALLY);
                shoes_plot_draw_label(cr, plot, x, y, tstr, RIGHT);
            }
        }
    }
}


void shoes_plot_draw_legend(cairo_t *cr, shoes_plot *plot) {
    int top, left, bottom, right;
    int width, height;
    left = plot->place.x;
    top = plot->graph_h + 5;
    right = plot->place.w;
    bottom = top + plot->legend_h;
    width = right - left;
    height = bottom - top;
    // TODO: Can some of this done in add/delete series ?
    // One Ugly Mess.
    int i, legend_width = 0;
    int x, y;
    int white_space = 0;
    PangoLayout *layouts[6];
    PangoLayout *space_layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (space_layout, plot->legend_pfd);
    pango_layout_set_text (space_layout, "  ", -1);
    PangoRectangle space_rect;
    pango_layout_get_pixel_extents (space_layout, NULL, &space_rect);
    white_space = space_rect.width;
    char *strary[6];
    int widary[6];
    for (i = 0; i <  6; i++) {
        strary[i] = NULL;
        widary[i] = 0;
        //layouts[i] = NULL;
    }
    for (i = 0; i < plot->seriescnt; i++) {
        if (i > 1) {
            legend_width += white_space;
        }
        VALUE cs = rb_ary_entry(plot->series, i);
        shoes_chart_series *ser;
        Data_Get_Struct(cs, shoes_chart_series, ser);
        //rbstr = rb_ary_entry(plot->long_names, i);
        strary[i] = RSTRING_PTR(ser->desc);
        layouts[i] = pango_cairo_create_layout (cr);
        pango_layout_set_font_description (layouts[i], plot->legend_pfd);
        pango_layout_set_text (layouts[i], strary[i], -1);
        PangoRectangle logical;
        pango_layout_get_pixel_extents (layouts[i], NULL, &logical);
        widary[i] = logical.width;
        legend_width += logical.width;
    }
    int xoffset = (plot->place.w / 2) - (legend_width / 2);
    x = xoffset - (plot->place.dx);
    int yhalf = (plot->legend_h / 2 );
    int yoffset = yhalf;
    y = yoffset;

    //int pos_x = plot->place.ix + x;
    int baseline = bottom - 5; //TODO: compute baseline better
    // printf("middle? w: %i, l: %i  pos_x: %i, strw: %i\n", width, left, pos_x, legend_width);
    cairo_move_to(cr, x, baseline);
    for (i = 0; i < plot->seriescnt; i++) {
        VALUE cs = rb_ary_entry(plot->series, i);
        shoes_chart_series *ser;
        Data_Get_Struct(cs, shoes_chart_series, ser);
        VALUE rbcolor = ser->color;
        shoes_color *color;
        Data_Get_Struct(rbcolor, shoes_color, color);
        cairo_set_source_rgba(cr, color->r / 255.0, color->g / 255.0,
                              color->b / 255.0, color->a / 255.0);
        pango_cairo_show_layout(cr, layouts[i]);
        g_object_unref(layouts[i]);
        x += (widary[i] +  white_space);
        cairo_move_to(cr, x, baseline);
    }
    g_object_unref(space_layout);
}

void shoes_plot_draw_tick(cairo_t *cr, shoes_plot *plot,
                          int x, int y, int orientation) {
    if (plot->auto_grid == 0) return;
    if (orientation == VERTICALLY) {
        cairo_move_to(cr, x, y);
        cairo_line_to(cr, x, plot->graph_y);
    } else if (orientation == HORIZONTALLY) {
        cairo_move_to(cr, x, y);
        cairo_line_to(cr, plot->graph_w, y);
    } else {
        printf("FAIL: shoes_plot_draw_tick  orientation\n");
    }
    cairo_stroke(cr);
}

void shoes_plot_draw_label(cairo_t *cr, shoes_plot *plot,
                           int x, int y, char *str, int where) {
    // TODO: Font was previously set to Helvetica 12 and color was setup
    // keep them for now

    cairo_font_extents_t ft; // TODO: pangocairo way
    cairo_font_extents(cr, &ft);
    int str_h = (int) ceil(ft.height);
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->label_pfd);
    pango_layout_set_text (layout, str, -1);
    PangoRectangle ct;
    pango_layout_get_pixel_extents (layout, NULL, &ct);
    int str_w = ct.width;
    int newx = 0;
    int newy = 0;
    if (where == LEFT) { // left side y-axis
        newx = x - (str_w + 3) - 1 ;
        newy = y - (str_h / 2);
    } else if (where == RIGHT) { // right side y-axis
        newx = x;
        newy = y - (str_h / 2);
        //printf("lbl rightx: %i, y: %i, %s\n", (int)newx, (int)newy, str);
    } else if (where == BELOW) { // bottom side x axis
        newx = x - (str_w / 2);
        newy = y + (str_h / 2);
    } else {
        printf("FAIL: shoes_plot_draw_label 'where ?'\n");
    }
    cairo_move_to(cr, newx, newy);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
    // printf("TODO: shoes_plot_draw_label called\n");
}



void shoes_plot_draw_title(cairo_t *cr, shoes_plot *plot) {
    char *str = RSTRING_PTR(plot->title);
    int x, y;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->title_pfd);
    pango_layout_set_text (layout, str, -1);
    PangoRectangle logical;
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    int xoffset = (plot->place.w / 2) - (logical.width / 2);
    x = xoffset - (plot->place.dx);
    int yhalf = (plot->title_h / 2 );
    int yoffset = yhalf;
    y = yoffset;
    if (plot->chart_type == PIE_CHART)
        y -= 8;
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout (cr, layout);
}

void shoes_plot_draw_caption(cairo_t *cr, shoes_plot *plot) {
    char *str = RSTRING_PTR(plot->caption);
    int x, y;
    PangoLayout *layout = pango_cairo_create_layout (cr);
    pango_layout_set_font_description (layout, plot->caption_pfd);
    pango_layout_set_text (layout, str, -1);
    PangoRectangle logical;
    pango_layout_get_pixel_extents (layout, NULL, &logical);
    int xoffset = (plot->place.w / 2) - (logical.width / 2);
    x = xoffset - (plot->place.dx);

    int yhalf = (plot->caption_h / 2 );
    int yoffset = yhalf + logical.height;
    //y = plot->place.ih;
    y = plot->place.h;
    //fprintf(stderr, "caption y = %i\n", y);
    y -= yoffset;
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout (cr, layout);
}

void shoes_plot_draw_nub(cairo_t *cr, shoes_plot *plot,  double x, double y, int nubt, int szhint ) {

    shoes_color *bgcolor;
    Data_Get_Struct(plot->background, shoes_color, bgcolor);
    switch (nubt) {
        case NUB_NONE:
            return; // probably shouldn't happen but just in case
        case NUB_DOT:
            cairo_save(cr);
            cairo_arc(cr, x, y, szhint, 0.0, 2*M_PI);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
            cairo_restore(cr);
            break;
        case NUB_CIRCLE:
            cairo_arc(cr, x, y, szhint, 0.0, 2*M_PI);
            cairo_stroke_preserve(cr);
            cairo_save(cr);
            cairo_set_source_rgba(cr, bgcolor->r / 255.0, bgcolor->g / 255.0,
                                  bgcolor->b / 255.0, 1.0);
            cairo_fill(cr);
            cairo_restore(cr);
            break;
        case NUB_BOX:
            cairo_rectangle(cr, x-1, y-1, szhint, szhint);
            cairo_stroke_preserve(cr);
            cairo_fill(cr);
            break;
        case NUB_RECT:
            cairo_rectangle(cr, x-1, y-1, szhint, szhint);
            cairo_stroke_preserve(cr);
            cairo_save(cr);
            cairo_set_source_rgba(cr, bgcolor->r / 255.0, bgcolor->g / 255.0,
                                  bgcolor->b / 255.0, 1.0);
            cairo_fill(cr);
            cairo_restore(cr);
            break;
        default: { // TODO: The hard way to draw a rect, line chart uses this until bug fixed
                int sz = 2;
                cairo_move_to(cr, x - sz, y - sz);
                cairo_line_to(cr, x + sz, y - sz);
                cairo_move_to(cr, x - sz, y - sz);
                cairo_line_to(cr, x - sz, y + sz);
                cairo_move_to(cr, x + sz, y + sz);
                cairo_line_to(cr, x + sz, y - sz);
                cairo_move_to(cr, x + sz, y + sz);
                cairo_line_to(cr, x - sz, y + sz);
                cairo_move_to(cr, x, y); // back to center point.
            }
    }
}

// initialize the default colors.  Yes it could be a chart option. TODO ?
void shoes_plot_util_default_colors(shoes_plot *plot) {
    rb_ary_store(plot->default_colors, 0, shoes_hash_get(cColors, rb_intern("blue")));
    rb_ary_store(plot->default_colors, 1, shoes_hash_get(cColors, rb_intern("red")));
    rb_ary_store(plot->default_colors, 2, shoes_hash_get(cColors, rb_intern("green")));
    rb_ary_store(plot->default_colors, 3, shoes_hash_get(cColors, rb_intern("coral")));
    rb_ary_store(plot->default_colors, 4, shoes_hash_get(cColors, rb_intern("purple")));
    rb_ary_store(plot->default_colors, 5, shoes_hash_get(cColors, rb_intern("orange")));
    rb_ary_store(plot->default_colors, 6, shoes_hash_get(cColors, rb_intern("aqua")));
    rb_ary_store(plot->default_colors, 7, shoes_hash_get(cColors, rb_intern("brown")));
    rb_ary_store(plot->default_colors, 8, shoes_hash_get(cColors, rb_intern("darkolive")));
    rb_ary_store(plot->default_colors, 9, shoes_hash_get(cColors, rb_intern("hotpink")));
    rb_ary_store(plot->default_colors, 10, shoes_hash_get(cColors, rb_intern("lightsky")));
    rb_ary_store(plot->default_colors, 11, shoes_hash_get(cColors, rb_intern("greenyellow")));
    rb_ary_store(plot->default_colors, 12, shoes_hash_get(cColors, rb_intern("gray")));
    rb_ary_store(plot->default_colors, 13, shoes_hash_get(cColors, rb_intern("black")));
}

int shoes_plot_util_quadrant(double angle) {
    if ((0 <= angle) && (angle < 0.5 * SHOES_PI)) {
        // first quadrant
        return QUAD_ONE;
    } else if ((0.5 * SHOES_PI <= angle) && (angle < SHOES_PI)) {
        // second quadrant
        return QUAD_TWO;
    } else if ((SHOES_PI <= angle) && (angle < 1.5 * SHOES_PI)) {
        // third quadrant
        return QUAD_THREE;
    } else if ((1.5 * SHOES_PI <= angle) && (angle < 2 * SHOES_PI)) {
        // fourth quadrant
        return QUAD_FOUR;
    } else {
        fprintf(stderr, "plot quadrant - bad news\n");
        return QUAD_ERR;
    }
}
