#ifndef SHOES_GTK_TEXT_VIEW_H
#define SHOES_GTK_TEXT_VIEW_H

SHOES_CONTROL_REF shoes_native_text_view(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
VALUE shoes_native_text_view_get_text(SHOES_CONTROL_REF ref);
void shoes_native_text_view_set_text(SHOES_CONTROL_REF ref, char *msg);
VALUE shoes_native_text_view_append(SHOES_CONTROL_REF ref, char *msg);

// gtk forward declaration
extern void shoes_widget_changed(GtkWidget *ref, gpointer data);
  
#endif