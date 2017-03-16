#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SWITCH_H
#define SHOES_SWITCH_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;
extern ID s_active;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_active(int argc, VALUE *argv, VALUE self);

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_switch(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
extern void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, gboolean activate);
extern gboolean shoes_native_switch_get_active(SHOES_CONTROL_REF ref);
extern void shoes_native_activate(GObject *switcher, GParamSpec *pspec, gpointer data);

/* each widget should have its own init function */
void shoes_switch_init();

// ruby
VALUE shoes_switch_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_switch_get_active(VALUE self);
VALUE shoes_switch_set_active(VALUE self, VALUE activate);

// canvas
VALUE shoes_canvas_switch(int argc, VALUE *argv, VALUE self);

#endif
