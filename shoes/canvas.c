//
// shoes/canvas.c
// Ruby methods for all the drawing ops.
//
#include "shoes/internal.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/world.h"
#include "shoes/native.h"

#define SETUP() \
  shoes_canvas *canvas; \
  cairo_t *cr; \
  Data_Get_Struct(self, shoes_canvas, canvas); \
  cr = canvas->cr

const double SHOES_PIM2   = 6.28318530717958647693;
const double SHOES_PI     = 3.14159265358979323846;
const double SHOES_RAD2PI = 0.01745329251994329577;

const char *dialog_title = "Shoes asks:";
const char *dialog_title_says = "Shoes says:";

static void shoes_canvas_send_start(VALUE);

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
  shoes_slot_scroll_to(canvas, NUM2INT(num), 0);
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
  scrollwidth = shoes_native_slot_gutter(&canvas->slot);
  return INT2NUM(scrollwidth);
}

VALUE
shoes_canvas_displace(VALUE self, VALUE dx, VALUE dy)
{
  SETUP();
  ATTRSET(canvas->attr, displace_left, dx);
  ATTRSET(canvas->attr, displace_top, dy);
  shoes_canvas_repaint_all(canvas->parent);
  return self;
}

VALUE
shoes_canvas_move(VALUE self, VALUE x, VALUE y)
{
  SETUP();
  ATTRSET(canvas->attr, left, x);
  ATTRSET(canvas->attr, top, y);
  shoes_canvas_repaint_all(canvas->parent);
  return self;
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
    shoes_canvas_repaint_all(canvas->parent);
  }

  return canvas->attr;
}

#define ELAPSED ((double)(clock() - start) / CLOCKS_PER_SEC)

static VALUE
shoes_canvas_paint_call(VALUE self)
{
  int n = 0;
  shoes_canvas *pc = NULL;
  shoes_code code = SHOES_OK;
  clock_t start = clock();

  if (self == Qnil)
    return self;

  SETUP();

  canvas->cr = cr = shoes_cairo_create(canvas);
  if (cr == NULL)
    return self;

  cairo_save(cr);
  shoes_canvas_draw(self, self, Qfalse);
  INFO("COMPUTE: %0.6f s\n", ELAPSED);
  shoes_canvas_draw(self, self, Qtrue);
  INFO("DRAW: %0.6f s\n", ELAPSED);
  cairo_restore(cr);

  if (cairo_status(cr)) {
    code = SHOES_FAIL;
    PUTS("Cairo is unhappy: %s\n", cairo_status_to_string (cairo_status (cr)));
    goto quit;
  }

  cairo_destroy(cr);
  canvas->cr = NULL;

  shoes_cairo_destroy(canvas);
  INFO("PAINT: %0.6f s\n", ELAPSED);
  shoes_canvas_send_start(self);
quit:
  return self;
}

void
shoes_canvas_paint(VALUE self)
{
  rb_rescue2(CASTHOOK(shoes_canvas_paint_call), self, 
    CASTHOOK(shoes_canvas_error), self, rb_cObject, 0);
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

static void
shoes_add_ele(shoes_canvas *canvas, VALUE ele)
{
  if (canvas->insertion == -1)
    rb_ary_push(canvas->contents, ele);
  else
  {
    rb_ary_insert_at(canvas->contents, canvas->insertion, 0, ele);
    canvas->insertion++;
  }
}

static VALUE
shoes_canvas_shape_end(VALUE self, VALUE x, VALUE y, int w, int h)
{
  VALUE shape;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  cairo_restore(canvas->cr);
  shape = shoes_shape_new(cairo_copy_path(canvas->cr), self, x, y, w, h);
  shoes_add_ele(canvas, shape);
  return shape;
}

static void
shoes_canvas_mark(shoes_canvas *canvas)
{
  rb_gc_mark_maybe(canvas->fg);
  rb_gc_mark_maybe(canvas->bg);
  rb_gc_mark_maybe(canvas->contents);
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
  canvas->insertion = -1;
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
  canvas->cr = NULL;
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
  canvas->place.dx = canvas->place.dy = 0;
  canvas->place.ix = canvas->place.iy = 0;
  canvas->hover = 0;
  canvas->cx = 0;
  canvas->cy = 0;
  canvas->endy = 0;
  canvas->endx = 0;
  canvas->topy = 0;
  canvas->fully = 0;
  shoes_group_clear(&canvas->group);
}

shoes_canvas *
shoes_canvas_init(VALUE self, SHOES_SLOT_OS slot, VALUE attr, int width, int height)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  canvas->attr = attr;
  canvas->place.iw = canvas->place.w = canvas->width = width;
  canvas->place.ih = canvas->place.h = canvas->height = height;
  return canvas;
}

