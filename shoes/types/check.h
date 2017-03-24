#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_CHECK_H
#define SHOES_CHECK_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

SYMBOL_ID(checked);

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_click(int argc, VALUE *argv, VALUE self);

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
extern VALUE shoes_native_check_get(SHOES_CONTROL_REF ref);
extern void shoes_native_check_set(SHOES_CONTROL_REF ref, int on);

/* each widget should have its own init function */
void shoes_check_init();

// ruby
VALUE shoes_check_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_check_is_checked(VALUE self);
VALUE shoes_check_set_checked(VALUE self, VALUE on);

// canvas
VALUE shoes_canvas_check(int, VALUE *, VALUE);

#endif
