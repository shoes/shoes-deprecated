#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "shoes/native/gtk.h"
#include "shoes/native/gtk/gtkscrolledwindowalt.h"
#include "shoes/native/gtk/gtkeditbox.h"

SHOES_CONTROL_REF shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    GtkTextBuffer *buffer;
    GtkWidget* textview = gtk_text_view_new();
    SHOES_CONTROL_REF ref = gtk_scrolled_window_alt_new(NULL, NULL);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ref),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(ref), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(ref), textview);

    g_signal_connect(G_OBJECT(buffer), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);

    return ref;
}

VALUE shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter begin, end;
    gtk_text_buffer_get_bounds(buffer, &begin, &end);
    return rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
}

void shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
}

void shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    GtkTextIter begin, end;
    gtk_text_buffer_get_bounds(buffer, &begin, &end);
    gtk_text_buffer_insert(buffer, &end, msg, strlen(msg));
    gtk_text_buffer_get_bounds(buffer, &begin, &end);

}

void shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref) {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    GtkTextIter end;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_get_end_iter (buffer, &end);
    /* get the current ( cursor )mark name */
    GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);

    /* move mark and selection bound to the end */
    gtk_text_buffer_place_cursor(buffer, &end);

    /* scroll to the end view */
    gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (textview),
                                  insert_mark, 0.0, TRUE, 0.0, 1.0);
}