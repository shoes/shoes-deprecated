#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_PROGRESS_TYPE_H
#define SHOES_PROGRESS_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// forward progress.c's own s_progress, other components are using it.
ID s_progress;

// native forward declarations
SHOES_CONTROL_REF shoes_native_progress(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
double shoes_native_progress_get_fraction(SHOES_CONTROL_REF);
void shoes_native_progress_set_fraction(SHOES_CONTROL_REF, double);

/* each widget should have its own init function */
void shoes_progress_init();

// ruby
VALUE shoes_progress_draw(VALUE, VALUE, VALUE);
VALUE shoes_progress_get_fraction(VALUE self);
VALUE shoes_progress_set_fraction(VALUE self, VALUE _perc);

// canvas
VALUE shoes_canvas_progress(int, VALUE *, VALUE);

#endif
