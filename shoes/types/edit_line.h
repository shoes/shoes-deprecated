#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_EDIT_LINE_H
#define SHOES_EDIT_LINE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

/* each widget should have its own init function */
void shoes_edit_line_init();

// ruby
VALUE shoes_edit_line_get_text(VALUE self);
VALUE shoes_edit_line_set_text(VALUE self, VALUE text);
VALUE shoes_edit_line_enterkey(VALUE self, VALUE proc);
VALUE shoes_edit_line_cursor_to_end(VALUE self);
VALUE shoes_edit_line_draw(VALUE self, VALUE c, VALUE actual);

// canvas
VALUE shoes_canvas_edit_line(int argc, VALUE *argv, VALUE self);

#endif
