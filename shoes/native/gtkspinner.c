#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtkspinner.h"

SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg) {
    SHOES_CONTROL_REF ref = gtk_spinner_new();

    if (!NIL_P(shoes_hash_get(attr, rb_intern("start")))) {
        if (Qtrue == shoes_hash_get(attr, rb_intern("start")))
            gtk_spinner_start(GTK_SPINNER(ref));
    }

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    return ref;
}

void shoes_native_spinner_start(SHOES_CONTROL_REF ref) {
    gtk_spinner_start(GTK_SPINNER(ref));
}

void shoes_native_spinner_stop(SHOES_CONTROL_REF ref) {
    gtk_spinner_stop(GTK_SPINNER(ref));
}

gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref) {
    gboolean active;
    g_object_get (GTK_SPINNER(ref), "active", &active, NULL);
    return active;
}
