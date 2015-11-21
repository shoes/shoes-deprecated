//
// shoes/canvas.c
// Ruby methods for all the drawing ops.
//
#include "shoes/internal.h"
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/http.h"

#define SETUP() \
  shoes_canvas *canvas; \
  cairo_t *cr; \
  Data_Get_Struct(self, shoes_canvas, canvas); \
  cr = CCR(canvas)
#define SETUP_IMAGE() \
  shoes_place place; \
  GET_STRUCT(image, image); \
  shoes_image_ensure_dup(image); \
  shoes_place_exact(&place, attr, 0, 0); \
  if (NIL_P(attr)) attr = image->attr; \
  else if (!NIL_P(image->attr)) attr = rb_funcall(image->attr, s_merge, 1, attr);
#define SETUP_SHAPE() \
  shoes_canvas *canvas = NULL; \
  VALUE c = shoes_find_canvas(self); \
  Data_Get_Struct(c, shoes_canvas, canvas)

const double SHOES_PIM2   = 6.28318530717958647693;
const double SHOES_PI     = 3.14159265358979323846;
const double SHOES_HALFPI = 1.57079632679489661923;
const double SHOES_RAD2PI = 0.01745329251994329577;

//const char *dialog_title = USTR("Shoes asks:");
//const char *dialog_title_says = USTR("Shoes says:");

static void shoes_canvas_send_start(VALUE);
static void shoes_canvas_send_finish(VALUE);

shoes_transform *
shoes_transform_new(shoes_transform *o)
{
  shoes_transform *n = SHOE_ALLOC(shoes_transform);
  n->mode = s_center;
  n->refs = 1;
  cairo_matrix_init_identity(&n->tf);
  if (o != NULL)
  {
    cairo_matrix_multiply(&n->tf, &n->tf, &o->tf);
    n->mode = o->mode;
  }
  return n;
}

shoes_transform *
shoes_transform_touch(shoes_transform *st)
{
  if (st != NULL) st->refs++;
  return st;
}

shoes_transform *
shoes_transform_detach(shoes_transform *old)
{
  if (old != NULL && old->refs == 1) return old;
  if (old != NULL) old->refs--;
  return shoes_transform_new(old);
}

void
shoes_transform_release(shoes_transform *st)
{
  if (st == NULL) return;
  if (--st->refs) return;
  SHOE_FREE(st);
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
  return INT2NUM(canvas->slot->scrolly);
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
  scrollwidth = shoes_native_slot_gutter(canvas->slot);
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
  rb_arg_list args;
  SETUP();

  switch (rb_parse_args(argc, argv, "kh,h,", &args))
  {
    case 1:
      shoes_app_style(canvas->app, args.a[0], args.a[1]);
    break;

    case 2:
      if (NIL_P(canvas->attr)) canvas->attr = rb_hash_new();
      rb_funcall(canvas->attr, s_update, 1, args.a[0]);
      shoes_canvas_repaint_all(canvas->parent);
    break;

    case 3: return rb_obj_freeze(rb_obj_dup(canvas->attr));
  }

  return self;
}

#define ELAPSED (shoes_diff_time(&start, &mid) * 0.001)

