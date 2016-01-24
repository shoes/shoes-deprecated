//
// shoes/native-gtk.c
// GTK+ code for Shoes.
//   Modified for Gtk-3.0 by Cecil Coupe (cjc)
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#include <fontconfig/fontconfig.h>
#ifndef RUBY_HTTP
#ifndef SHOES_GTK_WIN32
#include <curl/curl.h>
#endif
#endif
#include <pthread.h>
#include <glib/gprintf.h>

#include "gtkfixedalt.h"
#include "gtkentryalt.h"
#include "gtkcomboboxtextalt.h"
#include "gtkbuttonalt.h"
#include "gtkscrolledwindowalt.h"
#include "gtkprogressbaralt.h"

#define GTK_CHILD(child, ptr) \
  GList *children = gtk_container_get_children(GTK_CONTAINER(ptr)); \
  child = children->data

#define HEIGHT_PAD 0

#define SHOES_GTK_INVISIBLE_CHAR (gunichar)0x2022

static VALUE
shoes_make_font_list(FcFontSet *fonts, VALUE ary)
{
  int i = 0;
  for (i = 0; i < fonts->nfont; i++)
  {
    FcValue val;
    FcPattern *p = fonts->fonts[i];
    if (FcPatternGet(p, FC_FAMILY, 0, &val) == FcResultMatch)
      rb_ary_push(ary, rb_str_new2((char *)val.u.s));
  }
  rb_funcall(ary, rb_intern("uniq!"), 0);
  rb_funcall(ary, rb_intern("sort!"), 0);
  return ary;
}

VALUE
shoes_font_list()
{
  VALUE ary = rb_ary_new();
  FcConfig *fc = FcConfigGetCurrent();
  FcFontSet *fonts = FcConfigGetFonts(fc, FcSetApplication);
  if (fonts) shoes_make_font_list(fonts, ary);
  fonts = FcConfigGetFonts(fc, FcSetSystem);
  if (fonts) shoes_make_font_list(fonts, ary);
  return ary;
}

VALUE
shoes_load_font(const char *filename)
{
  FcConfig *fc = FcConfigGetCurrent();
  FcFontSet *fonts = FcFontSetCreate();
  if (!FcFileScan(fonts, NULL, NULL, NULL, (const FcChar8 *)filename, FcTrue))
    return Qnil;

  VALUE ary = rb_ary_new();
  shoes_make_font_list(fonts, ary);
  FcFontSetDestroy(fonts);

  if (!FcConfigAppFontAddFile(fc, (const FcChar8 *)filename))
    return Qnil;

  // refresh the FONTS list
  shoes_update_fonts(shoes_font_list());
  return ary;
}

#if 0
// FIXME: experiment with font settings
// Borrowed from http://ricardo.ecn.wfu.edu/~cottrell/gtk_win32/
#ifdef G_OS_WIN32
static char appfontname[128] = "sans-serif 12"; /* fallback value */
#else
static char appfontname[128] = "Sans-Serif 10";  // gtk doc says 'Sans 10'
#endif

void set_app_font (const char *fontname)
{
  GtkSettings *settings;
  gchar *themedir;
  gchar *themename;

  if (fontname != NULL && *fontname == 0) return;

  settings = gtk_settings_get_default();
  themedir = gtk_rc_get_theme_dir ();
  g_object_get(settings, "gtk-theme-name", &themename, NULL);
  printf("dir=%s, name: %s\n", themedir,themename);


  if (fontname == NULL) {
	g_object_set(G_OBJECT(settings), "gtk-font-name", appfontname, NULL);
  } else {
	GtkWidget *w;
	PangoFontDescription *pfd;
	PangoContext *pc;
	PangoFont *pfont;

	w = gtk_label_new(NULL);
	pfd = pango_font_description_from_string(fontname);
	pc = gtk_widget_get_pango_context(w);
	pfont = pango_context_load_font(pc, pfd);

	if (pfont != NULL) {
	  strcpy(appfontname, fontname);
	  g_object_set(G_OBJECT(settings), "gtk-font-name", appfontname, NULL);
	}

	gtk_widget_destroy(w);
	pango_font_description_free(pfd);
  }
}

void shoes_native_print_env()
{
  GtkSettings *settings;
  gchar *themedir;
  gchar *themename;

  gchar *rcfile = "shoesgtk.rc";
  //gchar *rcfiles[] = {rcfile, NULL};
  gchar **defs;
  //gtk_rc_set_default_files(rcfiles);
  //gtk_rc_parse(rcfile);
  //printf("ask theme = %s\n", rcfile);
  defs = gtk_rc_get_default_files ();
  int i = 0;
  while ((rcfile = defs[i]) != NULL) {
	  printf("%d: %s\n", i+1, rcfile);
	  i++;
  }
  settings = gtk_settings_get_default();
  themedir = gtk_rc_get_theme_dir ();
  g_object_get(settings, "gtk-theme-name", &themename, NULL);
  printf("dir=%s, name: %s\n", themedir,themename);
}
#endif

void shoes_native_init()
{
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
  curl_global_init(CURL_GLOBAL_ALL);
#endif
  gtk_init(NULL, NULL);
  //set_app_font(NULL);  // experiment failed
  //shoes_native_print_env();
}

void shoes_native_cleanup(shoes_world_t *world)
{
#if !defined(RUBY_HTTP) && !defined(SHOES_GTK_WIN32)
  curl_global_cleanup();
#endif
}

void shoes_native_quit()
{
  gtk_main_quit();
}


#ifdef SHOES_GTK_WIN32
int
shoes_win32_cmdvector(const char *cmdline, char ***argv)
{
//  return rb_w32_cmdvector(cmdline, argv);
}

void shoes_get_time(SHOES_TIME *ts)
{
	*ts = g_get_monotonic_time();  // Should work for GTK3 w/o win32
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end)
{
  return *end - *start;
}
#else
void shoes_get_time(SHOES_TIME *ts)
{
#ifdef SHOES_GTK_OSX
  gettimeofday(ts, NULL);
#else
  clock_gettime(CLOCK_REALTIME, ts);
#endif
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end)
{
  unsigned long usec;
  if ((end->tv_nsec-start->tv_nsec)<0) {
    usec = (end->tv_sec-start->tv_sec - 1) * 1000;
    usec += (1000000000 + end->tv_nsec - start->tv_nsec) / 1000000;
  } else {
    usec = (end->tv_sec - start->tv_sec) * 1000;
    usec += (end->tv_nsec - start->tv_nsec) / 1000000;
  }
  return usec;
}
#endif

typedef struct {
  unsigned int name;
  VALUE obj;
  void *data;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int ret;
} shoes_gtk_msg;


static gboolean
shoes_gtk_catch_message(gpointer user) {
  shoes_gtk_msg *msg = (shoes_gtk_msg *)user;
  pthread_mutex_lock(&msg->mutex);
  msg->ret = shoes_catch_message(msg->name, msg->obj, msg->data);
  pthread_cond_signal(&msg->cond);
  pthread_mutex_unlock(&msg->mutex);
  return FALSE;
}

