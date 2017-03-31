
#ifndef GTK_PROGRESS_BAR_ALT_H
#define	GTK_PROGRESS_BAR_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_PROGRESS_BAR_ALT           (gtk_progress_bar_alt_get_type())
#define GTK_PROGRESS_BAR_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_GTK_PROGRESS_BAR_ALT, GtkProgressBar_Alt))
#define GTK_PROGRESS_BAR_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_GTK_PROGRESS_BAR_ALT, GtkProgressBar_AltClass))
#define IS_GTK_PROGRESS_BAR_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_GTK_PROGRESS_BAR_ALT))
#define IS_GTK_PROGRESS_BAR_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_GTK_PROGRESS_BAR_ALT))
#define GTK_GTK_PROGRESS_BAR_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GTK_PROGRESS_BAR_ALT, GtkProgressBar_AltClass))

typedef struct _GtkProgressBar_AltPrivate GtkProgressBar_AltPrivate;

typedef struct _GtkProgressBar_Alt {
    GtkProgressBar parent_instance;
} GtkProgressBar_Alt;

typedef struct _GtkProgressBar_AltClass {
    GtkProgressBarClass parent_class;
} GtkProgressBar_AltClass;

GType gtk_progress_bar_alt_get_type(void) G_GNUC_CONST;
GtkWidget* gtk_progress_bar_alt_new(void);

G_END_DECLS

#endif	/* GTK_PROGRESS_BAR_ALT_H */

