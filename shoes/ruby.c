//
// shoes/ruby.c
// Just little bits of Ruby I've become accustomed to.
//
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/dialogs.h"
#include "shoes/internal.h"

VALUE cShoes, cCanvas, cFlow, cStack, cPath, cImage, cAnim, cBackground, cTextClass, cButton, cEditLine, cEditBox, cListBox, cProgress, cColor, cLink;
VALUE reHEX_SOURCE, reHEX3_SOURCE, reRGB_SOURCE, reRGBA_SOURCE, reGRAY_SOURCE, reGRAYA_SOURCE;
ID s_aref, s_perc, s_bind, s_new, s_run, s_to_s, s_arrow, s_call, s_center, s_change, s_click, s_corner, s_draw, s_font, s_hand, s_hidden, s_insert, s_items, s_match, s_text, s_top, s_right, s_bottom, s_left, s_height, s_remove, s_width, s_margin, s_margin_left, s_margin_right, s_margin_top, s_margin_bottom, s_radius;

//
// Mauricio's instance_eval hack (he bested my cloaker back in 06 Jun 2006)
//
VALUE instance_eval_proc, exception_proc, exception_alert_proc;

VALUE
mfp_instance_eval(VALUE obj, VALUE block)
{
  return rb_funcall(instance_eval_proc, s_call, 2, obj, block);
}

long
rb_ary_index_of(VALUE ary, VALUE val)
{
  long i;
 
  for (i=0; i<RARRAY(ary)->len; i++) {
    if (rb_equal(RARRAY(ary)->ptr[i], val))
      return i;
  }

  return -1;
}

VALUE
rb_ary_insert_at(VALUE ary, long index, int len, VALUE ary2)
{
  rb_funcall(ary, s_aref, 3, LONG2NUM(index), INT2NUM(len), ary2);
  return ary;
}

typedef struct
{
  VALUE canvas;
  VALUE block;
  VALUE args;
} safe_block;

static VALUE
shoes_safe_block_call(VALUE rb_sb)
{
  int i;
  VALUE vargs[10];
  safe_block *sb = (safe_block *)rb_sb;
  for (i = 0; i < RARRAY_LEN(sb->args); i++)
    vargs[i] = rb_ary_entry(sb->args, i);
  return rb_funcall2(sb->block, s_call, RARRAY_LEN(sb->args), vargs);
}

static VALUE
shoes_safe_block_exception(VALUE rb_sb, VALUE e)
{
  safe_block *sb = (safe_block *)rb_sb;
  rb_iv_set(sb->canvas, "@exc", e);
  return mfp_instance_eval(sb->canvas, exception_alert_proc);
}

VALUE
shoes_safe_block(VALUE canvas, VALUE block, VALUE args)
{
  safe_block sb;
  VALUE v;
  sb.canvas = canvas;
  sb.block = block;
  sb.args = args;
  rb_gc_register_address(&args);
  v = rb_rescue2(CASTHOOK(shoes_safe_block_call), (VALUE)&sb, 
    CASTHOOK(shoes_safe_block_exception), (VALUE)&sb, rb_cObject, 0);
  rb_gc_unregister_address(&args);
  return v;
}

int
shoes_px(VALUE attr, ID k, int dv, int pv)
{
  int px;
  VALUE obj = shoes_hash_get(attr, k);
  if (rb_obj_is_kind_of(obj, rb_cFloat)) {
    px = (int)((double)pv * NUM2DBL(obj));
  } else {
    if (NIL_P(obj))
      px = dv;
    else
      px = NUM2INT(obj);
    if (px < 0)
      px += pv;
  }
  return px;
}

int
shoes_px2(VALUE attr, ID k1, ID k2, int dv, int dr, int pv)
{
  int px;
  VALUE obj = shoes_hash_get(attr, k2);
  if (!NIL_P(obj))
  {
    px = shoes_px(attr, k2, 0, pv);
    px = (pv - dr) - px;
  }
  else
  {
    px = shoes_px(attr, k1, dv, pv);
  }
  return px;
}

VALUE
shoes_hash_set(VALUE hsh, ID key, VALUE val)
{
  if (NIL_P(hsh))
    hsh = rb_hash_new();
  rb_hash_aset(hsh, ID2SYM(key), val);
  return hsh;
}

VALUE
shoes_hash_get(VALUE hsh, ID key)
{
  VALUE v;

  if (!NIL_P(hsh))
  {
    v = rb_hash_aref(hsh, ID2SYM(key));
    if (!NIL_P(v)) return v;
  }

  return Qnil;
}

int
shoes_hash_int(VALUE hsh, ID key, int dn)
{
  VALUE v = shoes_hash_get(hsh, key);
  if (!NIL_P(v)) return NUM2INT(v);
  return dn;
}

double
shoes_hash_dbl(VALUE hsh, ID key, double dn)
{
  VALUE v = shoes_hash_get(hsh, key);
  if (!NIL_P(v)) return NUM2DBL(v);
  return dn;
}

char *
shoes_hash_cstr(VALUE hsh, ID key, char *dn)
{
  VALUE v = shoes_hash_get(hsh, key);
  if (!NIL_P(v)) return RSTRING_PTR(v);
  return dn;
}

VALUE
rb_str_to_pas(VALUE str)
{
  VALUE str2;
  char strlen[2];
  strlen[0] = RSTRING_LEN(str);
  strlen[1] = '\0';
  str2 = rb_str_new2(strlen);
  rb_str_append(str2, str);
  return str2;
}

void
shoes_place_decide(shoes_place *place, VALUE c, VALUE attr, int dw, int dh, char rel)
{
  shoes_canvas *canvas = NULL;
  VALUE ck = rb_obj_class(c);
  if (!NIL_P(c))
    Data_Get_Struct(c, shoes_canvas, canvas);

  ATTR_MARGINS(attr, 0);
  int testw = dw;
  if (testw == 0) testw = lmargin + 1 + rmargin;

  if (canvas == NULL)
  {
    place->x = 0;
    place->y = 0;
    place->w = dw;
    place->h = dh;
  }
  else
  {
    int cx, cy, ox, oy, tw, th;

    if (rel == REL_WINDOW)
    {
      cx = 0; cy = 0;
      ox = 0; oy = 0;
    }
    else if (rel == REL_CANVAS || rel == REL_TILE)
    {
      cx = canvas->cx - canvas->place.x;
      cy = canvas->cy - canvas->place.y;
      ox = canvas->place.x;
      oy = canvas->place.y;
      if (rel == REL_TILE)
      {
        tw = dw; th = dh;
        testw = dw = canvas->place.w; dh = canvas->height;
      }
    }
    else
    {
      cx = 0; cy = 0;
      ox = canvas->cx; oy = canvas->cy;
    }

    place->w = PX(attr, width, testw, canvas->place.w);
    if (dw == 0 && place->w + (int)canvas->cx > canvas->place.w) {
      canvas->cx = canvas->endx = canvas->place.x;
      canvas->cy = canvas->endy;
      place->w = canvas->place.w;
    }
    place->h = PX(attr, height, dh, canvas->fully);

    if (rel != REL_TILE)
    {
      tw = place->w; th = place->h;
    }

    place->x = PX2(attr, left, right, cx, tw, canvas->place.w) + ox;
    place->y = PX2(attr, top, bottom, cy, th, canvas->fully) + oy;
    place->absx = (NIL_P(ATTR(attr, left)) && NIL_P(ATTR(attr, right)) ? 0 : 1);
    place->absy = (NIL_P(ATTR(attr, top)) && NIL_P(ATTR(attr, bottom)) ? 0 : 1);
    if (place->absy == 0 && (ck == cStack || place->x + place->w > canvas->place.w))
    {
      canvas->cx = place->x = canvas->place.x;
      canvas->cy = place->y = canvas->endy;
    }
  }

  place->w -= lmargin + rmargin;
  place->h -= tmargin + bmargin;
  place->x += lmargin;
  place->y += tmargin;
  INFO("PLACE: (%d, %d), (%d, %d)\n", place->x, place->y, place->w, place->h);
}

void
shoes_cairo_rect(cairo_t *cr, double x, double y, double w, double h, double r)
{
  double rc = r * BEZIER;
  cairo_new_path(cr);
  cairo_move_to(cr, x + r, y);
  cairo_rel_line_to(cr, w - 2 * r, 0.0);
  cairo_rel_curve_to(cr, rc, 0.0, r, rc, r, r);
  cairo_rel_line_to(cr, 0, h - 2 * r);
  cairo_rel_curve_to(cr, 0.0, rc, rc - r, r, -r, r);
  cairo_rel_line_to(cr, -w + 2 * r, 0);
  cairo_rel_curve_to(cr, -rc, 0, -r, -rc, -r, -r);
  cairo_rel_line_to(cr, 0, -h + 2 * r);
  cairo_rel_curve_to(cr, 0.0, -rc, r - rc, -r, r, -r);
  cairo_close_path(cr);
}