// Only called by image.c
int shoes_throw_message(unsigned int name, VALUE obj, void *data)
{
  int ret;
  shoes_gtk_msg *msg = SHOE_ALLOC(shoes_gtk_msg);
  msg->name = name;
  msg->obj = obj;
  msg->data = data;
  pthread_mutex_init(&msg->mutex, NULL);
  pthread_cond_init(&msg->cond, NULL);
  msg->ret = 0;

  pthread_mutex_lock(&msg->mutex);
  g_idle_add_full(G_PRIORITY_DEFAULT, shoes_gtk_catch_message, msg, NULL);
  pthread_cond_wait(&msg->cond, &msg->mutex);
  ret = msg->ret;
  pthread_mutex_unlock(&msg->mutex);

  free(msg);
  return ret;
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_reset(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_clear(shoes_canvas *canvas)
{
  if (canvas->slot->vscroll)
  {
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
#ifdef GTK3
    gtk_adjustment_set_value(adj, gtk_adjustment_get_lower(adj));
#else
    gtk_adjustment_set_value(adj, adj->lower);
#endif
  }
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
  gtk_widget_queue_draw(slot->oscanvas);
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy)
{
  if (slot->vscroll)
  {
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
#ifdef GTK3
    if (gtk_adjustment_get_upper(adj) != (gdouble)endy)
    {
      gtk_range_set_range(GTK_RANGE(slot->vscroll), 0., (gdouble)endy);

      if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
#else
    if (adj->upper != (gdouble)endy)
    {
      gtk_range_set_range(GTK_RANGE(slot->vscroll), 0., (gdouble)endy);

      if (adj->page_size >= adj->upper)
#endif
        gtk_widget_hide(slot->vscroll);
      else
        gtk_widget_show(slot->vscroll);
    }
  }
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
  if (slot->vscroll)
    gtk_range_set_value(GTK_RANGE(slot->vscroll), slot->scrolly);
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  if (slot->vscroll)
  {
#ifdef GTK3
    GtkRequisition rnat;
    gtk_widget_get_preferred_size(slot->vscroll, NULL, &rnat);
    return rnat.width;
#else
    GtkRequisition req;
    gtk_widget_size_request(slot->vscroll, &req);
    return req.width;
#endif
  }
  return 0;
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c)
{
}

//
// Window-level events
//
static gboolean
shoes_app_gtk_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
  GdkModifierType state;
  shoes_app *app = (shoes_app *)data;
  if (!event->is_hint)
  {
    shoes_canvas *canvas;
    Data_Get_Struct(app->canvas, shoes_canvas, canvas);
    state = (GdkModifierType)event->state;
    shoes_app_motion(app, (int)event->x, (int)event->y + canvas->slot->scrolly);
  }
  return TRUE;
}

static gboolean
shoes_app_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  shoes_app *app = (shoes_app *)data;
  shoes_canvas *canvas;
  Data_Get_Struct(app->canvas, shoes_canvas, canvas);
  if (event->type == GDK_BUTTON_PRESS)
  {
    shoes_app_click(app, event->button, event->x, event->y + canvas->slot->scrolly);
  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    shoes_app_release(app, event->button, event->x, event->y + canvas->slot->scrolly);
  }
  return TRUE;
}

static gboolean
shoes_app_gtk_wheel(GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
  ID wheel;
  shoes_app *app = (shoes_app *)data;

  switch (event->direction)
  {
    case GDK_SCROLL_UP:    wheel = s_up;    break;
    case GDK_SCROLL_DOWN:  wheel = s_down;  break;
    case GDK_SCROLL_LEFT:  wheel = s_left;  break;
    case GDK_SCROLL_RIGHT: wheel = s_right; break;
    default: return TRUE;
  }

  shoes_app_wheel(app, wheel, event->x, event->y);
  return TRUE;
}

#ifdef GTK3
static void
shoes_app_gtk_paint(GtkWidget *widget, cairo_t *cr, gpointer data)
#else
static void
shoes_app_gtk_paint(GtkWidget *widget, GdkEventExpose *event, gpointer data)
#endif
{
  shoes_app *app = (shoes_app *)data;
  gtk_window_get_size(GTK_WINDOW(app->os.window), &app->width, &app->height);
  shoes_canvas_size(app->canvas, app->width, app->height);
}

#ifdef GTK3
#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_KEY_##name) \
    v = ID2SYM(rb_intern("" # sym))
#else
#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_##name) \
    v = ID2SYM(rb_intern("" # sym))
#endif

static gboolean
shoes_app_gtk_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
  VALUE v = Qnil;
  guint modifiers = event->state;
  shoes_app *app = (shoes_app *)data;
#ifdef GTK3
  if (event->keyval == GDK_KEY_Return)
#else
  if (event->keyval == GDK_Return)
#endif
  {
    v = rb_str_new2("\n");
  }
#ifdef GTK3
  KEY_SYM(BackSpace, backspace);	// GTK3 <bs> has length of 1. Go figure.
#endif
  KEY_SYM(Escape, escape);
  else if (event->length > 0)
  {
    if ((event->state & GDK_CONTROL_MASK) || (event->state & GDK_MOD1_MASK))
    {
      gint len;
      gunichar ch;
      char chbuf[7] = {0};

      ch = gdk_keyval_to_unicode(event->keyval);
      len = g_unichar_to_utf8(ch, chbuf);
      chbuf[len] = '\0';

      v = ID2SYM(rb_intern(chbuf));
      if (modifiers & GDK_SHIFT_MASK) modifiers ^= GDK_SHIFT_MASK;
    }
    else
    {
      if (event->string[0] == '\r' && event->length == 1)
        v = rb_str_new2("\n");
      else
        v = rb_str_new(event->string, event->length);
    }
  }
  KEY_SYM(Insert, insert);
  KEY_SYM(Delete, delete);
#ifndef GTK3
  KEY_SYM(BackSpace, backspace);
#endif
  KEY_SYM(Tab, tab);
  KEY_SYM(ISO_Left_Tab, tab);
  KEY_SYM(Page_Up, page_up);
  KEY_SYM(Page_Down, page_down);
  KEY_SYM(Home, home);
  KEY_SYM(End, end);
  KEY_SYM(Left, left);
  KEY_SYM(Up, up);
  KEY_SYM(Right, right);
  KEY_SYM(Down, down);
  KEY_SYM(F1, f1);
  KEY_SYM(F2, f2);
  KEY_SYM(F3, f3);
  KEY_SYM(F4, f4);
  KEY_SYM(F5, f5);
  KEY_SYM(F6, f6);
  KEY_SYM(F7, f7);
  KEY_SYM(F8, f8);
  KEY_SYM(F9, f9);
  KEY_SYM(F10, f10);
  KEY_SYM(F11, f11);
  KEY_SYM(F12, f12);

  if (v != Qnil) {
    if (event->type == GDK_KEY_PRESS) {
      shoes_app_keydown(app, v);

      if (event->keyval == GDK_KEY_Return)
        if ((event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) != 0)
          v = ID2SYM(rb_intern("enter"));

      if (SYMBOL_P(v))
      {
        if (modifiers & GDK_MOD1_MASK)
          KEY_STATE(alt);
        if (modifiers & GDK_SHIFT_MASK)
          KEY_STATE(shift);
        if (modifiers & GDK_CONTROL_MASK)
          KEY_STATE(control);
      }

      shoes_app_keypress(app, v);
    } else {
      shoes_app_keyup(app, v);
    }
  }

  return FALSE;
}

static gboolean
shoes_app_gtk_quit(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  shoes_app *app = (shoes_app *)data;
  if (shoes_app_remove(app))
    gtk_main_quit();
  return FALSE;
}

static void
shoes_canvas_gtk_paint_children(GtkWidget *widget, gpointer data)
{
  shoes_canvas *canvas = (shoes_canvas *)data;
#ifdef GTK3
  gtk_container_propagate_draw(GTK_CONTAINER(canvas->slot->oscanvas), widget,
    canvas->slot->drawevent);  // cjc  Is this Wrong?
#else
  gtk_container_propagate_expose(GTK_CONTAINER(canvas->slot->oscanvas), widget, canvas->slot->expose);
#endif
}

#ifdef GTK3
static void
shoes_canvas_gtk_paint(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  canvas->slot->drawevent = cr;		// stash it for the children

  // getting widget dirty area, already clipped
  cairo_rectangle_int_t rect;
  gdk_cairo_get_clip_rectangle(cr, &rect);
  
  shoes_canvas_paint(c);
  // Gtk3 doc says gtk_container_foreach is preferable over gtk_container_forall
  gtk_container_foreach(GTK_CONTAINER(widget), shoes_canvas_gtk_paint_children, canvas);
  
  canvas->slot->drawevent = NULL;
}
#else
static void
shoes_canvas_gtk_paint(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  INFO("EXPOSE: (%d, %d) (%d, %d) %lu, %d, %d\n", event->area.x, event->area.y,
    event->area.width, event->area.height, c, (int)event->send_event, event->count);
  Data_Get_Struct(c, shoes_canvas, canvas);

  //
  // Since I'm using a GtkFixed container, I need to force it to be clipped on its boundaries.
  // This could be done by using a whole lot of gdk_window_begin_paint_region calls, but that
  // would also mean masking every region for every element... This approach is simple.  Clip
  // the expose region and pass it on.
  //
  canvas->slot->expose = event;
  GdkRegion *region = event->region;
  GdkRectangle rect = event->area;
  event->region = gdk_region_rectangle(&canvas->slot->oscanvas->allocation);
  gdk_region_intersect(event->region, region);
  gdk_region_get_clipbox(event->region, &event->area);

  shoes_canvas_paint(c);
  gtk_container_forall(GTK_CONTAINER(widget), shoes_canvas_gtk_paint_children, canvas);

  //
  // Restore the full region to the event.
  //
  gdk_region_destroy(event->region);
  event->region = region;
  event->area = rect;
  canvas->slot->expose = NULL;
}
#endif

static void
shoes_canvas_gtk_size(GtkWidget *widget, GtkAllocation *size, gpointer data)
{
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  if (canvas->slot->vscroll &&
    (size->height != canvas->slot->scrollh || size->width != canvas->slot->scrollw))
  {
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot->vscroll));
    gtk_widget_set_size_request(canvas->slot->vscroll, -1, size->height);

    //gtk_widget_set_size_request(GTK_CONTAINER(widget), canvas->app->width, size->height);
#ifdef GTK3
    GtkAllocation alloc;
    gtk_widget_get_allocation((GtkWidget *)canvas->slot->vscroll, &alloc);
    gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
        size->width - alloc.width, 0);
    gtk_adjustment_set_page_size(adj, size->height);
    gtk_adjustment_set_page_increment(adj, size->height - 32);

    if (gtk_adjustment_get_page_size(adj) >= gtk_adjustment_get_upper(adj))
#else
    gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas), canvas->slot->vscroll,
        size->width - canvas->slot->vscroll->allocation.width, 0);
    adj->page_size = size->height;
    adj->page_increment = size->height - 32;

    if (adj->page_size >= adj->upper)

#endif
      gtk_widget_hide(canvas->slot->vscroll);
    else
      gtk_widget_show(canvas->slot->vscroll);

    canvas->slot->scrollh = size->height;
    canvas->slot->scrollw = size->width;
  }
}