static VALUE
shoes_canvas_paint_call(VALUE self)
{
  shoes_code code = SHOES_OK;
  SHOES_TIME start, mid;
  shoes_get_time(&start);

  if (self == Qnil)
    return self;

  SETUP();

  if (canvas->cr != NULL)
    goto quit;

  canvas->cr = cr = shoes_cairo_create(canvas);
  if (cr == NULL)
    goto quit;

  cairo_save(cr);
  shoes_canvas_draw(self, self, Qfalse);
  shoes_get_time(&mid);
  INFO("COMPUTE: %0.6f s\n", ELAPSED);
  cairo_restore(cr);

  canvas->cr = cr;
  cairo_save(cr);
  shoes_canvas_draw(self, self, Qtrue);
  shoes_get_time(&mid);
  INFO("DRAW: %0.6f s\n", ELAPSED);
  cairo_restore(cr);

  if (cairo_status(cr)) {
    code = SHOES_FAIL;
    PUTS("Cairo is unhappy: %s\n", cairo_status_to_string (cairo_status (cr)));
    goto quit;
  }

  cairo_destroy(cr);
  cr = canvas->cr = NULL;

  shoes_cairo_destroy(canvas);
  shoes_get_time(&mid);
  INFO("PAINT: %0.6f s\n", ELAPSED);
  shoes_canvas_send_start(self);
quit:
  if (cr != NULL) cairo_destroy(cr);
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
shoes_apply_transformation(cairo_t *cr, shoes_transform *st, shoes_place *place, unsigned char force)
{
  double x, y, w, h;
  cairo_save(cr);
  w = place->iw / 2.;
  h = place->ih / 2.;
  if (st != NULL)
  {
    x = (place->ix + place->dx) + w;
    y = (place->iy + place->dy) + h;

    if (st->mode == s_center)
      cairo_translate(cr, x, y);
     cairo_transform(cr, &st->tf);
    if (st->mode == s_center)
      cairo_translate(cr, -x, -y);
  }
}

void
shoes_undo_transformation(cairo_t *cr, shoes_transform *st, shoes_place *place, unsigned char force)
{
  cairo_restore(cr);
}

static VALUE
shoes_add_ele(shoes_canvas *canvas, VALUE ele)
{
  if (NIL_P(ele)) return ele;
  if (canvas->insertion <= -1)
    rb_ary_push(canvas->contents, ele);
  else
  {
    rb_ary_insert_at(canvas->contents, canvas->insertion, 0, ele);
    canvas->insertion++;
  }
  return ele;
}

void
shoes_canvas_mark(shoes_canvas *canvas)
{
  shoes_native_slot_mark(canvas->slot);
  rb_gc_mark_maybe(canvas->contents);
  rb_gc_mark_maybe(canvas->attr);
  rb_gc_mark_maybe(canvas->parent);
}

static void
shoes_canvas_reset_transform(shoes_canvas *canvas)
{
  if (canvas->sts != NULL)
  {
    int i;
    for (i = 0; i < canvas->stl; i++)
      shoes_transform_release(canvas->sts[i]);
    SHOE_FREE(canvas->sts);
    canvas->sts = NULL;
  }
  canvas->stl = 0;
  canvas->stt = 0;

  if (canvas->st != NULL)
  {
    shoes_transform_release(canvas->st);
    canvas->st = NULL;
  }
}

static void
shoes_canvas_free(shoes_canvas *canvas)
{
  if (canvas->slot != NULL && canvas->slot->owner == canvas)
    SHOE_FREE(canvas->slot);
  shoes_canvas_reset_transform(canvas);
  RUBY_CRITICAL(free(canvas));
}

VALUE
shoes_canvas_alloc(VALUE klass)
{
  shoes_canvas *canvas = SHOE_ALLOC(shoes_canvas);
  SHOE_MEMZERO(canvas, shoes_canvas, 1);
  canvas->app = NULL;
  canvas->stage = CANVAS_NADA;
  canvas->contents = Qnil;
  canvas->shape = NULL;
  canvas->insertion = -2;
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
shoes_canvas_empty(shoes_canvas *canvas, int extras)
{
  unsigned char stage = canvas->stage;
  canvas->stage = CANVAS_EMPTY;
  shoes_ele_remove_all(canvas->contents);
  if (extras) shoes_extras_remove_all(canvas);
  canvas->stage = stage;
}

void
shoes_canvas_clear(VALUE self)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  canvas->cr = NULL;
  canvas->attr = rb_hash_new();
  ATTRSET(canvas->attr, cap, Qnil);
  ATTRSET(canvas->attr, strokewidth, rb_float_new(1.));
  ATTRSET(canvas->attr, stroke, shoes_color_new(0, 0, 0, 0xFF));
  ATTRSET(canvas->attr, fill, shoes_color_new(0, 0, 0, 0xFF));
  canvas->parent = Qnil;
  canvas->stl = 0;
  canvas->stt = 0;
  shoes_canvas_reset_transform(canvas);
  shoes_canvas_empty(canvas, TRUE);
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
shoes_canvas_init(VALUE self, SHOES_SLOT_OS *slot, VALUE attr, int width, int height)
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
    canvas->slot->scrolly += dy;
  else
    canvas->slot->scrolly = dy;

  if (canvas->slot->scrolly > canvas->endy - canvas->height)
    canvas->slot->scrolly = canvas->endy - canvas->height;
  if (canvas->slot->scrolly < 0)
    canvas->slot->scrolly = 0;
  if (DC(canvas->app->slot) == DC(canvas->slot)) canvas->app->slot->scrolly = canvas->slot->scrolly;
  shoes_native_slot_scroll_top(canvas->slot);
  shoes_slot_repaint(canvas->slot);
}

VALUE
shoes_canvas_nostroke(VALUE self)
{
  SETUP_BASIC();
  ATTRSET(basic->attr, stroke, Qnil);
  return self;
}

VALUE
shoes_canvas_stroke(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  SETUP_BASIC();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  if (!rb_obj_is_kind_of(pat, cColor))
    pat = rb_funcall(pat, s_to_pattern, 0);
  ATTRSET(basic->attr, stroke, pat);
  return pat;
}

VALUE
shoes_canvas_strokewidth(VALUE self, VALUE w)
{
  SETUP_BASIC();
  ATTRSET(basic->attr, strokewidth, w);
  return self;
}

VALUE
shoes_canvas_dash(VALUE self, VALUE dash)
{
  SETUP_BASIC();
  ATTRSET(basic->attr, dash, dash);
  return self;
}

VALUE
shoes_canvas_cap(VALUE self, VALUE cap)
{
  SETUP_BASIC();
  ATTRSET(basic->attr, cap, cap);
  return self;
}

VALUE
shoes_canvas_nofill(VALUE self)
{
  SETUP_BASIC();
  ATTRSET(basic->attr, fill, Qnil);
  return self;
}

VALUE
shoes_canvas_fill(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
  SETUP_BASIC();
  if (argc == 1 && rb_respond_to(argv[0], s_to_pattern))
    pat = argv[0];
  else
    pat = shoes_pattern_args(argc, argv, self);
  if (!rb_obj_is_kind_of(pat, cColor))
    pat = rb_funcall(pat, s_to_pattern, 0);
  ATTRSET(basic->attr, fill, pat);
  return pat;
}

VALUE
shoes_add_shape(VALUE self, ID name, VALUE attr, cairo_path_t *line)
{
  if (rb_obj_is_kind_of(self, cImage))
  {
    SETUP_IMAGE();
    shoes_shape_sketch(image->cr, name, &place, NULL, attr, line, 1);
    return self;
  }

  SETUP();
  if (canvas->shape != NULL)
  {
    shoes_place place;
    shoes_place_exact(&place, attr, 0, 0);
    cairo_new_sub_path(canvas->shape);
    shoes_shape_sketch(canvas->shape, name, &place, canvas->st, attr, line, 0);
    return self;
  }

  return shoes_add_ele(canvas, shoes_shape_new(self, name, attr, canvas->st, line));
}

VALUE
shoes_canvas_arc(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 6, s_left, s_top, s_width, s_height, s_angle1, s_angle2);
  return shoes_add_shape(self, s_arc, attr, NULL);
}

