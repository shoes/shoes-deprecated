#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtksystray.h"

//extern GApplication *shoes_GApp;
#if !defined(SHOES_GTK_WIN32) && defined(GNOTE)  // some Linux, not Windows
void shoes_native_systray(char *title, char *message, char *path) {
  GApplication *gapp = g_application_get_default();
  GNotification *note;
  note = g_notification_new (title);
  g_notification_set_body (note, message);
  GFile *iconf = g_file_new_for_path (path);
  GIcon *icon = g_file_icon_new (iconf);
  g_notification_set_icon(note, icon);
  g_application_send_notification (gapp, "Shoes", note);
}
#else
// use gtk_status_icon for Windows, deprecated but GNotification doesn't work
static GtkStatusIcon *stsicon = NULL;
static char *stspath = NULL;
void shoes_native_systray(char *title, char *message, char *path) {
    if (stsicon == NULL) {
        stsicon = gtk_status_icon_new_from_file(path);
        stspath = path;
    }
    // detect change of icon
    if (strcmp(path, stspath)) {
        stspath = path;
        gtk_status_icon_set_from_file (stsicon, stspath);
    }
    gtk_status_icon_set_title(stsicon, title);
    gtk_status_icon_set_tooltip_text(stsicon, message);
}
#endif
