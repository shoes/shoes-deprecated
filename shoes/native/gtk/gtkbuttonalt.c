
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkbuttonalt.h"

/* Private class member */
#define GTK_BUTTON_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_BUTTON_ALT, GtkButton_AltPrivate))

typedef struct _GtkButton_AltPrivate GtkButton_AltPrivate;

struct _GtkButton_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_button_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_button_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkButton_Alt type and inherit from GtkButton */
G_DEFINE_TYPE(GtkButton_Alt, gtk_button_alt, GTK_TYPE_BUTTON);

/* Initialize the GtkButton_Alt class */
static void gtk_button_alt_class_init(GtkButton_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_button_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_button_alt_get_preferred_height;

    /* Override GtkButton methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkButton_AltPrivate));
}

/* Initialize a new GtkButton_Alt instance */
static void gtk_button_alt_init(GtkButton_Alt *buttontAlt) {
    /* This means that GtkButton_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(buttontAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkButton_AltPrivate *priv = GTK_BUTTON_ALT_PRIVATE(buttontAlt);

}

/* Return a new GtkButton_Alt cast to a GtkWidget */
GtkWidget *gtk_button_alt_new() {
    return GTK_WIDGET(g_object_new(gtk_button_alt_get_type(), NULL));
}

GtkWidget *gtk_button_alt_new_with_label(const gchar *label) {
    return GTK_WIDGET(g_object_new (gtk_button_alt_get_type(), "label", label, NULL));
}

static void gtk_button_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void gtk_button_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}
