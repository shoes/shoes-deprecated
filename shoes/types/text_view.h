#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_TEXT_VIEW_H
#define SHOES_TEXT_VIEW_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern VALUE shoes_control_change(int argc, VALUE *argv, VALUE self);

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_text_view(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
extern VALUE shoes_native_text_view_get_text(SHOES_CONTROL_REF ref);
extern void shoes_native_text_view_set_text(SHOES_CONTROL_REF ref, char *msg);
extern VALUE shoes_native_text_view_append(SHOES_CONTROL_REF ref, char *msg);

/* each widget should have its own init function */
void shoes_text_view_init();

// ruby
VALUE shoes_text_view_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_text_view_get_text(VALUE self);
VALUE shoes_text_view_set_text(VALUE self, VALUE text);
VALUE shoes_text_view_append (VALUE self, VALUE text);
VALUE shoes_text_view_insert (VALUE self, VALUE args);
VALUE shoes_text_view_delete( VALUE self, VALUE args);
VALUE shoes_text_view_get(VALUE self, VALUE args);
VALUE shoes_text_view_create_insertion(VALUE self, VALUE args);
VALUE shoes_text_view_current_insertion(VALUE self);
VALUE shoes_text_view_scroll_to_insertion(VALUE seff, VALUE insert_pt);
VALUE shoes_text_view_scroll_to_end (VALUE self);

// canvas
VALUE shoes_canvas_text_view(int, VALUE *, VALUE);

#endif