VALUE
shoes_canvas_rect(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 5, s_left, s_top, s_width, s_height, s_curve);
  return shoes_add_shape(self, s_rect, attr, NULL);
}

VALUE
shoes_canvas_oval(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 4, s_left, s_top, s_width, s_height);
  //VALUE attr = shoes_shape_attr(argc, argv, 3, s_left, s_top, s_radius);
  //rb_warn("shoes_canvas_oval: %s\n", RSTRING_PTR(rb_inspect(attr)));
  return shoes_add_shape(self, s_oval, attr, NULL);
}

VALUE
shoes_canvas_line(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 4, s_left, s_top, s_right, s_bottom);
  return shoes_add_shape(self, s_line, attr, NULL);
}

VALUE
shoes_canvas_arrow(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 3, s_left, s_top, s_width);
  return shoes_add_shape(self, s_arrow, attr, NULL);
}

VALUE
shoes_canvas_star(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 5, s_left, s_top, s_points, s_outer, s_inner);
  return shoes_add_shape(self, s_star, attr, NULL);
}

VALUE
shoes_add_effect(VALUE self, ID name, VALUE attr)
{
  if (rb_obj_is_kind_of(self, cImage))
  {
    shoes_effect_filter filter = shoes_effect_for_type(name);
    SETUP_IMAGE();
    if (filter == NULL) return self;
    filter(image->cr, attr, &place);
    return self;
  }

  SETUP();
  return shoes_add_ele(canvas, shoes_effect_new(name, attr, self));
}

VALUE
shoes_canvas_blur(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 1, s_radius);
  return shoes_add_effect(self, s_blur, attr);
}

VALUE
shoes_canvas_glow(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 1, s_radius);
  return shoes_add_effect(self, s_glow, attr);
}

VALUE
shoes_canvas_shadow(int argc, VALUE *argv, VALUE self)
{
  VALUE attr = shoes_shape_attr(argc, argv, 2, s_distance, s_radius);
  return shoes_add_effect(self, s_shadow, attr);
}

#define MARKUP_BLOCK(klass) \
  text = shoes_textblock_new(klass, msgs, attr, self, canvas->st); \
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
  if (shoes_load_imagesize(_path, &w, &h) == SHOES_OK)
    return rb_ary_new3(2, INT2NUM(w), INT2NUM(h));
  return Qnil;
}

VALUE
shoes_canvas_background(int argc, VALUE *argv, VALUE self)
{
  VALUE pat;
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
  rb_raise(eNotImpl, "no video support");
}

VALUE
shoes_canvas_image(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE path = Qnil, attr = Qnil, _w, _h, image;

  switch (rb_parse_args(argc, argv, "ii|h,s|h,|h", &args))
  {
    case 1:
      _w = args.a[0];
      _h = args.a[1];
      attr = args.a[2];
      ATTRSET(attr, width, _w);
      ATTRSET(attr, height, _h);
      if (rb_block_given_p()) ATTRSET(attr, draw, rb_block_proc());
    break;

    case 2:
      path = args.a[0];
      attr = args.a[1];
      if (rb_block_given_p()) ATTRSET(attr, click, rb_block_proc());
    break;

    case 3:
      attr = args.a[0];
      if (rb_block_given_p()) ATTRSET(attr, draw, rb_block_proc());
    break;
  }

  if (rb_obj_is_kind_of(self, cImage))
  {
    shoes_image_image(self, path, attr);
    return self;
  }

  SETUP();
  image = shoes_image_new(cImage, path, attr, self, canvas->st);
  shoes_add_ele(canvas, image);

  return image;
}

VALUE
shoes_canvas_animate(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE anim;
  SETUP();

  rb_parse_args(argc, argv, "|I&", &args);
  anim = shoes_timer_new(cAnim, args.a[0], args.a[1], self);
  rb_ary_push(canvas->app->extras, anim);
  return anim;
}

VALUE
shoes_canvas_every(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE ev;
  SETUP();

  rb_parse_args(argc, argv, "|F&", &args);
  ev = shoes_timer_new(cEvery, args.a[0], args.a[1], self);
  rb_ary_push(canvas->app->extras, ev);
  return ev;
}

VALUE
shoes_canvas_timer(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE timer;
  SETUP();

  rb_parse_args(argc, argv, "|I&", &args);
  timer = shoes_timer_new(cTimer, args.a[0], args.a[1], self);
  rb_ary_push(canvas->app->extras, timer);
  return timer;
}


VALUE
shoes_canvas_svg(int argc, VALUE *argv, VALUE self)
{
  VALUE widget;
  SETUP();
  widget = shoes_svg_new(argc, argv, self);
  shoes_add_ele(canvas, widget);
}


VALUE
shoes_canvas_shape(int argc, VALUE *argv, VALUE self)
{
  int x;
  double x1, y1, x2, y2;
  cairo_t *shape = NULL;
  cairo_path_t *line = NULL;
  SETUP_SHAPE();

  shape = canvas->shape;
  VALUE attr = shoes_shape_attr(argc, argv, 2, s_left, s_top);
  canvas->shape = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
  cairo_move_to(canvas->shape, 0, 0);
  if (rb_block_given_p()) rb_funcall(rb_block_proc(), s_call, 0);

#if CAIRO_VERSION_MAJOR == 1 && CAIRO_VERSION_MINOR <= 4
  cairo_fill_extents(canvas->shape, &x1, &y1, &x2, &y2);
#else
  cairo_path_extents(canvas->shape, &x1, &y1, &x2, &y2);
#endif
  x = ROUND(x2 - x1);
  ATTRSET(attr, width, INT2NUM(x));
  x = ROUND(y2 - y1);
  ATTRSET(attr, height, INT2NUM(x));
  line = cairo_copy_path(canvas->shape);
  canvas->shape = shape;
  return shoes_add_shape(self, s_shape, attr, line);
}

