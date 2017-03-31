#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_NATIVE_TYPE_H
#define SHOES_NATIVE_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

VALUE cNative;

#define CONTROL_NORMAL   0
#define CONTROL_READONLY 1
#define CONTROL_DISABLED 2

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    SHOES_CONTROL_REF ref;
} shoes_control;

/* each widget should have its own init function */
void shoes_0_native_type_init();

// ruby
void shoes_control_mark(shoes_control *control);
void shoes_control_free(shoes_control *control);
VALUE shoes_control_new(VALUE klass, VALUE attr, VALUE parent);
VALUE shoes_control_alloc(VALUE klass);
VALUE shoes_control_focus(VALUE self);
VALUE shoes_control_get_state(VALUE self);
VALUE shoes_control_set_state(VALUE self, VALUE state);
VALUE shoes_control_temporary_hide(VALUE self);
VALUE shoes_control_hide(VALUE self);
VALUE shoes_control_temporary_show(VALUE self);
VALUE shoes_control_show(VALUE self);
VALUE shoes_control_remove(VALUE self);
void shoes_control_check_styles(shoes_control *self_t);
void shoes_control_send(VALUE self, ID event);
VALUE shoes_control_get_tooltip(VALUE self);
VALUE shoes_control_set_tooltip(VALUE self, VALUE tooltip);
void shoes_control_hide_ref(SHOES_CONTROL_REF ref);
void shoes_control_show_ref(SHOES_CONTROL_REF ref);

// canvas
VALUE shoes_canvas_hide(VALUE self);
VALUE shoes_canvas_show(VALUE self) ;

#endif
