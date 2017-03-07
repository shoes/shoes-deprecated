#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_SLIDER_H
#define SHOES_SLIDER_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

/* each widget should have its own init function */
void shoes_slider_init();

// ruby
VALUE shoes_slider_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_slider_get_fraction(VALUE self);
VALUE shoes_slider_set_fraction(VALUE self, VALUE _perc);

// canvas
VALUE shoes_canvas_slider(int, VALUE *, VALUE);

#endif
