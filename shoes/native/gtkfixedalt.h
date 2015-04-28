
#ifndef GtKFixed_Alt_H
#define GtKFixed_Alt_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTKFIXED_ALT_TYPE            (gtkfixed_alt_get_type())
#define GTKFIXED_ALT(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTKFIXED_ALT_TYPE, GtKFixed_Alt))
#define GTKFIXED_ALT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), GTKFIXED_ALT_TYPE, GtKFixed_AltClass))
#define IS_GTKFIXED_ALT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTKFIXED_ALT_TYPE))
#define IS_GTKFIXED_ALT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), GTKFIXED_ALT_TYPE))

typedef struct _GtKFixed_Alt {
    GtkFixed parent_instance;
} GtKFixed_Alt;

typedef struct _GtKFixed_AltClass {
    GtkFixedClass parent_class;
} GtKFixed_AltClass;

GType gtkfixed_alt_get_type(void) G_GNUC_CONST;
GtkWidget *gtkfixed_alt_new(void);

G_END_DECLS

#endif /* GtKFixed_Alt_H */
