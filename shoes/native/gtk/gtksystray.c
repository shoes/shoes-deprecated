#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtksystray.h"
// This may not work so don't worry about purity yet
extern GApplication *shoes_GApp;
#ifndef SHOES_GTK_WIN32
void shoes_native_systray(char *title, char *message, char *path) {
  GApplication *gapp = g_application_get_default();
  GNotification *note;
  note = g_notification_new (title);
  g_notification_set_body (note, message);
  GFile *iconf = g_file_new_for_path (path);
  GIcon *icon = g_file_icon_new (iconf);
  g_notification_set_icon(note, icon);
  g_application_send_notification (shoes_GApp, "Shoes", note);
}
#else
  // try older gtk_status_icon for Windows to see what happens
static GtkStatusIcon *stsicon = NULL;
void shoes_native_systray(char *title, char *message, char *path) {
    if (stsicon == NULL) {
        stsicon = gtk_status_icon_new_from_file(path);
    }
    gtk_status_icon_set_title(stsicon, title);
    gtk_status_icon_set_tooltip_text(stsicon, message);
}
#endif
