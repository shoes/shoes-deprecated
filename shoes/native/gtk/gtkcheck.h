#ifndef SHOES_GTK_CHECK_H
#define SHOES_GTK_CHECK_H

SHOES_CONTROL_REF shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
VALUE shoes_native_check_get(SHOES_CONTROL_REF ref);
void shoes_native_check_set(SHOES_CONTROL_REF ref, int on);

SYMBOL_EXTERN(checked);

// gtk forward declaration
extern gboolean shoes_button_gtk_clicked(GtkButton *button, gpointer data);
  
#endif
