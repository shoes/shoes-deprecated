#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_BUTTON_H
#define SHOES_BUTTON_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_click(int argc, VALUE *argv, VALUE self);
extern SHOES_CONTROL_REF shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);

/* each widget should have its own init function */
void shoes_button_init();

// ruby
VALUE shoes_button_draw(VALUE self, VALUE c, VALUE actual);
void shoes_button_send_click(VALUE control);

// canvas
VALUE shoes_canvas_button(int argc, VALUE *argv, VALUE self);

#endif
