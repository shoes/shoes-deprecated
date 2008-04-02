//
// shoes/canvas.c
// Ruby methods for all the drawing ops.
//
#include "shoes/internal.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/world.h"

#define SETUP() \
  shoes_canvas *canvas; \
  cairo_t *cr; \
  Data_Get_Struct(self, shoes_canvas, canvas); \
  cr = canvas->cr

const double PIM2   = 6.28318530717958647693;
const double PI     = 3.14159265358979323846;
const double RAD2PI = 0.01745329251994329577;

static void shoes_canvas_send_start(VALUE);

#ifdef SHOES_GTK
static void
shoes_canvas_gtk_paint_children(GtkWidget *widget, gpointer data)
{
  shoes_canvas *canvas = (shoes_canvas *)data;
  gtk_container_propagate_expose(GTK_CONTAINER(canvas->slot.canvas), widget, canvas->slot.expose);
}

static void
shoes_canvas_gtk_paint (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{ 
  GtkRequisition req;
  VALUE c = (VALUE)data;
  shoes_canvas *canvas;
  Data_Get_Struct(c, shoes_canvas, canvas);
  shoes_canvas_paint(c);
  canvas->slot.expose = event;
  gtk_container_forall(GTK_CONTAINER(widget), shoes_canvas_gtk_paint_children, canvas);
  canvas->slot.expose = NULL;
}

static gboolean
shoes_canvas_gtk_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{ 
  GdkModifierType state;
  VALUE c = (VALUE)data;
  if (!event->is_hint)
  {
    state = (GdkModifierType)event->state;
    shoes_canvas_send_motion(c, (int)event->x, (int)event->y, Qnil);
  }
  return TRUE;
}

static gboolean
shoes_canvas_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data)
{ 
  VALUE c = (VALUE)data;
  if (event->type == GDK_BUTTON_PRESS)
  {
    shoes_canvas_send_click(c, event->button, event->x, event->y);
  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    shoes_canvas_send_release(c, event->button, event->x, event->y);
  }
  return TRUE;
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
#endif

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(c, shoes_canvas, canvas);
  slot = &canvas->slot;

#ifdef SHOES_GTK
  slot->box = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(slot->box), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_widget_set_size_request(slot->box, width, height);
  slot->canvas = gtk_layout_new(NULL, NULL);
  gtk_widget_set_events(slot->canvas,
    gtk_widget_get_events(slot->canvas) | GDK_POINTER_MOTION_MASK | GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(G_OBJECT(slot->canvas), "expose-event",
                   G_CALLBACK(shoes_canvas_gtk_paint), (gpointer)c);
  if (!toplevel)
    g_signal_connect(G_OBJECT(slot->canvas), "motion-notify-event",
                     G_CALLBACK(shoes_canvas_gtk_motion), (gpointer)c);
  g_signal_connect(G_OBJECT(slot->canvas), "button-press-event",
                   G_CALLBACK(shoes_canvas_gtk_button), (gpointer)c);
  g_signal_connect(G_OBJECT(slot->canvas), "button-release-event",
                   G_CALLBACK(shoes_canvas_gtk_button), (gpointer)c);
  g_signal_connect(G_OBJECT(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(slot->box))), 
                   "value-changed", G_CALLBACK(shoes_canvas_gtk_scroll), (gpointer)c);
  if (toplevel)
    gtk_container_add(GTK_CONTAINER(parent->canvas), slot->box);
  else
    gtk_layout_put(GTK_LAYOUT(parent->canvas), slot->box, x, y);
  gtk_container_add(GTK_CONTAINER(slot->box), slot->canvas);
  GTK_LAYOUT(slot->canvas)->hadjustment->step_increment = 5;
  GTK_LAYOUT(slot->canvas)->vadjustment->step_increment = 5;
  slot->expose = NULL;
#endif

#ifdef SHOES_QUARTZ
  slot->controls = parent->controls;
  shoes_slot_quartz_create(c, parent, x, y, width, height);
#endif

#ifdef SHOES_WIN32
  if (toplevel)
  {
    slot->dc = parent->dc;
    slot->window = parent->window;
    slot->controls = parent->controls;
  }
  else
  {
    slot->controls = rb_ary_new();
    slot->dc = NULL;
    slot->window = CreateWindowEx(0, SHOES_SLOTCLASS, "Shoes Slot Window",
      WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VISIBLE,
      x, y, width, height, parent->window, NULL, 
      (HINSTANCE)GetWindowLong(parent->window, GWL_HINSTANCE), NULL);
    SetWindowLong(slot->window, GWL_USERDATA, (long)c);
  }
#endif

  if (toplevel) shoes_canvas_size(c, width, height);
  INFO("shoes_slot_init(%d, %d)\n", width, height);
}

cairo_t *
shoes_cairo_create(SHOES_SLOT_OS *slot, int width, int height, int border)
{
  cairo_t *cr;
#ifdef SHOES_GTK
  cr = gdk_cairo_create(GTK_LAYOUT(slot->canvas)->bin_window);
#endif
#ifdef SHOES_WIN32
  cr = cairo_create(slot->surface);
#endif
#ifdef SHOES_QUARTZ
  cr = cairo_create(slot->surface);
#endif
  cairo_save(cr);
  return cr;
}

VALUE
shoes_canvas_owner(VALUE self)
{
  SETUP();
  return canvas->app->owner;
}

VALUE
shoes_canvas_close(VALUE self)
{
  SETUP();
  return shoes_app_close_window(canvas->app);
}

VALUE
shoes_canvas_get_scroll_top(VALUE self)
{
  GET_STRUCT(canvas, canvas);
  return INT2NUM(canvas->slot.scrolly);
}

VALUE
shoes_canvas_set_scroll_top(VALUE self, VALUE num)
{
  SETUP();
  canvas->slot.scrolly = NUM2INT(num);
#ifdef SHOES_GTK
  GtkRange *r = GTK_RANGE(gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(canvas->slot.box)));
  gtk_range_set_value(r, canvas->slot.scrolly);
#endif
#ifdef SHOES_WIN32
  SetScrollPos(canvas->slot.window, SB_VERT, canvas->slot.scrolly, TRUE);
#endif
  shoes_canvas_repaint_all(self);
  return num;
}

VALUE
shoes_canvas_get_scroll_max(VALUE self)
{
  SETUP();
  return INT2NUM(max(0, canvas->fully - canvas->height));
}

VALUE
shoes_canvas_get_scroll_height(VALUE self)
{
  SETUP();
  return INT2NUM(canvas->fully);
}

VALUE
shoes_canvas_get_gutter_width(VALUE self)
{
  int scrollwidth = 0;
  GET_STRUCT(canvas, canvas);
#ifdef SHOES_WIN32
  scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
#endif
#ifdef SHOES_QUARTZ
  GetThemeMetric(kThemeMetricScrollBarWidth, (SInt32 *)&scrollwidth);
#endif
#ifdef SHOES_GTK
  GtkRequisition req;
  GtkWidget *vsb = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(canvas->slot.box));
  gtk_widget_size_request(vsb, &req);
  scrollwidth = req.width;
#endif
  return INT2NUM(scrollwidth);
}

VALUE
shoes_canvas_style(int argc, VALUE *argv, VALUE self)
{
  VALUE klass, attr;
  SETUP();

  rb_scan_args(argc, argv, "02", &klass, &attr);
  if (!NIL_P(attr))
    shoes_app_style(canvas->app, klass, attr);
  else if (!NIL_P(klass))
  {
    if (NIL_P(canvas->attr)) canvas->attr = rb_hash_new();
    rb_funcall(canvas->attr, s_update, 1, klass);
  }

  return canvas->attr;
}

