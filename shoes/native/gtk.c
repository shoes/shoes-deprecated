//
// shoes/native-gtk.c
// GTK+ code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

void shoes_native_init()
{
  gtk_init(NULL, NULL);
}

void shoes_native_cleanup(shoes_world_t *world)
{
}

void shoes_native_quit()
{
  gtk_main_quit();
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_reset(SHOES_SLOT_OS *slot) {}
void shoes_native_slot_clear(SHOES_SLOT_OS *slot)
{
  if (slot->vscroll)
  {
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(slot->vscroll));
    gtk_adjustment_set_value(adj, adj->lower);
  }
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
  gtk_widget_queue_draw(slot->canvas);
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
    GtkRequisition req;
    gtk_widget_size_request(slot->vscroll, &req);
    return req.width;
  }
  return 0;
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot)
{
}

//
// Window-level events
//
static VALUE
shoes_app_gtk_exception(VALUE v, VALUE exc)
{
  if (rb_obj_is_kind_of(exc, rb_eInterrupt))
    gtk_main_quit();
  return Qnil;
}

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
    shoes_app_motion(app, (int)event->x, (int)event->y + canvas->slot.scrolly);
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
    shoes_app_click(app, event->button, event->x, event->y + canvas->slot.scrolly);
  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    shoes_app_release(app, event->button, event->x, event->y + canvas->slot.scrolly);
  }
  return TRUE;
}