VALUE
shoes_canvas_move_to(VALUE self, VALUE _x, VALUE _y)
{
  double x, y;
  SETUP_SHAPE();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);

  if (canvas->shape != NULL) cairo_move_to(canvas->shape, x, y);
  return self;
}

VALUE
shoes_canvas_line_to(VALUE self, VALUE _x, VALUE _y)
{
  double x, y;
  SETUP_SHAPE();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);

  if (canvas->shape != NULL) cairo_line_to(canvas->shape, x, y);
  return self;
}

VALUE
shoes_canvas_curve_to(VALUE self, VALUE _x1, VALUE _y1, VALUE _x2, VALUE _y2, VALUE _x3, VALUE _y3)
{
  double x1, y1, x2, y2, x3, y3;
  SETUP_SHAPE();

  x1 = NUM2DBL(_x1);
  y1 = NUM2DBL(_y1);
  x2 = NUM2DBL(_x2);
  y2 = NUM2DBL(_y2);
  x3 = NUM2DBL(_x3);
  y3 = NUM2DBL(_y3);

  if (canvas->shape != NULL) cairo_curve_to(canvas->shape, x1, y1, x2, y2, x3, y3);
  return self;
}

VALUE
shoes_canvas_arc_to(VALUE self, VALUE _x, VALUE _y, VALUE _w, VALUE _h, VALUE _a1, VALUE _a2)
{
  double x, y, w, h, a1, a2;
  SETUP_SHAPE();

  x = NUM2DBL(_x);
  y = NUM2DBL(_y);
  w = NUM2DBL(_w);
  h = NUM2DBL(_h);
  a1 = NUM2DBL(_a1);
  a2 = NUM2DBL(_a2);

  if (canvas->shape != NULL)
  {
    cairo_save(canvas->shape);
    shoes_cairo_arc(canvas->shape, x, y, w, h, a1, a2);
    cairo_restore(canvas->shape);
  }
  return self;
}

VALUE
shoes_canvas_push(VALUE self)
{
  shoes_transform *m;
  SETUP();

  m = canvas->st;
  if (canvas->stl + 1 > canvas->stt)
  {
    canvas->stt += 8;
    SHOE_REALLOC_N(canvas->sts, shoes_transform *, canvas->stt);
  }
  canvas->st = shoes_transform_new(m);
  canvas->sts[canvas->stl++] = m;
  return self;
}

VALUE
shoes_canvas_pop(VALUE self)
{
  SETUP();

  if (canvas->stl >= 1)
  {
    shoes_transform_release(canvas->st);
    canvas->stl--;
    canvas->st = canvas->sts[canvas->stl];
  }
  return self;
}

VALUE
shoes_canvas_reset(VALUE self)
{
  SETUP();
  shoes_canvas_reset_transform(canvas);
  return self;
}

VALUE
shoes_canvas_button(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE text = Qnil, attr = Qnil, button;
  SETUP();

  switch (rb_parse_args(argc, argv, "s|h,|h", &args))
  {
    case 1:
      text = args.a[0];
      attr = args.a[1];
    break;

    case 2:
      attr = args.a[0];
    break;
  }

  if (!NIL_P(text))
    ATTRSET(attr, text, text);

  if (rb_block_given_p())
    ATTRSET(attr, click, rb_block_proc());

  button = shoes_control_new(cButton, attr, self);
  shoes_add_ele(canvas, button);
  return button;
}

VALUE
shoes_canvas_edit_line(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE phrase = Qnil, attr = Qnil, edit_line;
  SETUP();

  switch (rb_parse_args(argc, argv, "h,S|h,", &args))
  {
    case 1:
      attr = args.a[0];
    break;

    case 2:
      phrase = args.a[0];
      attr = args.a[1];
    break;
  }

  if (!NIL_P(phrase))
    ATTRSET(attr, text, phrase);

  if (rb_block_given_p())
    ATTRSET(attr, change, rb_block_proc());

  edit_line = shoes_control_new(cEditLine, attr, self);
  shoes_add_ele(canvas, edit_line);
  return edit_line;
}

VALUE
shoes_canvas_edit_box(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE phrase = Qnil, attr = Qnil, edit_box;
  SETUP();

  switch (rb_parse_args(argc, argv, "h,S|h,", &args))
  {
    case 1:
      attr = args.a[0];
    break;

    case 2:
      phrase = args.a[0];
      attr = args.a[1];
    break;
  }

  if (!NIL_P(phrase))
    ATTRSET(attr, text, phrase);

  if (rb_block_given_p())
    ATTRSET(attr, change, rb_block_proc());

  edit_box = shoes_control_new(cEditBox, attr, self);
  shoes_add_ele(canvas, edit_box);
  return edit_box;
}