//
// Macros for setting up drawing
//
#define SETUP(self_type, rel, dw, dh) \
  self_type *self_t; \
  shoes_place place; \
  shoes_canvas *canvas; \
  VALUE ck = rb_obj_class(c); \
  Data_Get_Struct(self, self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  shoes_place_decide(&place, c, self_t->attr, dw, dh, rel)

#define SETUP_CONTROL(dh) \
  char *msg = ""; \
  int len = 200; \
  shoes_control *self_t; \
  shoes_canvas *canvas; \
  shoes_place place; \
  VALUE text = Qnil, ck = rb_obj_class(c); \
  Data_Get_Struct(self, shoes_control, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  text = ATTR(self_t->attr, text); \
  if (!NIL_P(text)) { \
    msg = RSTRING_PTR(text); \
    len = (RSTRING_LEN(text) * 6) + 32; \
  } \
  shoes_place_decide(&place, c, self_t->attr, len, 28 + dh, REL_CANVAS)

#define FINISH() \
  self_t->place = place; \
  if (!self_t->place.absy) { \
    canvas->cx += self_t->place.w; \
    canvas->cy = self_t->place.y; \
    canvas->endx = canvas->cx; \
    canvas->endy = self_t->place.y + self_t->place.h; \
  } \
  if (ck == cStack) { \
    canvas->cx = 0; \
    canvas->cy = canvas->endy; \
  }

#define CHANGED_COORDS() self_t->place.x != place.x || self_t->place.y != place.y || self_t->place.w != place.w || self_t->place.h - HEIGHT_PAD != place.h
#define PLACE_COORDS() place.h -= HEIGHT_PAD; self_t->place = place
#ifdef SHOES_GTK
#define HEIGHT_PAD 0

#define GTK_CHILD(child, ptr) \
  GList *children = gtk_container_get_children(GTK_CONTAINER(ptr)); \
  child = children->data

#define PLACE_CONTROL() \
  gtk_widget_set_size_request(self_t->ref, place.w, place.h); \
  gtk_layout_put(GTK_LAYOUT(canvas->slot.canvas), self_t->ref, place.x, place.y); \
  gtk_widget_show(self_t->ref); \
  PLACE_COORDS();

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    gtk_layout_move(GTK_LAYOUT(canvas->slot.canvas), self_t->ref, place.x, place.y); \
    gtk_widget_set_size_request(self_t->ref, place.w, place.h); \
    PLACE_COORDS(); \
  } \
  if (canvas->slot.expose != NULL) \
  { \
    gtk_container_propagate_expose(GTK_CONTAINER(canvas->slot.canvas), self_t->ref, canvas->slot.expose); \
  }
#endif

#ifdef SHOES_QUARTZ
#define HEIGHT_PAD 10

#define PLACE_CONTROL() \
  HIRect hr; \
  hr.origin.x = place.x; hr.origin.y = place.y; \
  hr.size.width = place.w; hr.size.height = place.h - HEIGHT_PAD; \
  HIViewAddSubview(canvas->slot.view, self_t->ref); \
  SetControlCommandID(self_t->ref, SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls)); \
  SetControlReference(self_t->ref, (SInt32)canvas->slot.scrollview); \
  HIViewSetFrame(self_t->ref, &hr); \
  HIViewSetVisible(self_t->ref, true); \
  rb_ary_push(canvas->slot.controls, self); \
  PLACE_COORDS();

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    HIRect hr; \
    hr.origin.x = place.x; hr.origin.y = place.y; \
    hr.size.width = place.w; hr.size.height = place.h - HEIGHT_PAD; \
    HIViewSetFrame(self_t->ref, &hr); \
    PLACE_COORDS(); \
  }

static CFStringRef
shoes_rb2cf(VALUE str)
{
  CFStringRef cf;
  char *msg = "";
  if (!NIL_P(str)) msg = RSTRING_PTR(str);
  cf = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
  return cf;
}

static VALUE
shoes_cf2rb(CFStringRef cf)
{
  VALUE str;
  char *text;
  CFIndex len = CFStringGetLength(cf) * 2;
  if (len > 0)
  {
    text = SHOE_ALLOC_N(char, len);
    CFStringGetCString(cf, text, len, kCFStringEncodingUTF8);
    str = rb_str_new2(text);
    SHOE_FREE(text);
  }
  else
  {
    str = rb_str_new2("");
  }
  return str;
}
#endif

#ifdef SHOES_WIN32
#define HEIGHT_PAD 6

#define PLACE_CONTROL() \
  PLACE_COORDS()

#define REPAINT_CONTROL() \
  if (CHANGED_COORDS()) { \
    PLACE_COORDS(); \
    MoveWindow(self_t->ref, place.x, place.y, place.w, place.h, FALSE); \
  }

inline void shoes_win32_control_font(int id, HWND hwnd)
{
  SendDlgItemMessage(hwnd, id, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(true, 0));
}
#endif

//
// Shoes::Path
//
static void
shoes_path_mark(shoes_path *path)
{
  rb_gc_mark_maybe(path->parent);
  rb_gc_mark_maybe(path->attr);
}

static void
shoes_path_free(shoes_path *path)
{
  if (path->line != NULL)
    cairo_path_destroy(path->line);
  free(path);
}

VALUE
shoes_path_new(cairo_path_t *line, VALUE parent, VALUE x, VALUE y)
{
  shoes_path *path;
  shoes_canvas *canvas;
  VALUE obj = shoes_path_alloc(cPath);
  Data_Get_Struct(obj, shoes_path, path);
  Data_Get_Struct(parent, shoes_canvas, canvas);
  path->line = line;
  path->parent = parent;
  path->fg = canvas->fg;
  path->bg = canvas->bg;
  path->sw = canvas->sw;
  ATTRSET(path->attr, left, x);
  ATTRSET(path->attr, top, y);
  return obj;
}

VALUE
shoes_path_alloc(VALUE klass)
{
  shoes_path *path;
  VALUE obj = Data_Make_Struct(klass, shoes_path, shoes_path_mark, shoes_path_free, path);
  path->line = NULL;
  path->attr = Qnil;
  path->parent = Qnil;
  return obj;
}

