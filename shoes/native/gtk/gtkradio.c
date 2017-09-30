#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtkradio.h"

SHOES_CONTROL_REF shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group) {
    SHOES_CONTROL_REF ref;
    GSList *list = NULL;
    
    if (!NIL_P(group)) {
        shoes_control *lctrl;
        VALUE leader = rb_ary_entry(group, 0);
        Data_Get_Struct(leader, shoes_control, lctrl);
        list = gtk_radio_button_get_group(GTK_RADIO_BUTTON(lctrl->ref));
    }

    ref = gtk_radio_button_new(list);

    if (!NIL_P(shoes_hash_get(attr, rb_intern("tooltip")))) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(ref), RSTRING_PTR(shoes_hash_get(attr, rb_intern("tooltip"))));
    }

    g_signal_connect(G_OBJECT(ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);
    return ref;
}
