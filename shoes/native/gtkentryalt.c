

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "gtkentryalt.h"

/* Private class member */
#define GTKENTRY_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
	GTK_TYPE_ENTRY_ALT, GtKEntry_AltPrivate))

typedef struct _GtKEntry_AltPrivate GtKEntry_AltPrivate;

struct _GtKEntry_AltPrivate
{
  
};

/* Forward declarations */
static void gtkentry_alt_get_preferred_width(GtkWidget *widget,
                                        int *minimal, int *natural);
static void gtkentry_alt_get_preferred_height(GtkWidget *widget,
                                        int *minimal, int *natural);

/* Define the GtKEntry_Alt type and inherit from GtkEntry */
G_DEFINE_TYPE(GtKEntry_Alt, gtkentry_alt, GTK_TYPE_ENTRY);

/* Initialize the GtKEntry_Alt class */
static void
gtkentry_alt_class_init(GtKEntry_AltClass *klass)
{
	/* Override GtkWidget methods */
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->get_preferred_width = gtkentry_alt_get_preferred_width;
	widget_class->get_preferred_height = gtkentry_alt_get_preferred_height;
	//widget_class->size_allocate = gtkfixed_alt_size_allocate;

	/* Override GtkEntry methods */
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	//GtKEntryClass *entry_class = GTK_ENTRY_CLASS(klass);
    // ...

	/* Add private indirection member */
	g_type_class_add_private(klass, sizeof(GtKEntry_AltPrivate));
}

/* Initialize a new GtKEntry_Alt instance */
static void
gtkentry_alt_init(GtKEntry_Alt *entryAlt)
{
	/* This means that GtKEntry_Alt doesn't supply its own GdkWindow */
	gtk_widget_set_has_window(GTK_WIDGET(entryAlt), FALSE);

	/* Initialize private members */
	GtKEntry_AltPrivate *priv = GTKENTRY_ALT_PRIVATE(entryAlt);
	
}

/* Return a new GtKEntry_Alt cast to a GtkWidget */
GtkWidget *
gtkentry_alt_new()
{
  return GTK_WIDGET(g_object_new(gtkentry_alt_get_type(), NULL));
}


static void
gtkentry_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void
gtkentry_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

