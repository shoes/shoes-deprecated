#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_RADIO_H
#define SHOES_RADIO_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

SYMBOL_EXTERN(checked);

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_click(int argc, VALUE *argv, VALUE self);

// native forward declarations
SHOES_CONTROL_REF shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group);

VALUE cRadio;

/* each widget should have its own init function */
void shoes_radio_init();

// ruby
VALUE shoes_radio_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_check_set_checked_m(VALUE self, VALUE on);
void shoes_radio_button_click(VALUE control);

// reuse code from check type
extern VALUE shoes_check_is_checked(VALUE self);
extern VALUE shoes_check_set_checked(VALUE self, VALUE on);

// canvas
VALUE shoes_canvas_radio(int, VALUE *, VALUE);

#endif
