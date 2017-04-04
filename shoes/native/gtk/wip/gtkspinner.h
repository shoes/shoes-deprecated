#ifndef SHOES_GTK_SPINNER_H
#define SHOES_GTK_SPINNER_H

SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
void shoes_native_spinner_start(SHOES_CONTROL_REF ref);
void shoes_native_spinner_stop(SHOES_CONTROL_REF ref);
gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref);
  
#endif