void
shoes_slot_scroll_to(shoes_canvas *canvas, int dy, int rel)
{
  if (rel)
    canvas->slot.scrolly += dy;
  else
    canvas->slot.scrolly = dy;

  if (canvas->slot.scrolly > canvas->endy - canvas->height)
    canvas->slot.scrolly = canvas->endy - canvas->height;
  if (canvas->slot.scrolly < 0)
    canvas->slot.scrolly = 0;
  shoes_native_slot_scroll_top(&canvas->slot);
  shoes_slot_repaint(&canvas->slot);
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
  cairo_arc(cr, 0., 0., 1., 0., SHOES_PIM2);
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

  theta = (points - 1) * SHOES_PI / (points * 1.);
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

VALUE
shoes_canvas_blur(int argc, VALUE *argv, VALUE self)
{
  VALUE x, y, fx, attr;
  SETUP();

  rb_scan_args(argc, argv, "02", &x, &y);
  if (NIL_P(y)) y = x;

  if (rb_obj_is_kind_of(x, rb_cHash))
    attr = x;
  else
  {
    attr = rb_hash_new();
    if (!NIL_P(x)) rb_hash_aset(attr, ID2SYM(s_width), x);
    if (!NIL_P(y)) rb_hash_aset(attr, ID2SYM(s_height), y);
  }

  fx = shoes_effect_new(cBlur, attr, self);
  shoes_add_ele(canvas, fx);
  return fx;
}

VALUE
shoes_canvas_glow(int argc, VALUE *argv, VALUE self)
{
  VALUE x, y, fx, attr;
  SETUP();

  rb_scan_args(argc, argv, "02", &x, &y);
  if (NIL_P(y)) y = x;

  if (rb_obj_is_kind_of(x, rb_cHash))
    attr = x;
  else
  {
    attr = rb_hash_new();
    if (!NIL_P(x)) rb_hash_aset(attr, ID2SYM(s_width), x);
    if (!NIL_P(y)) rb_hash_aset(attr, ID2SYM(s_height), y);
  }

  fx = shoes_effect_new(cGlow, attr, self);
  shoes_add_ele(canvas, fx);
  return fx;
}

VALUE
shoes_canvas_shadow(int argc, VALUE *argv, VALUE self)
{
  VALUE dist, x, y, fx, attr;
  SETUP();

  rb_scan_args(argc, argv, "03", &dist, &x, &y);
  if (NIL_P(y)) y = x;

  if (rb_obj_is_kind_of(x, rb_cHash))
    attr = x;
  else
  {
    attr = rb_hash_new();
    if (!NIL_P(dist)) rb_hash_aset(attr, ID2SYM(s_distance), dist);
    if (!NIL_P(x)) rb_hash_aset(attr, ID2SYM(s_width), x);
    if (!NIL_P(y)) rb_hash_aset(attr, ID2SYM(s_height), y);
  }

  fx = shoes_effect_new(cShadow, attr, self);
  shoes_add_ele(canvas, fx);
  return fx;
}

#define MARKUP_BLOCK(klass) \
  text = shoes_textblock_new(klass, msgs, attr, self); \
  shoes_add_ele(canvas, text)

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
  int w, h;
  if (shoes_load_image(_path, &w, &h, FALSE))
    return rb_ary_new3(2, INT2NUM(w), INT2NUM(h));
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
    shoes_add_ele(canvas, pat);
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
    shoes_add_ele(canvas, pat);
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
  shoes_add_ele(canvas, video);
  return video;
#else
  rb_raise(eNotImpl, "no video support");
#endif
}