VALUE
shoes_canvas_text_edit_box(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE phrase = Qnil, attr = Qnil, text_edit_box;
  SETUP();

  switch (rb_parse_args(argc, argv, "h,S|h,", &args))
  {
    case 1:
      attr = args.a[0];
    break;

    case 2:
      phrase = args.a[0];
      attr = args.a[1];
    break;
  }

  if (!NIL_P(phrase))
    ATTRSET(attr, text, phrase);

  if (rb_block_given_p())
    ATTRSET(attr, change, rb_block_proc());

  text_edit_box = shoes_control_new(cTextEditBox, attr, self);
  shoes_add_ele(canvas, text_edit_box);
  return text_edit_box;
}

VALUE
shoes_canvas_list_box(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE list_box;
  SETUP();

  rb_parse_args(argc, argv, "|h&", &args);

  if (!NIL_P(args.a[1]))
    ATTRSET(args.a[0], change, args.a[1]);

  list_box = shoes_control_new(cListBox, args.a[0], self);
  shoes_add_ele(canvas, list_box);
  return list_box;
}

VALUE
shoes_canvas_progress(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE progress;
  SETUP();

  rb_parse_args(argc, argv, "|h", &args);
  progress = shoes_control_new(cProgress, args.a[0], self);
  shoes_add_ele(canvas, progress);
  return progress;
}

VALUE
shoes_canvas_slider(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE slider;
  SETUP();

  rb_parse_args(argc, argv, "|h&", &args);

  if (!NIL_P(args.a[1]))
    ATTRSET(args.a[0], change, args.a[1]);

  slider = shoes_control_new(cSlider, args.a[0], self);
  shoes_add_ele(canvas, slider);
  return slider;
}

VALUE
shoes_canvas_radio(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE group = Qnil, attr = Qnil, radio;
  SETUP();

  switch (rb_parse_args(argc, argv, "h,o|h,", &args))
  {
    case 1:
      attr = args.a[0];
    break;

    case 2:
      group = args.a[0];
      attr = args.a[1];
    break;
  }

  if (!NIL_P(group))
    ATTRSET(attr, group, group);
  if (rb_block_given_p())
    ATTRSET(attr, click, rb_block_proc());

  radio = shoes_control_new(cRadio, attr, self);
  shoes_add_ele(canvas, radio);
  return radio;
}

VALUE
shoes_canvas_check(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE check;
  SETUP();

  rb_parse_args(argc, argv, "|h", &args);

  if (rb_block_given_p())
    ATTRSET(args.a[0], click, rb_block_proc());

  check = shoes_control_new(cCheck, args.a[0], self);
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
  shoes_native_remove_item(self_t->slot, item, c);
  if (t)
  {
    i = rb_ary_index_of(self_t->app->extras, item);
    if (i >= 0)
      rb_ary_insert_at(self_t->app->extras, i, 1, Qnil);
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
  shoes_canvas *parent;
  Data_Get_Struct(c, shoes_canvas, parent);

  self_t->cr = parent->cr;
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
  shoes_canvas_empty(self_t, TRUE);
  if (!NIL_P(self_t->parent))
  {
    shoes_canvas *pc;
    shoes_canvas_remove_item(self_t->parent, self, 0, 0);
    Data_Get_Struct(self_t->parent, shoes_canvas, pc);
    if (pc != self_t && DC(self_t->slot) != DC(pc->slot))
      shoes_slot_destroy(self_t, pc);
    shoes_canvas_repaint_all(self);
  }
  shoes_canvas_send_finish(self);
  return self;
}

VALUE
shoes_canvas_refresh_slot(VALUE self)
{
  shoes_canvas_repaint_all(self);
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
    shoes_canvas_reflow(self_t, c);
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
        shoes_place_decide(&c1->place, c1->parent, c1->attr, self_t->place.iw, 0, REL_CANVAS, FALSE);
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
    for (i = 0; i < RARRAY_LEN(self_t->app->extras); i++)
    {
      VALUE ele = rb_ary_entry(self_t->app->extras, i);
      if (rb_respond_to(ele, s_draw))
        rb_funcall(ele, s_draw, 2, self, actual);
    }
  }
  canvas->endx = canvas->cx = self_t->place.x + self_t->width;
  if (canvas->endy < self_t->endy)
    canvas->endy = self_t->endy;
      
  if (self_t == canvas || DC(self_t->slot) != DC(canvas->slot))
  {
    int endy = (int)self_t->endy;
    if (endy < self_t->height) endy = self_t->height;
    self_t->fully = endy;
    if (RTEST(actual))
    {
      self_t->slot->scrolly = min(self_t->slot->scrolly, self_t->fully - self_t->height);
      if (NIL_P(self_t->parent) || RTEST(ATTR(self_t->attr, scroll)))
        shoes_native_slot_lengthen(self_t->slot, self_t->height, endy);
    }
  }
  else
  {
    int bmargin = CPB(self_t);
    self_t->fully = canvas->endy = max(canvas->endy, self_t->endy + bmargin);
    self_t->place.ih = (canvas->endy - self_t->place.iy) - bmargin;
    self_t->place.h = canvas->endy - self_t->place.y;
  }

  if (self_t->cr == canvas->cr)
    self_t->cr = NULL;
  return self;
}

static void
shoes_canvas_memdraw(VALUE self, VALUE block)
{
  SETUP();
  DRAW(self, canvas->app, rb_funcall(block, s_call, 0));
}

typedef cairo_public cairo_surface_t * (cairo_surface_function_t) (const char *filename, double width, double height);

static cairo_surface_function_t *
shoes_get_snapshot_surface(VALUE _format)
{
  ID format = SYM2ID (_format);
  if (format == rb_intern ("pdf"))  return & cairo_pdf_surface_create;
  if (format == rb_intern ("ps"))   return & cairo_ps_surface_create;
  if (format == rb_intern ("svg"))  return & cairo_svg_surface_create;
  return NULL;
}