VALUE
shoes_path_move(VALUE self, VALUE x, VALUE y)
{
  shoes_path *self_t;
  Data_Get_Struct(self, shoes_path, self_t);
  ATTRSET(self_t->attr, left, x);
  ATTRSET(self_t->attr, top, y);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE
shoes_path_remove(VALUE self)
{
  shoes_path *self_t;
  Data_Get_Struct(self, shoes_path, self_t);
  shoes_canvas_remove_item(self_t->parent, self);
  return self;
}

VALUE
shoes_path_draw(VALUE self, VALUE c)
{
  double x, y;
  shoes_path *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_path, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

  x = ATTR2(dbl, self_t->attr, left, 0);
  y = ATTR2(dbl, self_t->attr, top, 0);
  cairo_save(canvas->cr);
  cairo_new_path(canvas->cr);
  cairo_translate(canvas->cr, x, y);
  cairo_set_line_width(canvas->cr, self_t->sw);
  cairo_append_path(canvas->cr, self_t->line);

  if (self_t->bg.on)
  {
    cairo_set_source_rgba(canvas->cr, self_t->bg.r / 255., self_t->bg.g / 255., self_t->bg.b / 255., self_t->bg.a / 255.);
    cairo_fill_preserve(canvas->cr);
  }
  if (self_t->fg.on)
  {
    cairo_set_source_rgba(canvas->cr, self_t->fg.r / 255., self_t->fg.g / 255., self_t->fg.b / 255., self_t->fg.a / 255.);
    cairo_stroke_preserve(canvas->cr);
  }
  cairo_restore(canvas->cr);

  return self;
}

//
// Shoes::Image
//
static void
shoes_image_mark(shoes_image *image)
{
  rb_gc_mark_maybe(image->path);
  rb_gc_mark_maybe(image->parent);
  rb_gc_mark_maybe(image->attr);
}

static void
shoes_image_free(shoes_image *image)
{
  if (image->surface != NULL)
    cairo_surface_destroy(image->surface);
  free(image);
}

VALUE
shoes_image_new(VALUE klass, VALUE path, VALUE attr, VALUE parent)
{
  shoes_image *image;
  VALUE obj = shoes_image_alloc(klass);
  Data_Get_Struct(obj, shoes_image, image);
  image->path = path;
  image->surface = cairo_image_surface_create_from_png(RSTRING_PTR(path));
  image->width = cairo_image_surface_get_width(image->surface);
  image->height = cairo_image_surface_get_height(image->surface);
  image->attr = attr;
  image->parent = parent;
  return obj;
}

VALUE
shoes_image_alloc(VALUE klass)
{
  shoes_image *image;
  VALUE obj = Data_Make_Struct(klass, shoes_image, shoes_image_mark, shoes_image_free, image);
  image->path = Qnil;
  image->surface = NULL;
  image->attr = Qnil;
  image->parent = Qnil;
  image->width = 0.0;
  image->height = 0.0;
  return obj;
}

VALUE
shoes_image_remove(VALUE self)
{
  shoes_image *self_t;
  Data_Get_Struct(self, shoes_image, self_t);
  shoes_canvas_remove_item(self_t->parent, self);
  return self;
}

VALUE
shoes_image_draw(VALUE self, VALUE c)
{
  SETUP(shoes_image, REL_CANVAS, self_t->width, self_t->height);
  shoes_canvas_shape_do(canvas, place.x, place.y, place.w, place.h, FALSE);
  if (place.w != self_t->width || place.h != self_t->height)
  {
    cairo_scale(canvas->cr, place.w/self_t->width, place.h/self_t->height);
  }
  cairo_set_source_surface(canvas->cr, self_t->surface, -place.w / 2., -place.h / 2.);
  cairo_paint(canvas->cr);
  cairo_restore(canvas->cr);
  FINISH();
  return self;
}

//
// Shoes::Pattern
//
static void
shoes_pattern_mark(shoes_pattern *pattern)
{
  rb_gc_mark_maybe(pattern->source);
  rb_gc_mark_maybe(pattern->parent);
  rb_gc_mark_maybe(pattern->attr);
}

static void
shoes_pattern_free(shoes_pattern *pattern)
{
  if (pattern->surface != NULL)
    cairo_surface_destroy(pattern->surface);
  if (pattern->pattern != NULL)
    cairo_pattern_destroy(pattern->pattern);
  free(pattern);
}

VALUE
shoes_pattern_new(VALUE klass, VALUE source, VALUE attr, VALUE parent)
{
  shoes_pattern *pattern;
  VALUE obj = shoes_pattern_alloc(klass);
  VALUE rgb;
  Data_Get_Struct(obj, shoes_pattern, pattern);
  pattern->source = source;

  if (!rb_obj_is_kind_of(source, cColor))
  {
    rgb = shoes_color_parse(cColor, source);
    if (!NIL_P(rgb)) source = rgb;
  }

  if (rb_obj_is_kind_of(source, cColor))
  {
    pattern->pattern = shoes_color_pattern(source);
    pattern->width = pattern->height = 1.;
  }
  else
  {
    pattern->surface = cairo_image_surface_create_from_png(RSTRING_PTR(source));
    pattern->width = cairo_image_surface_get_width(pattern->surface);
    pattern->height = cairo_image_surface_get_height(pattern->surface);
    pattern->pattern = cairo_pattern_create_for_surface(pattern->surface);
  }
  cairo_pattern_set_extend(pattern->pattern, CAIRO_EXTEND_REPEAT);

  pattern->attr = attr;
  pattern->parent = parent;
  return obj;
}

VALUE
shoes_pattern_alloc(VALUE klass)
{
  shoes_pattern *pattern;
  VALUE obj = Data_Make_Struct(klass, shoes_pattern, shoes_pattern_mark, shoes_pattern_free, pattern);
  pattern->source = Qnil;
  pattern->surface = NULL;
  pattern->pattern = NULL;
  pattern->attr = Qnil;
  pattern->parent = Qnil;
  return obj;
}

VALUE
shoes_pattern_remove(VALUE self)
{
  shoes_pattern *self_t;
  Data_Get_Struct(self, shoes_pattern, self_t);
  shoes_canvas_remove_item(self_t->parent, self);
  return self;
}

VALUE
shoes_pattern_draw(VALUE self, VALUE c)
{
  SETUP(shoes_pattern, REL_CANVAS, 0, 0);
  shoes_canvas_shape_do(canvas, place.x, place.y, place.w, place.h, FALSE);
  cairo_set_source_surface(canvas->cr, self_t->surface, -place.w / 2., -place.h / 2.);
  cairo_paint(canvas->cr);
  cairo_restore(canvas->cr);
  FINISH();
  return self;
}

VALUE
shoes_background_draw(VALUE self, VALUE c)
{
  double r;
  SETUP(shoes_pattern, REL_TILE, self_t->width, self_t->height);
  r = ATTR2(dbl, self_t->attr, radius, 0.);
  shoes_canvas_shape_do(canvas, place.x, place.y, 0, 0, FALSE);
  cairo_set_source(canvas->cr, self_t->pattern);
  shoes_cairo_rect(canvas->cr, 0, 0, place.w, place.h, r);
  INFO("BACKGROUND: (%d, %d), (%d, %d)\n", place.x, place.y, place.w, place.h);
  cairo_fill(canvas->cr);
  place.w = 0; place.h = 0;
  cairo_restore(canvas->cr);
  FINISH();
  return self;
}

//
// Shoes::Color
//
static void
shoes_color_mark(shoes_color *color)
{
}

static void
shoes_color_free(shoes_color *color)
{
  free(color);
}

VALUE
shoes_color_new(int r, int g, int b, int a)
{
  shoes_color *color;
  VALUE obj = shoes_color_alloc(cColor);
  Data_Get_Struct(obj, shoes_color, color);
  color->r = r;
  color->g = g;
  color->b = b;
  color->a = a;
  return obj;
}

VALUE
shoes_color_alloc(VALUE klass)
{
  shoes_color *color;
  VALUE obj = Data_Make_Struct(klass, shoes_color, shoes_color_mark, shoes_color_free, color);
  color->r = 0x00;
  color->g = 0x00;
  color->b = 0x00;
  color->a = 0xFF;
  color->on = TRUE;
  return obj;
}

VALUE
shoes_color_rgb(int argc, VALUE *argv, VALUE self)
{
  int a;
  VALUE _r, _g, _b, _a;
  rb_scan_args(argc, argv, "31", &_r, &_g, &_b, &_a);

  a = 0xFF;
  if (!NIL_P(_a)) a = NUM2RGBINT(_a);
  return shoes_color_new(NUM2RGBINT(_r), NUM2RGBINT(_g), NUM2RGBINT(_b), a);
}

VALUE
shoes_color_gray(int argc, VALUE *argv, VALUE self)
{
  shoes_color *color;
  VALUE _g, _a;
  int g, a;
  rb_scan_args(argc, argv, "11", &_g, &_a);

  a = 0xFF;
  g = NUM2RGBINT(_g);
  if (!NIL_P(_a)) a = NUM2RGBINT(_a);
  return shoes_color_new(g, g, g, a);
}

cairo_pattern_t *
shoes_color_pattern(VALUE obj)
{
  shoes_color *color;
  Data_Get_Struct(obj, shoes_color, color);
  if (color->a == 255)
    return cairo_pattern_create_rgb(color->r / 255., color->g / 255., color->b / 255.);
  else
    return cairo_pattern_create_rgba(color->r / 255., color->g / 255., color->b / 255., color->a / 255.);
}

VALUE
shoes_color_args(int argc, VALUE *argv, VALUE self)
{
  VALUE _r, _g, _b, _a, _color;
  argc = rb_scan_args(argc, argv, "13", &_r, &_g, &_b, &_a);

  if (argc == 1 && rb_obj_is_kind_of(_r, cColor))
    _color = _r;
  else if (argc == 1 && rb_obj_is_kind_of(_r, rb_cString))
    _color = shoes_color_parse(cColor, _r);
  else if (argc == 1 || argc == 2)
    _color = shoes_color_gray(argc, argv, cColor);
  else
    _color = shoes_color_rgb(argc, argv, cColor);

  return _color;
}

VALUE
shoes_color_parse(VALUE self, VALUE source)
{
  shoes_color *color;
  VALUE obj = shoes_color_alloc(cColor);
  VALUE reg;
  Data_Get_Struct(obj, shoes_color, color);

  reg = rb_funcall(source, s_match, 1, reHEX3_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = NUM2INT(rb_str2inum(rb_reg_nth_match(1, reg), 16)) * 17;
    color->g = NUM2INT(rb_str2inum(rb_reg_nth_match(2, reg), 16)) * 17;
    color->b = NUM2INT(rb_str2inum(rb_reg_nth_match(3, reg), 16)) * 17;
    return obj;
  }

  reg = rb_funcall(source, s_match, 1, reHEX_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = NUM2INT(rb_str2inum(rb_reg_nth_match(1, reg), 16));
    color->g = NUM2INT(rb_str2inum(rb_reg_nth_match(2, reg), 16));
    color->b = NUM2INT(rb_str2inum(rb_reg_nth_match(3, reg), 16));
    return obj;
  }

  reg = rb_funcall(source, s_match, 1, reRGB_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
    color->g = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
    color->b = NUM2INT(rb_Integer(rb_reg_nth_match(3, reg)));
    return obj;
  }

  reg = rb_funcall(source, s_match, 1, reRGBA_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
    color->g = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
    color->b = NUM2INT(rb_Integer(rb_reg_nth_match(3, reg)));
    color->a = NUM2INT(rb_Integer(rb_reg_nth_match(4, reg)));
    return obj;
  }

  reg = rb_funcall(source, s_match, 1, reGRAY_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = color->g = color->b = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
    return obj;
  }

  reg = rb_funcall(source, s_match, 1, reGRAYA_SOURCE);
  if (!NIL_P(reg))
  {
    color->r = color->g = color->b = NUM2INT(rb_Integer(rb_reg_nth_match(1, reg)));
    color->a = NUM2INT(rb_Integer(rb_reg_nth_match(2, reg)));
    return obj;
  }

  return Qnil;
}

VALUE
shoes_color_to_s(VALUE self)
{
  shoes_color *color;
  Data_Get_Struct(self, shoes_color, color);
  VALUE ary = rb_ary_new3(4, INT2NUM(color->r), INT2NUM(color->g), INT2NUM(color->b), INT2NUM(color->a));
  if (color->a == 1.0)
    return rb_funcall(rb_str_new2("rgb(%d, %d, %d)"), s_perc, 1, ary);
  else
    return rb_funcall(rb_str_new2("rgb(%d, %d, %d, %d)"), s_perc, 1, ary);
}

//
// Shoes::Link
//
static void
shoes_link_mark(shoes_link *link)
{
  rb_gc_mark_maybe(link->url);
}

static void
shoes_link_free(shoes_link *link)
{
  free(link);
}

VALUE
shoes_link_new(VALUE url, int start, int end)
{
  shoes_link *link;
  VALUE obj = shoes_link_alloc(cLink);
  Data_Get_Struct(obj, shoes_link, link);
  link->url = url;
  link->start = start;
  link->end = end;
  return obj;
}

VALUE
shoes_link_alloc(VALUE klass)
{
  shoes_link *link;
  VALUE obj = Data_Make_Struct(klass, shoes_link, shoes_link_mark, shoes_link_free, link);
  link->start = 0;
  link->end = 0;
  link->url = Qnil;
  return obj;
}

VALUE
shoes_link_at(VALUE self, int index)
{
  shoes_link *link;
  Data_Get_Struct(self, shoes_link, link);
  if (link->start <= index && link->end >= index)
  {
    return link->url;
  }
  return Qnil;
}

//
// Shoes::Text
//
static void
shoes_text_mark(shoes_text *text)
{
  rb_gc_mark_maybe(text->markup);
  rb_gc_mark_maybe(text->string);
  rb_gc_mark_maybe(text->links);
  rb_gc_mark_maybe(text->parent);
  rb_gc_mark_maybe(text->attr);
  rb_gc_mark_maybe(text->linku);
}

static void
shoes_text_free(shoes_text *text)
{
  if (text->layout == NULL)
    g_object_unref(text->layout);
  free(text);
}

static void
start_element_handler(GMarkupParseContext *context,
					  const gchar *element_name,
					  const gchar **attribute_names,
					  const gchar **attribute_values,
					  gpointer user_data,
					  GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;

	if (!strcmp(element_name, "a"))
	{
		const gchar *url = NULL;
		int line_number;
		int char_number;
		int i;

		g_markup_parse_context_get_position(context, &line_number,
											&char_number);

		for (i = 0; attribute_names[i] != NULL; i++)
		{
			const gchar *attr = attribute_names[i];

			if (!strcmp(attr, "href"))
			{
				if (url != NULL)
				{
					g_set_error(error, G_MARKUP_ERROR,
								G_MARKUP_ERROR_INVALID_CONTENT,
								"Attribute '%s' occurs twice on <a> tag "
								"on line %d char %d, may only occur once",
								attribute_names[i], line_number, char_number);
					return;
				}

				url = attribute_values[i];
			}
			else
			{
				g_set_error(error, G_MARKUP_ERROR,
							G_MARKUP_ERROR_UNKNOWN_ATTRIBUTE,
							"Attribute '%s' is not allowed on the <a> tag "
							"on line %d char %d",
							attribute_names[i], line_number, char_number);
				return;
			}
		}

		if (url == NULL)
		{
			g_set_error(error, G_MARKUP_ERROR,
						G_MARKUP_ERROR_INVALID_CONTENT,
						"Attribute 'href' was missing on the <a> tag "
						"on line %d char %d",
						line_number, char_number);
			return;
		}

		g_string_append(self_t->tmp,
						"<span color=\"blue\" underline=\"single\">");

    self_t->linku = rb_str_new2(url);
    self_t->linki = self_t->i;
	}
	else
	{
		int i;

		g_string_append_printf(self_t->tmp,
							   "<%s", element_name);

		for (i = 0; attribute_names[i] != NULL; i++)
		{
			const gchar *attr  = attribute_names[i];
			const gchar *value = attribute_values[i];

			g_string_append_printf(self_t->tmp,
								   " %s=\"%s\"",
								   attr, value);
		}

		g_string_append_c(self_t->tmp, '>');
	}
}

static void
end_element_handler(GMarkupParseContext *context,
					const gchar *element_name,
					gpointer user_data,
					GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;

	if (!strcmp(element_name, "a"))
	{
    if (self_t->linki >= 0)
    {
      rb_ary_push(self_t->links, shoes_link_new(self_t->linku, self_t->linki, self_t->i - 1));
      self_t->linku = Qnil;
      self_t->linki = -1;
    }
		g_string_append(self_t->tmp, "</span>");
	}
	else
	{
		g_string_append_printf(self_t->tmp,
							   "</%s>", element_name);
	}
}

static void
text_handler(GMarkupParseContext *context,
			 const gchar *text,
			 gsize text_len,
			 gpointer user_data,
			 GError **error)
{
  shoes_text *self_t = (shoes_text *)user_data;
  self_t->i += text_len;
	gchar *newtext = g_markup_escape_text(text, text_len);
	g_string_append_len(self_t->tmp, newtext, strlen (newtext));
	g_free (newtext);
}

static const GMarkupParser markup_parser =
{
  start_element_handler,
  end_element_handler,
  text_handler,
  NULL,
  NULL
};

VALUE
shoes_text_parse(VALUE self, VALUE markup)
{
  shoes_text *self_t;
  GMarkupParseContext *context = NULL;
  GError *error = NULL;
  const gchar *start;
  gsize len;
  VALUE str = rb_str_new2("<span rise='10000' color='#333'>");
  rb_str_append(str, markup);
  rb_str_cat2(str, "</span>");

  Data_Get_Struct(self, shoes_text, self_t);
  start = RSTRING_PTR(str);
  len = RSTRING_LEN(str);

  self_t->string = markup;
  self_t->tmp = g_string_new(NULL);
  self_t->links = rb_ary_new();
  self_t->i = 0;

  context = g_markup_parse_context_new(&markup_parser, (GMarkupParseFlags)0, self_t, NULL);
  if (g_markup_parse_context_parse(context, start, len, &error) &&
      g_markup_parse_context_end_parse(context, &error))
  {
    self_t->markup = rb_str_new2(g_string_free(self_t->tmp, FALSE));
  }
  else
  {
    g_string_free(self_t->tmp, TRUE);
  }

  self_t->tmp = NULL;
  if (error != NULL)
    g_error_free(error);

  g_markup_parse_context_free(context);

  return Qtrue;
}

VALUE
shoes_text_new(VALUE markup, VALUE attr, VALUE parent)
{
  shoes_text *text;
  VALUE obj = shoes_text_alloc(cTextClass);
  Data_Get_Struct(obj, shoes_text, text);
  text->attr = attr;
  text->parent = parent;
  text->linku = Qnil;
  text->linki = -1;
  shoes_text_parse(obj, markup);
  return obj;
}

VALUE
shoes_text_alloc(VALUE klass)
{
  shoes_text *text;
  VALUE obj = Data_Make_Struct(klass, shoes_text, shoes_text_mark, shoes_text_free, text);
  text->markup = Qnil;
  text->string = Qnil;
  text->links = Qnil;
  text->attr = Qnil;
  text->parent = Qnil;
  text->layout = NULL;
  text->cursor = Qnil;
  return obj;
}

VALUE
shoes_text_set_cursor(VALUE self, VALUE pos)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  self_t->cursor = pos;
  return pos;
}

