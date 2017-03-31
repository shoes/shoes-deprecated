#ifndef SHOES_GTK_RADIO_H
#define SHOES_GTK_RADIO_H

SHOES_CONTROL_REF shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group);

// gtk forward declaration
extern gboolean shoes_button_gtk_clicked(GtkButton *button, gpointer data);
  
#endif