VALUE
shoes_canvas_image(int argc, VALUE *argv, VALUE self)
{
  VALUE path, realpath, attr, image, block;
  SETUP();

  if (argc == 0 || (argc == 1 && rb_obj_is_kind_of(argv[0], rb_cHash)))
  {
    rb_scan_args(argc, argv, "01&", &attr, &block);
    if (NIL_P(attr)) attr = rb_hash_new();
    path = ATTR(attr, width);
    realpath = ATTR(attr, height);
  }
  else
    rb_scan_args(argc, argv, "12&", &path, &realpath, &attr, &block);

  if (NIL_P(path) || FIXNUM_P(path))
  {
    int w = canvas->width;
    if (!NIL_P(path)) w = NUM2INT(path);
    int h = canvas->height;
    if (!NIL_P(realpath)) h = NUM2INT(realpath);
    image = shoes_canvas_imageblock(self, w, h, attr, block);
  }
  else
  {
    if (!NIL_P(block))
    {
      if (NIL_P(attr)) attr = rb_hash_new();
      rb_hash_aset(attr, ID2SYM(s_click), block);
    }
    image = shoes_image_new(cImage, path, realpath, attr, self, canvas->tf, canvas->mode);
  }

  if (!NIL_P(image))
    shoes_add_ele(canvas, image);
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
  shoes_add_ele(canvas, button);
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
  shoes_add_ele(canvas, edit_line);
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
  shoes_add_ele(canvas, edit_box);
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
  shoes_add_ele(canvas, list_box);
  return list_box;
}

VALUE
shoes_canvas_progress(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, progress;
  SETUP();
  rb_scan_args(argc, argv, "01", &attr);

  progress = shoes_control_new(cProgress, attr, self);
  shoes_add_ele(canvas, progress);
  return progress;
}

VALUE
shoes_canvas_radio(int argc, VALUE *argv, VALUE self)
{
  VALUE group, attr, block, radio;
  SETUP();
  rb_scan_args(argc, argv, "02&", &group, &attr, &block);

  if (rb_obj_is_kind_of(group, rb_cHash))
  {
    attr = group;
    group = Qnil;
  }

  if (!NIL_P(group))
    attr = shoes_hash_set(attr, s_group, group);
  if (!NIL_P(block))
    attr = shoes_hash_set(attr, s_click, block);

  radio = shoes_control_new(cRadio, attr, self);
  shoes_add_ele(canvas, radio);
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
  shoes_add_ele(canvas, check);
  return check;
}

VALUE
shoes_canvas_contents(VALUE self)
{
  GET_STRUCT(canvas, self_t);
  return self_t->contents;
}

VALUE
shoes_canvas_children(VALUE self)
{
  GET_STRUCT(canvas, self_t);
  return self_t->contents;
}

void
shoes_canvas_remove_item(VALUE self, VALUE item, char c, char t)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  shoes_native_remove_item(&self_t->slot, item, c);
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
  {
    shoes_canvas *pc;
    shoes_canvas_remove_item(self_t->parent, self, 0, 0);
    Data_Get_Struct(self_t->parent, shoes_canvas, pc);
    if (pc != self_t && DC(self_t->slot) != DC(pc->slot))
      shoes_slot_destroy(self_t, pc);
  }
  return self;
}