static void
shoes_canvas_gtk_scroll(GtkRange *r, gpointer data)
{
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  canvas->slot->scrolly = (int)gtk_range_get_value(r);
  shoes_slot_repaint(canvas->app->slot);
}

#ifndef SHOES_GTK_WIN32
static gint
shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout)
{
  struct timeval tv;

  rb_fdset_t rset, wset, xset;
  GPollFD *f;
  int ready;
  int maxfd = 0;

  rb_fd_init(&rset); // was FD_ZERO()
  rb_fd_init(&wset);
  rb_fd_init(&xset);

  for (f = fds; f < &fds[nfds]; ++f)
     if (f->fd >= 0)
     {
       if (f->events & G_IO_IN)
         //FD_SET (f->fd, &rset);
         rb_fd_set(f->fd, &rset);
       if (f->events & G_IO_OUT)
         //FD_SET (f->fd, &wset);
         rb_fd_set(f->fd, &wset);
       if (f->events & G_IO_PRI)
         //FD_SET (f->fd, &xset);
         rb_fd_set(f->fd, &xset);
       if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
         maxfd = f->fd;
     }

  //
  // If we poll indefinitely, then the window updates will
  // pile up for as long as Ruby is churning away.
  //
  // Give Ruby half-seconds in which to work, in order to
  // keep it from completely blocking the GUI.
  //
  // cjc: timeout appears to be in millisecs.
  if (timeout == -1 || timeout > 500)
    timeout = 500;

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  ready = rb_thread_fd_select (maxfd + 1, &rset, &wset, &xset, &tv);
  if (ready > 0)
     // I think the idea here is that the Ruby threads might cause Gtk events
     // and that's supposed to pass back in the f->revents
     // Does Ruby/Windows execute this code more often or incorrectly?
     for (f = fds; f < &fds[nfds]; ++f)
     {
       f->revents = 0;
       if (f->fd >= 0)
       {
         //if (FD_ISSET (f->fd, &rset))
         if (rb_fd_isset (f->fd, &rset))
           f->revents |= G_IO_IN;
         //if (FD_ISSET (f->fd, &wset))
         if (rb_fd_isset (f->fd, &wset))
           f->revents |= G_IO_OUT;
         //if (FD_ISSET (f->fd, &xset))
         if (rb_fd_isset (f->fd, &xset))
           f->revents |= G_IO_PRI;
       }
     }
  // Free the allocated storage from rb_fd_init
  rb_fd_term(&rset);
  rb_fd_term(&wset);
  rb_fd_term(&xset);
  return ready;
}
#else
/*
 * Fake a rb_fd to always have something to read
 * Ruby run select
 * if nothing selected (only the fake is returned) then
 *   run a poll/select on the Gtk fds
 *   and return that number.
 */

static gint
shoes_app_g_poll(GPollFD *fds, guint nfds, gint timeout)
{
  struct timeval tv;
  rb_fdset_t rset, wset, xset; //ruby
  GPollFD *f;
  int ready;
  int maxfd = 0;



  rb_fd_init(&rset); // These are ruby rd (sets
  rb_fd_init(&wset);
  rb_fd_init(&xset);

  int i;
  // On Windows GTK GPollFD.fd could be a handle, not a small int.
  for (i = 0; i < nfds; i++)
  {
    f = &fds[i];
    if (f->fd >= 0)
    {
       if (f->events & G_IO_IN)
         rb_fd_set(f->fd, &rset);
       if (f->events & G_IO_OUT)
         rb_fd_set (f->fd, &wset);
       if (f->events & G_IO_PRI)
         rb_fd_set (f->fd, &xset);
      // if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
      //   maxfd = f->fd;
       if (i > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
         maxfd = i;
    }
  }
  //
  // If we poll indefinitely, then the window updates will
  // pile up for as long as Ruby is churning away.
  //
  // Give Ruby half-seconds in which to work, in order to
  // keep it from completely blocking the GUI.
  //
  if (timeout == -1 || timeout > 500)
    timeout = 500;

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  ready = rb_fd_select (maxfd + 1, &rset, &wset, &xset, &tv); // NOT deprecated in 2.1.0
  int really_ready = 0;
  if (ready == 0)
  {
     for (f = fds; f < &fds[nfds]; ++f)
     {
       f->revents = 0;
       if (f->fd >= 0)
       {
         if (rb_fd_isset (f->fd, &rset))
           f->revents |= G_IO_IN;
         if (rb_fd_isset (f->fd, &wset))
           f->revents |= G_IO_OUT;
         if (rb_fd_isset (f->fd, &xset))
           f->revents |= G_IO_PRI;
         if (f->revents > 0) really_ready++;
       }
     }
  }

  rb_fd_term(&rset);
  rb_fd_term(&wset);
  rb_fd_term(&xset);
  if (ready < 0)
  {
    printf("Poll fail %d\n", errno);
    ready = 0;
  }
  if (ready != really_ready)
  {
	  printf("R %d. RR %d\n", ready, really_ready);
	  ready = really_ready;
  }
  return ready;
}
#endif

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
#ifdef GTK3
  if (app->os.window == NULL || gtk_widget_get_window(app->os.window)== NULL || app->cursor == cursor)
#else
  if (app->os.window == NULL || app->os.window->window == NULL || app->cursor == cursor)
#endif
    goto done;

  GdkCursor *c;
  if (cursor == s_hand || cursor == s_link)
  {
    c = gdk_cursor_new(GDK_HAND2);
  }
  else if (cursor == s_arrow)
  {
    c = gdk_cursor_new(GDK_ARROW);
  }
  else if (cursor == s_text)
  {
    c = gdk_cursor_new(GDK_XTERM);
  }
  else
    goto done;
#ifdef GTK3
  gdk_window_set_cursor(gtk_widget_get_window(app->os.window), c);
#else
  gdk_window_set_cursor(app->os.window->window, c);
#endif
  app->cursor = cursor;

done:
  return SHOES_OK;
}