VALUE
shoes_canvas_snapshot(int argc, VALUE *argv, VALUE self)
{
  SETUP();
  rb_arg_list args;
  ID   s_filename = rb_intern ("filename");
  ID   s_format   = rb_intern ("format");
  VALUE _filename, _format;
  rb_parse_args(argc, argv, "h&", &args);
  //rb_parse_args(argc, argv, "h", &args);

  _filename = ATTR(args.a[0], filename);
  _format   = ATTR(args.a[0], format);
  if (NIL_P(args.a[1]) || NIL_P(_filename) || NIL_P(_format))
  {
    rb_raise(rb_eArgError, "wrong arguments for _snapshot({:filename=>'...',"
                              ":format=>:pdf|:ps|:svg}, &block)\n");
  }
  else
  {
    const char      * filename = RSTRING_PTR(_filename);
    cairo_surface_t * surface  = shoes_get_snapshot_surface(_format)
                                      (filename, canvas->width, canvas->height);
    if (surface == NULL) {
        rb_raise(rb_eArgError, "Failed to create %s surface for file %s\n", 
           RSTRING_PTR(rb_inspect(_format)),
           RSTRING_PTR(rb_inspect(_filename)));
    }
    else
    {
      cairo_t * waz_cr = canvas->cr;
      cairo_t * cr     = canvas->cr = cairo_create(surface);
      DRAW(self, canvas->app, rb_funcall(args.a[1], s_call, 0));
      //shoes_canvas_draw (self, self, Qfalse);
      shoes_canvas_draw(self, self, Qtrue);
      canvas->cr = waz_cr;
      cairo_show_page(cr);
      cairo_destroy(cr);
      cairo_surface_destroy(surface);
      //  TODO  detect cairo outrages here
    }
  }
  return Qnil;
}

void
shoes_canvas_compute(VALUE self)
{
  SETUP();
  if (!shoes_canvas_independent(canvas))
    return shoes_canvas_compute(canvas->parent);

  cairo_save(cr);
  shoes_canvas_draw(self, self, Qfalse);
  cairo_restore(cr);
}

static void
shoes_canvas_insert(VALUE self, long i, VALUE ele, VALUE block)
{
  SETUP();

  if (canvas->insertion != -2)
    rb_raise(eInvMode, "this slot is already being modified by an append, clear, etc.");

  if (!NIL_P(ele))
    i = rb_ary_index_of(canvas->contents, ele) - i;

  canvas->insertion = i;
  if (rb_respond_to(block, s_widget))
    rb_funcall(block, s_widget, 1, self);
  else
    shoes_canvas_memdraw(self, block);
  canvas->insertion = -2;
  shoes_canvas_repaint_all(self);
}

VALUE
shoes_canvas_after(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "eo,e&", &args);
  shoes_canvas_insert(self, -1, args.a[0], args.a[1]);
  return self;
}

VALUE
shoes_canvas_before(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "eo,e&", &args);
  shoes_canvas_insert(self, 0, args.a[0], args.a[1]);
  return self;
}

VALUE
shoes_canvas_append(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "o,&", &args);
  shoes_canvas_insert(self, -1, Qnil, args.a[0]);
  return self;
}

VALUE
shoes_canvas_prepend(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  rb_parse_args(argc, argv, "o,&", &args);
  shoes_canvas_insert(self, 0, Qnil, args.a[0]);
  return self;
}

VALUE
shoes_canvas_clear_contents(int argc, VALUE *argv, VALUE self)
{
  VALUE block = Qnil;
  SETUP();
  
  int i;
  for (i = 0; i < RARRAY_LEN(canvas->contents); i++)
  {
    VALUE ele = rb_ary_entry(canvas->contents, i);
    if (rb_obj_class(ele) == cRadio)
    {
        shoes_control *self_t;
        Data_Get_Struct(ele, shoes_control, self_t);
        
        VALUE group = ATTR(self_t->attr, group);
        if (NIL_P(group)) group = self_t->parent;
        if (!shoes_hash_get(canvas->app->groups, group) == Qnil);
              shoes_hash_set(canvas->app->groups, group, Qnil);
    }
  }
  
  if (rb_block_given_p()) block = rb_block_proc();
  shoes_canvas_empty(canvas, FALSE);
  if (!NIL_P(block))
    shoes_canvas_memdraw(self, block);
  shoes_canvas_repaint_all(self);
  return self;
}

VALUE
shoes_canvas_flow(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE flow;
  SETUP();

  rb_parse_args(argc, argv, "|h&", &args);
  flow = shoes_flow_new(args.a[0], self);
  if (!NIL_P(args.a[1]))
  {
    DRAW(flow, canvas->app, rb_funcall(args.a[1], s_call, 0));
  }
  shoes_add_ele(canvas, flow);
  return flow;
}

VALUE
shoes_canvas_stack(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE stack;
  SETUP();

  rb_parse_args(argc, argv, "|h&", &args);
  stack = shoes_stack_new(args.a[0], self);
  if (!NIL_P(args.a[1]))
  {
    DRAW(stack, canvas->app, rb_funcall(args.a[1], s_call, 0));
  }
  shoes_add_ele(canvas, stack);
  
  shoes_canvas *self_t;
  Data_Get_Struct(stack, shoes_canvas, self_t);
  return stack;
}

