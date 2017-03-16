
#ifndef GTK_BUTTON_ALT_H
#define	GTK_BUTTON_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_BUTTON_ALT           (gtk_button_alt_get_type())
#define GTK_BUTTON_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_GTK_BUTTON_ALT, GtkButton_Alt))
#define GTK_BUTTON_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_GTK_BUTTON_ALT, GtkButton_AltClass))
#define IS_GTK_BUTTON_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_GTK_BUTTON_ALT))
#define IS_GTK_BUTTON_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_GTK_BUTTON_ALT))
#define GTK_GTK_BUTTON_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GTK_BUTTON_ALT, GtkButton_AltClass))

typedef struct _GtkButton_AltPrivate GtkButton_AltPrivate;

typedef struct _GtkButton_Alt {
    GtkButton parent_instance;
} GtkButton_Alt;

typedef struct _GtkButton_AltClass {
    GtkButtonClass parent_class;
} GtkButton_AltClass;

GType gtk_button_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtk_button_alt_new(void);
GtkWidget *gtk_button_alt_new_with_label(const gchar *label);

G_END_DECLS

#endif /* GtkButton_Alt_H */