VALUE
shoes_text_get_cursor(VALUE self)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  return self_t->cursor;
}

VALUE
shoes_text_remove(VALUE self)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  shoes_canvas_remove_item(self_t->parent, self);
  return self;
}

VALUE
shoes_text_get_markup(VALUE self)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  return self_t->string;
}

VALUE
shoes_text_set_markup(VALUE self, VALUE markup)
{
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  shoes_text_parse(self, markup);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

static char
shoes_text_is_here(VALUE self, int x, int y)
{
  shoes_text *self_t;
  int index, trailing, i;
  Data_Get_Struct(self, shoes_text, self_t);
  if (self_t->layout == NULL) return Qnil;

  x -= self_t->place.x;
  y -= self_t->place.y;
  return pango_layout_xy_to_index(self_t->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing);
}

static VALUE
shoes_text_hover(VALUE self, int x, int y)
{
  int index, trailing, i;
  shoes_text *self_t;
  Data_Get_Struct(self, shoes_text, self_t);
  if (self_t->layout == NULL) return Qnil;

  x -= self_t->place.x;
  y -= self_t->place.y;
  if (pango_layout_xy_to_index(self_t->layout, x * PANGO_SCALE, y * PANGO_SCALE, &index, &trailing))
  {
    for (i = 0; i < RARRAY_LEN(self_t->links); i++)
    {
      VALUE url = shoes_link_at(rb_ary_entry(self_t->links, i), index);
      if (!NIL_P(url))
      {
        return url;
      }
    }
  }

  return Qnil;
}

VALUE
shoes_text_motion(VALUE self, int x, int y)
{
  if (shoes_text_is_here(self, x, y))
  {
    VALUE url = shoes_text_hover(self, x, y);
    if (!NIL_P(url))
      shoes_app_cursor(global_app, s_hand);
    return url;
  }
  return Qnil;
}

VALUE
shoes_text_click(VALUE self, int button, int x, int y)
{
  if (button == 1)
    return shoes_text_hover(self, x, y);

  return Qnil;
}

VALUE
shoes_text_draw(VALUE self, VALUE c)
{
  int px, py, pd, li, m;
  double cx, cy;
  char *font;
  shoes_text *self_t;
  shoes_canvas *canvas;
  PangoLayoutLine *last;
  PangoRectangle lrect;
  PangoFontDescription *desc;

  VALUE ck = rb_obj_class(c);
  Data_Get_Struct(self, shoes_text, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

  ATTR_MARGINS(self_t->attr, 4);
  self_t->place.x = ATTR2(int, self_t->attr, left, canvas->cx) + lmargin;
  self_t->place.y = ATTR2(int, self_t->attr, top, canvas->cy) + tmargin;
  self_t->place.w = ATTR2(int, self_t->attr, width, canvas->place.w - (canvas->cx - self_t->place.x)) - (lmargin + rmargin);
  font = ATTR2(cstr, self_t->attr, font, "Arial 16px");

  if (self_t->layout != NULL)
    g_object_unref(self_t->layout);

  self_t->layout = pango_cairo_create_layout(canvas->cr);
  pd = 0;
  if (self_t->place.x == canvas->cx + lmargin)
  {
    if (self_t->place.x - canvas->place.x > (self_t->place.w - (lmargin + rmargin)) - 20)
    {
      self_t->place.x = canvas->place.x + lmargin;
      self_t->place.y = canvas->endy;
    } else {
      if (self_t->place.x > canvas->place.x) {
        pd = (self_t->place.x - (canvas->place.x + lmargin));
        pango_layout_set_indent(self_t->layout, pd * PANGO_SCALE);
        self_t->place.w = (canvas->place.w - (canvas->cx - self_t->place.x)) - rmargin;
      }
    }
    cairo_move_to(canvas->cr, canvas->place.x + lmargin, self_t->place.y);
  }
  else
    cairo_move_to(canvas->cr, self_t->place.x, self_t->place.y);
  if (canvas->fg.on)
  {
    cairo_set_source_rgba(canvas->cr, canvas->fg.r / 255., canvas->fg.g / 255., canvas->fg.b / 255., canvas->fg.a / 255.);
  // cairo_stroke(canvas->cr);
  }

  INFO("TEXT: %d, %d (%d, %d) / %d, %d / %d, %d [%d]\n", canvas->cx, canvas->cy,
    canvas->place.w, canvas->height, self_t->place.x, self_t->place.y, self_t->place.w, self_t->place.h, pd);
  pango_layout_set_markup(self_t->layout, RSTRING_PTR(self_t->markup), -1);
  pango_layout_set_width(self_t->layout, self_t->place.w * PANGO_SCALE);
  desc = pango_font_description_from_string(font);
  pango_layout_set_font_description(self_t->layout, desc);
  pango_font_description_free(desc);

  pango_cairo_update_layout(canvas->cr, self_t->layout);
  pango_cairo_show_layout(canvas->cr, self_t->layout);

  // draw the cursor
  if (!NIL_P(self_t->cursor))
  {
    int cursor = NUM2INT(self_t->cursor);
    PangoRectangle crect;
    double crx, cry;
    if (cursor < 0) cursor += RSTRING_LEN(self_t->markup) + 1;
    pango_layout_index_to_pos(self_t->layout, cursor, &crect);
    crx = self_t->place.x + (crect.x / PANGO_SCALE);
    cry = self_t->place.y + (crect.y / PANGO_SCALE);

    cairo_save(canvas->cr);
    cairo_new_path(canvas->cr);
    cairo_move_to(canvas->cr, crx, cry);
    cairo_line_to(canvas->cr, crx, cry + (crect.height / PANGO_SCALE));
    cairo_set_antialias(canvas->cr, CAIRO_ANTIALIAS_NONE);
    cairo_set_source_rgb(canvas->cr, 0., 0., 0.);
    cairo_set_line_width(canvas->cr, 0.8);
    cairo_stroke(canvas->cr);
    cairo_restore(canvas->cr);
  }

  li = pango_layout_get_line_count(self_t->layout) - 1;
  last = pango_layout_get_line(self_t->layout, li);
  pango_layout_line_get_pixel_extents(last, NULL, &lrect);
  pango_layout_get_pixel_size(self_t->layout, &px, &py);

  // newlines have an empty size
  canvas->endx += px + rmargin;
  canvas->endy += py + bmargin;
  if (ck == cStack) {
    canvas->cx = 0;
    canvas->cy = canvas->endy;
  } else {
    if (lrect.width == 0) {
      canvas->cx = (double)lrect.x;
    } else {
      canvas->cx = (double)((lrect.x + lrect.width) + rmargin);
    }
  }
  return self;
}

//
// Shoes::Button
//
static void
shoes_control_mark(shoes_control *control)
{
  rb_gc_mark_maybe(control->parent);
  rb_gc_mark_maybe(control->attr);
}

static void
shoes_control_free(shoes_control *control)
{
#ifdef SHOES_GTK
  gtk_widget_destroy(control->ref);
#endif
#ifdef SHOES_QUARTZ
  DisposeControl(control->ref);
#endif
  free(control);
}

VALUE
shoes_control_new(VALUE klass, VALUE attr, VALUE parent)
{
  shoes_control *control;
  VALUE obj = shoes_control_alloc(klass);
  Data_Get_Struct(obj, shoes_control, control);
  control->attr = attr;
  control->parent = parent;
  return obj;
}

VALUE
shoes_control_alloc(VALUE klass)
{
  shoes_control *control;
  VALUE obj = Data_Make_Struct(klass, shoes_control, shoes_control_mark, shoes_control_free, control);
  control->place.x = control->place.y = control->place.w = control->place.h = 0;
  control->ref = NULL;
  control->attr = Qnil;
  control->parent = Qnil;
  return obj;
}

VALUE
shoes_control_remove(VALUE self)
{
  shoes_control *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_control, self_t);
  shoes_canvas_remove_item(self_t->parent, self);

  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
#ifdef SHOES_GTK
  gtk_container_remove(GTK_CONTAINER(canvas->slot.canvas), self_t->ref);
#endif
#ifdef SHOES_QUARTZ
  HIViewRemoveFromSuperview(self_t->ref);
#endif
#ifdef SHOES_WIN32
  DestroyWindow(self_t->ref);
#endif
  return self;
}

void
shoes_control_send(VALUE self, ID event)
{
  VALUE click;
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);

  click = ATTR(self_t->attr, click);
  if (!NIL_P(click))
    shoes_safe_block(self_t->parent, click, rb_ary_new());
}

