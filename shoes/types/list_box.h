#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_LIST_BOX_H
#define SHOES_LIST_BOX_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

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
