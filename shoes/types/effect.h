#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_EFFECT_H
#define SHOES_EFFECT_H

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

/* each widget should have its own init function */
void shoes_effect_init();

// ruby
VALUE shoes_effect_new(ID name, VALUE attr, VALUE parent);
VALUE shoes_effect_alloc(VALUE klass);
VALUE shoes_effect_draw(VALUE self, VALUE c, VALUE actual);

void shoes_effect_mark(shoes_effect *fx);
void shoes_effect_free(shoes_effect *fx);
shoes_effect_filter shoes_effect_for_type(ID name);

// canvas
VALUE shoes_add_effect(VALUE self, ID name, VALUE attr);

#endif