void
shoes_native_app_resized(shoes_app *app)
{
  // Not needed anymore ?
  //  if (app->os.window != NULL)
  //    gtk_widget_set_size_request(app->os.window, app->width, app->height);
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
  gtk_window_set_title(GTK_WINDOW(app->os.window), _(msg));
}

void
shoes_native_app_fullscreen(shoes_app *app, char yn)
{
  gtk_window_set_keep_above(GTK_WINDOW(app->os.window), (gboolean)yn);
  if (yn)
    gtk_window_fullscreen(GTK_WINDOW(app->os.window));
  else
    gtk_window_unfullscreen(GTK_WINDOW(app->os.window));
}

// new in 3.2.19
void
shoes_native_app_set_icon(shoes_app *app, char *icon_path)
{
  // replace default icon
  gboolean err;
  err = gtk_window_set_icon_from_file((GtkWindow *) app->slot->oscanvas, icon_path, NULL);
  err = gtk_window_set_default_icon_from_file(icon_path, NULL);
}

// new in 3.2.19
void shoes_native_app_set_wtitle(shoes_app *app, char *wtitle)
{
  gtk_window_set_title(GTK_WINDOW(app->slot->oscanvas), _(wtitle));
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path, int dialog)
{
  char icon_path[SHOES_BUFSIZE];
  shoes_app_gtk *gk = &app->os;

#if !defined(SHOES_GTK_WIN32)
  sprintf(icon_path, "%s/static/app-icon.png", shoes_world->path);
  gtk_window_set_default_icon_from_file(icon_path, NULL);
#endif
  gk->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(gk->window), GTK_WIN_POS_CENTER);
  // commit https://github.com/shoes/shoes/commit/4e7982ddcc8713298b6959804dab8d20111c0038
  if (!app->resizable)
  {
    gtk_widget_set_size_request(gk->window, app->width, app->height);
    gtk_window_set_resizable(GTK_WINDOW(gk->window), FALSE);
  }
  else if (app->minwidth < app->width || app->minheight < app->height)
  {
    GdkGeometry hints;
    hints.min_width = app->minwidth;
    hints.min_height = app->minheight;
    gtk_window_set_geometry_hints(GTK_WINDOW(gk->window), gk->window,
      &hints, GDK_HINT_MIN_SIZE);
  }
#ifdef GTK3
  // TODO - does GTk2 need this?
  gtk_window_set_default_size(GTK_WINDOW(gk->window), app->width, app->height);
#else
  // CJC - Yes, gtk2 does need it.
  gtk_window_set_default_size(GTK_WINDOW(gk->window), app->width, app->height);
#endif

  if (app->fullscreen) shoes_native_app_fullscreen(app, 1);

  gtk_widget_set_events(gk->window, GDK_POINTER_MOTION_MASK | GDK_SCROLL_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(G_OBJECT(gk->window), "size-allocate",
                   G_CALLBACK(shoes_app_gtk_paint), app);
  g_signal_connect(G_OBJECT(gk->window), "motion-notify-event",
                   G_CALLBACK(shoes_app_gtk_motion), app);
  g_signal_connect(G_OBJECT(gk->window), "button-press-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "button-release-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "scroll-event",
                   G_CALLBACK(shoes_app_gtk_wheel), app);
  g_signal_connect(G_OBJECT(gk->window), "key-press-event",
                   G_CALLBACK(shoes_app_gtk_keypress), app);
  g_signal_connect(G_OBJECT(gk->window), "key-release-event",
                   G_CALLBACK(shoes_app_gtk_keypress), app);
  g_signal_connect(G_OBJECT(gk->window), "delete-event",
                   G_CALLBACK(shoes_app_gtk_quit), app);

  app->slot->oscanvas = gk->window;
  return SHOES_OK;
}

void
shoes_native_app_show(shoes_app *app)
{
  gtk_widget_show_all(app->os.window);
}

#ifdef SHOES_GTK_WIN32
static GSource *gtkrb_source;
static GSource *gtkrb_init_source();
static  GSourceFuncs gtkrb_func_tbl;
/*
static GSource *gtkrb_init_source()
{
  // fill in the struct
  gtkrb_source = g_source_new(&gtkrb_func_tbl, (guint) sizeof(gtkrb_func_tbl));
}
*/

static gboolean gtkrb_idle()
{  rb_thread_schedule();
	return 1; // keep timeout
}
#endif

void
shoes_native_loop()
{
#ifndef SHOES_GTK_WIN32
  g_main_context_set_poll_func(g_main_context_default(), shoes_app_g_poll);
#else
  /* Win32 (should work for Linux too when finished)
   * Build: struct GSourceFuncs
   * Call: g_source_new() with that struct
   * Call: g_source_attach() with that
   */
   //gtkrb_source = gtkrb_init_source();
   //g_source_attach(gtkrb_source, (gpointer) NULL);
   //g_idle_add(gtkrb_idle, NULL);
   g_timeout_add(100, gtkrb_idle, NULL);
#endif
  GLOBAL_APP(app);
  if (APP_WINDOW(app)) gtk_main();
}

void
shoes_native_app_close(shoes_app *app)
{
  shoes_app_gtk_quit(app->os.window, NULL, (gpointer)app);
  gtk_widget_destroy(app->os.window);
  app->os.window = NULL;
}

void
shoes_browser_open(char *url)
{
  VALUE browser = rb_str_new2("/etc/alternatives/x-www-browser '");
  rb_str_cat2(browser, url);
  rb_str_cat2(browser, "' 2>/dev/null &");
  shoes_sys(RSTRING_PTR(browser), 1);
}

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(c, shoes_canvas, canvas);

  slot = shoes_slot_alloc(canvas, parent, toplevel);

#ifdef GTK3
/* Subclassing GtkFixed so we can override gtk3 size management which creates
   problems with slot height being always tied to inside widgets cumulative heights
   creating heights overflow with no scrollbar !
*/
  slot->oscanvas = gtkfixed_alt_new();

  g_signal_connect(G_OBJECT(slot->oscanvas), "draw",
                   G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
#else
  slot->oscanvas = gtk_fixed_new();

  g_signal_connect(G_OBJECT(slot->oscanvas), "expose-event",
                   G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
#endif

  g_signal_connect(G_OBJECT(slot->oscanvas), "size-allocate",
                   G_CALLBACK(shoes_canvas_gtk_size), (gpointer)c);
  INFO("shoes_slot_init(%lu)\n", c);

  if (toplevel)
    gtk_container_add(GTK_CONTAINER(parent->oscanvas), slot->oscanvas);
  else
    gtk_fixed_put(GTK_FIXED(parent->oscanvas), slot->oscanvas, x, y);

  slot->scrollh = slot->scrollw = 0;
  slot->vscroll = NULL;
  if (scrolls)
  {
#ifdef GTK3
    slot->vscroll = gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL,NULL);
    GtkAdjustment *adj;
    adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
    gtk_adjustment_set_step_increment(adj, 16);
    gtk_adjustment_set_page_increment(adj, height - 32);
    slot->drawevent = NULL;
#else
    slot->vscroll = gtk_vscrollbar_new(NULL);
    gtk_range_get_adjustment(GTK_RANGE(slot->vscroll))->step_increment = 16;
    gtk_range_get_adjustment(GTK_RANGE(slot->vscroll))->page_increment = height - 32;
    slot->expose = NULL;
#endif

    g_signal_connect(G_OBJECT(slot->vscroll), "value-changed",
                     G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
    gtk_fixed_put(GTK_FIXED(slot->oscanvas), slot->vscroll, -100, -100);

    gtk_widget_set_size_request(slot->oscanvas, width, height);
    //gtk_widget_set_size_request(slot->oscanvas, canvas->app->minwidth, canvas->app->minheight);

    if (!toplevel) ATTRSET(canvas->attr, wheel, scrolls);
  }

  if (toplevel) shoes_canvas_size(c, width, height);
  else
  {
    gtk_widget_show_all(slot->oscanvas);
    canvas->width = 100;
    canvas->height = 100;
  }
}

void
shoes_slot_destroy(shoes_canvas *canvas, shoes_canvas *pc)
{
  if (canvas->slot->vscroll)
    gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), canvas->slot->vscroll);
  gtk_container_remove(GTK_CONTAINER(pc->slot->oscanvas), canvas->slot->oscanvas);
}

