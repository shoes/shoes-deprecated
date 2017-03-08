#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_EDIT_BOX_H
#define SHOES_EDIT_BOX_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

/* each widget should have its own init function */
void shoes_edit_box_init();

// ruby
VALUE shoes_edit_box_get_text(VALUE self);
VALUE shoes_edit_box_set_text(VALUE self, VALUE text);
VALUE shoes_edit_box_append(VALUE self, VALUE text);
VALUE shoes_edit_box_scroll_to_end(VALUE self);
VALUE shoes_edit_box_draw(VALUE self, VALUE c, VALUE actual);

// canvas
VALUE shoes_canvas_edit_box(int, VALUE *, VALUE);

#endif
