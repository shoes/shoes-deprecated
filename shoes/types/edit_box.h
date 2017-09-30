#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_EDIT_BOX_TYPE_H
#define SHOES_EDIT_BOX_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_edit_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
extern VALUE shoes_native_edit_box_get_text(SHOES_CONTROL_REF);
extern void shoes_native_edit_box_set_text(SHOES_CONTROL_REF, char *);
// 3.2.25 adds
extern void shoes_native_edit_box_append(SHOES_CONTROL_REF, char *);
extern void shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF);

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
