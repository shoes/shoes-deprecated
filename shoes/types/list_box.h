#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_LIST_BOX_H
#define SHOES_LIST_BOX_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_list_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
extern void shoes_native_list_box_update(SHOES_CONTROL_REF, VALUE);
extern VALUE shoes_native_list_box_get_active(SHOES_CONTROL_REF, VALUE);
extern void shoes_native_list_box_set_active(SHOES_CONTROL_REF, VALUE, VALUE);

/* each widget should have its own init function */
void shoes_list_box_init();

// ruby
VALUE shoes_list_box_choose(VALUE self, VALUE item);
VALUE shoes_list_box_text(VALUE self);
VALUE shoes_list_box_items_get(VALUE self);
VALUE shoes_list_box_items_set(VALUE self, VALUE items);
VALUE shoes_list_box_draw(VALUE self, VALUE c, VALUE actual);

// canvas
VALUE shoes_canvas_list_box(int, VALUE *, VALUE);

#endif