void
shoes_canvas_paint(VALUE self)
{
  int n = 0;
  shoes_code code = SHOES_OK;

  if (self == Qnil)
    return;

  SETUP();

#ifdef SHOES_QUARTZ
  canvas->slot.surface = cairo_quartz_surface_create_for_cg_context(canvas->slot.context, canvas->width, canvas->height);
#endif

#ifdef SHOES_WIN32
  PAINTSTRUCT paint_struct;
  int width, height;
  HBITMAP bitmap, bitold;
  width = canvas->width; height = canvas->height;
  HDC hdc = BeginPaint(canvas->slot.window, &paint_struct);
  canvas->slot.dc = CreateCompatibleDC(hdc);
  bitmap = CreateCompatibleBitmap(hdc, width, max(canvas->height, canvas->fully));
  bitold = (HBITMAP)SelectObject(canvas->slot.dc, bitmap);
  canvas->slot.surface = cairo_win32_surface_create(canvas->slot.dc);
#endif

  INFO("shoes_cairo_create: (%d, %d)\n", canvas->width, canvas->height);
  if (canvas->cr != NULL)
    cairo_destroy(canvas->cr);

  canvas->cr = cr = shoes_cairo_create(&canvas->slot, canvas->width, canvas->height, 0);
  shoes_canvas_draw(self, self, Qfalse);
  shoes_canvas_draw(self, self, Qtrue);
  cairo_restore(cr);

  if (cairo_status(cr)) {
    QUIT("Cairo is unhappy: %s\n", cairo_status_to_string (cairo_status (cr)));
  }

  cairo_destroy(cr);
  canvas->cr = NULL;

#ifdef SHOES_QUARTZ
  cairo_surface_destroy(canvas->slot.surface);
#endif

#ifdef SHOES_WIN32
  BitBlt(hdc, 0, 0, width, height, canvas->slot.dc, 0, canvas->slot.scrolly, SRCCOPY);
  cairo_surface_destroy(canvas->slot.surface);
  EndPaint(canvas->slot.window, &paint_struct);
  SelectObject(canvas->slot.dc, bitold);
  DeleteObject(bitmap);
  DeleteDC(canvas->slot.dc);
#endif

  shoes_canvas_send_start(self);
quit:
  return;
}

void
shoes_apply_transformation(shoes_canvas *canvas, cairo_matrix_t *tf, 
  double x, double y, double w, double h, VALUE mode)
{
  if (tf)
  {
    w /= 2.; h /= 2.;

    if (mode == s_center)
    {
      cairo_translate(canvas->cr, w, h);
      cairo_transform(canvas->cr, tf);
    }
    else
    {
      cairo_translate(canvas->cr, -x, -y);
      cairo_transform(canvas->cr, tf);
      cairo_translate(canvas->cr, x + w, y + h);
    }
  }
}

void
shoes_canvas_shape_do(shoes_canvas *canvas, double x, double y, double w, double h, unsigned char center)
{
  if (center)
  {
    w = 0.; h = 0.;
  }

  cairo_save(canvas->cr);
  shoes_apply_transformation(canvas, canvas->tf, x, y, w, h, canvas->mode);
}

static VALUE
shoes_canvas_shape_end(VALUE self, VALUE x, VALUE y, int w, int h)
{
  VALUE shape;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  cairo_restore(canvas->cr);
  shape = shoes_shape_new(cairo_copy_path(canvas->cr), self, x, y, w, h);
  rb_ary_push(canvas->contents, shape);
  return shape;
}

static void
shoes_canvas_mark(shoes_canvas *canvas)
{
  rb_gc_mark_maybe(canvas->fg);
  rb_gc_mark_maybe(canvas->bg);
  rb_gc_mark_maybe(canvas->contents);
  rb_gc_mark_maybe(canvas->click);
  rb_gc_mark_maybe(canvas->release);
  rb_gc_mark_maybe(canvas->motion);
  rb_gc_mark_maybe(canvas->keypress);
  rb_gc_mark_maybe(canvas->start);
  rb_gc_mark_maybe(canvas->finish);
  rb_gc_mark_maybe(canvas->attr);
  rb_gc_mark_maybe(canvas->parent);
}

static void
shoes_canvas_free(shoes_canvas *canvas)
{
  free(canvas->gr);
  RUBY_CRITICAL(free(canvas));
}

VALUE
shoes_canvas_alloc(VALUE klass)
{
  shoes_canvas *canvas = SHOE_ALLOC(shoes_canvas);
  SHOE_MEMZERO(canvas, shoes_canvas, 1);
  canvas->app = NULL;
  canvas->stage = CANVAS_NADA;
  canvas->width = 0;
  canvas->height = 0;
  canvas->grl = 1;
  canvas->grt = 8;
  canvas->gr = SHOE_ALLOC_N(cairo_matrix_t, canvas->grt);
  canvas->contents = Qnil;
  cairo_matrix_init_identity(canvas->gr);
  VALUE rb_canvas = Data_Wrap_Struct(klass, shoes_canvas_mark, shoes_canvas_free, canvas);
  return rb_canvas;
}

VALUE
shoes_canvas_new(VALUE klass, shoes_app *app)
{
  shoes_canvas *canvas;
  VALUE self = shoes_canvas_alloc(klass);
  Data_Get_Struct(self, shoes_canvas, canvas);
  canvas->app = app;
  return self;
}

static void
shoes_canvas_empty(shoes_canvas *canvas)
{
  shoes_ele_remove_all(canvas->contents);
}

void
shoes_canvas_clear(VALUE self)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  if (canvas->cr != NULL)
    cairo_destroy(canvas->cr);
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));;
  canvas->sw = 1.;
  canvas->fg = rb_funcall(shoes_color_new(0, 0, 0, 0xFF), s_to_pattern, 0);
  canvas->bg = rb_funcall(shoes_color_new(0, 0, 0, 0xFF), s_to_pattern, 0);
  canvas->mode = s_center;
  canvas->parent = Qnil;
  canvas->attr = Qnil;
  canvas->grl = 1;
  cairo_matrix_init_identity(canvas->gr);
  canvas->tf = canvas->gr;
  shoes_canvas_empty(canvas);
  canvas->contents = rb_ary_new();
  canvas->place.x = canvas->place.y = 0;
  canvas->place.ix = canvas->place.iy = 0;
  canvas->cx = 0;
  canvas->cy = 0;
  canvas->endy = 0;
  canvas->endx = 0;
  canvas->topy = 0;
  canvas->fully = 0;
  canvas->click = Qnil;
  canvas->release = Qnil;
  canvas->motion = Qnil;
  canvas->release = Qnil;
  canvas->keypress = Qnil;
  canvas->start = Qnil;
  canvas->finish = Qnil;
#ifdef SHOES_GTK
  canvas->radios = NULL;
  canvas->layout = NULL;
#endif
}

shoes_canvas *
shoes_canvas_init(VALUE self, SHOES_SLOT_OS slot, VALUE attr, int width, int height)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  // canvas->slot = slot;
  canvas->attr = attr;
  canvas->place.iw = canvas->place.w = canvas->width = width;
  canvas->place.ih = canvas->place.h = canvas->height = height;
  return canvas;
}

VALUE
shoes_canvas_nostroke(VALUE self)
{
  SETUP();
  canvas->fg = Qnil;
  return self;
}

VALUE
shoes_canvas_stroke(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  shoes_pattern *pattern;
  SETUP();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  pat = rb_funcall(pat, s_to_pattern, 0);
  canvas->fg = pat;
  return pat;
}

VALUE
shoes_canvas_strokewidth(VALUE self, VALUE _w)
{
  double w = NUM2DBL(_w);
  SETUP();
  canvas->sw = w;
  return self;
}

VALUE
shoes_canvas_nofill(VALUE self)
{
  SETUP();
  canvas->bg = Qnil;
  return self;
}

VALUE
shoes_canvas_fill(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  shoes_pattern *pattern;
  SETUP();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  pat = rb_funcall(pat, s_to_pattern, 0);
  canvas->bg = pat;
  return pat;
}