#ifdef SHOES_GTK
static gboolean
shoes_button_gtk_clicked(GtkButton *button, gpointer data)
{ 
  VALUE self = (VALUE)data;
  shoes_control_send(self, s_click);
  return TRUE;
}
#endif

VALUE
shoes_button_draw(VALUE self, VALUE c)
{
  SETUP_CONTROL(2);

  if (self_t->ref == NULL)
  {

#ifdef SHOES_GTK
    self_t->ref = gtk_button_new_with_label(_(msg));
    g_signal_connect(G_OBJECT(self_t->ref), "clicked",
                     G_CALLBACK(shoes_button_gtk_clicked),
                     (gpointer)self);
#endif

#ifdef SHOES_QUARTZ
    Rect r = {place.y, place.x, place.y + place.h, place.x + place.w};
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    CreatePushButtonControl(NULL, &r, cfmsg, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls);
    self_t->ref = CreateWindowEx(0, TEXT("BUTTON"), msg,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        place.x, place.y, place.w, place.h, canvas->slot.window, (HMENU)cid, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    shoes_win32_control_font(cid, canvas->slot.window);
    rb_ary_push(canvas->slot.controls, self);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_edit_line_get_text(VALUE self)
{
  VALUE text;
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);
  if (self_t->ref == NULL) text = Qnil;
#ifdef SHOES_GTK
  text = rb_str_new2(gtk_entry_get_text(GTK_ENTRY(self_t->ref)));
#endif
#ifdef SHOES_WIN32
  LONG i;
  TCHAR *buffer;
  i = (LONG)SendMessage(self_t->ref, WM_GETTEXTLENGTH, 0, 0) + 1;
  buffer = SHOE_ALLOC_N(TCHAR, i);
  SendMessage(self_t->ref, WM_GETTEXT, i, (LPARAM)buffer);
  text = rb_str_new2(buffer);
  SHOE_FREE(buffer);
#endif
#ifdef SHOES_QUARTZ
  CFStringRef controlText;
  Size* size = NULL;
  GetControlData(self_t->ref, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &controlText, size);
  text = shoes_cf2rb(controlText);
  CFRelease(controlText);
#endif
  return text;
}

VALUE
shoes_edit_line_set_text(VALUE self, VALUE text)
{
  char *msg = "";
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);
  if (!NIL_P(text)) msg = RSTRING_PTR(text);
#ifdef SHOES_GTK
  gtk_entry_set_text(GTK_ENTRY(self_t->ref), _(msg));
#endif
#ifdef SHOES_WIN32
  SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
#ifdef SHOES_QUARTZ
  CFStringRef controlText = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
  SetControlData(self_t->ref, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &controlText);
  CFRelease(controlText);
#endif
  return text;
}

VALUE
shoes_edit_line_draw(VALUE self, VALUE c)
{
  SETUP_CONTROL(0);

#ifdef SHOES_QUARTZ
  place.x += 4;
#endif
  if (self_t->ref == NULL)
  {
#ifdef SHOES_GTK
    self_t->ref = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(self_t->ref), _(msg));
#endif

#ifdef SHOES_QUARTZ
    Boolean nowrap = true;
    Rect r;
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, place.x, place.y, place.x + place.w, place.y + place.h);
    CreateEditUnicodeTextControl(NULL, &r, cfmsg, false, NULL, &self_t->ref);
    SetControlData(self_t->ref, kControlEntireControl, kControlEditTextSingleLineTag, sizeof(Boolean), &nowrap);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls);
    self_t->ref = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), NULL,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        place.x, place.y, place.w, place.h, canvas->slot.window, (HMENU)cid, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    shoes_win32_control_font(cid, canvas->slot.window);
    rb_ary_push(canvas->slot.controls, self);
    SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_edit_box_get_text(VALUE self)
{
  VALUE text;
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);
  if (self_t->ref == NULL) text = Qnil;
