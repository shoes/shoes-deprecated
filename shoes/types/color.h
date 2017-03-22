#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_COLOR_H
#define SHOES_COLOR_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

/* Should be automatically available but ruby.c is not sharing enough information */
extern void shoes_pattern_gradient(shoes_pattern *pattern, VALUE r1, VALUE r2, VALUE attr);

// native forward declarations

VALUE cColor, cColors;

typedef struct {
    unsigned char r, g, b, a, on;
} shoes_color;

#define SHOES_COLOR_OPAQUE 0xFF
#define SHOES_COLOR_TRANSPARENT 0x0
#define SHOES_COLOR_DARK   (0x55 * 3)
#define SHOES_COLOR_LIGHT  (0xAA * 3)

#define DEF_COLOR(name, r, g, b) rb_hash_aset(cColors, ID2SYM(rb_intern("" # name)), shoes_color_new(r, g, b, 255))
#define NEW_COLOR(v, o) \
  shoes_color *v; \
  VALUE o = shoes_color_alloc(cColor); \
  Data_Get_Struct(o, shoes_color, v)

/* each widget should have its own init function */
void shoes_color_init();

// ruby
void shoes_color_mark(shoes_color *color);
void shoes_color_free(shoes_color *color);
VALUE shoes_color_new(int r, int g, int b, int a);
VALUE shoes_color_alloc(VALUE klass);
VALUE shoes_color_rgb(int argc, VALUE *argv, VALUE self);
VALUE shoes_color_gradient(int argc, VALUE *argv, VALUE self);
VALUE shoes_color_gray(int argc, VALUE *argv, VALUE self);
cairo_pattern_t *shoes_color_pattern(VALUE self);
void shoes_color_grad_stop(cairo_pattern_t *pattern, double stop, VALUE self);
VALUE shoes_color_args(int argc, VALUE *argv, VALUE self);
VALUE shoes_color_parse(VALUE self, VALUE source);
VALUE shoes_color_spaceship(VALUE self, VALUE c2);
VALUE shoes_color_equal(VALUE self, VALUE c2);
VALUE shoes_color_get_red(VALUE self);
VALUE shoes_color_get_green(VALUE self);
VALUE shoes_color_get_blue(VALUE self);
VALUE shoes_color_get_alpha(VALUE self);
VALUE shoes_color_is_black(VALUE self);
VALUE shoes_color_is_dark(VALUE self);
VALUE shoes_color_is_light(VALUE self);
VALUE shoes_color_is_opaque(VALUE self);
VALUE shoes_color_is_transparent(VALUE self);
VALUE shoes_color_is_white(VALUE self);
VALUE shoes_color_invert(VALUE self);
VALUE shoes_color_to_s(VALUE self);
VALUE shoes_color_to_pattern(VALUE self);
VALUE shoes_color_method_missing(int argc, VALUE *argv, VALUE self);

// canvas

#endif
