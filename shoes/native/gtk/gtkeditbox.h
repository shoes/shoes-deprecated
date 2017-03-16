#ifndef SHOES_GTK_EDIT_BOX_H
#define SHOES_GTK_EDIT_BOX_H

SHOES_CONTROL_REF shoes_native_edit_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_edit_box_get_text(SHOES_CONTROL_REF);
void shoes_native_edit_box_set_text(SHOES_CONTROL_REF, char *);
// 3.2.25 adds
void shoes_native_edit_box_append(SHOES_CONTROL_REF, char *);
void shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF);

// gtk forward declaration
extern void shoes_widget_changed(GtkWidget *ref, gpointer data);

#endif