VALUE
shoes_canvas_rect(int argc, VALUE *argv, VALUE self)
{
  VALUE _x, _y, _w, _h, _r;
  VALUE center = Qfalse;
  double x, y, w, h, r, rc;
  SETUP();

  argc = rb_scan_args(argc, argv, "14", &_x, &_y, &_w, &_h, &_r);

  if (argc == 1 && rb_obj_is_kind_of(_x, rb_cHash))
  {
    VALUE hsh = _x;
    _x = ATTR(hsh, left);
    _y = ATTR(hsh, top);
    _w = ATTR(hsh, width);
    _h = ATTR(hsh, height);
    _r = ATTR(hsh, radius);
    if (!NIL_P(ATTR(hsh, center))) center = ATTR(hsh, center);
  }

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);
  w = NUM2DBL(_w);
  h = NUM2DBL(_h);
  r = 0.0;
  if (!NIL_P(_r)) r = NUM2DBL(_r);

  shoes_canvas_shape_do(canvas, x, y, w, h, RTEST(center));
  shoes_cairo_rect(cr, -w / 2., -h / 2., w, h, r);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), w, h);
}

VALUE
shoes_canvas_oval(int argc, VALUE *argv, VALUE self)
{
  VALUE _x, _y, _w, _h;
  VALUE center = Qfalse;
  double x, y, w, h;
  SETUP();

  argc = rb_scan_args(argc, argv, "13", &_x, &_y, &_w, &_h);

  if (argc == 1 && rb_obj_is_kind_of(_x, rb_cHash))
  {
    VALUE hsh = _x;
    _x = _y = INT2NUM(0);
    _h = _w = ATTR(hsh, radius);
    if (!NIL_P(ATTR(hsh, left))) _x = ATTR(hsh, left);
    if (!NIL_P(ATTR(hsh, top))) _y = ATTR(hsh, top);
    if (!NIL_P(ATTR(hsh, width))) _w = ATTR(hsh, width);
    if (!NIL_P(ATTR(hsh, height))) _h = ATTR(hsh, height);
    if (!NIL_P(ATTR(hsh, center))) center = ATTR(hsh, center);
  }

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);
  h = w = NUM2DBL(_w);
  if (!NIL_P(_h)) h = NUM2DBL(_h);

  shoes_canvas_shape_do(canvas, x, y, w, h, RTEST(center));
  cairo_scale(cr, w / 2., h / 2.);
  cairo_move_to(cr, 0, 0);
  cairo_new_path(cr);
  cairo_arc(cr, 0., 0., 1., 0., PIM2);
  cairo_close_path(cr);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), w, h);
}

VALUE
shoes_canvas_line(VALUE self, VALUE _x1, VALUE _y1, VALUE _x2, VALUE _y2)
{
  double x, y, x1, y1, x2, y2, w, h;
  SETUP();

  x1 = NUM2DBL(_x1);
  y1 = NUM2DBL(_y1);
  x2 = NUM2DBL(_x2);
  y2 = NUM2DBL(_y2);

  x = ((x2 - x1) / 2.) + x1;
  y = ((y2 - y1) / 2.) + y1;
  w = x2 - x1;
  if (x1 > x2) w = x1 - x2;
  h = y2 - y1;
  if (y1 > y2) h = y1 - y2;
  shoes_canvas_shape_do(canvas, x, y, 0, 0, FALSE);
  cairo_new_path(cr);
  cairo_move_to(cr, x1 - x, y1 - y);
  cairo_line_to(cr, x2 - x, y2 - y);
  cairo_close_path(cr);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), w, h);
}

VALUE
shoes_canvas_arrow(VALUE self, VALUE _x, VALUE _y, VALUE _w)
{
  double x, y, w, h, tip;
  SETUP();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);
  w = NUM2DBL(_w);
  h = w * 0.8;
  tip = w * 0.42;

  shoes_canvas_shape_do(canvas, x, y, -w, 0, FALSE);
  cairo_new_path(cr);
  cairo_move_to(cr, w / 2., 0);
  cairo_rel_line_to(cr, -tip, +(h*0.5));
  cairo_rel_line_to(cr, 0, -(h*0.25));
  cairo_rel_line_to(cr, -(w-tip), 0);
  cairo_rel_line_to(cr, 0, -(h*0.5));
  cairo_rel_line_to(cr, +(w-tip), 0);
  cairo_rel_line_to(cr, 0, -(h*0.25));
  cairo_close_path(cr);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), w, h);
}

VALUE
shoes_canvas_star(int argc, VALUE *argv, VALUE self)
{
  VALUE _x, _y, _points, _outer, _inner;
  double x, y, outer, inner, theta;
  int i, points;
  SETUP();

  rb_scan_args(argc, argv, "23", &_x, &_y, &_points, &_outer, &_inner);
  x = NUM2DBL(_x);
  y = NUM2DBL(_y);
  points = 10;
  if (!NIL_P(_points)) points = NUM2INT(_points);
  outer = 100.0;
  if (!NIL_P(_outer)) outer = NUM2DBL(_outer);
  inner = 50.0;
  if (!NIL_P(_inner)) inner = NUM2DBL(_inner);

  theta = (points - 1) * PI / (points * 1.);
  shoes_canvas_shape_do(canvas, 0, 0, 0, 0, FALSE); /* TODO: find star's center */
  cairo_new_path(cr);
  cairo_move_to(cr, x, y);
  for (i = 0; i < points - 1; i++) {
    cairo_rel_line_to(cr, outer, 0);
    cairo_rotate(cr, theta);
  }
  cairo_close_path(cr);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), (int)outer, (int)outer);
}

#define MARKUP_BLOCK(klass) \
  text = shoes_textblock_new(klass, msgs, attr, self); \
  rb_ary_push(canvas->contents, text)

#define MARKUP_INLINE(klass) \
  text = shoes_text_new(klass, msgs, attr)

#define MARKUP_DEF(mname, fname, klass) \
  VALUE \
  shoes_canvas_##mname(int argc, VALUE *argv, VALUE self) \
  { \
    long i; \
    VALUE msgs, attr, text; \
    SETUP(); \
    msgs = rb_ary_new(); \
    attr = Qnil; \
    for (i = 0; i < argc; i++) \
    { \
      if (rb_obj_is_kind_of(argv[i], rb_cHash)) \
        attr = argv[i]; \
      else \
        rb_ary_push(msgs, argv[i]); \
    } \
    MARKUP_##fname(klass); \
    return text; \
  }

MARKUP_DEF(para, BLOCK, cPara);
MARKUP_DEF(banner, BLOCK, cBanner);
MARKUP_DEF(title, BLOCK, cTitle);
MARKUP_DEF(subtitle, BLOCK, cSubtitle);
MARKUP_DEF(tagline, BLOCK, cTagline);
MARKUP_DEF(caption, BLOCK, cCaption);
MARKUP_DEF(inscription, BLOCK, cInscription);

MARKUP_DEF(code, INLINE, cCode);
MARKUP_DEF(del, INLINE, cDel);
MARKUP_DEF(em, INLINE, cEm);
MARKUP_DEF(ins, INLINE, cIns);
MARKUP_DEF(span, INLINE, cSpan);
MARKUP_DEF(strong, INLINE, cStrong);
MARKUP_DEF(sub, INLINE, cSub);
MARKUP_DEF(sup, INLINE, cSup);

VALUE
shoes_canvas_link(int argc, VALUE *argv, VALUE self)
{
  long i;
  VALUE msgs, attr, text;
  SETUP();
  msgs = rb_ary_new();
  attr = Qnil;
  for (i = 0; i < argc; i++)
  {
    if (rb_obj_is_kind_of(argv[i], rb_cHash))
      attr = argv[i];
    else
      rb_ary_push(msgs, argv[i]);
  }

  if (rb_block_given_p())
  {
    if (NIL_P(attr)) attr = rb_hash_new();
    rb_hash_aset(attr, ID2SYM(s_click), rb_block_proc());
  }

  MARKUP_INLINE(cLink);
  return text;
}

VALUE
shoes_canvas_imagesize(VALUE self, VALUE _path)
{
  cairo_surface_t *image = shoes_load_image(_path);
  if (image)
  {
    double w = cairo_image_surface_get_width(image);
    double h = cairo_image_surface_get_height(image);
    cairo_surface_destroy(image);
    return rb_ary_new3(2, INT2NUM(w), INT2NUM(h));
  }
  return Qnil;
}

