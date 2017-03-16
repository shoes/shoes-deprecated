#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"

#ifndef SHOES_TIMERBASE_H
#define SHOES_TIMERBASE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// native forward declarations
extern SHOES_TIMER_REF shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval);
extern void shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref);

/* each widget should have its own init function */
void shoes_timerbase_init();

// ruby
VALUE shoes_timer_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_timer_stop(VALUE self);
VALUE shoes_timer_start(VALUE self);
VALUE shoes_timer_toggle(VALUE self);

void shoes_timer_call(VALUE self);
void shoes_timer_mark(shoes_timer *timer);
void shoes_timer_free(shoes_timer *timer);
VALUE shoes_timer_new(VALUE klass, VALUE rate, VALUE block, VALUE parent);
VALUE shoes_timer_alloc(VALUE klass);
VALUE shoes_timer_remove(VALUE self);


// canvas
VALUE shoes_canvas_animate(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_every(int argc, VALUE *argv, VALUE self);
VALUE shoes_canvas_timer(int argc, VALUE *argv, VALUE self);

#endif