#ifdef SHOES_GTK
  GtkWidget *textview;
  GTK_CHILD(textview, self_t->ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  GtkTextIter begin, end;
  gtk_text_buffer_get_bounds(buffer, &begin, &end);
  text = rb_str_new2(gtk_text_buffer_get_text(buffer, &begin, &end, TRUE));
#endif
#ifdef SHOES_WIN32
  LONG i;
  TCHAR *buffer;
  i = (LONG)SendMessage(self_t->ref, WM_GETTEXTLENGTH, 0, 0) + 1;
  buffer = SHOE_ALLOC_N(TCHAR, i);
  SendMessage(self_t->ref, WM_GETTEXT, i, (LPARAM)buffer);
  text = rb_str_new2(buffer);
  SHOE_FREE(buffer);
#endif
#ifdef SHOES_QUARTZ
  CFStringRef controlText;
  Size* size = NULL;
  GetControlData(self_t->ref, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &controlText, size);
  text = shoes_cf2rb(controlText);
  CFRelease(controlText);
#endif
  return text;
}

VALUE
shoes_edit_box_set_text(VALUE self, VALUE text)
{
  char *msg = "";
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);
  if (!NIL_P(text)) msg = RSTRING_PTR(text);
#ifdef SHOES_GTK
  GtkWidget *textview;
  GTK_CHILD(textview, self_t->ref);
  GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
  gtk_text_buffer_set_text(buffer, _(msg), -1);
#endif
#ifdef SHOES_WIN32
  SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
#ifdef SHOES_QUARTZ
  CFStringRef controlText = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
  SetControlData(self_t->ref, kControlEditTextPart, kControlEditTextCFStringTag, sizeof (CFStringRef), &controlText);
  CFRelease(controlText);
#endif
  return text;
}


VALUE
shoes_edit_box_draw(VALUE self, VALUE c)
{
  SETUP_CONTROL(80);

  if (self_t->ref == NULL)
  {

#ifdef SHOES_GTK
    GtkTextBuffer *buffer;
    GtkWidget* textview = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, _(msg), -1);
    self_t->ref = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(self_t->ref),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(self_t->ref), GTK_SHADOW_IN);
    gtk_container_add(GTK_CONTAINER(self_t->ref), textview);
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, place.x, place.y, place.x + place.w, place.y + place.h);
    CreateEditUnicodeTextControl(NULL, &r, cfmsg, false, NULL, &self_t->ref);
    CFRelease(cfmsg);
#endif

#ifdef SHOES_WIN32
    self_t->ref = CreateWindowEx(0, TEXT("EDIT"), NULL,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | ES_LEFT,
        place.x, place.y, place.w, place.h, canvas->slot.window, NULL, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    SendMessage(self_t->ref, WM_SETTEXT, 0, (LPARAM)msg);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

#ifdef SHOES_GTK
static void
shoes_list_box_update(GtkWidget *combo, VALUE ary)
{
  long i;
  for (i = 0; i < RARRAY_LEN(ary); i++)
  {
    gtk_combo_box_append_text(GTK_COMBO_BOX(combo), _(RSTRING_PTR(rb_ary_entry(ary, i))));
  }
}
#endif

#ifdef SHOES_QUARTZ
static void
shoes_list_box_update(MenuRef menu, VALUE ary)
{
  long i;
  for (i = 0; i < RARRAY_LEN(ary); i++)
  {
    CFStringRef cf = shoes_rb2cf(rb_ary_entry(ary, i));
    AppendMenuItemTextWithCFString(menu, cf, 0, 0, NULL);
    CFRelease(cf);
  }
}
#endif

#ifdef SHOES_WIN32
static void
shoes_list_box_update(HWND box, VALUE ary)
{
  long i;
  for (i = 0; i < RARRAY_LEN(ary); i++)
  {
    SendMessage(box, CB_ADDSTRING, 0, (LPARAM)RSTRING_PTR(rb_ary_entry(ary, i)));
  }
}
#endif

VALUE
shoes_list_box_text(VALUE self)
{
  VALUE text = Qnil;
  shoes_control *self_t;
  Data_Get_Struct(self, shoes_control, self_t);
  if (self_t->ref == NULL) text = Qnil;
#ifdef SHOES_GTK
  int sel = gtk_combo_box_get_active(GTK_COMBO_BOX(self_t->ref));
  if (sel >= 0)
    text = rb_ary_entry(ATTR(self_t->attr, items), sel);
#endif

#ifdef SHOES_QUARTZ
  MenuRef menu;
  GetControlData(self_t->ref, 0, kControlPopupButtonMenuRefTag, sizeof(MenuRef), &menu, NULL);
  int selected = HIViewGetValue(self_t->ref);
  if (selected > 0)
  {
    CFStringRef label;
    CopyMenuItemTextAsCFString(menu, selected, &label);
    text = shoes_cf2rb(label);
    CFRelease(label);
  }
#endif

#ifdef SHOES_WIN32
  int sel = SendMessage(self_t->ref, CB_GETCURSEL, 0, 0);
  if (sel >= 0)
    text = rb_ary_entry(ATTR(self_t->attr, items), sel);
#endif
  return text;
}

VALUE
shoes_list_box_draw(VALUE self, VALUE c)
{
  SETUP_CONTROL(0);

  if (self_t->ref == NULL)
  {
#ifdef SHOES_GTK
    self_t->ref = gtk_combo_box_new_text();
    if (!NIL_P(ATTR(self_t->attr, items)))
      shoes_list_box_update(self_t->ref, ATTR(self_t->attr, items));
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    int menuId = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls);
    CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
    SetRect(&r, place.x, place.y, place.x + place.w, place.y + place.h);
    CreatePopupButtonControl(NULL, &r, cfmsg, -12345, false, 0, 0, 0, &self_t->ref);
    CFRelease(cfmsg);

    MenuRef menuRef;
    CreateNewMenu(menuId, kMenuAttrExcludesMarkColumn, &menuRef);
    if (!NIL_P(ATTR(self_t->attr, items)))
      shoes_list_box_update(menuRef, ATTR(self_t->attr, items));
    SetControlData(self_t->ref, 0, kControlPopupButtonMenuRefTag, sizeof(MenuRef), &menuRef);              
#endif

#ifdef SHOES_WIN32
    int cid = SHOES_CONTROL1 + RARRAY_LEN(canvas->slot.controls);
    self_t->ref = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("COMBOBOX"), NULL,
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | WS_BORDER | CBS_DROPDOWNLIST,
        place.x, place.y, place.w, place.h, canvas->slot.window, (HMENU)cid, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
    shoes_win32_control_font(cid, canvas->slot.window);
    if (!NIL_P(ATTR(self_t->attr, items)))
      shoes_list_box_update(self_t->ref, ATTR(self_t->attr, items));
    rb_ary_push(canvas->slot.controls, self);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

VALUE
shoes_progress_draw(VALUE self, VALUE c)
{
  SETUP_CONTROL(0);

  if (self_t->ref == NULL)
  {
#ifdef SHOES_GTK
    self_t->ref = gtk_progress_bar_new();
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(self_t->ref), _(msg));
#endif

#ifdef SHOES_QUARTZ
    Rect r;
    SetRect(&r, place.x, place.y, place.x + place.w, place.y + place.h);
    CreateProgressBarControl(NULL, &r, 0, 0, 100, false, &self_t->ref);
#endif

#ifdef SHOES_WIN32
    self_t->ref = CreateWindowEx(0, PROGRESS_CLASS, msg,
        WS_VISIBLE | WS_CHILD | PBS_SMOOTH,
        place.x, place.y, place.w, place.h, canvas->slot.window, NULL, 
        (HINSTANCE)GetWindowLong(canvas->slot.window, GWL_HINSTANCE),
        NULL);
#endif
    PLACE_CONTROL();
  }
  else
  {
    REPAINT_CONTROL();
  }

  FINISH();

  return self;
}

//
// Shoes::Anim
//
//
void
shoes_anim_call(VALUE self)
{
  shoes_anim *anim;
  Data_Get_Struct(self, shoes_anim, anim);
  shoes_safe_block(anim->parent, anim->block, rb_ary_new3(1, INT2NUM(anim->frame)));
  anim->frame++;
}

#ifdef SHOES_GTK
static gboolean
shoes_gtk_animate(gpointer data)
{
  VALUE anim = (VALUE)data;
  shoes_anim *self_t;
  Data_Get_Struct(anim, shoes_anim, self_t);
  shoes_anim_call(anim);
  return self_t->started;
}
#endif

#ifdef SHOES_QUARTZ
pascal void
shoes_quartz_animate(EventLoopTimerRef timer, void* userData)
{
  VALUE anim = (VALUE)userData;
  shoes_anim_call(anim);
}
#endif

static void
shoes_anim_mark(shoes_anim *anim)
{
  rb_gc_mark_maybe(anim->block);
  rb_gc_mark_maybe(anim->parent);
}

static void
shoes_anim_free(shoes_anim *anim)
{
  free(anim);
}

VALUE
shoes_anim_new(VALUE klass, VALUE fps, VALUE block, VALUE parent)
{
  shoes_anim *anim;
  VALUE obj = shoes_anim_alloc(klass);
  Data_Get_Struct(obj, shoes_anim, anim);
  anim->block = block;
  if (!NIL_P(fps))
    anim->fps = NUM2INT(fps);
  anim->parent = parent;
  return obj;
}

VALUE
shoes_anim_alloc(VALUE klass)
{
  shoes_anim *anim;
  VALUE obj = Data_Make_Struct(klass, shoes_anim, shoes_anim_mark, shoes_anim_free, anim);
  anim->block = Qnil;
  anim->fps = 12;
  anim->frame = 0;
  anim->parent = Qnil;
  anim->started = FALSE;
  return obj;
}

VALUE
shoes_anim_remove(VALUE self)
{
  shoes_anim *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_anim, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  self_t->started = FALSE;
#ifdef SHOES_WIN32
  long nid = rb_ary_index_of(canvas->app->timers, self);
  KillTimer(canvas->slot.window, SHOES_CONTROL1 + nid);
#endif
  shoes_canvas_remove_item(self_t->parent, self);
  return self;
}

VALUE
shoes_anim_draw(VALUE self, VALUE c)
{
  shoes_anim *self_t;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_anim, self_t);
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  if (!self_t->started)
  {
    unsigned int interval = 1000 / self_t->fps;
    if (interval < 32) interval = 32;
    self_t->frame = 0;
#ifdef SHOES_GTK
    g_timeout_add(interval, shoes_gtk_animate, (gpointer)self);
#endif
#ifdef SHOES_QUARTZ
  EventLoopTimerRef timer;
  InstallEventLoopTimer(GetMainEventLoop(), 0.0, interval * kEventDurationMillisecond,
      NewEventLoopTimerUPP(shoes_quartz_animate), self, &timer);
#endif
#ifdef SHOES_WIN32
    long nid = rb_ary_index_of(canvas->app->timers, self);
    SetTimer(canvas->slot.window, SHOES_CONTROL1 + nid, interval, NULL);
#endif
    self_t->started = TRUE;
  }
  return self;
}