static void
shoes_canvas_place(shoes_canvas *self_t)
{
  shoes_canvas *pc;
  Data_Get_Struct(self_t->parent, shoes_canvas, pc);
  shoes_native_canvas_place(self_t, pc);
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
    canvas->group.radios = NULL;
#endif

  if (self_t->height > self_t->fully)
    self_t->fully = self_t->height;
  if (self_t != canvas)
  {
    if (ck != cImageBlock)
    {
      shoes_canvas_reflow(self_t, c);
#ifdef SHOES_GTK
      self_t->slot.expose = canvas->slot.expose;
#endif
    }
  }
  else
  {
    self_t->endx = self_t->cx = 0;
    self_t->topy = self_t->endy = self_t->cy = 0;
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
              if (c2->topy < c1->topy || ABSY(c2->place) || POS(c2->place) != REL_CANVAS)
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
        c1->height = c1->place.ih;
        c1->width = c1->place.iw;
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
        if (RTEST(actual))
        {
          shoes_canvas_place(c1);
        }
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
  if (self_t != canvas)
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
      if (NIL_P(self_t->parent) || RTEST(ATTR(self_t->attr, scroll)))
        shoes_native_slot_lengthen(&self_t->slot, self_t->height, endy);
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
    blk; \
    rb_ary_pop(app->nesting); \
    if (alter) \
      rb_ary_pop(app->nesting); \
  }

void
shoes_canvas_memdraw_begin(VALUE self)
{
  SETUP();
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
}

void
shoes_canvas_memdraw_end(VALUE self)
{
  SETUP();
  cairo_destroy(canvas->cr);
  canvas->cr = NULL;
  shoes_canvas_repaint_all(self);
}

static void
shoes_canvas_memdraw(VALUE self, VALUE block)
{
  SETUP();
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
  DRAW(self, canvas->app, rb_funcall(block, s_call, 0));
  cairo_destroy(canvas->cr);
  canvas->cr = NULL;
}

typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

static  cairo_public cairo_surface_t *
shoes_get_snapshot_nada (const char *filename, double width, double height)
{
  return NULL;
}

static cairo_surface_function_t *
shoes_get_snapshot_surface(VALUE _format)
{
  ID format = SYM2ID (_format);
  if (format == rb_intern ("pdf"))  return & cairo_pdf_surface_create;
  if (format == rb_intern ("ps"))   return & cairo_ps_surface_create;
  if (format == rb_intern ("svg"))  return & cairo_svg_surface_create;
  return shoes_get_snapshot_nada;
}

VALUE
shoes_canvas_snapshot(int argc, VALUE *argv, VALUE self)
{
  SETUP();
  ID   s_filename = rb_intern ("filename");
  ID   s_format   = rb_intern ("format");
  VALUE  block    = Qnil;
  VALUE _filename = Qnil;
  VALUE _format   = Qnil;
  VALUE  hash     = Qnil;
  argc = rb_scan_args (argc, argv, "1&", &hash, &block);

  if (argc == 1 && rb_obj_is_kind_of(hash, rb_cHash))
  {
    _filename = ATTR(hash, filename);
    _format   = ATTR(hash, format);
  }
  if (NIL_P(block) || NIL_P(_filename) || NIL_P(_format))
  {
    rb_raise(rb_eArgError, "wrong arguments for _snapshot({:filename=>'...',"
                              ":format=>:pdf|:ps|:svg}, &block)\n");
  }
  else
  {
    const char      * filename = RSTRING_PTR(_filename);
    cairo_surface_t * surface  = shoes_get_snapshot_surface (_format)
                                      (filename, canvas->width, canvas->height);
    if (surface == NULL) {
        rb_raise(rb_eArgError, "Failed to create %s surface for file %s\n", 
           RSTRING_PTR(rb_inspect(_format)),
           RSTRING_PTR(rb_inspect(_filename)));
    }
    else
    {
      cairo_t * waz_cr = canvas->cr;
      cairo_t * cr     = canvas->cr = cairo_create (surface);
      DRAW(self, canvas->app, rb_funcall(block, s_call, 0));
      shoes_canvas_draw (self, self, Qfalse);
      shoes_canvas_draw (self, self, Qtrue);
      canvas->cr = waz_cr;
      cairo_show_page (cr);
      cairo_destroy (cr);
      cairo_surface_destroy (surface);
      //  TODO  detect cairo outrages here
    }
  }
  return Qnil;
}

void
shoes_canvas_compute(VALUE self)
{
  SETUP();
  canvas->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));;
  shoes_canvas_draw(self, self, Qfalse);
  cairo_destroy(canvas->cr);
  canvas->cr = NULL;
}

static void
shoes_canvas_insert(VALUE self, long i, VALUE ele, VALUE block)
{
  VALUE ary;
  SETUP();

  if (!NIL_P(ele))
    i = rb_ary_index_of(canvas->contents, ele) - i;

  canvas->insertion = i;
  shoes_canvas_memdraw(self, block);
  canvas->insertion = -1;
  shoes_canvas_repaint_all(self);
}

VALUE
shoes_canvas_after(int argc, VALUE *argv, VALUE self)
{
  VALUE ele, block;
  rb_scan_args(argc, argv, "01&", &ele, &block);
  shoes_canvas_insert(self, -1, ele, block);
  return self;
}

VALUE
shoes_canvas_before(int argc, VALUE *argv, VALUE self)
{
  VALUE ele, block;
  rb_scan_args(argc, argv, "01&", &ele, &block);
  shoes_canvas_insert(self, 0, ele, block);
  return self;
}

VALUE
shoes_canvas_append(int argc, VALUE *argv, VALUE self)
{
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);
  shoes_canvas_insert(self, -1, Qnil, block);
  return self;
}

VALUE
shoes_canvas_prepend(int argc, VALUE *argv, VALUE self)
{
  VALUE block;
  rb_scan_args(argc, argv, "0&", &block);
  shoes_canvas_insert(self, 0, Qnil, block);
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
    DRAW(flow, canvas->app, rb_funcall(block, s_call, 0));
  }
  shoes_add_ele(canvas, flow);
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
    DRAW(stack, canvas->app, rb_funcall(block, s_call, 0));
  }
  shoes_add_ele(canvas, stack);
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
    DRAW(mask, canvas->app, rb_funcall(block, s_call, 0));
  }
  shoes_add_ele(canvas, mask);
  return mask;
}

