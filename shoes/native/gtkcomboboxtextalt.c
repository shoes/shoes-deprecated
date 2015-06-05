

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "gtkcomboboxtextalt.h"

/* Private class member */
#define GTK_COMBOBOXTEXT_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_COMBO_BOX_TEXT_ALT, GtkComboBoxText_AltPrivate))

typedef struct _GtkComboBoxText_AltPrivate GtkComboBoxText_AltPrivate;

struct _GtkComboBoxText_AltPrivate
{
  
};

/* Forward declarations */
static void gtk_combo_box_text_alt_get_preferred_width(GtkWidget *widget,
                                        int *minimal, int *natural);
static void gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget,
                                        int *minimal, int *natural);

/* Define the GtkComboBoxText_Alt type and inherit from GtkComboBoxText */
G_DEFINE_TYPE(GtkComboBoxText_Alt, gtk_combo_box_text_alt, GTK_TYPE_COMBO_BOX_TEXT);

/* Initialize the GtkComboBoxText_Alt class */
static void
gtk_combo_box_text_alt_class_init(GtkComboBoxText_AltClass *klass)
{
	/* Override GtkWidget methods */
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	widget_class->get_preferred_width = gtk_combo_box_text_alt_get_preferred_width;
	widget_class->get_preferred_height = gtk_combo_box_text_alt_get_preferred_height;

	/* Override GtkComboBoxText methods */
        GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    // ...

	/* Add private indirection member */
	g_type_class_add_private(klass, sizeof(GtkComboBoxText_AltPrivate));
}

/* Initialize a new GtkComboBoxText_Alt instance */
static void
gtk_combo_box_text_alt_init(GtkComboBoxText_Alt *comboboxtextAlt)
{
	/* This means that GtkComboBoxText_Alt doesn't supply its own GdkWindow */
	gtk_widget_set_has_window(GTK_WIDGET(comboboxtextAlt), FALSE);

	/* Initialize private members */
	GtkComboBoxText_AltPrivate *priv = GTK_COMBOBOXTEXT_ALT_PRIVATE(comboboxtextAlt);

}

/* Return a new GtkComboBoxText_Alt cast to a GtkWidget */
GtkWidget *
gtk_combo_box_text_alt_new()
{
  return GTK_WIDGET(g_object_new(gtk_combo_box_text_alt_get_type(), NULL));
}


static void
gtk_combo_box_text_alt_get_preferred_width(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}

static void
gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget != NULL);

    *minimal = 1;
    *natural = 1;
}
