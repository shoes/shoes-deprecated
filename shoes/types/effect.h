#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include <math.h>

#ifndef SHOES_EFFECT_TYPE_H
#define SHOES_EFFECT_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

typedef void (*shoes_effect_filter)(cairo_t *, VALUE attr, shoes_place *);

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    shoes_effect_filter filter;
} shoes_effect;

// TODO: this needs refactoring, perhaps should go to native directory?
#ifdef GTK3
#define RAW_FILTER_START(place) \
  int width, height, stride; \
  guchar *out; \
  static const cairo_user_data_key_t key; \
  cairo_surface_t *source = cairo_get_target(cr); \
  cairo_surface_t *target; \
  unsigned char *in = cairo_image_surface_get_data(source); \
  \
  place->x = place->y = 0; \
  place->w = width  = cairo_image_surface_get_width(source); \
  place->h = height = cairo_image_surface_get_height(source); \
  stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width); \
  \
  out = (guchar *)g_malloc(4 * width * height); \
  target = cairo_image_surface_create_for_data((unsigned char *)out, \
    CAIRO_FORMAT_ARGB32, \
    width, height, stride); \
  cairo_surface_set_user_data(target, &key, out, (cairo_destroy_func_t)g_free); \
  unsigned int len = 4 * width * height
#else
#define RAW_FILTER_START(place) \
  int width, height, stride; \
  guchar *out; \
  static const cairo_user_data_key_t key; \
  cairo_surface_t *source = cairo_get_target(cr); \
  cairo_surface_t *target; \
  unsigned char *in = cairo_image_surface_get_data(source); \
  \
  place->x = place->y = 0; \
  place->w = width  = cairo_image_surface_get_width(source); \
  place->h = height = cairo_image_surface_get_height(source); \
  stride = cairo_image_surface_get_stride(source); \
  \
  out = (guchar *)g_malloc(4 * width * height); \
  target = cairo_image_surface_create_for_data((unsigned char *)out, \
    CAIRO_FORMAT_ARGB32, \
    width, height, 4 * width); \
  cairo_surface_set_user_data(target, &key, out, (cairo_destroy_func_t)g_free); \
  unsigned int len = 4 * width * height
#endif

#define RAW_FILTER_END() \
  cairo_set_operator(cr, CAIRO_OPERATOR_CLEAR); \
  cairo_paint(cr); \
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER); \
  cairo_set_source_surface(cr, target, 0, 0); \
  cairo_paint(cr); \
  cairo_surface_destroy(target);

/* each widget should have its own init function */
void shoes_effect_init();

// ruby
VALUE shoes_effect_new(ID name, VALUE attr, VALUE parent);
VALUE shoes_effect_alloc(VALUE klass);
VALUE shoes_effect_draw(VALUE self, VALUE c, VALUE actual);

void shoes_effect_mark(shoes_effect *fx);
void shoes_effect_free(shoes_effect *fx);
shoes_effect_filter shoes_effect_for_type(ID name);

void shoes_gaussian_blur_filter(cairo_t *, VALUE, shoes_place *);
void shoes_shadow_filter(cairo_t *, VALUE, shoes_place *);
void shoes_glow_filter(cairo_t *, VALUE, shoes_place *);

// canvas
VALUE shoes_add_effect(VALUE self, ID name, VALUE attr);
VALUE shoes_canvas_blur(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_glow(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_shadow(int argc, VALUE *argv, VALUE self);

#endif