cairo_t *
shoes_cairo_create(shoes_canvas *canvas)
{
#ifdef GTK3
  GdkWindow *win = gtk_widget_get_window(canvas->slot->oscanvas);
  cairo_t *cr = gdk_cairo_create(win);
  if (canvas->slot->drawevent != NULL &&
     gtk_cairo_should_draw_window(canvas->slot->drawevent, win))
  {
    // Happens inside larger draw events (signal)
    // Pay attention - this could be wrong. cjc
    cairo_rectangle_int_t alloc;
    gtk_widget_get_allocation((GtkWidget *)canvas->slot->oscanvas, &alloc);
    cairo_region_t *region = cairo_region_create_rectangle(&alloc);
//    cairo_rectangle_int_t w_rect = {canvas->place.ix, canvas->place.iy, canvas->place.w, canvas->place.h};
    cairo_rectangle_int_t w_rect;
    gdk_cairo_get_clip_rectangle(cr, &w_rect);
    cairo_region_intersect_rectangle(region, &w_rect);
    gdk_cairo_region(cr, region);
    cairo_clip(cr);
    cairo_region_destroy(region);

    cairo_translate(cr, alloc.x, alloc.y - canvas->slot->scrolly);
  }
  return cr;
}
#else
  cairo_t *cr = gdk_cairo_create(canvas->slot->oscanvas->window);
  if (canvas->slot->expose != NULL)
  {
    GdkRegion *region = gdk_region_rectangle(&canvas->slot->oscanvas->allocation);
    gdk_region_intersect(region, canvas->slot->expose->region);
    gdk_cairo_region(cr, region);
    gdk_region_destroy(region);
    cairo_clip(cr);
    cairo_translate(cr, canvas->slot->oscanvas->allocation.x, canvas->slot->oscanvas->allocation.y - canvas->slot->scrolly);
  }
  return cr;
}
#endif

void
shoes_cairo_destroy(shoes_canvas *canvas)
{
}

void
shoes_group_clear(SHOES_GROUP_OS *group)
{
  group->radios = NULL;
  group->layout = NULL;
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
  int x, y, newy;
#ifdef GTK3
  GtkAllocation a;
  gtk_widget_get_allocation(self_t->slot->oscanvas, &a);
  gtk_widget_translate_coordinates(self_t->slot->oscanvas, pc->slot->oscanvas, 0, 0, &x, &y);
  newy = (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly;

  if (x != self_t->place.ix + self_t->place.dx || y != newy)
    gtk_fixed_move(GTK_FIXED(pc->slot->oscanvas), self_t->slot->oscanvas,
        self_t->place.ix + self_t->place.dx, newy);
  if (a.width != self_t->place.iw || a.height != self_t->place.ih)
    gtk_widget_set_size_request(self_t->slot->oscanvas, self_t->place.iw, self_t->place.ih);
#else
  GtkAllocation *a = &self_t->slot->oscanvas->allocation;
  gtk_widget_translate_coordinates(self_t->slot->oscanvas, pc->slot->oscanvas, 0, 0, &x, &y);
  newy = (self_t->place.iy + self_t->place.dy) - pc->slot->scrolly;

  if (x != self_t->place.ix + self_t->place.dx || y != newy)
    gtk_fixed_move(GTK_FIXED(pc->slot->oscanvas), self_t->slot->oscanvas,
        self_t->place.ix + self_t->place.dx, newy);
  if (a->width != self_t->place.iw || a->height != self_t->place.ih)
    gtk_widget_set_size_request(self_t->slot->oscanvas, self_t->place.iw, self_t->place.ih);
#endif
}

void
shoes_native_canvas_resize(shoes_canvas *canvas)
{
}

static void
shoes_widget_changed(GtkWidget *ref, gpointer data)
{
  VALUE self = (VALUE)data;
  shoes_control_send(self, s_change);
}

void
shoes_native_control_hide(SHOES_CONTROL_REF ref)
{
  gtk_widget_hide(ref);
}

void
shoes_native_control_show(SHOES_CONTROL_REF ref)
{
  gtk_widget_show(ref);
}

void
shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
  shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
  gtk_widget_set_size_request(ref, p2->iw, p2->ih);
  gtk_fixed_put(GTK_FIXED(canvas->slot->oscanvas), ref, p2->ix + p2->dx, p2->iy + p2->dy);
  gtk_widget_show_all(ref);
}

void
shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
  shoes_canvas *canvas, shoes_place *p2)
{
  p2->iy -= canvas->slot->scrolly;
  if (CHANGED_COORDS()) {
    PLACE_COORDS();
    gtk_fixed_move(GTK_FIXED(canvas->slot->oscanvas),
      ref, p2->ix + p2->dx, p2->iy + p2->dy);
    gtk_widget_set_size_request(ref, p2->iw, p2->ih);
  }
  p2->iy += canvas->slot->scrolly;
}

void
shoes_native_control_focus(SHOES_CONTROL_REF ref)
{
#ifdef GTK3
  if (gtk_widget_get_can_focus(ref)) gtk_widget_grab_focus(ref);
#else
  if (GTK_WIDGET_CAN_FOCUS(ref)) gtk_widget_grab_focus(ref);
#endif
}

void
shoes_native_control_state(SHOES_CONTROL_REF ref, gboolean sensitive, gboolean setting)
{
  gtk_widget_set_sensitive(ref, sensitive);
  if (GTK_IS_EDITABLE(ref))
    gtk_editable_set_editable(GTK_EDITABLE(ref), setting);
  else if (GTK_IS_SCROLLED_WINDOW(ref))
  {
    GtkWidget *textview;
    GTK_CHILD(textview, ref);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), setting);
  }
}

void
shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas)
{
  gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), ref);
}

void
shoes_native_control_free(SHOES_CONTROL_REF ref)
{
  //
  // no need to free gtk widgets, since gtk seems
  // to garbage collect them fine.  and memory
  // addresses often get reused.
  //
}


/*
 * video support
 */

// SHOES_SURFACE_REF and SHOES_CONTROL_REF expands the same : GtkWidget *
// ref in shoes_video struct was a SHOES_CONTROL_REF anyway
SHOES_CONTROL_REF
shoes_native_surface_new2(VALUE attr)
{
  SHOES_CONTROL_REF da = gtk_drawing_area_new();
  VALUE uc;
  if (!NIL_P(attr)) uc = ATTR(attr, bg_color);

  // TODO (better with GtkStyleProvider)
  shoes_color *col;
  GdkRGBA color = {.0, .0, .0, 1.0};
  if (!NIL_P(uc)) {
     Data_Get_Struct(uc, shoes_color, col);
     color.red = col->r*255; color.green = col->g*255; color.blue = col->b*255;
  }
  gtk_widget_override_background_color(GTK_WIDGET(da), 0, &color);  

  return da;
}

void
shoes_native_surface_remove(shoes_canvas *canvas, SHOES_CONTROL_REF ref)
{
  gtk_container_remove(GTK_CONTAINER(canvas->slot->oscanvas), ref);
}

