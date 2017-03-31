#include "shoes/ruby.h"

#ifndef SHOES_GTK_SWITCH_H
#define SHOES_GTK_SWITCH_H

// There is some magical way to create this but let's do this manually for now
SYMBOL_EXTERN(active);

SHOES_CONTROL_REF shoes_native_switch(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, gboolean activate);
gboolean shoes_native_switch_get_active(SHOES_CONTROL_REF ref);
void shoes_native_activate(GObject *switcher, GParamSpec *pspec, gpointer data);

// EVENT_COMMON generated functions
VALUE shoes_control_active(int argc, VALUE *argv, VALUE self);
  
#endif