VALUE
shoes_canvas_mask(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE mask;
  SETUP();

  rb_parse_args(argc, argv, "|h&", &args);
  mask = shoes_mask_new(args.a[0], self);
  if (!NIL_P(args.a[1]))
  {
    DRAW(mask, canvas->app, rb_funcall(args.a[1], s_call, 0));
  }
  shoes_add_ele(canvas, mask);
  return mask;
}

VALUE
shoes_canvas_widget(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE widget, attr = Qnil;
  SETUP();

  rb_parse_args(argc, argv, "k|", &args);
  if (TYPE(argv[argc-1]) == T_HASH)
    attr = argv[argc-1];
  widget = shoes_widget_new(args.a[0], attr, self);
  DRAW(widget, canvas->app, ts_funcall2(widget, rb_intern("initialize"), argc - 1, argv + 1));
  shoes_add_ele(canvas, widget);
  return widget;
}

VALUE
shoes_canvas_download(int argc, VALUE *argv, VALUE self)
{
  VALUE url, block, obj, attr = Qnil;
  SETUP();

  rb_scan_args(argc, argv, "11&", &url, &attr, &block);
  CHECK_HASH(attr);
  if (!NIL_P(block))
    ATTRSET(attr, finish, block);
  obj = shoes_http_threaded(self, url, attr);
  rb_ary_push(canvas->app->extras, obj);
  return obj;
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
    SETUP_BASIC();
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
    app = canvas->app->self;
    if (rb_block_given_p())
      mfp_instance_eval(app, rb_block_proc());
  }
  return app;
}

void
shoes_canvas_repaint_all(VALUE self)
{
  shoes_canvas *canvas;
  self = shoes_find_canvas(self);
  Data_Get_Struct(self, shoes_canvas, canvas);
  if (canvas->stage == CANVAS_EMPTY) return;
  shoes_canvas_compute(self);
  shoes_slot_repaint(canvas->slot);
}

typedef VALUE (*ccallfunc)(VALUE);
typedef void (*ccallfunc2)(SHOES_CONTROL_REF);

static void
shoes_canvas_ccall(VALUE self, ccallfunc func, ccallfunc2 func2, unsigned char check)
{
  shoes_canvas *self_t, *pc;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (check)
  {
    pc = self_t;
    while (!NIL_P(pc->parent))
    {
      Data_Get_Struct(pc->parent, shoes_canvas, pc);
      if (RTEST(ATTR(pc->attr, hidden)))
        return;
    }
  }

  if (!NIL_P(self_t->parent))
  {
    Data_Get_Struct(self_t->parent, shoes_canvas, pc);
    if (DC(self_t->slot) != DC(pc->slot))
      func2(DC(self_t->slot));
  }

  if (!NIL_P(self_t->contents))
  {
    long i;
    for (i = 0; i < RARRAY_LEN(self_t->contents); i++)
    {
      shoes_basic *basic;
      VALUE ele = rb_ary_entry(self_t->contents, i);
      Data_Get_Struct(ele, shoes_basic, basic);
      if (!RTEST(ATTR(basic->attr, hidden)))
      {
        if (rb_obj_is_kind_of(ele, cNative))
          func(ele);
        else if (rb_obj_is_kind_of(ele, cCanvas))
          shoes_canvas_ccall(ele, func, func2, 0);
      }
    }
  }
}

VALUE
shoes_canvas_hide(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  ATTRSET(self_t->attr, hidden, Qtrue);
  shoes_canvas_ccall(self, shoes_control_temporary_hide, shoes_native_control_hide, 1);
  shoes_canvas_repaint_all(self);
  return self;
}

VALUE
shoes_canvas_show(VALUE self)
{
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);
  ATTRSET(self_t->attr, hidden, Qfalse);
  shoes_canvas_ccall(self, shoes_control_temporary_show, shoes_native_control_show, 1);
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
EVENT_HANDLER(keydown);
EVENT_HANDLER(keypress);
EVENT_HANDLER(keyup);
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

    for (i = (int)RARRAY_LEN(canvas->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(canvas->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas) && shoes_canvas_inherits(ele, canvas))
        shoes_canvas_send_start(ele);
    }
    
    // Do we have a :start style attribute ? This is not the 'start' method/event which
    // is handled by shoes_canvas_start() build by EVENT_HANDLER(start).
    // This attribute is set either explicitely with a :start style in the slot declaration
    // either by the 'start' method/event of a slot (if by mistake, both are used the method takes precedence)
    VALUE start = ATTR(canvas->attr, start);
    if (!NIL_P(start))
    {
      shoes_safe_block(self, start, rb_ary_new3(1, self));
    }
  }
}

static void
shoes_canvas_send_finish(VALUE self)
{
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  VALUE finish = ATTR(canvas->attr, finish);
  if (!NIL_P(finish))
    shoes_safe_block(self, finish, rb_ary_new3(1, self));
}

