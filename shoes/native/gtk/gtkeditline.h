#ifndef SHOES_GTK_EDIT_LINE_H
#define SHOES_GTK_EDIT_LINE_H

SHOES_CONTROL_REF shoes_native_edit_line(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF);
void shoes_native_edit_line_set_text(SHOES_CONTROL_REF, char *);
VALUE shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF);
void shoes_native_enterkey(GtkWidget *ref, gpointer data);

// gtk forward declaration
extern void shoes_widget_changed(GtkWidget *ref, gpointer data);
extern void shoes_native_secrecy(SHOES_CONTROL_REF ref);

#endif
