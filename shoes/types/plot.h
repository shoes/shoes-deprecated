#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_PLOT_H
#define SHOES_PLOT_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// native forward declarations

/* each widget should have its own init function */
void shoes_plot_init();

// ruby

// canvas
VALUE shoes_canvas_plot(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_chart_series(int argc, VALUE *argv, VALUE self);

#endif