static VALUE
shoes_canvas_send_click2(VALUE self, int button, int x, int y, VALUE *clicked)
{
  long i;
  int ox = x, oy = y;
  VALUE v = Qnil;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (ORIGIN(self_t->place))
  {
    oy = y + self_t->slot->scrolly;
    ox = x - self_t->place.ix + self_t->place.dx;
    oy = oy - (self_t->place.iy + self_t->place.dy);
    if (oy < self_t->slot->scrolly || ox < 0 || oy > self_t->slot->scrolly + self_t->place.ih || ox > self_t->place.iw)
      return Qnil;
  }
  if (ATTR(self_t->attr, hidden) != Qtrue)
  {
    if (self_t->app->canvas == self) // when we are the app's slot
      y -= self_t->slot->scrolly;
    
    if (IS_INSIDE(self_t, x, y))
    {
      VALUE click = ATTR(self_t->attr, click);
      if (!NIL_P(click))
      {
        if (ORIGIN(self_t->place))
          y += self_t->slot->scrolly;
        shoes_safe_block(self, click, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
      }
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
        v = shoes_canvas_send_click(ele, button, ox, oy);
        *clicked = ele;
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        v = shoes_textblock_send_click(ele, button, ox, oy, clicked);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        v = shoes_image_send_click(ele, button, ox, oy);
        *clicked = ele;
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        v = shoes_shape_send_click(ele, button, ox, oy);
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
  int ox = x, oy = y;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  if (ORIGIN(self_t->place))
  {
    oy = y + self_t->slot->scrolly;
    ox = x - self_t->place.ix + self_t->place.dx;
    oy = oy - (self_t->place.iy + self_t->place.dy);
    if (oy < self_t->slot->scrolly || ox < 0 || oy > self_t->slot->scrolly + self_t->place.ih || ox > self_t->place.iw)
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
        if (ORIGIN(self_t->place))
          y += self_t->slot->scrolly;
        shoes_safe_block(self, release, rb_ary_new3(3, INT2NUM(button), INT2NUM(x), INT2NUM(y)));
      }
    }

    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--)
    {
      VALUE ele = rb_ary_entry(self_t->contents, i);
      if (rb_obj_is_kind_of(ele, cCanvas))
      {
        shoes_canvas_send_release(ele, button, ox, oy);
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        shoes_textblock_send_release(ele, button, ox, oy);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        shoes_image_send_release(ele, button, ox, oy);
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        shoes_shape_send_release(ele, button, ox, oy);
      }
    }
  }
}

VALUE
shoes_canvas_send_motion(VALUE self, int x, int y, VALUE url)
{
  char oh, ch = 0, h = 0, *n = 0;
  long i;
  int ox = x, oy = y;
  shoes_canvas *self_t;
  Data_Get_Struct(self, shoes_canvas, self_t);

  oh = self_t->hover;
  ch = h = IS_INSIDE(self_t, x, y);
  CHECK_HOVER(self_t, h, n);

  if (ORIGIN(self_t->place))
  {
    y += self_t->slot->scrolly;
    ox = x - self_t->place.ix + self_t->place.dx;
    oy = y - (self_t->place.iy + self_t->place.dy);
    if (!oh && (oy < self_t->slot->scrolly || ox < 0 || oy > self_t->slot->scrolly + self_t->place.ih || ox > self_t->place.iw))
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
        urll = shoes_canvas_send_motion(ele, ox, oy, url);
      }
      else if (rb_obj_is_kind_of(ele, cTextBlock))
      {
        urll = shoes_textblock_motion(ele, ox, oy, &h);
      }
      else if (rb_obj_is_kind_of(ele, cImage))
      {
        urll = shoes_image_motion(ele, ox, oy, NULL);
      }
      else if (rb_obj_is_kind_of(ele, cShape))
      {
        urll = shoes_shape_motion(ele, ox, oy, NULL);
      }

      if (NIL_P(url)) url = urll;
    }

    if (ch && NIL_P(url))
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
shoes_canvas_wheel_way(shoes_canvas *self_t, ID dir)
{
    if (dir == s_up)
        shoes_slot_scroll_to(self_t, -32, 1);
    else if (dir == s_down)
        shoes_slot_scroll_to(self_t, 32, 1);
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
        if (IS_INSIDE(self_t, x, y))
            shoes_canvas_wheel_way(self_t, dir);
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

#define DEF_SEND_KEY_EVENT(event_name) \
void \
shoes_canvas_send_##event_name (VALUE self, VALUE key) \
{ \
  long i; \
  shoes_canvas *self_t; \
  Data_Get_Struct(self, shoes_canvas, self_t); \
\
  if (ATTR(self_t->attr, hidden) != Qtrue) \
  { \
    VALUE handler = ATTR(self_t->attr, event_name); \
    if (!NIL_P(handler)) \
    { \
      shoes_safe_block(self, handler, rb_ary_new3(1, key)); \
    } \
\
    for (i = RARRAY_LEN(self_t->contents) - 1; i >= 0; i--) \
    { \
      VALUE ele = rb_ary_entry(self_t->contents, i); \
      if (rb_obj_is_kind_of(ele, cCanvas)) \
      { \
        shoes_canvas_send_ ## event_name (ele, key); \
      } \
    } \
  } \
}
DEF_SEND_KEY_EVENT(keydown)
DEF_SEND_KEY_EVENT(keypress)
DEF_SEND_KEY_EVENT(keyup)

SHOES_SLOT_OS *
shoes_slot_alloc(shoes_canvas *canvas, SHOES_SLOT_OS *parent, int toplevel)
{
  canvas->slot = SHOE_ALLOC(SHOES_SLOT_OS);
  SHOE_MEMZERO(canvas->slot, SHOES_SLOT_OS, 1);
  canvas->slot->owner = canvas;
  return canvas->slot;
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
  COPY_PENS(self_t->attr, pc->attr);
  int scrolls = RTEST(ATTR(self_t->attr, scroll));
  if ((attr != ssNestSlot && RTEST(ATTR(self_t->attr, height))) || scrolls) {
    //
    // create the slot off-screen until it can be properly placed
    //
    shoes_slot_init(self, pc->slot, -99, -99, 100, 100, scrolls, FALSE);
    self_t->place.x = self_t->place.y = 0;
    self_t->place.ix = self_t->place.iy = 0;
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
  SETUP();
  return shoes_app_window(argc, argv, cApp, canvas->app->self);
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

