
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "gtkscrolledwindowalt.h"

/* Private class member */
#define GTK_SCROLLED_WINDOW_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_SCROLLED_WINDOW_ALT, GtkScrolledWindow_AltPrivate))

typedef struct _GtkScrolledWindow_AltPrivate GtkScrolledWindow_AltPrivate;

struct _GtkScrolledWindow_AltPrivate {
    /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
    gchar dummy;
};

/* Forward declarations */
static void gtk_scrolled_window_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtk_scrolled_window_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);

/* Define the GtkScrolledWindow_Alt type and inherit from GtkScrolledWindow */
G_DEFINE_TYPE(GtkScrolledWindow_Alt, gtk_scrolled_window_alt, GTK_TYPE_SCROLLED_WINDOW);

/* Initialize the GtkScrolledWindow_Alt class */
static void gtk_scrolled_window_alt_class_init(GtkScrolledWindow_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtk_scrolled_window_alt_get_preferred_width;
    widget_class->get_preferred_height = gtk_scrolled_window_alt_get_preferred_height;

    /* Override GtkScrolledWindow methods */
    // TODO: determine whether gobject_class has any use.
    //GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtkScrolledWindow_AltPrivate));
}

/* Initialize a new GtkScrolledWindow_Alt instance */
static void gtk_scrolled_window_alt_init(GtkScrolledWindow_Alt *scrolledwindowAlt) {
    /* This means that GtkScrolledWindow_Alt doesn't supply its own GdkWindow */
    //gtk_widget_set_has_window(GTK_WIDGET(scrolledwindowAlt), FALSE);
    gtk_widget_set_has_window(GTK_WIDGET(scrolledwindowAlt),
                              gtk_widget_get_has_window(GTK_WIDGET(&(scrolledwindowAlt->parent_instance))));

    /* Initialize private members */
    // TODO: determine whether gobject_class has any use.
    //GtkScrolledWindow_AltPrivate *priv = GTK_SCROLLED_WINDOW_ALT_PRIVATE(scrolledwindowAlt);
}

/* Return a new GtkScrolledWindow_Alt cast to a GtkWidget */
GtkWidget *gtk_scrolled_window_alt_new(GtkAdjustment *hadjustment,
                                       GtkAdjustment *vadjustment) {
    if (hadjustment)
        g_return_val_if_fail (GTK_IS_ADJUSTMENT (hadjustment), NULL);

    if (vadjustment)
        g_return_val_if_fail (GTK_IS_ADJUSTMENT (vadjustment), NULL);

    return GTK_WIDGET(g_object_new (gtk_scrolled_window_alt_get_type(),
                                    "hadjustment", hadjustment,
                                    "vadjustment", vadjustment,
                                    NULL));
}

static void gtk_scrolled_window_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void gtk_scrolled_window_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}


