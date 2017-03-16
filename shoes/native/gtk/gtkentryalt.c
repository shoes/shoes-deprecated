

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "gtkentryalt.h"

/* Private class member */
#define GTKENTRY_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	GTK_TYPE_ENTRY_ALT, GtkEntry_AltPrivate))

typedef struct _GtkEntry_AltPrivate GtkEntry_AltPrivate;

struct _GtkEntry_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_entry_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_entry_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkEntry_Alt type and inherit from GtkEntry */
G_DEFINE_TYPE(GtkEntry_Alt, gtk_entry_alt, GTK_TYPE_ENTRY);

/* Initialize the GtkEntry_Alt class */
static void
gtk_entry_alt_class_init(GtkEntry_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_entry_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_entry_alt_get_preferred_height;

    /* Override GtkEntry methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    //GtkEntryClass *entry_class = GTK_ENTRY_CLASS(klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkEntry_AltPrivate));
}

/* Initialize a new GtkEntry_Alt instance */
static void
gtk_entry_alt_init(GtkEntry_Alt *entryAlt) {
    /* This means that GtkEntry_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(entryAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkEntry_AltPrivate *priv = GTKENTRY_ALT_PRIVATE(entryAlt);

}

/* Return a new GtkEntry_Alt cast to a GtkWidget */
GtkWidget *
gtk_entry_alt_new() {
    return GTK_WIDGET(g_object_new(gtk_entry_alt_get_type(), NULL));
}


static void
gtk_entry_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void
gtk_entry_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}
