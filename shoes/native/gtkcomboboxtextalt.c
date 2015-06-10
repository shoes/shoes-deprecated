

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include "gtkcomboboxtextalt.h"
#include <gtk/gtkcombobox.h>


struct _GtkComboBoxPrivate
{
  GtkTreeModel *model;

  GtkCellArea *area;

  gint col_column;
  gint row_column;

  gint wrap_width;
  GtkShadowType shadow_type;

  gint active; /* Only temporary */
  GtkTreeRowReference *active_row;

  GtkWidget *tree_view;

  GtkWidget *cell_view;
  GtkWidget *cell_view_frame;

  GtkWidget *button;
  GtkWidget *box;
  GtkWidget *arrow;
  GtkWidget *separator;

  GtkWidget *popup_widget;
  GtkWidget *popup_window;
  GtkWidget *scrolled_window;

  gulong inserted_id;
  gulong deleted_id;
  gulong reordered_id;
  gulong changed_id;
  guint popup_idle_id;
  guint activate_button;
  guint32 activate_time;
  guint scroll_timer;
  guint resize_idle_id;

  /* For "has-entry" specific behavior we track
   * an automated cell renderer and text column
   */
  gint  text_column;
  GtkCellRenderer *text_renderer;

  gint id_column;

  guint popup_in_progress : 1;
  guint popup_shown : 1;
  guint add_tearoffs : 1;
  guint has_frame : 1;
  guint is_cell_renderer : 1;
  guint editing_canceled : 1;
  guint auto_scroll : 1;
  guint focus_on_click : 1;
  guint button_sensitivity : 2;
  guint has_entry : 1;
  guint popup_fixed_width : 1;

  GtkTreeViewRowSeparatorFunc row_separator_func;
  gpointer                    row_separator_data;
  GDestroyNotify              row_separator_destroy;

  GdkDevice *grab_pointer;
  GdkDevice *grab_keyboard;

  gchar *tearoff_title;
};

/* Private class member */
#define GTK_COMBOBOXTEXT_ALT_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
  GTK_TYPE_COMBO_BOX_TEXT_ALT, GtkComboBoxText_AltPrivate))

typedef struct _GtkComboBoxText_AltPrivate GtkComboBoxText_AltPrivate;

struct _GtkComboBoxText_AltPrivate
{
  /* to avoid warnings (g_type_class_add_private: assertion `private_size > 0' failed) */
  gchar dummy;
};

/* Forward declarations */
static void gtk_combo_box_text_alt_get_preferred_width(GtkWidget *widget,
                                        int *minimal, int *natural);
static void gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget,
                                        int *minimal, int *natural);
static void gtk_combo_box_text_alt_get_preferred_height_for_width(GtkWidget *widget, 
                            gint avail_size, gint *minimum_size, gint *natural_size);

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
        widget_class->get_preferred_height_for_width = gtk_combo_box_text_alt_get_preferred_height_for_width;

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
    
    GtkComboBox *combo_box = GTK_COMBO_BOX(widget);
    GtkComboBoxPrivate *priv = combo_box->priv;
//    gint box_width;
//    gtk_widget_get_preferred_width(priv->box, &box_width, NULL);
    
    GList *renderers = gtk_cell_layout_get_cells((GtkCellLayout *)widget);
    GtkCellRenderer *cell = g_list_first(renderers)->data;    //only one renderer
  
    gint cell_width, cell_height;
    //gtk_cell_renderer_get_fixed_size((GtkCellRenderer *)priv->text_renderer, &cell_width, &cell_height);
    gtk_cell_renderer_get_fixed_size(cell, &cell_width, &cell_height);
    printf("cell_width cell_height : %d, %d\n", cell_width, cell_height);
    *minimal = cell_width;
    *natural = cell_width;
}

static void
gtk_combo_box_text_alt_get_preferred_height(GtkWidget *widget, int *minimal, int *natural)
{
    g_return_if_fail(widget != NULL);
    
    gint min_width, nat_width;
    GTK_WIDGET_GET_CLASS (widget)->get_preferred_width(widget, &min_width, &nat_width);
    GTK_WIDGET_GET_CLASS (widget)->get_preferred_height_for_width(widget, min_width, minimal, natural);
    
}

static void
gtk_combo_box_text_alt_get_preferred_height_for_width(GtkWidget *widget,
                                              gint       avail_size,
                                              gint      *minimum_size,
                                              gint      *natural_size)
{
    GList *renderers = gtk_cell_layout_get_cells((GtkCellLayout *)widget);
    GtkCellRenderer *cell = g_list_first(renderers)->data;    //only one renderer
  
    gint cell_width, cell_height;
    //gtk_cell_renderer_get_fixed_size((GtkCellRenderer *)priv->text_renderer, &cell_width, &cell_height);
    gtk_cell_renderer_get_fixed_size(cell, &cell_width, &cell_height);
    printf("avail_size : %d\n", avail_size);
    
    minimum_size = cell_height;
    natural_size = cell_height;
}