
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkprogressbaralt.h"

/* Private class member */
#define GTK_PROGRESS_BAR_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_PROGRESS_BAR_ALT, GtkProgressBar_AltPrivate))

typedef struct _GtkProgressBar_AltPrivate GtkProgressBar_AltPrivate;

struct _GtkProgressBar_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_progress_bar_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_progress_bar_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkProgressBar_Alt type and inherit from GtkProgressBar */
G_DEFINE_TYPE(GtkProgressBar_Alt, gtk_progress_bar_alt, GTK_TYPE_PROGRESS_BAR);

/* Initialize the GtkProgressBar_Alt class */
static void gtk_progress_bar_alt_class_init(GtkProgressBar_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_progress_bar_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_progress_bar_alt_get_preferred_height;

    /* Override GtkProgressBar methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkProgressBar_AltPrivate));
}

/* Initialize a new GtkProgressBar_Alt instance */
static void gtk_progress_bar_alt_init(GtkProgressBar_Alt *progressbarAlt) {
    /* This means that GtkProgressBar_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(progressbarAlt), FALSE);

    /* Initialize private members */
    // TODO: determine whether priv has any use.
    //GtkProgressBar_AltPrivate *priv = GTK_PROGRESS_BAR_ALT_PRIVATE(progressbarAlt);

}

/* Return a new GtkProgressBar_Alt cast to a GtkWidget */
GtkWidget *gtk_progress_bar_alt_new() {
    return GTK_WIDGET(g_object_new(gtk_progress_bar_alt_get_type(), NULL));
}

static void gtk_progress_bar_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void gtk_progress_bar_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}