VALUE
shoes_canvas_imageblock(VALUE self, int w, int h, VALUE attr, VALUE block)
{
  shoes_canvas *self_t, *pc;
  cairo_surface_t *surfc;
  VALUE imageblock = shoes_canvas_alloc(cImageBlock);

  shoes_canvas_clear(imageblock);
  Data_Get_Struct(self, shoes_canvas, pc);
  Data_Get_Struct(imageblock, shoes_canvas, self_t);
  self_t->cr = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h));
  self_t->fg = pc->fg;
  self_t->bg = pc->bg;
  self_t->sw = pc->sw;
  self_t->width = w;
  self_t->height = h;
  self_t->slot = pc->slot;
  self_t->parent = self;
  self_t->app = pc->app;
  self_t->attr = attr;
  if (!NIL_P(block))
  {
    DRAW(imageblock, pc->app, rb_funcall(block, s_call, 0));
  }

  shoes_imageblock_paint(imageblock, 1);
  shoes_add_ele(pc, imageblock);
  return imageblock;
}

VALUE
shoes_canvas_widget(int argc, VALUE *argv, VALUE self)
{
  VALUE klass, attr, args, widget;
  SETUP();

  rb_scan_args(argc, argv, "1*", &klass, &args);
  attr = rb_ary_pop(args);
  if (!rb_obj_is_kind_of(attr, rb_cHash))
    attr = Qnil;

  widget = shoes_widget_new(klass, attr, self);
  DRAW(widget, canvas->app, ts_funcall2(widget, rb_intern("initialize"), argc - 1, argv + 1));
  shoes_add_ele(canvas, widget);
  return widget;
}

void
shoes_canvas_size(VALUE self, int w, int h)
{
  SETUP();
  canvas->place.iw = canvas->place.w = canvas->width = w;
  canvas->place.ih = canvas->place.h = canvas->height = h;
  shoes_native_canvas_resize(canvas);
}

VALUE
shoes_find_canvas(VALUE self)
{
  while (!NIL_P(self) && !rb_obj_is_kind_of(self, cCanvas))
  {
    shoes_basic *basic;
    Data_Get_Struct(self, shoes_basic, basic);
    self = basic->parent;
  }
  return self;
}

VALUE
shoes_canvas_get_app(VALUE self)
{
  VALUE app = Qnil, c = shoes_find_canvas(self);
  if (rb_obj_is_kind_of(c, cCanvas))
  {
    shoes_canvas *canvas;
    Data_Get_Struct(c, shoes_canvas, canvas);
    app = canvas->app->canvas;
  }
  return app;
}

void
shoes_canvas_repaint_all(VALUE self)
{
  shoes_canvas *canvas;
  self = shoes_find_canvas(self);
  Data_Get_Struct(self, shoes_canvas, canvas);
  shoes_slot_repaint(&canvas->app->slot);
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
  if (RTEST(ATTR(self_t->attr, hidden)))
    shoes_canvas_show(self);
  else
    shoes_canvas_hide(self);
  return self;
}

#define EVENT_HANDLER(x) \
  VALUE \
  shoes_canvas_##x(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE val, block; \
    SETUP(); \
    rb_scan_args(argc, argv, "01&", &val, &block); \
    ATTRSET(canvas->attr, x, NIL_P(block) ? val : block); \
    return self; \
  }