/* doing this directly on control now
 * 
void
shoes_native_surface_position(SHOES_SURFACE_REF ref, shoes_place *p1,
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  shoes_native_control_position((SHOES_CONTROL_REF)ref, p1, self, canvas, p2);
}

void
//shoes_native_surface_hide(SHOES_SURFACE_REF ref)
shoes_native_surface_hide(SHOES_CONTROL_REF ref)
{
  shoes_native_control_hide(ref);
}

void
//shoes_native_surface_show(SHOES_SURFACE_REF ref)
shoes_native_surface_show(SHOES_CONTROL_REF ref)
{
  shoes_native_control_show(ref);
}
*/



static gboolean
shoes_button_gtk_clicked(GtkButton *button, gpointer data)
{
  VALUE self = (VALUE)data;
  shoes_control_send(self, s_click);
  return TRUE;
}

SHOES_CONTROL_REF
shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, char *msg)
{
#ifdef GTK3
  SHOES_CONTROL_REF ref = gtk_button_alt_new_with_label(_(msg));
#else
  SHOES_CONTROL_REF ref = gtk_button_new_with_label(_(msg));
#endif

  g_signal_connect(G_OBJECT(ref), "clicked",
                   G_CALLBACK(shoes_button_gtk_clicked),
                   (gpointer)self);
  return ref;
}

void
shoes_native_secrecy(SHOES_CONTROL_REF ref)
{
  gtk_entry_set_visibility(GTK_ENTRY(ref), FALSE);
  gtk_entry_set_invisible_char(GTK_ENTRY(ref), SHOES_GTK_INVISIBLE_CHAR);
}

// cjc 8-19-2014
void
shoes_native_enterkey(GtkWidget *ref, gpointer data)
{
  VALUE self = (VALUE)data;
  GET_STRUCT(control, self_t);
  VALUE click = ATTR(self_t->attr, donekey);
  if (!NIL_P(click))
  {
    shoes_safe_block(self_t->parent, click, rb_ary_new3(1, self));
  }
}

SHOES_CONTROL_REF
shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
#ifdef GTK3
  SHOES_CONTROL_REF ref = gtk_entry_alt_new();
#else
  SHOES_CONTROL_REF ref = gtk_entry_new();
#endif
  if (RTEST(ATTR(attr, secret))) shoes_native_secrecy(ref);
  gtk_entry_set_text(GTK_ENTRY(ref), _(msg));
  g_signal_connect(G_OBJECT(ref), "changed",
                   G_CALLBACK(shoes_widget_changed),
                   (gpointer)self);
  // cjc: try to intercept \n  bug 860 @ shoes4
  g_signal_connect(G_OBJECT(ref), "activate",
                   G_CALLBACK(shoes_native_enterkey), // fix name?
                   (gpointer)self);

  return ref;
}

VALUE
shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref)
{
  return rb_str_new2(gtk_entry_get_text(GTK_ENTRY(ref)));
}

void
shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  gtk_entry_set_text(GTK_ENTRY(ref), _(msg));
}

VALUE
shoes_native_edit_line_cursor_to_end(SHOES_CONTROL_REF ref)
{
  gtk_editable_set_position(GTK_EDITABLE(ref), -1);
  return Qnil;
}

SHOES_CONTROL_REF
shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  GtkTextBuffer *buffer;
  GtkWidget* textview = gtk_text_view_new();
#ifdef GTK3  
  SHOES_CONTROL_REF ref = gtk_scrolled_window_alt_new(NULL, NULL);
#else  
  SHOES_CONTROL_REF ref = gtk_scrolled_window_new(NULL, NULL);
#endif  
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_set_text(buffer, _(msg), -1);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ref),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(ref), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(ref), textview);
  g_signal_connect(G_OBJECT(buffer), "changed",
                   G_CALLBACK(shoes_widget_changed),
                   (gpointer)self);
  return ref;
}

VALUE
shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter begin, end;
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  return rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
}

void
shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_set_text(buffer, _(msg), -1);
}

void
shoes_native_edit_box_append(SHOES_CONTROL_REF ref, char *msg)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter begin, end;
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  gtk_text_buffer_insert(buffer, &end, msg, strlen(msg));
  gtk_text_buffer_get_bounds(buffer, &begin, &end);

}

void 
shoes_native_edit_box_scroll_to_end(SHOES_CONTROL_REF ref)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextIter end;
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_get_end_iter (buffer, &end); 
   /* get the current ( cursor )mark name */
  GtkTextMark *insert_mark = gtk_text_buffer_get_insert (buffer);
 
  /* move mark and selection bound to the end */
  gtk_text_buffer_place_cursor(buffer, &end);

  /* scroll to the end view */
  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (textview),
            insert_mark, 0.0, TRUE, 0.0, 1.0); 
}


SHOES_CONTROL_REF
shoes_native_text_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  GtkTextBuffer *buffer;
  GtkWidget* textview = gtk_text_view_new();
#ifdef GTK3  
  SHOES_CONTROL_REF ref = gtk_scrolled_window_alt_new(NULL, NULL);
#else  
  SHOES_CONTROL_REF ref = gtk_scrolled_window_new(NULL, NULL);
#endif  
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
  buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_set_text(buffer, _(msg), -1);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ref),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(ref), GTK_SHADOW_IN);
  gtk_container_add(GTK_CONTAINER(ref), textview);
  g_signal_connect(G_OBJECT(buffer), "changed",
                   G_CALLBACK(shoes_widget_changed),
                   (gpointer)self);
  return ref;
}

VALUE
shoes_native_text_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter begin, end;
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  return rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
}

void
shoes_native_text_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_set_text(buffer, _(msg), -1);
}

VALUE
shoes_native_text_edit_box_append(SHOES_CONTROL_REF ref, char *msg)
{
  GtkWidget *textview;
  GTK_CHILD(textview, ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter begin, end;
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  gtk_text_buffer_insert(buffer, &end, msg, strlen(msg));
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  // TODO: return something useful 
  return Qnil;
  //return rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
}
// -- end of shoes_native_text_edit_box methods

SHOES_CONTROL_REF
shoes_native_list_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
#ifdef GTK3
  /*get bottom margin : following macro gives us bmargin (also lmargin,tmargin,rmargin)*/
  ATTR_MARGINS(attr, 0, canvas);
  
  //SHOES_CONTROL_REF ref = gtk_combo_box_text_new();
  SHOES_CONTROL_REF ref = gtk_combo_box_text_alt_new(attr, bmargin);
#else
  SHOES_CONTROL_REF ref = gtk_combo_box_new_text();
#endif
  g_signal_connect(G_OBJECT(ref), "changed",
                   G_CALLBACK(shoes_widget_changed),
                   (gpointer)self);
  return ref;
}

void
shoes_native_list_box_update(SHOES_CONTROL_REF combo, VALUE ary)
{
  long i;
  gtk_list_store_clear(GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(combo))));
  for (i = 0; i < RARRAY_LEN(ary); i++)
  {
    VALUE msg = shoes_native_to_s(rb_ary_entry(ary, i));
#ifdef GTK3
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), NULL, _(RSTRING_PTR(msg)));
#else
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _(RSTRING_PTR(msg)));
#endif
  }
}

VALUE
shoes_native_list_box_get_active(SHOES_CONTROL_REF ref, VALUE items)
{
  int sel = gtk_combo_box_get_active(GTK_COMBO_BOX(ref));
  if (sel >= 0)
    return rb_ary_entry(items, sel);
  return Qnil;
}

void
shoes_native_list_box_set_active(SHOES_CONTROL_REF combo, VALUE ary, VALUE item)
{
  int idx = rb_ary_index_of(ary, item);
  if (idx < 0) return;
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), idx);
}

SHOES_CONTROL_REF
shoes_native_progress(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
#ifdef GTK3  
  SHOES_CONTROL_REF ref = gtk_progress_bar_alt_new();
#else  
  SHOES_CONTROL_REF ref = gtk_progress_bar_new();
#endif  
  gtk_progress_bar_set_text(GTK_PROGRESS_BAR(ref), _(msg));
  return ref;
}

