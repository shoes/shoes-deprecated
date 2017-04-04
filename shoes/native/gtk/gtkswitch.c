#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkswitch.h"


SHOES_CONTROL_REF shoes_native_switch(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_switch_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("active")))) {
        gtk_switch_set_active(GTK_SWITCH(ref), shoes_hash_get(attr, rb_intern("active")) == Qtrue);
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "notify::active",
                     G_CALLBACK(shoes_native_activate),
                     (gpointer)self);

    return ref;
}

void shoes_native_switch_set_active(SHOES_CONTROL_REF ref, gboolean activate) {
    gtk_switch_set_active(GTK_SWITCH(ref), activate);
}

gboolean shoes_native_switch_get_active(SHOES_CONTROL_REF ref) {
    return gtk_switch_get_active(GTK_SWITCH(ref));
}

void shoes_native_activate(GObject *switcher, GParamSpec *pspec, gpointer data) {
    VALUE self = (VALUE)data;

    shoes_control_send(self, s_active);
}
