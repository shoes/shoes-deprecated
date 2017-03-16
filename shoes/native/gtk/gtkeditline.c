#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "shoes/native/gtk/gtkentryalt.h"
#include "shoes/native/gtk/gtkeditline.h"

SHOES_CONTROL_REF shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_entry_alt_new();

    if (RTEST(ATTR(attr, secret))) shoes_native_secrecy(ref);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    gtk_entry_set_text(GTK_ENTRY(ref), _(msg));

    g_signal_connect(G_OBJECT(ref), "changed",
                     G_CALLBACK(shoes_widget_changed),
                     (gpointer)self);
    // cjc: try to intercept \n  bug 860 @ shoes4
    g_signal_connect(G_OBJECT(ref), "activate",
                     G_CALLBACK(shoes_native_enterkey), // fix name?
                     (gpointer)self);

    return ref;
}

VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref) {
    return rb_str_new2(gtk_entry_get_text(GTK_ENTRY(ref)));
}

void shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg) {
    gtk_entry_set_text(GTK_ENTRY(ref), _(msg));
}

VALUE shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF ref) {
    gtk_editable_set_position(GTK_EDITABLE(ref), -1);
    return Qnil;
}

void shoes_native_enterkey(GtkWidget *ref, gpointer data) {
    VALUE self = (VALUE)data;
    GET_STRUCT(control, self_t);
    VALUE click = ATTR(self_t->attr, donekey);
    if (!NIL_P(click)) {
        shoes_safe_block(self_t->parent, click, rb_ary_new3(1, self));
    }
}