VALUE
shoes_canvas_background(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  shoes_pattern *pattern;
  SETUP();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  if (!NIL_P(pat))
  {
    pat = rb_funcall(pat, s_to_pattern, 0);
    pat = shoes_subpattern_new(cBackground, pat, self);
    rb_ary_push(canvas->contents, pat);
  }
  return pat;
}

VALUE
shoes_canvas_border(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  shoes_pattern *pattern;
  SETUP();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  if (!NIL_P(pat))
  {
    pat = rb_funcall(pat, s_to_pattern, 0);
    pat = shoes_subpattern_new(cBorder, pat, self);
    rb_ary_push(canvas->contents, pat);
  }
  return pat;
}

VALUE
shoes_canvas_video(int argc, VALUE *argv, VALUE self)
{
#ifdef VIDEO
  VALUE path, attr, video;
  SETUP();

  rb_scan_args(argc, argv, "11", &path, &attr);
  video = shoes_video_new(cVideo, path, attr, self);
  rb_ary_push(canvas->contents, video);
  return video;
#else
  rb_raise(eNotImpl, "no video support");
#endif
}

VALUE
shoes_canvas_image(int argc, VALUE *argv, VALUE self)
{
  VALUE path, attr, image, block;
  SETUP();

  rb_scan_args(argc, argv, "11&", &path, &attr, &block);

  if (!NIL_P(block))
  {
    if (NIL_P(attr)) attr = rb_hash_new();
    rb_hash_aset(attr, ID2SYM(s_click), block);
  }
  image = shoes_image_new(cImage, path, attr, self, canvas->tf, canvas->mode);
  if (!NIL_P(image))
    rb_ary_push(canvas->contents, image);
  return image;
}

VALUE
shoes_canvas_animate(int argc, VALUE *argv, VALUE self)
{
  VALUE fps, block, anim;
  SETUP();

  rb_scan_args(argc, argv, "01&", &fps, &block);
  anim = shoes_timer_new(cAnim, fps, block, self);
  rb_ary_push(canvas->app->timers, anim);
  return anim;
}

VALUE
shoes_canvas_every(int argc, VALUE *argv, VALUE self)
{
  VALUE rate, block, ev;
  SETUP();

  rb_scan_args(argc, argv, "1&", &rate, &block);
  ev = shoes_timer_new(cEvery, rate, block, self);
  rb_ary_push(canvas->app->timers, ev);
  return ev;
}

VALUE
shoes_canvas_timer(int argc, VALUE *argv, VALUE self)
{
  VALUE period, block, timer;
  SETUP();

  rb_scan_args(argc, argv, "1&", &period, &block);
  timer = shoes_timer_new(cTimer, period, block, self);
  rb_ary_push(canvas->app->timers, timer);
  return timer;
}

VALUE
shoes_canvas_shape(int argc, VALUE *argv, VALUE self)
{
  VALUE _x, _y;
  double x, y;
  SETUP();

  rb_scan_args(argc, argv, "02", &_x, &_y);

  shoes_canvas_shape_do(canvas, 0, 0, 0, 0, FALSE); /* TODO: find path center */
  cairo_new_path(cr);
  if (!NIL_P(_x) && !NIL_P(_y))
  {
    x = NUM2DBL(_x);
    y = NUM2DBL(_y);
    cairo_move_to(cr, x, y);
  }
  if (rb_block_given_p())
  {
    rb_yield(Qnil);
  }
  cairo_close_path(cr);
  return shoes_canvas_shape_end(self, INT2NUM(x), INT2NUM(y), 40, 40); /* TODO: figure width and height */
}

VALUE
shoes_canvas_move_to(VALUE self, VALUE _x, VALUE _y)
{
  double x, y;
  SETUP();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);

  cairo_move_to(cr, x, y);
  return self;
}

VALUE
shoes_canvas_line_to(VALUE self, VALUE _x, VALUE _y)
{
  double x, y;
  SETUP();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);

  cairo_line_to(cr, x, y);
  return self;
}

VALUE
shoes_canvas_curve_to(VALUE self, VALUE _x1, VALUE _y1, VALUE _x2, VALUE _y2, VALUE _x3, VALUE _y3)
{
  double x1, y1, x2, y2, x3, y3;
  SETUP();

  x1 = NUM2DBL(_x1);
  y1 = NUM2DBL(_y1);
  x2 = NUM2DBL(_x2);
  y2 = NUM2DBL(_y2);
  x3 = NUM2DBL(_x3);
  y3 = NUM2DBL(_y3);

  cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
  return self;
}

VALUE
shoes_canvas_push(VALUE self)
{
  cairo_matrix_t *m;
  SETUP();

  m = canvas->tf;
  if (canvas->grl + 1 > canvas->grt)
  {
    canvas->grt += 8;
    SHOE_REALLOC_N(canvas->gr, cairo_matrix_t, canvas->grt);
  }
  canvas->tf = &canvas->gr[canvas->grl];
  canvas->grl++;
  cairo_matrix_init_identity(canvas->tf);
  cairo_matrix_multiply(canvas->tf, canvas->tf, m);
  return self;
}

VALUE
shoes_canvas_pop(VALUE self)
{
  SETUP();

  if (canvas->grl > 1)
  {
    canvas->grl--;
    canvas->tf = &canvas->gr[canvas->grl - 1];
  }
  return self;
}

VALUE
shoes_canvas_reset(VALUE self)
{
  SETUP();

  cairo_matrix_init_identity(canvas->tf);
  return self;
}

VALUE
shoes_canvas_button(int argc, VALUE *argv, VALUE self)
{
  VALUE text, attr, block, button;
  SETUP();
  rb_scan_args(argc, argv, "11&", &text, &attr, &block);

  if (!NIL_P(text))
    attr = shoes_hash_set(attr, s_text, text);

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_click, block);

  button = shoes_control_new(cButton, attr, self);
#ifdef SHOES_QUARTZ
  shoes_button_draw(button, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, button);
  return button;
}

VALUE
shoes_canvas_edit_line(int argc, VALUE *argv, VALUE self)
{
  VALUE phrase, attr, block, edit_line;
  SETUP();
  rb_scan_args(argc, argv, "02&", &phrase, &attr, &block);

  if (rb_obj_is_kind_of(phrase, rb_cHash))
    attr = phrase;
  else
  {
    if (NIL_P(attr)) attr = rb_hash_new();
    rb_hash_aset(attr, ID2SYM(s_text), phrase);
  }

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_change, block);

  edit_line = shoes_control_new(cEditLine, attr, self);
#ifdef SHOES_QUARTZ
  shoes_edit_line_draw(edit_line, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, edit_line);
  return edit_line;
}

VALUE
shoes_canvas_edit_box(int argc, VALUE *argv, VALUE self)
{
  VALUE phrase, attr, block, edit_box;
  SETUP();
  rb_scan_args(argc, argv, "02&", &phrase, &attr, &block);

  if (rb_obj_is_kind_of(phrase, rb_cHash))
    attr = phrase;
  else
  {
    if (NIL_P(attr)) attr = rb_hash_new();
    rb_hash_aset(attr, ID2SYM(s_text), phrase);
  }

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_change, block);

  edit_box = shoes_control_new(cEditBox, attr, self);
#ifdef SHOES_QUARTZ
  shoes_edit_box_draw(edit_box, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, edit_box);
  return edit_box;
}

VALUE
shoes_canvas_list_box(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, list_box;
  SETUP();
  rb_scan_args(argc, argv, "01&", &attr, &block);

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_change, block);

  list_box = shoes_control_new(cListBox, attr, self);
#ifdef SHOES_QUARTZ
  shoes_list_box_draw(list_box, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, list_box);
  return list_box;
}

VALUE
shoes_canvas_progress(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, progress;
  SETUP();
  rb_scan_args(argc, argv, "01", &attr);

  progress = shoes_control_new(cProgress, attr, self);
#ifdef SHOES_QUARTZ
  shoes_progress_draw(progress, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, progress);
  return progress;
}

VALUE
shoes_canvas_radio(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, radio;
  SETUP();
  rb_scan_args(argc, argv, "01&", &attr, &block);

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_click, block);

  radio = shoes_control_new(cRadio, attr, self);
