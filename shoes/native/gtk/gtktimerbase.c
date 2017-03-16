#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtktimerbase.h"

static gboolean shoes_gtk_animate(gpointer data) {
    VALUE timer = (VALUE)data;
    shoes_timer *self_t;
    Data_Get_Struct(timer, shoes_timer, self_t);
    if (self_t->started == ANIM_STARTED)
        shoes_timer_call(timer);
    return self_t->started == ANIM_STARTED;
}

void shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref) {
    g_source_remove(ref);
}

SHOES_TIMER_REF shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval) {
    return g_timeout_add(interval, shoes_gtk_animate, (gpointer)self);
}