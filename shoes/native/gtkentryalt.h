
#ifndef GTKENTRYALT_H
#define	GTKENTRYALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_ENTRY_ALT           (gtkentry_alt_get_type())
#define GTKENTRY_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_ENTRY_ALT, GtKEntry_Alt))
#define GTKENTRY_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_ENTRY_ALT, GtKEntry_AltClass))
#define IS_GTKENTRY_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_ENTRY_ALT))
#define IS_GTKENTRY_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_ENTRY_ALT))
#define GTK_ENTRY_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_ENTRY_ALT, GtKEntry_AltClass))

typedef struct _GtKEntry_AltPrivate GtKEntry_AltPrivate;

typedef struct _GtKEntry_Alt {
    GtkEntry parent_instance;
} GtKEntry_Alt;

typedef struct _GtKEntry_AltClass {
    GtkEntryClass parent_class;
} GtKEntry_AltClass;

GType gtkentry_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtkentry_alt_new(void);

G_END_DECLS

#endif /* GtKEntry_Alt_H */