#ifdef SHOES_QUARTZ
  shoes_radio_draw(radio, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, radio);
  return radio;
}

VALUE
shoes_canvas_check(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, check;
  SETUP();
  rb_scan_args(argc, argv, "01&", &attr, &block);

  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_click, block);

  check = shoes_control_new(cCheck, attr, self);
#ifdef SHOES_QUARTZ
  shoes_check_draw(check, self, Qtrue); 
#endif
  rb_ary_push(canvas->contents, check);
  return check;
}

VALUE
shoes_canvas_contents(VALUE self)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  return canvas->contents;
}

void
shoes_canvas_remove_item(VALUE self, VALUE item, char c, char t)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
#ifndef SHOES_GTK
  if (c)
  {
    i = rb_ary_index_of(self_t->slot.controls, item);
    if (i >= 0)
      rb_ary_insert_at(self_t->slot.controls, i, 1, Qnil);
  }
#endif
  if (t)
  {
    i = rb_ary_index_of(self_t->app->timers, item);
    if (i >= 0)
      rb_ary_insert_at(self_t->app->timers, i, 1, Qnil);
  }
  rb_ary_delete(self_t->contents, item);
}

static int
shoes_canvas_inherits(VALUE ele, shoes_canvas *pc)
{
  if (rb_obj_is_kind_of(ele, cCanvas))
  {
    shoes_canvas *c;
    Data_Get_Struct(ele, shoes_canvas, c);
    return (pc == c || DC(c->slot) == DC(pc->slot));
  }

  return TRUE;
}

int
shoes_canvas_independent(shoes_canvas *c)
{
  shoes_canvas *pc;
  if (NIL_P(c->parent)) return TRUE;

  Data_Get_Struct(c->parent, shoes_canvas, pc);
  return !(pc == c || DC(c->slot) == DC(pc->slot));
}

static void
shoes_canvas_reflow(shoes_canvas *self_t, VALUE c)
{
  VALUE attr = Qnil;
  shoes_canvas *parent;
  Data_Get_Struct(c, shoes_canvas, parent);

  self_t->cr = parent->cr;
  self_t->slot = parent->slot;
  shoes_place_decide(&self_t->place, c, self_t->attr, parent->place.iw, 0, REL_CANVAS, FALSE);
  self_t->width = self_t->place.w;
  self_t->height = self_t->place.h;

  self_t->cx = self_t->place.ix;
  self_t->cy = self_t->place.iy;
  self_t->endx = self_t->place.ix;
  self_t->endy = self_t->place.iy;
  INFO("REFLOW: %d, %d (%d, %d) / %d, %d / %d, %d (%d, %d)\n", self_t->cx, self_t->cy,
    self_t->endx, self_t->endy, self_t->place.x, self_t->place.y, self_t->width, self_t->height,
    parent->cx, parent->cy);
}

VALUE
shoes_canvas_remove(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  shoes_canvas_empty(self_t);
  if (!NIL_P(self_t->parent))
    shoes_canvas_remove_item(self_t->parent, self, 0, 0);
  return self;
}

VALUE
shoes_canvas_draw(VALUE self, VALUE c, VALUE actual)
{
  long i;
  shoes_canvas *self_t;
  shoes_canvas *canvas;
  VALUE ck = rb_obj_class(self);
  Data_Get_Struct(self, shoes_canvas, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

#ifdef SHOES_GTK
  if (!RTEST(actual))
    canvas->radios = NULL;
#endif

  INFO("DRAW\n");
  if (self_t->height > self_t->fully)
    self_t->fully = self_t->height;
  if (self_t != canvas)
  {
    shoes_canvas_reflow(self_t, c);
#ifdef SHOES_GTK
    self_t->slot.expose = canvas->slot.expose;
#endif
  }
  else
  {
    self_t->endx = self_t->cx = 0;
    self_t->topy = self_t->endy = self_t->cy = 0;
    if (!NIL_P(self_t->parent))
    {
      if (RTEST(actual))
      {
        shoes_canvas *pc;
        Data_Get_Struct(self_t->parent, shoes_canvas, pc);
#ifdef SHOES_GTK
        gtk_layout_move(GTK_LAYOUT(pc->slot.canvas), self_t->slot.box, self_t->place.ix, self_t->place.iy);
        gtk_widget_set_size_request(self_t->slot.box, self_t->place.iw, self_t->place.ih);
#endif
#ifdef SHOES_QUARTZ
        HIRect rect;
        rect.origin.x = self_t->place.ix * 1.;
        rect.origin.y = (self_t->place.iy * 1.) + 4;
        rect.size.width = (self_t->place.iw * 1.) + 4;
        rect.size.height = (self_t->place.ih * 1.) - 8;
        HIViewSetFrame(self_t->slot.scrollview, &rect);
#endif
#ifdef SHOES_WIN32
        MoveWindow(self_t->slot.window, self_t->place.ix, 
          self_t->place.iy - pc->slot.scrolly, self_t->place.iw, 
          self_t->place.ih, TRUE);
#endif
      }
    } 
    if (RTEST(actual))
    {
      cairo_set_source_rgba(self_t->cr, 1.0f, 1.0f, 1.0f, 1.0f);
      cairo_set_line_width(self_t->cr, 1.0);
      cairo_rectangle(self_t->cr, 0, 0, 4000, 4000);
      cairo_fill(self_t->cr);
    }
  }

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    VALUE masks = Qnil;
    cairo_t *cr = NULL, *crc = NULL, *crm = NULL;
    cairo_surface_t *surfc = NULL, *surfm = NULL;

    for (i = 0; i < RARRAY_LEN(self_t->contents); i++)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_class(ele) == cMask)
      {
        if (NIL_P(masks)) masks = rb_ary_new();
        rb_ary_push(masks, ele);
      }
    }

    if (!NIL_P(masks) && RTEST(actual))
    {
      cr = self_t->cr;
      surfc = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, canvas->place.iw, canvas->place.ih);
      surfm = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, canvas->place.iw, canvas->place.ih);
      crc = cairo_create(surfc);
      crm = cairo_create(surfm);
    }

    self_t->topy = canvas->cy;

    for (i = 0; i < RARRAY_LEN(self_t->contents); i++)
    {
      shoes_canvas *c1;
      VALUE ele = rb_ary_entry(self_t->contents, i);
      Data_Get_Struct(ele, shoes_canvas, c1);

      if (shoes_canvas_inherits(ele, self_t))
      {
        if (!NIL_P(masks) && RTEST(actual))
        {
          if (rb_obj_class(ele) == cMask)
            self_t->cr = crm;
          else
            self_t->cr = crc;
        }

        rb_funcall(ele, s_draw, 2, self, actual);

        if (rb_obj_is_kind_of(ele, cCanvas))
        {
          long j;
          //
          // update the height of all canvases in this row
          // 
          for (j = i - 1; j >= 0; j--)
          {
            shoes_canvas *c2;
            VALUE ele2 = rb_ary_entry(self_t->contents, j);
            if (rb_obj_is_kind_of(ele2, cCanvas))
            {
              Data_Get_Struct(ele2, shoes_canvas, c2);
              if (c2->topy < c1->topy || POS(c2->place) != REL_CANVAS)
                break;
              if (c1->fully > c2->fully)
                c2->fully = c1->fully;
              else
                c1->fully = c2->fully;
            }
          }
        }
      }
      else
      {
        shoes_place_decide(&c1->place, c1->parent, c1->attr, c1->width, c1->height, REL_CANVAS, FALSE);
        c1->height = c1->place.h;
        c1->width = c1->place.w;
        c1->place.flags |= FLAG_ORIGIN;
        if (!ABSY(c1->place)) {
          self_t->cx = c1->place.x + c1->place.w;
          self_t->cy = c1->place.y;
          self_t->endx = self_t->cx;
          self_t->endy = max(self_t->endy, c1->place.y + c1->place.h);
        }
        if (ck == cStack) {
          self_t->cx = self_t->place.x;
          self_t->cy = self_t->endy;
        }
        shoes_slot_repaint(&c1->slot);
      }
    }

    if (!NIL_P(masks) && RTEST(actual))
    {
      cairo_set_source_surface(cr, surfc, 0., 0.);
      cairo_mask_surface(cr, surfm, 0., 0.);
      cairo_surface_destroy(surfm);
      cairo_surface_destroy(surfc);
      cairo_destroy(crc);
      cairo_destroy(crm);
      self_t->cr = cr;
    }
  }

  if (self_t == canvas)
  {
    for (i = 0; i < RARRAY_LEN(self_t->app->timers); i++)
    {
      VALUE ele = rb_ary_entry(self_t->app->timers, i);
      rb_funcall(ele, s_draw, 2, self, actual);
    }
  }

  canvas->endx = canvas->cx = self_t->place.x + self_t->width;
  if (canvas->endy < self_t->endy)
    canvas->endy = self_t->endy;
      