double
shoes_native_progress_get_fraction(SHOES_CONTROL_REF ref)
{
  return gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(ref));
}

void
shoes_native_progress_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
  gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(ref), perc);
}

SHOES_CONTROL_REF
shoes_native_slider(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
#ifdef GTK3
  SHOES_CONTROL_REF ref = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0., 1., 0.01);
#else
  SHOES_CONTROL_REF ref = gtk_hscale_new_with_range(0., 1., 0.01);
#endif
  gtk_scale_set_draw_value(GTK_SCALE(ref), FALSE);
  g_signal_connect(G_OBJECT(ref), "value-changed",
                   G_CALLBACK(shoes_widget_changed), (gpointer)self);
  return ref;
}

double
shoes_native_slider_get_fraction(SHOES_CONTROL_REF ref)
{
  return gtk_range_get_value(GTK_RANGE(ref));
}

void
shoes_native_slider_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
  gtk_range_set_value(GTK_RANGE(ref), perc);
}

// cjc bug264 - don't trigger callback on setup.
SHOES_CONTROL_REF
shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  SHOES_CONTROL_REF ref = gtk_check_button_new();
  // set visual state before connecting signal
  if (RTEST(ATTR(attr, checked)))
  {
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ref), TRUE);
  }
  g_signal_connect(G_OBJECT(ref), "clicked",
                   G_CALLBACK(shoes_button_gtk_clicked),
                   (gpointer)self);
  return ref;
}

VALUE
shoes_native_check_get(SHOES_CONTROL_REF ref)
{
  return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref)) ? Qtrue : Qfalse;
}

void
shoes_native_check_set(SHOES_CONTROL_REF ref, int on)
{
  // bug264 - don't toggle if already set to desired state.
  gboolean new_state;
  new_state = on ? TRUE : FALSE;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ref)) != new_state)
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ref), new_state);
}

SHOES_CONTROL_REF
shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, VALUE group)
{
  SHOES_CONTROL_REF ref;
  GSList *list = NULL;
  if (!NIL_P(group))
  {
    shoes_control *lctrl;
    VALUE leader = rb_ary_entry(group, 0);
    Data_Get_Struct(leader, shoes_control, lctrl);
    list = gtk_radio_button_get_group(GTK_RADIO_BUTTON(lctrl->ref));
  }
  ref = gtk_radio_button_new(list);
  g_signal_connect(G_OBJECT(ref), "clicked",
                   G_CALLBACK(shoes_button_gtk_clicked),
                   (gpointer)self);
  return ref;
}

static gboolean
shoes_gtk_animate(gpointer data)
{
  VALUE timer = (VALUE)data;
  shoes_timer *self_t;
  Data_Get_Struct(timer, shoes_timer, self_t);
  if (self_t->started == ANIM_STARTED)
    shoes_timer_call(timer);
  return self_t->started == ANIM_STARTED;
}

void
shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref)
{
  g_source_remove(ref);
}

SHOES_TIMER_REF
shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval)
{
  return g_timeout_add(interval, shoes_gtk_animate, (gpointer)self);
}

VALUE
shoes_native_clipboard_get(shoes_app *app)
{
  //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  if (gtk_clipboard_wait_is_text_available(primary))
  {
    gchar *string = gtk_clipboard_wait_for_text(primary);
    return rb_str_new2(string);
  }
  return Qnil;
}

void
shoes_native_clipboard_set(shoes_app *app, VALUE string)
{
  //GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
  gtk_clipboard_set_text(primary, RSTRING_PTR(string), RSTRING_LEN(string));
}

VALUE
shoes_native_to_s(VALUE text)
{
  text = rb_funcall(text, s_to_s, 0);
  return text;
}

#if defined(GTK3) && !defined(SHOES_GTK_WIN32)
//  NOTE: These are untested. I can't find where shoes calls them
//  (or if it does) cjc
// called by window_plain and dialog_plain (tested on linux)
// a start at styling windows and dialogs differently ?
VALUE
shoes_native_window_color(shoes_app *app)
{
  GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
  GdkRGBA bg;
  gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
  return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
    (int)(bg.blue * 255) , SHOES_COLOR_OPAQUE);
}

VALUE
shoes_native_dialog_color(shoes_app *app)
{
  GdkRGBA bg;
  GtkStyleContext *style = gtk_widget_get_style_context(GTK_WIDGET(APP_WINDOW(app)));
  gtk_style_context_lookup_color(style, GTK_STATE_NORMAL, &bg);
  return shoes_color_new((int)(bg.red * 255), (int)(bg.green * 255),
    (int)(bg.blue * 255) , SHOES_COLOR_OPAQUE);
}
#else
VALUE
shoes_native_window_color(shoes_app *app)
{
  GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
  GdkColor bg = style->bg[GTK_STATE_NORMAL];
  return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257 , SHOES_COLOR_OPAQUE);
}

VALUE
shoes_native_dialog_color(shoes_app *app)
{
  GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(app)));
  GdkColor bg = style->bg[GTK_STATE_NORMAL];
  return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257 , SHOES_COLOR_OPAQUE);
}
#endif

VALUE
shoes_dialog_alert(int argc, VALUE *argv, VALUE self)
{
    //GLOBAL_APP(app);
    //ACTUAL_APP(app);
    GTK_APP_VAR(app);

    char *apptitle = RSTRING_PTR(app->title); //default is "Shoes"
    char atitle[50];
    g_sprintf(atitle, "%s says", apptitle);
    rb_arg_list args;
    rb_parse_args(argc, argv, "S|h", &args);
    char *msg = RSTRING_PTR(shoes_native_to_s(args.a[0]));

    gchar *format_string = "<span size='larger'>%s</span>\n\n%s";
    if (argc == 2)
    {
       if (RTEST(ATTR(args.a[1], title)))
        {
			VALUE tmpstr = ATTR(args.a[1], title);
            strcpy(atitle,RSTRING_PTR(shoes_native_to_s(tmpstr)));
        }
        else
        {
            g_stpcpy(atitle," ");
        }
    }

    GtkWidget *dialog = gtk_message_dialog_new_with_markup(
            APP_WINDOW(app), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
            format_string, atitle, msg );

    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return Qnil;
}

VALUE
shoes_dialog_ask(int argc, VALUE *argv, VALUE self)
{
  char atitle[50];
  //GLOBAL_APP(app);
  //ACTUAL_APP(app);
  GTK_APP_VAR(app);

  char *apptitle = RSTRING_PTR(app->title);
  VALUE answer = Qnil;
  rb_arg_list args;
  rb_parse_args(argc, argv, "S|h", &args);

    switch(argc)
    {
    case 1:
        sprintf(atitle, "%s asks", apptitle);
        break;
    case 2:
        if (RTEST(ATTR(args.a[1], title)))
        {
		  VALUE tmpstr = ATTR(args.a[1], title);
          strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
		}
        else
        {
            g_stpcpy(atitle," ");
        }
        break;
    }

  // GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle,
  // APP_WINDOW(app), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
   GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, APP_WINDOW(app), GTK_DIALOG_MODAL,
#ifdef GTK3
    _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);
#else
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
#endif

  gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
#ifdef GTK3
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);
#else
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 6);
#endif
  GtkWidget *question = gtk_label_new(RSTRING_PTR(shoes_native_to_s(args.a[0])));
  gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
  GtkWidget *_answer = gtk_entry_new();
  if (RTEST(ATTR(args.a[1], secret))) shoes_native_secrecy(_answer);
#ifdef GTK3
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), _answer, FALSE, TRUE, 3);
#else
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), question, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), _answer, FALSE, TRUE, 3);
#endif
  gtk_widget_show_all(dialog);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
  {
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(_answer));
    answer = rb_str_new2(txt);
  }
  gtk_widget_destroy(dialog);
  return answer;
}


