#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SPINNER_TYPE_H
#define SHOES_SPINNER_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
extern void shoes_native_spinner_start(SHOES_CONTROL_REF ref);
extern void shoes_native_spinner_stop(SHOES_CONTROL_REF ref);
extern gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref);

/* each widget should have its own init function */
void shoes_spinner_init();

// ruby
VALUE shoes_spinner_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_spinner_start(VALUE self);
VALUE shoes_spinner_stop(VALUE self);
VALUE shoes_spinner_started(VALUE self);

// canvas
VALUE shoes_canvas_spinner(int argc, VALUE *argv, VALUE self);

#endif