#ifdef SHOES_GTK
  self_t->slot.expose = NULL;
#endif

  if (self_t == canvas || DC(self_t->slot) != DC(canvas->slot))
  {
    int endy = (int)self_t->endy;
    if (endy < self_t->height) endy = self_t->height;
    self_t->fully = endy;
    if (RTEST(actual))
    {
      self_t->slot.scrolly = min(self_t->slot.scrolly, self_t->fully - self_t->height);
#ifdef SHOES_GTK
      gtk_layout_set_size(GTK_LAYOUT(self_t->slot.canvas), self_t->width, endy);
#endif
#ifdef SHOES_QUARTZ
      HIRect hr;
      EventRef theEvent;

      HIViewGetFrame(self_t->slot.view, &hr);
      if (hr.size.width != (float)self_t->width || hr.size.height != (float)endy)
      {
        hr.size.width = (float)self_t->width;
        hr.size.height = (float)endy - (endy == self_t->height ? 8 : 0);
        HIViewSetFrame(self_t->slot.view, &hr);

        CreateEvent(NULL, kEventClassScrollable,
              kEventScrollableInfoChanged, 
              GetCurrentEventTime(),
              kEventAttributeUserEvent, 
              &theEvent);
        SendEventToEventTarget(theEvent, GetControlEventTarget(self_t->slot.scrollview));
        ReleaseEvent(theEvent);
      }
#endif
#ifdef SHOES_WIN32
      SCROLLINFO si;
      // maxScroll = max(canvas->fully - newHeight, 0);
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
      si.nMin = 0;
      si.nMax = self_t->fully - 1; 
      si.nPage = self_t->height;
      si.nPos = self_t->slot.scrolly;
      INFO("SetScrollInfo(%d, nMin: %d, nMax: %d, nPage: %d)\n", 
        si.nPos, si.nMin, si.nMax, si.nPage);
      SetScrollInfo(self_t->slot.window, SB_VERT, &si, TRUE);
#endif
    }
  }
  else
  {
    int bmargin = CPB(self_t);
    self_t->fully = canvas->endy = max(canvas->endy, self_t->endy + bmargin);
    self_t->place.ih = (canvas->endy - self_t->place.iy) - bmargin;
    self_t->place.h = canvas->endy - self_t->place.y;
  }

  if (RTEST(actual))
  {
    if (self_t->cr == canvas->cr)
      self_t->cr = NULL;
  }

  return self;
}

#define DRAW(c, app, blk) \
  { \
    unsigned char alter = 0; \
    if (RARRAY_LEN(app->nesting) == 0) \
    { \
      alter = 1; \
      rb_ary_push(app->nesting, app->nestslot); \
    } \
    rb_ary_push(app->nesting, c); \
    rb_funcall(blk, s_call, 0); \
    rb_ary_pop(app->nesting); \
    if (alter) \
      rb_ary_pop(app->nesting); \
  }


static void
shoes_canvas_memdraw(VALUE self, VALUE block)
{
  SETUP();
  if (canvas->cr != NULL)
    cairo_destroy(canvas->cr);
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));;
  DRAW(self, canvas->app, block);
}

void
shoes_canvas_compute(VALUE self)
{
  SETUP();
  if (canvas->cr != NULL)
    cairo_destroy(canvas->cr);
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));;
  shoes_canvas_draw(self, self, Qfalse);
  cairo_destroy(canvas->cr);
  canvas->cr = NULL;
}

static void
shoes_canvas_insert(VALUE self, long i, long mod, VALUE ele, VALUE block)
{
  VALUE ary;
  SETUP();

  if (!NIL_P(ele))
    i = rb_ary_index_of(canvas->contents, ele);
  if (i < 0)
    i += RARRAY_LEN(canvas->contents) + 1;

  ary = canvas->contents;
  canvas->contents = rb_ary_new();
  shoes_canvas_memdraw(self, block);
  rb_ary_insert_at(ary, i + mod, 0, canvas->contents);
  canvas->contents = ary;
  shoes_canvas_repaint_all(self);
}

VALUE
shoes_canvas_after(int argc, VALUE *argv, VALUE self)
{
  VALUE ele, block;
  rb_scan_args(argc, argv, "01&", &ele, &block);
  shoes_canvas_insert(self, -2, 1, ele, block);
  return self;
}

VALUE
shoes_canvas_before(int argc, VALUE *argv, VALUE self)
{
  VALUE ele, block;
  rb_scan_args(argc, argv, "01&", &ele, &block);
  shoes_canvas_insert(self, 0, 0, ele, block);
  return self;
}

VALUE
shoes_canvas_append(int argc, VALUE *argv, VALUE self)
{
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);
  shoes_canvas_insert(self, -2, 1, Qnil, block);
  return self;
}

VALUE
shoes_canvas_prepend(int argc, VALUE *argv, VALUE self)
{
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);
  shoes_canvas_insert(self, 0, 0, Qnil, block);
  return self;
}

VALUE
shoes_canvas_clear_contents(int argc, VALUE *argv, VALUE self)
{
  VALUE block;
  SETUP();

  rb_scan_args(argc, argv, "0&", &block);
  shoes_canvas_empty(canvas);
  if (!NIL_P(block))
    shoes_canvas_memdraw(self, block);
  shoes_canvas_repaint_all(self);
  return self;
}

VALUE
shoes_canvas_flow(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, flow;
  SETUP();

  rb_scan_args(argc, argv, "01&", &attr, &block);
  flow = shoes_flow_new(attr, self);
  if (!NIL_P(block))
  {
    DRAW(flow, canvas->app, block);
  }
  rb_ary_push(canvas->contents, flow);
  return flow;
}

VALUE
shoes_canvas_stack(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, stack;
  SETUP();

  rb_scan_args(argc, argv, "01&", &attr, &block);
  stack = shoes_stack_new(attr, self);
  if (!NIL_P(block))
  {
    DRAW(stack, canvas->app, block);
  }
  rb_ary_push(canvas->contents, stack);
  return stack;
}

VALUE
shoes_canvas_mask(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block, mask;
  SETUP();

  rb_scan_args(argc, argv, "01&", &attr, &block);
  mask = shoes_mask_new(attr, self);
  if (!NIL_P(block))
  {
    DRAW(mask, canvas->app, block);
  }
  rb_ary_push(canvas->contents, mask);
  return mask;
}

VALUE
shoes_canvas_widget(int argc, VALUE *argv, VALUE self)
{
  VALUE klass, attr, block, widget;
  SETUP();

  rb_scan_args(argc, argv, "11&", &klass, &attr, &block);
  widget = shoes_widget_new(klass, attr, self);
  if (!NIL_P(block))
  {
    DRAW(widget, canvas->app, block);
  }
  rb_ary_push(canvas->contents, widget);
  return widget;
}

void
shoes_canvas_size(VALUE self, int w, int h)
{
  SETUP();
  canvas->place.iw = canvas->place.w = canvas->width = w;
  canvas->place.ih = canvas->place.h = canvas->height = h;
#ifdef SHOES_QUARTZ
  HIRect hr;
  HIViewGetFrame(canvas->slot.scrollview, &hr);
  hr.size.width = canvas->width;
  hr.size.height = canvas->height;
  HIViewSetFrame(canvas->slot.scrollview, &hr);
#endif
}

