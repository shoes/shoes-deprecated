#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_SHAPE_TYPE_H
#define SHOES_SHAPE_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

VALUE cShape;

SYMBOL_ID(shape);

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    ID name;
    char hover;
    cairo_path_t *line;
    shoes_transform *st;
} shoes_shape;

// native forward declarations

/* each widget should have its own init function */
void shoes_shape_init();

// ruby
VALUE shoes_shape_draw(VALUE self, VALUE c, VALUE actual);
void shoes_shape_mark(shoes_shape *path);
void shoes_shape_free(shoes_shape *path);
VALUE shoes_shape_attr(int argc, VALUE *argv, int syms, ...);
unsigned char shoes_shape_check(cairo_t *cr, shoes_place *place);
void shoes_shape_sketch(cairo_t *cr, ID name, shoes_place *place, shoes_transform *st, VALUE attr, cairo_path_t* line, unsigned char draw);
VALUE shoes_shape_new(VALUE parent, ID name, VALUE attr, shoes_transform *st, cairo_path_t *line);
VALUE shoes_shape_alloc(VALUE klass);
VALUE shoes_shape_motion(VALUE self, int x, int y, char *touch);
VALUE shoes_shape_send_click(VALUE self, int button, int x, int y);
void shoes_shape_send_release(VALUE self, int button, int x, int y);

// canvas
VALUE shoes_canvas_shape(int argc, VALUE *argv, VALUE self);
VALUE shoes_add_shape(VALUE self, ID name, VALUE attr, cairo_path_t *line);
VALUE shoes_canvas_arc(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_rect(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_oval(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_line(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_arrow(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_star(int argc, VALUE *argv, VALUE self);
  
#endif
