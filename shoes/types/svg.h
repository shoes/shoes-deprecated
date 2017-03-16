#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SVG_H
#define SHOES_SVG_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// SvgHandle struct - not a graphical widget
// new in 3.3.0
typedef struct _svghandle {
    RsvgHandle *handle;
    RsvgDimensionData svghdim;
    RsvgPositionData svghpos;
    char *path;
    char *data;
    char *subid;
    double aspect;
} shoes_svghandle;

//
// SVG struct
//
typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    double scalew;
    double scaleh;
    VALUE svghandle;
    char hover;
    shoes_transform *st;
} shoes_svg;

/* each widget should have its own init function */
void shoes_svg_init();

// ruby (svg)
VALUE shoes_svg_new(int, VALUE *, VALUE);
VALUE shoes_svg_alloc(VALUE);
VALUE shoes_svg_draw(VALUE, VALUE, VALUE);
VALUE shoes_svg_get_handle(VALUE);
VALUE shoes_svg_set_handle(VALUE, VALUE);
VALUE shoes_svg_get_dpi(VALUE);
VALUE shoes_svg_set_dpi(VALUE, VALUE);
VALUE shoes_svg_export(VALUE, VALUE);
VALUE shoes_svg_save(VALUE, VALUE);
VALUE shoes_svg_show(VALUE);
VALUE shoes_svg_hide(VALUE);
VALUE shoes_svg_get_actual_width(VALUE);
VALUE shoes_svg_get_actual_height(VALUE);
VALUE shoes_svg_get_actual_left(VALUE);
VALUE shoes_svg_get_actual_top(VALUE);
VALUE shoes_svg_get_parent(VALUE);
VALUE shoes_svg_get_offsetX(VALUE);
VALUE shoes_svg_get_offsetY(VALUE);
VALUE shoes_svg_preferred_height(VALUE);
VALUE shoes_svg_preferred_width(VALUE);
VALUE shoes_svg_remove(VALUE);
VALUE shoes_svg_has_group(VALUE, VALUE);

// ruby (svghandle)
VALUE shoes_svghandle_new(int argc, VALUE *argv, VALUE self);
VALUE shoes_svghandle_alloc(VALUE);
VALUE shoes_svghandle_get_width(VALUE);
VALUE shoes_svghandle_get_height(VALUE);
VALUE shoes_svghandle_has_group(VALUE, VALUE);

// canvas
VALUE shoes_canvas_svg(int, VALUE *, VALUE);
VALUE shoes_svg_motion(VALUE, int, int, char *);
VALUE shoes_svg_send_click(VALUE, int, int, int);
void shoes_svg_send_release(VALUE, int, int, int);

VALUE shoes_canvas_svghandle(int, VALUE *, VALUE);

#endif