EVENT_HANDLER(click);
EVENT_HANDLER(hover);
EVENT_HANDLER(leave);
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

    VALUE start = ATTR(canvas->attr, start);
    if (!NIL_P(start))
    {
      shoes_safe_block(self, start, rb_ary_new3(1, self));
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

  if (ORIGIN(self_t->place))
  {
    x -= self_t->place.ix + self_t->place.dx;
    y -= (self_t->place.iy + self_t->place.dy) - self_t->slot.scrolly;
    if (y < self_t->slot.scrolly || x < 0 || y > self_t->slot.scrolly + self_t->height || x > self_t->width)
      return Qnil;
  }

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (IS_INSIDE(self_t, x, y))
    {
      VALUE click = ATTR(self_t->attr, click);
      if (!NIL_P(click))
      {
        shoes_safe_block(self, click, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
      }
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
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
  return rb_ary_new3(3, INT2NUM(self_t->app->mouseb), 
    INT2NUM(self_t->app->mousex), INT2NUM(self_t->app->mousey));
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
  if (ORIGIN(self_t->place))
  {
    x -= (self_t->place.ix + self_t->place.dx);
    y -= (self_t->place.iy + self_t->place.dy) - self_t->slot.scrolly;
    if (y < self_t->slot.scrolly || x < 0 || y > self_t->slot.scrolly + self_t->height || x > self_t->width)
      return;
  }

  // INFO("release(%d, %d, %d)\n", button, x, y);

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (IS_INSIDE(self_t, x, y))
    {
      VALUE release = ATTR(self_t->attr, release);
      if (!NIL_P(release))
      {
        shoes_safe_block(self, release, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
      }
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
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
  char h = 0, *n = 0;
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  h = IS_INSIDE(self_t, x, y);
  CHECK_HOVER(self_t, h, n);

  if (ORIGIN(self_t->place))
  {
    x -= (self_t->place.ix + self_t->place.dx);
    y -= (self_t->place.iy + self_t->place.dy) - self_t->slot.scrolly;
    if (y < self_t->slot.scrolly || x < 0 || y > self_t->slot.scrolly + self_t->height || x > self_t->width)
      return Qnil;
  }

  h = 0;
  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    VALUE motion = ATTR(self_t->attr, motion);
    if (!NIL_P(motion))
    {
      shoes_safe_block(self, motion, rb_ary_new3(2, INT2NUM(x), INT2NUM(y)));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE urll = Qnil;
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
        urll = shoes_canvas_send_motion(ele, x, y, url);
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        urll = shoes_textblock_motion(ele, x, y, &h);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        urll = shoes_image_motion(ele, x, y, NULL);
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        urll = shoes_shape_motion(ele, x, y, NULL);
      }

      if (NIL_P(url)) url = urll;
    }

    if (NIL_P(url))
    {
      shoes_canvas *self_t;
      Data_Get_Struct(self, shoes_canvas, self_t);
      if (self_t->app->cursor == s_link)
        shoes_app_cursor(self_t->app, s_arrow);
    }
  }

  if (h) shoes_canvas_repaint_all(self);

  return url;
}

void
shoes_canvas_send_wheel(VALUE self, ID dir, int x, int y)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    VALUE wheel = ATTR(self_t->attr, wheel);
    if (!NIL_P(wheel))
    {
      shoes_safe_block(self, wheel, rb_ary_new3(3, ID2SYM(dir), INT2NUM(x), INT2NUM(y)));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
        shoes_canvas_send_wheel(ele, dir, x, y);
      }
    }
  }
}

void
shoes_canvas_send_keypress(VALUE self, VALUE key)
{
  long i;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    VALUE keypress = ATTR(self_t->attr, keypress);
    if (!NIL_P(keypress))
    {
      shoes_safe_block(self, keypress, rb_ary_new3(1, key));
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
        shoes_canvas_send_keypress(ele, key);
      }
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
  int scrolls = RTEST(ATTR(self_t->attr, scroll));
  if ((attr != ssNestSlot && RTEST(ATTR(self_t->attr, height))) || scrolls) {
    //
    // create the slot off-screen until it can be properly placed
    //
    shoes_slot_init(self, &pc->slot, -99, -99, 100, 100, scrolls, FALSE);
#ifdef SHOES_GTK
    gtk_widget_show_all(self_t->slot.canvas);
    self_t->width = 100;
    self_t->height = 100;
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

VALUE
shoes_canvas_get_cursor(VALUE self)
{
  SETUP();
  return ID2SYM(canvas->app->cursor);
}

VALUE
shoes_canvas_set_cursor(VALUE self, VALUE name)
{
  SETUP();
  shoes_app_cursor(canvas->app, SYM2ID(name));
  return name;
}

//
// Global clipboard getter and setter
//
VALUE
shoes_canvas_get_clipboard(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  return shoes_native_clipboard_get(self_t->app);
}

VALUE
shoes_canvas_set_clipboard(VALUE self, VALUE string)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  string = shoes_native_to_s(string);
  shoes_native_clipboard_set(self_t->app, string);
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
    return shoes_app_window(argc, argv, cApp, canvas->app->self);

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
  return shoes_native_window_color(canvas->app);
}

VALUE
shoes_canvas_dialog_plain(VALUE self)
{
  SETUP();
  return shoes_native_dialog_color(canvas->app);
}
