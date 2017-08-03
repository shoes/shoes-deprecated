#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/native.h"
#include "shoes/internal.h"
#include "shoes/native/gtk/gtksystray.h"

// This doesn't work so don't worry about purity
void shoes_native_systray(char *title, char *message, char *path) {
  GApplication *gapp = g_application_get_default();
  GNotification *note;
  note = g_notification_new (title);
  g_notification_set_body (note, message);
  // GFile *iconf = g_file_new_for_path (const char *path);
  // GIcon *icon = g_file_icon_new (iconf);
  g_application_send_notification (gapp, "Shoes", note);
}
