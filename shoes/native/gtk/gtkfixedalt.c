/*
 * Thanks to Philip Chimento
 * http://ptomato.name/advanced-gtk-techniques/html/custom-container.html
*/

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/internal.h"

#include "gtkfixedalt.h"

/* Private class member */
#define GTKFIXED_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	GTKFIXED_ALT_TYPE, GtKFixed_AltPrivate))

typedef struct _GtKFixed_AltPrivate GtKFixed_AltPrivate;

struct _GtKFixed_AltPrivate {
    GList *children;
};

/* Forward declarations */
static void gtkfixed_alt_get_preferred_width(GtkWidget *widget,
        int *minimal, int *natural);
static void gtkfixed_alt_get_preferred_height(GtkWidget *widget,
        int *minimal, int *natural);
//static void gtkfixed_alt_size_allocate(GtkWidget *widget,
//                                    GtkAllocation *allocation);
//static GType gtkfixed_alt_child_type(GtkContainer *container);

/* Define the GtKFixed_Alt type and inherit from GtkFixed */
G_DEFINE_TYPE(GtKFixed_Alt, gtkfixed_alt, GTK_TYPE_FIXED);


/* Initialize the GtKFixed_Alt class */
static void
gtkfixed_alt_class_init(GtKFixed_AltClass *klass) {
    /* Override GtkWidget methods */
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    widget_class->get_preferred_width = gtkfixed_alt_get_preferred_width;
    widget_class->get_preferred_height = gtkfixed_alt_get_preferred_height;
    //widget_class->size_allocate = gtkfixed_alt_size_allocate;

    /* Override GtkFixed methods */
    // TODO: determine whether fixed_class has any use.
    //GtkFixedClass *fixed_class = GTK_FIXED_CLASS(klass);
    // ...

    /* Add private indirection member */
    g_type_class_add_private(klass, sizeof(GtKFixed_AltPrivate));
}

/* Initialize a new GtKFixed_Alt instance */
static void
gtkfixed_alt_init(GtKFixed_Alt *fixedAlt) {
    /* This means that GtKFixed_Alt doesn't supply its own GdkWindow */
    gtk_widget_set_has_window(GTK_WIDGET(fixedAlt), FALSE);

    /* Initialize private members */
    GtKFixed_AltPrivate *priv = GTKFIXED_ALT_PRIVATE(fixedAlt);
    priv->children = NULL;
}

/* Return a new GtKFixed_Alt cast to a GtkWidget */
GtkWidget *
gtkfixed_alt_new() {
    return GTK_WIDGET(g_object_new(gtkfixed_alt_get_type(), NULL));
}


/* Get the width of the container(GtkFixed_Alt)
 * don't ask for children size so we can shrink as in gtk2
*/
static void
gtkfixed_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_GTKFIXED_ALT(widget));

    *minimal = 1;
    *natural = 1;
}

/* Get the height of the container(GtkFixed_Alt)
 * don't ask for children size so we can shrink as in gtk2
*/
static void
gtkfixed_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural) {
    g_return_if_fail(widget != NULL);
    g_return_if_fail(IS_GTKFIXED_ALT(widget));

    *minimal = 1;
    *natural = 1;
}
