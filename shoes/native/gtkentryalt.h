
#ifndef GTKENTRYALT_H
#define	GTKENTRYALT_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTK_TYPE_ENTRY_ALT           (gtk_entry_alt_get_type())
#define GTKENTRY_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTK_TYPE_ENTRY_ALT, GtkEntry_Alt))
#define GTKENTRY_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTK_TYPE_ENTRY_ALT, GtkEntry_AltClass))
#define IS_GTKENTRY_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTK_TYPE_ENTRY_ALT))
#define IS_GTKENTRY_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTK_TYPE_ENTRY_ALT))
#define GTK_ENTRY_ALT_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_ENTRY_ALT, GtkEntry_AltClass))

typedef struct _GtkEntry_AltPrivate GtkEntry_AltPrivate;

typedef struct _GtkEntry_Alt {
    GtkEntry parent_instance;
} GtkEntry_Alt;

typedef struct _GtkEntry_AltClass {
    GtkEntryClass parent_class;
} GtkEntry_AltClass;

GType gtk_entry_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtk_entry_alt_new(void);

G_END_DECLS

#endif /* GtkEntry_Alt_H */
