
#ifndef GTK_COMBO_BOX_TEXT_ALT_H
#define	GTK_COMBO_BOX_TEXT_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>
#include "shoes/ruby.h"
G_BEGIN_DECLS

#define GTK_TYPE_COMBO_BOX_TEXT_ALT           (gtk_combo_box_text_alt_get_type())
#define GTK_COMBO_BOX_TEXT_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_GTK_COMBO_BOX_TEXT_ALT, GtkComboBoxText_Alt))
#define GTK_COMBO_BOX_TEXT_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_GTK_COMBO_BOX_TEXT_ALT, GtkComboBoxText_AltClass))
#define IS_GTK_COMBO_BOX_TEXT_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_GTK_COMBO_BOX_TEXT_ALT))
#define IS_GTK_COMBO_BOX_TEXT_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_GTK_COMBO_BOX_TEXT_ALT))
#define GTK_GTK_COMBO_BOX_TEXT_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GTK_COMBO_BOX_TEXT_ALT, GtkComboBoxText_AltClass))

typedef struct _GtkComboBoxText_AltPrivate GtkComboBoxText_AltPrivate;

typedef struct _GtkComboBoxText_Alt {
    GtkComboBoxText parent_instance;
} GtkComboBoxText_Alt;

typedef struct _GtkComboBoxText_AltClass {
    GtkComboBoxTextClass parent_class;
} GtkComboBoxText_AltClass;

GType gtk_combo_box_text_alt_get_type(void) G_GNUC_CONST;
//GtkWidget *gtk_combo_box_text_alt_new(void);
GtkWidget *gtk_combo_box_text_alt_new(VALUE attribs, int bottom_margin);

G_END_DECLS

#endif /* GtkComboBoxText_Alt_H */
