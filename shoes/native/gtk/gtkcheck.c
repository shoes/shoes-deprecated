#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkcheck.h"

SHOES_CONTROL_REF shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_check_button_new();
    // set visual state before connecting signal
    if (RTEST(ATTR(attr, checked))) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ref), TRUE);
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);
    return ref;
}

VALUE shoes_native_check_get(SHOES_CONTROL_REF ref) {
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref)) ? Qtrue : Qfalse;
}

void shoes_native_check_set(SHOES_CONTROL_REF ref, int on) {
    // bug264 - don't toggle if already set to desired state.
    gboolean new_state;
    new_state = on ? TRUE : FALSE;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref)) != new_state)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ref), new_state);
}