static void
shoes_app_gtk_paint (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{ 
  shoes_app *app = (shoes_app *)data;
  gtk_window_get_size(GTK_WINDOW(app->os.window), &app->width, &app->height);
  shoes_canvas_size(app->canvas, app->width, app->height);
}

#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_##name) \
    v = ID2SYM(rb_intern("" # sym))

static gboolean
shoes_app_gtk_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{ 
  VALUE v = Qnil;
  guint modifiers;
  shoes_app *app = (shoes_app *)data;
  if (event->length > 0)
  {
    if (event->string[0] == '\r' && event->length == 1)
      v = rb_str_new2("\n");
    else
      v = rb_str_new(event->string, event->length);
  }
  KEY_SYM(BackSpace, backspace);
  KEY_SYM(Tab, tab);
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

  if (SYMBOL_P(v))
  {
    if (event->state & GDK_MOD1_MASK)
      KEY_STATE(alt);
    if (event->state & GDK_SHIFT_MASK)
      KEY_STATE(shift);
    if (event->state & GDK_CONTROL_MASK)
      KEY_STATE(control);
  }
  else
  {
    if (event->state & GDK_MOD1_MASK)
      KEY_STATE(alt);
  }

  if (v != Qnil)
    shoes_app_keypress(app, v);
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
  gtk_container_propagate_expose(GTK_CONTAINER(canvas->slot.canvas), widget, canvas->slot.expose);
}

static void
shoes_canvas_gtk_paint(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{ 
  GtkRequisition req;
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  INFO("EXPOSE: (%d, %d) (%d, %d) %lu, %d, %d\n", event->area.x, event->area.y,
    event->area.width, event->area.height, event->window, (int)event->send_event, event->count);
  Data_Get_Struct(c, shoes_canvas, canvas);

  //
  // Since I'm using a GtkFixed container, I need to force it to be clipped on its boundaries.
  // This could be done by using a whole lot of gdk_window_begin_paint_region calls, but that
  // would also mean masking every region for every element... This approach is simple.  Clip
  // the expose region and pass it on.
  //
  canvas->slot.expose = event;
  GdkRegion *region = event->region;
  GdkRectangle rect = event->area;
  event->region = gdk_region_rectangle(&canvas->slot.canvas->allocation);
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
  canvas->slot.expose = NULL;
}

static void
shoes_canvas_gtk_size(GtkWidget *widget, GtkAllocation *size, gpointer data)
{
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  if (canvas->slot.vscroll && size->height != canvas->slot.scrollh && size->width != canvas->slot.scrollw)
  {
    GtkAdjustment *adj = gtk_range_get_adjustment(GTK_RANGE(canvas->slot.vscroll));
    gtk_widget_set_size_request(canvas->slot.vscroll, -1, size->height);
    gtk_fixed_move(GTK_FIXED(canvas->slot.canvas), canvas->slot.vscroll,
      size->width - canvas->slot.vscroll->allocation.width, 0);
    adj->page_size = size->height;
    if (adj->page_size >= adj->upper)
      gtk_widget_hide(canvas->slot.vscroll);
    else
      gtk_widget_show(canvas->slot.vscroll);
    canvas->slot.scrollh = size->height;
    canvas->slot.scrollw = size->width;
  }
}

static void
shoes_canvas_gtk_scroll(GtkRange *r, gpointer data)
{ 
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  canvas->slot.scrolly = (int)gtk_range_get_value(r);
  shoes_slot_repaint(&canvas->slot);
}

static gint                                                           
shoes_app_g_poll (GPollFD *fds, guint nfds, gint timeout)
{
  struct timeval tv;
  fd_set rset, wset, xset;
  GPollFD *f;
  int ready;
  int maxfd = 0;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  for (f = fds; f < &fds[nfds]; ++f)
     if (f->fd >= 0)
     {
       if (f->events & G_IO_IN)
         FD_SET (f->fd, &rset);
       if (f->events & G_IO_OUT)
         FD_SET (f->fd, &wset);
       if (f->events & G_IO_PRI)
         FD_SET (f->fd, &xset);
       if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
         maxfd = f->fd;
     }

  //
  // If we poll indefinitely, then the window updates will
  // pile up for as long as Ruby is churning away.
  //
  if (timeout == -1)
    timeout = 500;

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

  ready = rb_thread_select (maxfd + 1, &rset, &wset, &xset,
               timeout == -1 ? NULL : &tv);
  if (ready > 0)
     for (f = fds; f < &fds[nfds]; ++f)
     {
       f->revents = 0;
       if (f->fd >= 0)
       {
         if (FD_ISSET (f->fd, &rset))
           f->revents |= G_IO_IN;
         if (FD_ISSET (f->fd, &wset))
           f->revents |= G_IO_OUT;
         if (FD_ISSET (f->fd, &xset))
           f->revents |= G_IO_PRI;
       }
     }

  return ready;
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
  if (app->os.window == NULL || app->os.window->window == NULL || app->cursor == cursor)
    goto done;

  GdkCursor *c;
  if (cursor == s_hand)
  {
    c = gdk_cursor_new(GDK_HAND2);
  }
  else if (cursor == s_arrow)
  {
    c = gdk_cursor_new(GDK_ARROW);
  }
  else
    goto done;

  gdk_window_set_cursor(app->os.window->window, c);

  app->cursor = cursor;

done:
  return SHOES_OK;
}

void
shoes_native_app_resized(shoes_app *app)
{
  if (app->os.window != NULL)
    gtk_widget_set_size_request(app->os.window, app->width, app->height);
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
  gtk_window_set_title(GTK_WINDOW(app->os.window), _(msg));
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path)
{
  char icon_path[SHOES_BUFSIZE];
  shoes_app_gtk *gk = &app->os;
  shoes_slot_gtk *gs = &app->slot;

  sprintf(icon_path, "%s/static/shoes-icon.png", shoes_world->path);
  gtk_window_set_default_icon_from_file(icon_path, NULL);
  gk->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (!app->resizable)
    gtk_window_set_resizable(GTK_WINDOW(gk->window), FALSE);
  gtk_widget_set_events(gk->window, GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(gk->window), "size-allocate",
                   G_CALLBACK(shoes_app_gtk_paint), app);
  g_signal_connect(G_OBJECT(gk->window), "motion-notify-event", 
                   G_CALLBACK(shoes_app_gtk_motion), app);
  g_signal_connect(G_OBJECT(gk->window), "button-press-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "button-release-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "key-press-event",
                   G_CALLBACK(shoes_app_gtk_keypress), app);
  g_signal_connect(G_OBJECT(gk->window), "delete-event",
                   G_CALLBACK(shoes_app_gtk_quit), app);
  app->slot.canvas = gk->window;
  return SHOES_OK;
}

void
shoes_native_app_show(shoes_app *)
{
  gtk_widget_show_all(app->os.window);
}

void
shoes_native_loop()
{
  g_main_set_poll_func(shoes_app_g_poll);
  gtk_main();
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
  slot = &canvas->slot;

  slot->canvas = gtk_fixed_new();
  g_signal_connect(G_OBJECT(slot->canvas), "expose-event",
                   G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
  g_signal_connect(G_OBJECT(slot->canvas), "size-allocate",
                   G_CALLBACK(shoes_canvas_gtk_size), (gpointer)c);
  if (toplevel)
    gtk_container_add(GTK_CONTAINER(parent->canvas), slot->canvas);
  else
    gtk_fixed_put(GTK_FIXED(parent->canvas), slot->canvas, x, y);

  slot->scrollh = slot->scrollw = 0;
  slot->vscroll = NULL;
  if (scrolls)
  {
    slot->vscroll = gtk_vscrollbar_new(NULL);
    gtk_range_get_adjustment(GTK_RANGE(slot->vscroll))->step_increment = 5;
    g_signal_connect(G_OBJECT(slot->vscroll), "value-changed",
                     G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
    gtk_fixed_put(GTK_FIXED(slot->canvas), slot->vscroll, -100, -100);
  }

  gtk_widget_set_size_request(slot->canvas, width, height);
  slot->expose = NULL;
  if (toplevel) shoes_canvas_size(c, width, height);
}

cairo_t *
shoes_cairo_create(SHOES_SLOT_OS *slot, shoes_canvas *parent, int width, int height, int toplevel)
{
  cairo_t *cr = gdk_cairo_create(slot->canvas->window);
  if (slot->expose != NULL)
  {
    GdkRegion *region = gdk_region_rectangle(&slot->canvas->allocation);
    gdk_region_intersect(region, slot->expose->region);
    gdk_cairo_region(cr, region);
    cairo_clip(cr);
    cairo_translate(cr, slot->canvas->allocation.x, slot->canvas->allocation.y - slot->scrolly);
  }
  return cr;
}

void
shoes_cairo_destroy(SHOES_SLOT_OS *)
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
  GtkAllocation *a = &self_t->slot.canvas->allocation;
  int newy = (self_t->place.iy + self_t->place.dy) - pc->slot.scrolly;
  if (a->x != self_t->place.ix + self_t->place.dx || a->y != newy)
    gtk_fixed_move(GTK_FIXED(pc->slot.canvas), self_t->slot.canvas, 
        self_t->place.ix + self_t->place.dx, newy);
  if (a->width != self_t->place.iw || a->height != self_t->place.ih)
    gtk_widget_set_size_request(self_t->slot.canvas, self_t->place.iw, self_t->place.ih);
}