void
shoes_canvas_repaint_all(VALUE self)
{
  SETUP();
  shoes_slot_repaint(&canvas->slot);
}

typedef VALUE (*ccallfunc)(VALUE);

static void
shoes_canvas_ccall(VALUE self, ccallfunc func)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  if (!NIL_P(self_t->contents))
  {
    long i;
    for (i = 0; i < RARRAY_LEN(self_t->contents); i++)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cNative))
        func(ele);
      else if (rb_obj_is_kind_of(ele, cCanvas))
        shoes_canvas_ccall(ele, func);
    }
  }
}

VALUE
shoes_canvas_hide(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  ATTRSET(self_t->attr, hidden, Qtrue);
  shoes_canvas_ccall(self, shoes_control_hide);
  shoes_canvas_repaint_all(self);
  return self;
}

VALUE
shoes_canvas_show(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  ATTRSET(self_t->attr, hidden, Qfalse);
  shoes_canvas_ccall(self, shoes_control_show);
  shoes_canvas_repaint_all(self);
  return self;
}

VALUE
shoes_canvas_toggle(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  ATTRSET(self_t->attr, hidden, ATTR(self_t->attr, hidden) == Qtrue ? Qfalse : Qtrue);
  shoes_canvas_repaint_all(self);
  return self;
}

#define EVENT_HANDLER(x) \
  VALUE \
  shoes_canvas_##x(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE val, block; \
    SETUP(); \
    rb_scan_args(argc, argv, "01&", &val, &block); \
    canvas->x = NIL_P(block) ? val : block; \
    return self; \
  }

EVENT_HANDLER(click);
EVENT_HANDLER(release);
EVENT_HANDLER(motion);
EVENT_HANDLER(keypress);
EVENT_HANDLER(start);
EVENT_HANDLER(finish);

static void
shoes_canvas_send_start(VALUE self)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);

  if (canvas->stage == CANVAS_NADA)
  {
    int i;
    canvas->stage = CANVAS_STARTED;

    for (i = RARRAY_LEN(canvas->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(canvas->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas) && shoes_canvas_inherits(ele, canvas))
        shoes_canvas_send_start(ele);
    }

    if (!NIL_P(canvas->start))
    {
      shoes_safe_block(self, canvas->start, rb_ary_new());
    }
  }
}

static VALUE
shoes_canvas_send_click2(VALUE self, int button, int x, int y, VALUE *clicked)
{
  long i;
  VALUE v = Qnil;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

#ifdef SHOES_QUARTZ
  if (ORIGIN(self_t->place))
  {
    x -= self_t->place.ix;
    y -= self_t->place.iy - self_t->slot.scrolly;
  }
#endif

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (!NIL_P(self_t->click))
    {
      shoes_safe_block(self, self_t->click, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
#ifndef SHOES_QUARTZ
        if (!shoes_canvas_inherits(ele, self_t))
          continue;
#endif
        v = shoes_canvas_send_click(ele, button, x, y);
        *clicked = ele;
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        v = shoes_textblock_send_click(ele, button, x, y, clicked);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        v = shoes_image_send_click(ele, button, x, y);
        *clicked = ele;
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        v = shoes_shape_send_click(ele, button, x, y);
        *clicked = ele;
      }

      if (!NIL_P(v))
        return v;
    }
  }

  return Qnil;
}

VALUE
shoes_canvas_mouse(VALUE self)
{
  int x = 0, y = 0, button = 0;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  return rb_ary_new3(3, INT2NUM(0), INT2NUM(self_t->app->mousex), INT2NUM(self_t->app->mousey));
}

VALUE
shoes_canvas_goto(VALUE self, VALUE url)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  shoes_app_goto(self_t->app, RSTRING_PTR(url));
  return self;
}

VALUE
shoes_canvas_send_click(VALUE self, int button, int x, int y)
{
  // INFO("click(%d, %d, %d)\n", button, x, y);
  VALUE clicked = Qnil;
  VALUE url = shoes_canvas_send_click2(self, button, x, y, &clicked);
  if (!NIL_P(url))
  {
    if (rb_obj_is_kind_of(url, rb_cProc))
      shoes_safe_block(self, url, rb_ary_new3(1, clicked));
    else
    {
      shoes_canvas *self_t;
      Data_Get_Struct(self, shoes_canvas, self_t);
      shoes_app_goto(self_t->app, RSTRING_PTR(url));
    }
  }
  return Qnil;
}

void
shoes_canvas_send_release(VALUE self, int button, int x, int y)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
#ifdef SHOES_QUARTZ
  if (ORIGIN(self_t->place))
  {
    x -= self_t->place.ix;
    y -= self_t->place.iy - self_t->slot.scrolly;
  }
#endif

  // INFO("release(%d, %d, %d)\n", button, x, y);

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (!NIL_P(self_t->release))
    {
      shoes_safe_block(self, self_t->release, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
#ifndef SHOES_QUARTZ
        if (!shoes_canvas_inherits(ele, self_t))
          continue;
#endif
        shoes_canvas_send_release(ele, button, x, y);
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        shoes_textblock_send_release(ele, button, x, y);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        shoes_image_send_release(ele, button, x, y);
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        shoes_shape_send_release(ele, button, x, y);
      }
    }
  }
}

VALUE
shoes_canvas_send_motion(VALUE self, int x, int y, VALUE url)
{
  int h = 0;
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

#ifdef SHOES_QUARTZ
  if (ORIGIN(self_t->place))
  {
    x -= self_t->place.ix;
    y -= self_t->place.iy - self_t->slot.scrolly;
  }
#endif

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (!NIL_P(self_t->motion))
    {
      shoes_safe_block(self, self_t->motion, rb_ary_new3(2, INT2NUM(x), INT2NUM(y)));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE urll = Qnil;
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
#ifndef SHOES_QUARTZ
        if (!shoes_canvas_inherits(ele, self_t))
          continue;
#endif
        urll = shoes_canvas_send_motion(ele, x, y, url);
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        urll = shoes_textblock_motion(ele, x, y, &h);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        urll = shoes_image_motion(ele, x, y, &h);
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        urll = shoes_shape_motion(ele, x, y, &h);
      }

      if (NIL_P(url)) url = urll;
    }

    if (NIL_P(url))
    {
      shoes_canvas *self_t;
      Data_Get_Struct(self, shoes_canvas, self_t);
      shoes_app_cursor(self_t->app, s_arrow);
    }
  }

  if (h)
  {
    shoes_canvas_repaint_all(self);
  }

  return url;
}

void
shoes_canvas_send_keypress(VALUE self, VALUE key)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (!NIL_P(self_t->keypress))
    {
      shoes_safe_block(self, self_t->keypress, rb_ary_new3(1, key));
    }
  }
}

VALUE
shoes_slot_new(VALUE klass, VALUE attr, VALUE parent)
{
  shoes_canvas *self_t, *pc;
  VALUE self = shoes_canvas_alloc(klass);
  shoes_canvas_clear(self);
  Data_Get_Struct(parent, shoes_canvas, pc);
  Data_Get_Struct(self, shoes_canvas, self_t);
  self_t->cr = pc->cr;
  self_t->slot = pc->slot;
  self_t->parent = parent;
  self_t->app = pc->app;
  self_t->attr = attr;
  if (!NIL_P(ATTR(self_t->attr, height))) {
    int x, y, w, h;
    x = ATTR2(int, self_t->attr, left, 0);
    y = ATTR2(int, self_t->attr, top, 0);
    w = ATTR2(int, self_t->attr, width, 100);
    h = ATTR2(int, self_t->attr, height, 100);

    shoes_slot_init(self, &pc->slot, x, y, w, h, FALSE);
#ifdef SHOES_GTK
    gtk_widget_show_all(self_t->slot.box);
    self_t->width = w - 20;
    self_t->height = h - 20;
#endif
    self_t->place.x = self_t->place.y = 0;
    self_t->place.ix = self_t->place.iy = 0;
    self_t->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
  } else {
    shoes_canvas_reflow(self_t, parent);
  }
  return self;
}

