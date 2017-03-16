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

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_edit_line(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
extern VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF);
extern void shoes_native_edit_line_set_text(SHOES_CONTROL_REF, char *);
extern VALUE shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF);
extern void shoes_native_enterkey(GtkWidget *ref, gpointer data);

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
