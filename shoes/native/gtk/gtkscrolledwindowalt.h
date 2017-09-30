
#ifndef GTK_SCROLLED_WINDOW_ALT_H
#define	GTK_SCROLLED_WINDOW_ALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_SCROLLED_WINDOW_ALT           (gtk_scrolled_window_alt_get_type())
#define GTK_SCROLLED_WINDOW_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_GTK_SCROLLED_WINDOW_ALT, GtkScrolledWindow_Alt))
#define GTK_SCROLLED_WINDOW_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_GTK_SCROLLED_WINDOW_ALT, GtkScrolledWindow_AltClass))
#define IS_GTK_SCROLLED_WINDOW_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_GTK_SCROLLED_WINDOW_ALT))
#define IS_GTK_SCROLLED_WINDOW_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_GTK_SCROLLED_WINDOW_ALT))
#define GTK_GTK_SCROLLED_WINDOW_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_GTK_SCROLLED_WINDOW_ALT, GtkScrolledWindow_AltClass))

typedef struct _GtkScrolledWindow_AltPrivate GtkScrolledWindow_AltPrivate;

typedef struct _GtkScrolledWindow_Alt {
    GtkScrolledWindow parent_instance;
} GtkScrolledWindow_Alt;

typedef struct _GtkScrolledWindow_AltClass {
    GtkScrolledWindowClass parent_class;
} GtkScrolledWindow_AltClass;

GType gtk_scrolled_window_alt_get_type(void) G_GNUC_CONST;
GtkWidget* gtk_scrolled_window_alt_new(GtkAdjustment *hadjustment,
                                       GtkAdjustment *vadjustment);

G_END_DECLS

#endif	/* GTK_SCROLLED_WINDOW_ALT_H */