//
// Shoes::Flow
//
VALUE
shoes_flow_new(VALUE attr, VALUE parent)
{
  return shoes_slot_new(cFlow, attr, parent);
}

//
// Shoes::Stack
//
VALUE
shoes_stack_new(VALUE attr, VALUE parent)
{
  return shoes_slot_new(cStack, attr, parent);
}

//
// Shoes::Mask
//
VALUE
shoes_mask_new(VALUE attr, VALUE parent)
{
  return shoes_slot_new(cMask, attr, parent);
}

//
// Shoes::Widget
//
VALUE
shoes_widget_new(VALUE klass, VALUE attr, VALUE parent)
{
  return shoes_slot_new(klass, attr, parent);
}

#ifdef SHOES_QUARTZ
static char clip_buf[SHOES_BUFSIZE];

char*
shoes_apple_pasteboard_get(void)
{
  char *s, *t;
  CFArrayRef flavors;
  CFDataRef data;
  CFIndex nflavor, ndata, j;
  CFStringRef type;
  ItemCount nitem;
  PasteboardItemID id;
  PasteboardSyncFlags flags;
  PasteboardRef clip = shoes_world->os.clip;
  UInt32 i;

  flags = PasteboardSynchronize(clip);
  if(flags&kPasteboardClientIsOwner){
    s = strdup(clip_buf);
    return s;
  }
  if(PasteboardGetItemCount(clip, &nitem) != noErr){
    INFO("apple pasteboard get item count failed\n");
    return nil;
  }
  for(i=1; i<=nitem; i++){
    if(PasteboardGetItemIdentifier(clip, i, &id) != noErr)
      continue;
    if(PasteboardCopyItemFlavors(clip, id, &flavors) != noErr)
      continue;
    nflavor = CFArrayGetCount(flavors);
    for(j=0; j<nflavor; j++){
      type = (CFStringRef)CFArrayGetValueAtIndex(flavors, j);
      if(!UTTypeConformsTo(type, CFSTR("public.utf8-plain-text")))
        continue;
      if(PasteboardCopyItemFlavorData(clip, id, type, &data) != noErr)
        continue;
      ndata = CFDataGetLength(data);
      s = (char*)CFDataGetBytePtr(data);
      CFRelease(flavors);
      CFRelease(data);
      for(t=s; *t; t++)
        if(*t == '\r')
          *t = '\n';
      return s;
    }
    CFRelease(flavors);
  }
  return nil;   
}

void
shoes_apple_pasteboard_put(char *s)
{
  CFDataRef cfdata;
  PasteboardRef clip = shoes_world->os.clip;
  PasteboardSyncFlags flags;

  if(strlen(s) >= SHOES_BUFSIZE)
    return;
  strcpy(clip_buf, s);
  if(PasteboardClear(clip) != noErr){
    INFO("apple pasteboard clear failed\n");
    return;
  }
  flags = PasteboardSynchronize(clip);
  if((flags&kPasteboardModified) || !(flags&kPasteboardClientIsOwner)){
    INFO("apple pasteboard cannot assert ownership\n");
    return;
  }
  cfdata = CFDataCreate(kCFAllocatorDefault, 
    (unsigned char*)clip_buf, strlen(clip_buf)*2);
  if(cfdata == nil){
    INFO("apple pasteboard cfdatacreate failed\n");
    return;
  }
  if(PasteboardPutItemFlavor(clip, (PasteboardItemID)1,
    CFSTR("public.utf8-plain-text"), cfdata, 0) != noErr){
    INFO("apple pasteboard putitem failed\n");
    CFRelease(cfdata);
    return;
  }
  /* CFRelease(cfdata); ??? */
}
#endif

//
// Global clipboard getter and setter
//
VALUE
shoes_canvas_get_clipboard(VALUE self)
{
  VALUE paste = Qnil;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

#ifdef SHOES_GTK
  GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  if (gtk_clipboard_wait_is_text_available(primary))
  {
    gchar *string = gtk_clipboard_wait_for_text(primary);
    paste = rb_str_new2(string);
  }
#endif

#ifdef SHOES_QUARTZ
  char *str = shoes_apple_pasteboard_get();
  paste = rb_str_new2(str);
#endif

#ifdef SHOES_WIN32
  if (OpenClipboard(self_t->app->slot.window))
  {
    HANDLE hclip = GetClipboardData(CF_TEXT);
    char *buffer = (char *)GlobalLock(hclip);
    paste = rb_str_new2(buffer);
    GlobalUnlock(hclip);
    CloseClipboard();
  }
#endif

  return paste;
}

VALUE
shoes_canvas_set_clipboard(VALUE self, VALUE string)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  StringValue(string);

#ifdef SHOES_GTK
  GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
  gtk_clipboard_set_text(primary, RSTRING_PTR(string), RSTRING_LEN(string));
#endif

#ifdef SHOES_QUARTZ
  shoes_apple_pasteboard_put(RSTRING_PTR(string));
#endif

#ifdef SHOES_WIN32
  if (OpenClipboard(self_t->app->slot.window))
  {
    char *buffer;
    HGLOBAL hclip;
    EmptyClipboard();
    hclip = GlobalAlloc(GMEM_DDESHARE, RSTRING_LEN(string)+1);
    buffer = (char *)GlobalLock(hclip);
    strncpy(buffer, RSTRING_PTR(string), RSTRING_LEN(string)+1);
    GlobalUnlock(hclip);
    SetClipboardData(CF_TEXT, hclip);
    CloseClipboard();
  }
#endif

  return string;
}

//
// Window creation
//
VALUE
shoes_canvas_window(int argc, VALUE *argv, VALUE self)
{
  VALUE uri, attr, block, app;
  SETUP();

  if (rb_block_given_p())
    return shoes_app_window(argc, argv, cApp, self);

  rb_scan_args(argc, argv, "02&", &uri, &attr, &block);
  if (rb_obj_is_kind_of(uri, rb_cHash))
  {
    attr = uri;
    uri = Qnil;
  }

  if (!NIL_P(uri))
    shoes_load(RSTRING_PTR(uri));

  // TODO: do I send back an array of created App objects I guess?
  return Qnil;
}

VALUE
shoes_canvas_dialog(int argc, VALUE *argv, VALUE self)
{
  return shoes_app_window(argc, argv, cDialog, self);
}

VALUE
shoes_canvas_window_plain(VALUE self)
{
  SETUP();
#ifdef SHOES_GTK
  GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(canvas->app)));
  GdkColor bg = style->bg[GTK_STATE_NORMAL];
  return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257 , SHOES_COLOR_OPAQUE);
#endif
#ifdef SHOES_QUARTZ
  // ThemeBrush bg;
  // RGBColor _color;
  // HIWindowGetThemeBackground(canvas->app->os.window, &bg);
  // GetThemeBrushAsColor(bg, 32, true, &_color);
  // return shoes_color_new(_color.red/256, _color.green/256, _color.blue/256, SHOES_COLOR_OPAQUE);
  return shoes_color_new(255, 255, 255, 255);
#endif
#ifdef SHOES_WIN32
  DWORD winc = GetSysColor(COLOR_WINDOW);
  return shoes_color_new(GetRValue(winc), GetGValue(winc), GetBValue(winc), SHOES_COLOR_OPAQUE);
#endif
}

VALUE
shoes_canvas_dialog_plain(VALUE self)
{
  SETUP();
#ifdef SHOES_GTK
  GtkStyle *style = gtk_widget_get_style(GTK_WIDGET(APP_WINDOW(canvas->app)));
  GdkColor bg = style->bg[GTK_STATE_NORMAL];
  return shoes_color_new(bg.red / 257, bg.green / 257, bg.blue / 257 , SHOES_COLOR_OPAQUE);
#endif
#ifdef SHOES_QUARTZ
  return shoes_color_new(255, 255, 255, 255);
#endif
#ifdef SHOES_WIN32
  DWORD winc = GetSysColor(COLOR_3DFACE);
  return shoes_color_new(GetRValue(winc), GetGValue(winc), GetBValue(winc), SHOES_COLOR_OPAQUE);
#endif
}
