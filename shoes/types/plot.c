#include "shoes/plot/plot.h"
#include "shoes/types/plot.h"

// ruby
VALUE cPlot, cChartSeries;

FUNC_M("+plot", plot, -1);
FUNC_M("+chart_series", chart_series, -1);

// The next two macros are very important for new widget writers.
CLASS_COMMON2(plot)
TRANS_COMMON(plot, 1);

void shoes_plot_init() {
    cPlot   = rb_define_class_under(cTypes, "Plot", rb_cObject); // 3.3.2
    rb_define_alloc_func(cPlot, shoes_plot_alloc);

    // methods unique to plot
    rb_define_method(cPlot, "add", CASTHOOK(shoes_plot_add), 1);
    rb_define_method(cPlot, "redraw_to", CASTHOOK(shoes_plot_redraw_to), 1);
    rb_define_method(cPlot, "delete", CASTHOOK(shoes_plot_delete), 1);
    rb_define_method(cPlot, "id",  CASTHOOK(shoes_plot_find_name), 1);
    rb_define_method(cPlot, "count", CASTHOOK(shoes_plot_get_count), 0);
    rb_define_method(cPlot, "first", CASTHOOK(shoes_plot_get_first), 0);
    rb_define_method(cPlot, "set_first", CASTHOOK(shoes_plot_set_first), 1);
    rb_define_method(cPlot, "last", CASTHOOK(shoes_plot_get_last), 0);
    rb_define_method(cPlot, "set_last", CASTHOOK(shoes_plot_set_last), 1);
    rb_define_method(cPlot, "zoom", CASTHOOK(shoes_plot_zoom), 2);
    rb_define_method(cPlot, "save_as", CASTHOOK(shoes_plot_save_as), -1);
    rb_define_method(cPlot, "near_x", CASTHOOK(shoes_plot_near), 1);

    // methods commom to many Shoes widgets
    rb_define_method(cPlot, "draw", CASTHOOK(shoes_plot_draw), 2);
    rb_define_method(cPlot, "remove", CASTHOOK(shoes_plot_remove), 0);
    rb_define_method(cPlot, "parent", CASTHOOK(shoes_plot_get_parent), 0);
    rb_define_method(cPlot, "style", CASTHOOK(shoes_plot_style), -1);
    rb_define_method(cPlot, "move", CASTHOOK(shoes_plot_move), 2);
    rb_define_method(cPlot, "displace", CASTHOOK(shoes_plot_displace), 2);
    rb_define_method(cPlot, "hide", CASTHOOK(shoes_plot_hide), 0);
    rb_define_method(cPlot, "show", CASTHOOK(shoes_plot_show), 0);
    rb_define_method(cPlot, "toggle", CASTHOOK(shoes_plot_toggle), 0);
    rb_define_method(cPlot, "hidden?", CASTHOOK(shoes_plot_is_hidden), 0);
    rb_define_method(cPlot, "click", CASTHOOK(shoes_plot_click), -1);
    rb_define_method(cPlot, "released", CASTHOOK(shoes_plot_release), -1);
    rb_define_method(cPlot, "hover", CASTHOOK(shoes_plot_hover), -1);
    rb_define_method(cPlot, "leave", CASTHOOK(shoes_plot_leave), -1);
    rb_define_method(cPlot, "top", CASTHOOK(shoes_plot_get_actual_top), 0);
    rb_define_method(cPlot, "left", CASTHOOK(shoes_plot_get_actual_left), 0);
    rb_define_method(cPlot, "width", CASTHOOK(shoes_plot_get_actual_width), 0);
    rb_define_method(cPlot, "height", CASTHOOK(shoes_plot_get_actual_height), 0);
    rb_define_method(cPlot, "transform", CASTHOOK(shoes_plot_transform), 1);
    rb_define_method(cPlot, "translate", CASTHOOK(shoes_plot_translate), 2);
    rb_define_method(cPlot, "rotate", CASTHOOK(shoes_plot_rotate), 1);
    rb_define_method(cPlot, "scale", CASTHOOK(shoes_plot_scale), -1);
    rb_define_method(cPlot, "skew", CASTHOOK(shoes_plot_skew), -1);

    cChartSeries = rb_define_class_under(cTypes, "chart_series", rb_cObject); // 3.3.2
    rb_define_alloc_func(cChartSeries, shoes_chart_series_alloc);

    //  simple getters/setters
    rb_define_method(cChartSeries, "values", CASTHOOK(shoes_chart_series_values), 0);
    rb_define_method(cChartSeries, "labels", CASTHOOK(shoes_chart_series_labels), 0);
    rb_define_method(cChartSeries, "min", CASTHOOK(shoes_chart_series_min), 0);
    rb_define_method(cChartSeries, "min=", CASTHOOK(shoes_chart_series_min_set), 1);
    rb_define_method(cChartSeries, "max", CASTHOOK(shoes_chart_series_max), 0);
    rb_define_method(cChartSeries, "max=", CASTHOOK(shoes_chart_series_max_set), 1);
    rb_define_method(cChartSeries, "name", CASTHOOK(shoes_chart_series_name), 0);
    rb_define_method(cChartSeries, "desc", CASTHOOK(shoes_chart_series_desc), 0);
    rb_define_method(cChartSeries, "desc=", CASTHOOK(shoes_chart_series_desc_set), 1);
    rb_define_method(cChartSeries, "color", CASTHOOK(shoes_chart_series_color), 0);
    rb_define_method(cChartSeries, "color=", CASTHOOK(shoes_chart_series_color_set), 1);
    rb_define_method(cChartSeries, "strokewidth", CASTHOOK(shoes_chart_series_strokewidth), 0);
    rb_define_method(cChartSeries, "strokewidth=", CASTHOOK(shoes_chart_series_strokewidth_set), 1);
    rb_define_method(cChartSeries, "points", CASTHOOK(shoes_chart_series_points), 0);
    rb_define_method(cChartSeries, "points=", CASTHOOK(shoes_chart_series_points_set), 1);

    // more complcated methods
    rb_define_method(cChartSeries, "at", CASTHOOK(shoes_chart_series_get), 1);
    rb_define_method(cChartSeries, "get", CASTHOOK(shoes_chart_series_get), 1);
    rb_define_method(cChartSeries, "set", CASTHOOK(shoes_chart_series_set), 2);

    RUBY_M("+plot", plot, -1);
    RUBY_M("+chart_series", chart_series, -1);
}

// canvas
VALUE shoes_canvas_plot(int argc, VALUE *argv, VALUE self) {
    VALUE widget;

    SETUP_CANVAS();

    widget = shoes_plot_new(argc, argv, self);
    shoes_add_ele(canvas, widget);

    return widget;
}

VALUE shoes_canvas_chart_series(int argc, VALUE *argv, VALUE self) {
    SETUP_CANVAS();

    return shoes_chart_series_new(argc, argv, self);
}