VALUE
shoes_dialog_confirm(int argc, VALUE *argv, VALUE self)
{
  VALUE answer = Qfalse;
  char atitle[50];
  //GLOBAL_APP(app);
  GTK_APP_VAR(app);
  char *apptitle = RSTRING_PTR(app->title);
  rb_arg_list args;
  rb_parse_args(argc, argv, "S|h", &args);
  VALUE quiz = shoes_native_to_s(args.a[0]);

    switch(argc)
    {
    case 1:
        sprintf(atitle, "%s asks", apptitle);
        break;
    case 2:
        if (RTEST(ATTR(args.a[1], title)))
        {
		  VALUE tmpstr = ATTR(args.a[1], title);
          strcpy(atitle, RSTRING_PTR(shoes_native_to_s(tmpstr)));
		}
        else
        {
            g_stpcpy(atitle," ");
        }
         break;
    }



  GtkWidget *dialog = gtk_dialog_new_with_buttons(atitle, APP_WINDOW(app), GTK_DIALOG_MODAL,
#ifdef GTK3
    _("_Cancel"), GTK_RESPONSE_CANCEL, _("_OK"), GTK_RESPONSE_OK, NULL);
#else
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
#endif

  gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
#ifdef GTK3
  gtk_container_set_border_width(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), 6);
#else
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 6);
#endif

  GtkWidget *question = gtk_label_new(RSTRING_PTR(quiz));
  gtk_misc_set_alignment(GTK_MISC(question), 0, 0);

#ifdef GTK3
  gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), question, FALSE, FALSE, 3);
#else
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), question, FALSE, FALSE, 3);
#endif

  gtk_widget_show_all(dialog);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
    answer = Qtrue;
  gtk_widget_destroy(dialog);
  return answer;

}

VALUE
shoes_dialog_color(VALUE self, VALUE title)
{
  VALUE color = Qnil;
  GLOBAL_APP(app);
  title = shoes_native_to_s(title);
#if defined(GTK3) 
  GtkWidget *dialog = gtk_color_chooser_dialog_new(RSTRING_PTR(title), NULL);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
  {
    GdkRGBA _color;
    gtk_color_chooser_get_rgba((GtkColorChooser *)dialog, &_color);
    color = shoes_color_new((int)(_color.red*255), (int)(_color.green*255),
      (int)(_color.blue*255), SHOES_COLOR_OPAQUE);
  }
#else
  GtkWidget *dialog = gtk_color_selection_dialog_new(RSTRING_PTR(title));
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
  {
    GdkColor _color;
    gtk_color_selection_get_current_color(
      GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel),
      &_color);
    color = shoes_color_new(_color.red/256, _color.green/256, _color.blue/256, SHOES_COLOR_OPAQUE);
  }
#endif
  gtk_widget_destroy(dialog);
  return color;
}

#ifdef GTK3
VALUE
#else
static VALUE
#endif
shoes_dialog_chooser(VALUE self, char *title, GtkFileChooserAction act, const gchar *button, VALUE attr)
{
  VALUE path = Qnil;
  GLOBAL_APP(app);
  GtkWidget *dialog = gtk_file_chooser_dialog_new(title, APP_WINDOW(app), act,
#ifdef GTK3
    _("_Cancel"), GTK_RESPONSE_CANCEL, button, GTK_RESPONSE_ACCEPT, NULL);
#else
    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, button, GTK_RESPONSE_ACCEPT, NULL);
#endif
  if (act == GTK_FILE_CHOOSER_ACTION_SAVE)
    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
  if(RTEST(shoes_hash_get(attr, rb_intern("save"))))
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog),
            RSTRING_PTR(shoes_hash_get(attr, rb_intern("save"))));
  if(RTEST(shoes_hash_get(attr, rb_intern("types"))) && TYPE(shoes_hash_get(attr, rb_intern("types"))) == T_HASH) {
    VALUE hsh = shoes_hash_get(attr, rb_intern("types"));
    VALUE keys = rb_funcall(hsh, s_keys, 0);
    int i;
    for(i = 0; i < RARRAY_LEN(keys); i++) {
      VALUE key = rb_ary_entry(keys, i);
      VALUE val = rb_hash_aref(hsh, key);
      GtkFileFilter *ff = gtk_file_filter_new();
      gtk_file_filter_set_name(ff, RSTRING_PTR(key));
      gtk_file_filter_add_pattern(ff, RSTRING_PTR(val));
      gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), ff);
    }
  }
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    path = rb_str_new2(filename);
  }
  gtk_widget_destroy(dialog);
  return path;
}

VALUE
shoes_dialog_open(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "|h", &args);
  return shoes_dialog_chooser(self, "Open file...", GTK_FILE_CHOOSER_ACTION_OPEN,
#ifdef GTK3
    _("_Open"), args.a[0]);
#else
    GTK_STOCK_OPEN, args.a[0]);
#endif
}

VALUE
shoes_dialog_save(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "|h", &args);
  return shoes_dialog_chooser(self, "Save file...", GTK_FILE_CHOOSER_ACTION_SAVE,
#ifdef GTK3
    _("_Save"), args.a[0]);
#else
    GTK_STOCK_SAVE, args.a[0]);
#endif
}

VALUE
shoes_dialog_open_folder(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "|h", &args);
  return shoes_dialog_chooser(self, "Open folder...", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
#ifdef GTK3
    _("_Open"), args.a[0]);
#else
    GTK_STOCK_OPEN, args.a[0]);
#endif
}

VALUE
shoes_dialog_save_folder(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "|h", &args);
  return shoes_dialog_chooser(self, "Save folder...", GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
#ifdef GTK3
    _("_Save"), args.a[0]);
#else
    GTK_STOCK_SAVE, args.a[0]);
#endif
}

// June 1, 2015 - kind of ugly. 
#ifdef SHOES_GTK_WIN32
// hat tip: https://justcheckingonall.wordpress.com/2008/08/29/console-window-win32-app/
#include <stdio.h>
#include <io.h>
#include <fcntl.h>

// called from main.c(skel) on Windows - works fine
static FILE* shoes_console_out = NULL;
static FILE* shoes_console_in = NULL;

int shoes_win32_console()
{
	
    if (AllocConsole() == 0) {
      // cshoes.exe can get here
      printf("Already have console\n");
      return 0;
    }

    HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
    int hCrt = _open_osfhandle((long) handle_out, _O_TEXT);
    FILE* hf_out = _fdopen(hCrt, "w");
    setvbuf(hf_out, NULL, _IONBF, 1);
    *stdout = *hf_out;
    HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
    hCrt = _open_osfhandle((long) handle_in, _O_TEXT);
    FILE* hf_in = _fdopen(hCrt, "r");
    setvbuf(hf_in, NULL, _IONBF, 128);
    *stdin = *hf_in;
    
    //* stash handles 
    shoes_console_out = hf_out;
    shoes_console_in = hf_in;
    return 1;
}

// Called by Shoes after ruby/gtk/shoes is initialized and running
int shoes_native_console()
{
	// has a console been setup by --console flag?
	if (shoes_console_out == NULL) {
	  if (shoes_win32_console() == 0) // cshoes.exe can do this
	     return 1;
	}
	// convert the (cached) FILE * for what ruby wants for fd[0], [1]...
    if (dup2(_fileno(shoes_console_out), 1) == -1)
      printf("failed dup2 of stdout\n");
    if (dup2(_fileno(shoes_console_out), 2) == -1)
      printf("failed dup2 of stderr\n");
    if (dup2(_fileno(shoes_console_in), 0) == -1)
      printf("failed dup2 of stdin\n");
    printf("created win32 console\n");
    return 1;
}
#else
int shoes_native_console()
{
  printf("init gtk console\n");
  shoes_native_app_console();
  printf("gtk\010k\t console \t\tcreated\n"); //test \b \t in string
  //int i;
  //for (i=0; i < 24; i++) printf("Line %d\n", i+1);
  return 1;
}
#endif