static VALUE
shoes_debug(VALUE self, VALUE str)
{
#ifdef SHOES_WIN32
  odprintf("%s\n", RSTRING_PTR(str));
#else
  printf("%s\n", RSTRING_PTR(str));
#endif
  return Qnil;
}

static VALUE
shoes_p(VALUE self, VALUE obj)
{
  return shoes_debug(self, rb_inspect(obj));
}

#define C(n, s) \
  re##n = rb_eval_string(s); \
  rb_const_set(cShoes, rb_intern("" # n), re##n);

//
// Everything exposed to Ruby is exposed here.
//
void
shoes_ruby_init()
{
  char proc[SHOES_BUFSIZE];
  instance_eval_proc = rb_eval_string("lambda{|o,b| o.instance_eval(&b)}");
  rb_gc_register_address(&instance_eval_proc);
  sprintf(proc, 
    "proc do;"
      "e = @exc;"
      EXC_MARKUP
    "end"
  );
  exception_proc = rb_eval_string(proc);
  rb_gc_register_address(&exception_proc);
  exception_alert_proc = rb_eval_string(EXC_ALERT);
  rb_gc_register_address(&exception_alert_proc);
  s_aref = rb_intern("[]=");
  s_perc = rb_intern("%");
  s_bind = rb_intern("bind");
  s_new = rb_intern("new");
  s_run = rb_intern("run");
  s_to_s = rb_intern("to_s");
  s_arrow = rb_intern("arrow");
  s_call = rb_intern("call");
  s_center = rb_intern("center");
  s_change = rb_intern("change");
  s_click = rb_intern("click");
  s_corner = rb_intern("corner");
  s_draw = rb_intern("draw");
  s_font = rb_intern("font");
  s_hand = rb_intern("hand");
  s_hidden = rb_intern("hidden");
  s_insert = rb_intern("insert");
  s_items = rb_intern("items");
  s_match = rb_intern("match");
  s_text = rb_intern("text");
  s_top = rb_intern("top");
  s_right = rb_intern("right");
  s_bottom = rb_intern("bottom");
  s_left = rb_intern("left");
  s_height = rb_intern("height");
  s_remove = rb_intern("remove");
  s_width = rb_intern("width");
  s_margin = rb_intern("margin");
  s_margin_left = rb_intern("margin_left");
  s_margin_right = rb_intern("margin_right");
  s_margin_top = rb_intern("margin_top");
  s_margin_bottom = rb_intern("margin_bottom");
  s_radius = rb_intern("radius");

  cCanvas = rb_define_class("Canvas", rb_cObject);
  rb_define_alloc_func(cCanvas, shoes_canvas_alloc);

  cShoes = rb_define_class("Shoes", cCanvas);
  C(HEX_SOURCE, "/^(?:0x|#)?([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})$/i");
  C(HEX3_SOURCE, "/^(?:0x|#)?([0-9A-F])([0-9A-F])([0-9A-F])$/i");
  C(RGB_SOURCE, "/^rgb\\((\\d+), *(\\d+), *(\\d+)\\)$/i");
  C(RGBA_SOURCE, "/^rgb\\((\\d+), *(\\d+), *(\\d+), *(\\d+)\\)$/i");
  C(GRAY_SOURCE, "/^gray\\((\\d+)\\)$/i");
  C(GRAYA_SOURCE, "/^gray\\((\\d+), *(\\d+)\\)$/i");
  rb_eval_string(
    "def Shoes.escape(string);"
       "string.gsub(/&/n, '&amp;').gsub(/\\\"/n, '&quot;').gsub(/>/n, '&gt;').gsub(/</n, '&lt;');"
    "end"
  );

  rb_define_singleton_method(cShoes, "app", CASTHOOK(shoes_app_main), -1);
  rb_define_singleton_method(cShoes, "p", CASTHOOK(shoes_p), 1);
  rb_define_singleton_method(cShoes, "debug", CASTHOOK(shoes_debug), 1);

  rb_define_method(cCanvas, "width", CASTHOOK(shoes_canvas_get_width), 0);
  rb_define_method(cCanvas, "height", CASTHOOK(shoes_canvas_get_height), 0);
  rb_define_method(cCanvas, "nostroke", CASTHOOK(shoes_canvas_nostroke), 0);
  rb_define_method(cCanvas, "stroke", CASTHOOK(shoes_canvas_stroke), -1);
  rb_define_method(cCanvas, "strokewidth", CASTHOOK(shoes_canvas_strokewidth), 1);
  rb_define_method(cCanvas, "nofill", CASTHOOK(shoes_canvas_nofill), 0);
  rb_define_method(cCanvas, "fill", CASTHOOK(shoes_canvas_fill), -1);
  rb_define_method(cCanvas, "rgb", CASTHOOK(shoes_color_rgb), -1);
  rb_define_method(cCanvas, "gray", CASTHOOK(shoes_color_gray), -1);
  rb_define_method(cCanvas, "rect", CASTHOOK(shoes_canvas_rect), -1);
  rb_define_method(cCanvas, "oval", CASTHOOK(shoes_canvas_oval), -1);
  rb_define_method(cCanvas, "line", CASTHOOK(shoes_canvas_line), 4);
  rb_define_method(cCanvas, "arrow", CASTHOOK(shoes_canvas_arrow), 3);
  rb_define_method(cCanvas, "star", CASTHOOK(shoes_canvas_star), -1);
  rb_define_method(cCanvas, "text", CASTHOOK(shoes_canvas_markup), -1);
  rb_define_method(cCanvas, "background", CASTHOOK(shoes_canvas_background), -1);
  rb_define_method(cCanvas, "image", CASTHOOK(shoes_canvas_image), -1);
  rb_define_method(cCanvas, "imagesize", CASTHOOK(shoes_canvas_imagesize), 1);
  rb_define_method(cCanvas, "animate", CASTHOOK(shoes_canvas_animate), -1);
  rb_define_method(cCanvas, "path", CASTHOOK(shoes_canvas_path), -1);
  rb_define_method(cCanvas, "move_to", CASTHOOK(shoes_canvas_move_to), 2);
  rb_define_method(cCanvas, "line_to", CASTHOOK(shoes_canvas_line_to), 2);
  rb_define_method(cCanvas, "curve_to", CASTHOOK(shoes_canvas_curve_to), 6);
  rb_define_method(cCanvas, "transform", CASTHOOK(shoes_canvas_transform), 1);
  rb_define_method(cCanvas, "translate", CASTHOOK(shoes_canvas_translate), 2);
  rb_define_method(cCanvas, "rotate", CASTHOOK(shoes_canvas_rotate), 1);
  rb_define_method(cCanvas, "scale", CASTHOOK(shoes_canvas_scale), -1);
  rb_define_method(cCanvas, "skew", CASTHOOK(shoes_canvas_skew), -1);
  rb_define_method(cCanvas, "push", CASTHOOK(shoes_canvas_push), 0);
  rb_define_method(cCanvas, "pop", CASTHOOK(shoes_canvas_pop), 0);
  rb_define_method(cCanvas, "reset", CASTHOOK(shoes_canvas_reset), 0);
  rb_define_method(cCanvas, "button", CASTHOOK(shoes_canvas_button), -1);
  rb_define_method(cCanvas, "list_box", CASTHOOK(shoes_canvas_list_box), -1);
  rb_define_method(cCanvas, "edit_line", CASTHOOK(shoes_canvas_edit_line), -1);
  rb_define_method(cCanvas, "edit_box", CASTHOOK(shoes_canvas_edit_box), -1);
  rb_define_method(cCanvas, "progress", CASTHOOK(shoes_canvas_progress), -1);
  rb_define_method(cCanvas, "contents", CASTHOOK(shoes_canvas_contents), 0);
  rb_define_method(cCanvas, "draw", CASTHOOK(shoes_canvas_draw), 1);
  rb_define_method(cCanvas, "after", CASTHOOK(shoes_canvas_after), -1);
  rb_define_method(cCanvas, "before", CASTHOOK(shoes_canvas_before), -1);
  rb_define_method(cCanvas, "append", CASTHOOK(shoes_canvas_append), -1);
  rb_define_method(cCanvas, "prepend", CASTHOOK(shoes_canvas_prepend), -1);
  rb_define_method(cCanvas, "flow", CASTHOOK(shoes_canvas_flow), -1);
  rb_define_method(cCanvas, "stack", CASTHOOK(shoes_canvas_stack), -1);
  rb_define_method(cCanvas, "hide", CASTHOOK(shoes_canvas_hide), 0);
  rb_define_method(cCanvas, "show", CASTHOOK(shoes_canvas_show), 0);
  rb_define_method(cCanvas, "toggle", CASTHOOK(shoes_canvas_toggle), 0);
  rb_define_method(cCanvas, "click", CASTHOOK(shoes_canvas_click), -1);
  rb_define_method(cCanvas, "release", CASTHOOK(shoes_canvas_release), -1);
  rb_define_method(cCanvas, "motion", CASTHOOK(shoes_canvas_motion), -1);
  rb_define_method(cCanvas, "keypress", CASTHOOK(shoes_canvas_keypress), -1);
  rb_define_method(cCanvas, "quit", CASTHOOK(shoes_app_quit), 0);
  rb_define_method(cCanvas, "clear", CASTHOOK(shoes_canvas_clear_contents), -1);
  rb_define_method(cCanvas, "goto", CASTHOOK(shoes_canvas_goto), 1);
  rb_define_method(cCanvas, "remove", CASTHOOK(shoes_canvas_remove), 0);
  rb_define_method(cCanvas, "mouse", CASTHOOK(shoes_canvas_mouse), 0);

  cFlow    = rb_define_class_under(cShoes, "Flow", cCanvas);
  cStack   = rb_define_class_under(cShoes, "Stack", cCanvas);

  cPath    = rb_define_class_under(cShoes, "Path", rb_cObject);
  rb_define_alloc_func(cPath, shoes_path_alloc);
  rb_define_method(cPath, "draw", CASTHOOK(shoes_path_draw), 1);
  rb_define_method(cPath, "move", CASTHOOK(shoes_path_move), 2);
  rb_define_method(cPath, "remove", CASTHOOK(shoes_path_remove), 0);

  cImage    = rb_define_class_under(cShoes, "Image", rb_cObject);
  rb_define_alloc_func(cImage, shoes_image_alloc);
  rb_define_method(cImage, "draw", CASTHOOK(shoes_image_draw), 1);
  rb_define_method(cImage, "remove", CASTHOOK(shoes_image_remove), 0);
  cBackground = rb_define_class_under(cShoes, "Background", rb_cObject);
  rb_define_alloc_func(cBackground, shoes_image_alloc);
  rb_define_method(cBackground, "draw", CASTHOOK(shoes_background_draw), 1);
  rb_define_method(cBackground, "remove", CASTHOOK(shoes_pattern_remove), 0);
  cTextClass = rb_define_class_under(cShoes, "Text", rb_cObject);
  rb_define_alloc_func(cTextClass, shoes_text_alloc);
  rb_define_method(cTextClass, "draw", CASTHOOK(shoes_text_draw), 1);
  rb_define_method(cTextClass, "cursor=", CASTHOOK(shoes_text_set_cursor), 1);
  rb_define_method(cTextClass, "cursor", CASTHOOK(shoes_text_get_cursor), 0);
  rb_define_method(cTextClass, "remove", CASTHOOK(shoes_text_remove), 0);
  rb_define_method(cTextClass, "to_s", CASTHOOK(shoes_text_get_markup), 0);
  rb_define_method(cTextClass, "replace", CASTHOOK(shoes_text_set_markup), 1);

  cButton  = rb_define_class_under(cShoes, "Button", rb_cObject);
  rb_define_alloc_func(cButton, shoes_control_alloc);
  rb_define_method(cButton, "draw", CASTHOOK(shoes_button_draw), 1);
  rb_define_method(cButton, "remove", CASTHOOK(shoes_control_remove), 0);
  cEditLine  = rb_define_class_under(cShoes, "EditLine", rb_cObject);
  rb_define_alloc_func(cEditLine, shoes_control_alloc);
  rb_define_method(cEditLine, "text", CASTHOOK(shoes_edit_line_get_text), 0);
  rb_define_method(cEditLine, "text=", CASTHOOK(shoes_edit_line_set_text), 1);
  rb_define_method(cEditLine, "draw", CASTHOOK(shoes_edit_line_draw), 1);
  rb_define_method(cEditLine, "remove", CASTHOOK(shoes_control_remove), 0);
  cEditBox  = rb_define_class_under(cShoes, "EditBox", rb_cObject);
  rb_define_alloc_func(cEditBox, shoes_control_alloc);
  rb_define_method(cEditBox, "text", CASTHOOK(shoes_edit_box_get_text), 0);
  rb_define_method(cEditBox, "text=", CASTHOOK(shoes_edit_box_set_text), 1);
  rb_define_method(cEditBox, "draw", CASTHOOK(shoes_edit_box_draw), 1);
  rb_define_method(cEditBox, "remove", CASTHOOK(shoes_control_remove), 0);
  cListBox  = rb_define_class_under(cShoes, "ListBox", rb_cObject);
  rb_define_alloc_func(cListBox, shoes_control_alloc);
  rb_define_method(cListBox, "text", CASTHOOK(shoes_list_box_text), 0);
  rb_define_method(cListBox, "draw", CASTHOOK(shoes_list_box_draw), 1);
  rb_define_method(cListBox, "remove", CASTHOOK(shoes_control_remove), 0);
  cProgress  = rb_define_class_under(cShoes, "Progress", rb_cObject);
  rb_define_alloc_func(cProgress, shoes_control_alloc);
  rb_define_method(cProgress, "draw", CASTHOOK(shoes_progress_draw), 1);
  rb_define_method(cProgress, "remove", CASTHOOK(shoes_control_remove), 0);
  cAnim    = rb_define_class_under(cShoes, "Animation", rb_cObject);
  rb_define_alloc_func(cAnim, shoes_anim_alloc);
  rb_define_method(cAnim, "draw", CASTHOOK(shoes_anim_draw), 1);
  rb_define_method(cAnim, "remove", CASTHOOK(shoes_anim_remove), 0);

  cColor   = rb_define_class_under(cShoes, "Color", rb_cObject);
  rb_define_alloc_func(cColor, shoes_color_alloc);
  rb_define_singleton_method(cColor, "rgb", CASTHOOK(shoes_color_rgb), -1);
  rb_define_singleton_method(cColor, "gray", CASTHOOK(shoes_color_gray), -1);
  rb_define_singleton_method(cColor, "parse", CASTHOOK(shoes_color_parse), 1);
  rb_define_method(cColor, "to_s", CASTHOOK(shoes_color_to_s), 0);
  cLink    = rb_define_class_under(cShoes, "Link", rb_cObject);

  rb_define_method(rb_mKernel, "alert", CASTHOOK(shoes_dialog_alert), 1);
  rb_define_method(rb_mKernel, "ask", CASTHOOK(shoes_dialog_ask), 1);
  rb_define_method(rb_mKernel, "confirm", CASTHOOK(shoes_dialog_confirm), 1);
  rb_define_method(rb_mKernel, "ask_color", CASTHOOK(shoes_dialog_color), 1);
  rb_define_method(rb_mKernel, "ask_open_file", CASTHOOK(shoes_dialog_open), 0);
  rb_define_method(rb_mKernel, "ask_save_file", CASTHOOK(shoes_dialog_save), 0);
}
