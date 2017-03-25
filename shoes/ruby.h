//
// shoes/ruby.h
// Some defs for Ruby.
//
#include <ruby.h>
#include "shoes/canvas.h"

#if defined(__cplusplus)
extern "C" {
#if 0
} /* satisfy cc-mode */
#endif
#endif

#ifdef RUBY_RUBY_H
#define RUBY_CRITICAL(statements) do {statements;} while (0)
#else
#include <rubysig.h>
#include <st.h>
#ifdef RUBY_1_8
#include <node.h>
#endif
#endif

#if defined(__cplusplus)
#if 0
{
    /* satisfy cc-mode */
#endif
}  /* extern "C" { */
#endif

#ifndef SHOES_RUBY_H
#define SHOES_RUBY_H

#if _MSC_VER
typedef VALUE (*HOOK)(...);
typedef int (*FOREACH)(...);
#define CASTFOREACH(x) reinterpret_cast<FOREACH>(x)
#define CASTHOOK(x) reinterpret_cast<HOOK>(x)
#undef fopen
#undef fclose
#undef fread
#else
typedef VALUE (*HOOK)();
#define CASTHOOK(x) x
#define CASTFOREACH(x) x
#endif

#ifdef WORDS_BIGENDIAN
#define BE_CPU_N(x, n)
#define LE_CPU_N(x, n) flip_endian((unsigned char *)(x), n)
#else
#define BE_CPU_N(x, n) flip_endian((unsigned char *)(x), n)
#define LE_CPU_N(x, n)
#endif

#define BE_CPU(x) BE_CPU_N(&(x), sizeof(x))
#define LE_CPU(x) LE_CPU_N(&(x), sizeof(x))

static inline void flip_endian(unsigned char* x, int length) {
    int i;
    unsigned char tmp;

    for(i = 0; i < (length / 2); i++) {
        tmp = x[i];
        x[i] = x[length - i - 1];
        x[length - i - 1] = tmp;
    }
}

#ifndef RARRAY_LEN
#define RARRAY_LEN(arr)  RARRAY(arr)->len
#define RARRAY_PTR(arr)  RARRAY(arr)->ptr
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

#undef s_host

extern VALUE cShoes, cApp, cDialog, cTypes, cShoesWindow, cMouse, cCanvas;
extern VALUE cFlow, cStack, cMask, cImage;
extern VALUE cProgress;
extern VALUE ssNestSlot;
extern VALUE cWidget;
extern VALUE aMsgList;
extern VALUE eInvMode, eNotImpl, eImageError;
extern VALUE reHEX_SOURCE, reHEX3_SOURCE, reRGB_SOURCE, reRGBA_SOURCE, reGRAY_SOURCE, reGRAYA_SOURCE, reLF;
extern VALUE symAltQuest, symAltSlash, symAltDot, symAltEqual, symAltSemiColon;
extern VALUE instance_eval_proc;
extern ID s_perc, s_fraction, s_aref, s_mult, s_donekey, s_progress;

typedef struct {
    int n;
    VALUE a[10];
} rb_arg_list;

VALUE mfp_instance_eval(VALUE, VALUE);
VALUE ts_funcall2(VALUE, ID, int, VALUE *);
int rb_parse_args(int, const VALUE *, const char *, rb_arg_list *);
int rb_parse_args_allow(int, const VALUE *, const char *, rb_arg_list *);
long rb_ary_index_of(VALUE, VALUE);
VALUE rb_ary_insert_at(VALUE, long, int, VALUE);
VALUE shoes_safe_block(VALUE, VALUE, VALUE);
void shoes_ruby_init(void);
VALUE shoes_exit_setup(VALUE);
#define BEZIER 0.55228475;

VALUE call_cfunc(HOOK func, VALUE recv, int len, int argc, VALUE *argv);

//
// Exception handling strings for eval
//
#define SHOES_META \
  "(class << Shoes; self; end).instance_eval do;"

#if defined(SHOES_WIN32) && ! defined(SHOES_GTK_WIN32)
#define QUIT_ALERT_MSG() MessageBox(NULL, RSTRING_PTR(msg), "Shoes", MB_OK)
#else
#define QUIT_ALERT_MSG() printf("%s\n", RSTRING_PTR(msg))
#endif
#define QUIT_ALERT(v) \
   VALUE line_re = rb_eval_string("/:\\d+:\\s*/"); \
   VALUE msg = rb_funcall(v, rb_intern("message"), 0); \
   VALUE msg2 = rb_funcall(msg, rb_intern("split"), 2, line_re, INT2NUM(2)); \
   if (RARRAY_LEN(msg2) > 1) msg = rb_ary_entry(msg2, 1); \
   QUIT_ALERT_MSG(); \
   return SHOES_QUIT;

#define NUM2RGBINT(x) (rb_obj_is_kind_of(x, rb_cFloat) ? ROUND(NUM2DBL(x) * 255) : NUM2INT(x))

#define GET_STRUCT(ele, var) \
  shoes_##ele *var; \
  Data_Get_Struct(self, shoes_##ele, var)


//
// Common funcs for dealing with attribute hashes
//
#define CHECK_HASH(hsh)                if (TYPE(hsh) != T_HASH) { hsh = Qnil; }
#define ATTR(attr, n)                  shoes_hash_get(attr, s_##n)
#define PX(attr, n, dn, pn)            shoes_px(shoes_hash_get(attr, s_##n), dn, pn, 1)
#define PXN(attr, n, dn, pn)            shoes_px(shoes_hash_get(attr, s_##n), dn, pn, 0)
#define PX2(attr, n1, n2, dn, dr, pn)  shoes_px2(attr, s_##n1, s_##n2, dn, dr, pn)
#define ATTR2(typ, attr, n, dn)        shoes_hash_##typ(attr, s_##n, dn)
#define ATTRSET(attr, k, v)            attr = shoes_hash_set(attr, s_##k, v)
#define ATTR_MARGINS(attr, dm, canvas) \
  int lmargin, rmargin, tmargin, bmargin; \
  VALUE margino = ATTR(attr, margin); \
  if (rb_obj_is_kind_of(margino, rb_cArray)) \
  { \
    lmargin = shoes_px(rb_ary_entry(margino, 0), dm, CPW(canvas), 1); \
    tmargin = shoes_px(rb_ary_entry(margino, 1), dm, CPH(canvas), 1); \
    rmargin = shoes_px(rb_ary_entry(margino, 2), dm, CPW(canvas), 1); \
    bmargin = shoes_px(rb_ary_entry(margino, 3), dm, CPH(canvas), 1); \
  } \
  else \
  { \
    lmargin = rmargin = PX(attr, margin, dm, CPW(canvas)); \
    tmargin = bmargin = PX(attr, margin, dm, CPH(canvas)); \
  } \
  lmargin = PX(attr, margin_left, lmargin, CPW(canvas)); \
  rmargin = PX(attr, margin_right, rmargin, CPW(canvas)); \
  tmargin = PX(attr, margin_top, tmargin, CPH(canvas)); \
  bmargin = PX(attr, margin_bottom, bmargin, CPH(canvas))

#define CHECK_HOVER(self_t, h, touch) \
  if ((self_t->hover & HOVER_MOTION) != h && !NIL_P(self_t->attr)) \
  { \
    VALUE action = ID2SYM(h ? s_hover : s_leave); \
    VALUE proc = rb_hash_aref(self_t->attr, action); \
    if (!NIL_P(proc)) \
      shoes_safe_block(self, proc, rb_ary_new3(1, self)); \
    if (touch != NULL) *touch += 1; \
    self_t->hover = (self_t->hover & HOVER_CLICK) | h; \
  }

#define IS_INSIDE(self_t, x, y) \
  (self_t->place.iw > 0 && self_t->place.ih > 0 && \
   x >= self_t->place.ix + self_t->place.dx && \
   x <= self_t->place.ix + self_t->place.dx + self_t->place.iw && \
   y >= self_t->place.iy + self_t->place.dy && \
   y <= self_t->place.iy + self_t->place.dy + self_t->place.ih)

#define RUBY_M(name, func, argc) \
  rb_define_method(cCanvas, name + 1, CASTHOOK(shoes_canvas_c_##func), -1); \
  rb_define_method(cApp, name + 1, CASTHOOK(shoes_app_c_##func), -1)

//
// Defines a redirecting function which applies the element or transformation
// to the currently active canvas.  This is used in place of the old instance_eval
// and ensures that you have access to the App's instance variables while
// assembling elements in a layout.
//
#define FUNC_M(name, func, argn) \
  VALUE \
  shoes_canvas_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas, obj; \
    GET_STRUCT(canvas, self_t); \
    char *n = name; \
    if (rb_ary_entry(self_t->app->nesting, 0) == self || \
         ((rb_obj_is_kind_of(self, cWidget) || self == self_t->app->nestslot) && \
          RARRAY_LEN(self_t->app->nesting) > 0)) \
      canvas = rb_ary_entry(self_t->app->nesting, RARRAY_LEN(self_t->app->nesting) - 1); \
    else \
      canvas = self; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    obj = call_cfunc(CASTHOOK(shoes_canvas_##func), canvas, argn, argc, argv); \
    if (n[0] == '+' && RARRAY_LEN(self_t->app->nesting) == 0) shoes_canvas_repaint_all(self); \
    return obj; \
  } \
  VALUE \
  shoes_app_c_##func(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE canvas; \
    char *n = name; \
    GET_STRUCT(app, app); \
    if (RARRAY_LEN(app->nesting) > 0) \
      canvas = rb_ary_entry(app->nesting, RARRAY_LEN(app->nesting) - 1); \
    else \
      canvas = app->canvas; \
    if (!rb_obj_is_kind_of(canvas, cCanvas)) \
      return ts_funcall2(canvas, rb_intern(n + 1), argc, argv); \
    return shoes_canvas_c_##func(argc, argv, canvas); \
  }

#define SETUP_CONTROL(dh, dw, flex) \
  char *msg = ""; \
  int len = dw ? dw : 200; \
  shoes_control *self_t; \
  shoes_canvas *canvas; \
  shoes_place place; \
  VALUE text = Qnil, ck = rb_obj_class(c); \
  Data_Get_Struct(self, shoes_control, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  text = ATTR(self_t->attr, text); \
  if (!NIL_P(text)) { \
    text = shoes_native_to_s(text); \
    msg = RSTRING_PTR(text); \
    if (flex) len = ((int)RSTRING_LEN(text) * 8) + 32; \
  } \
  shoes_place_decide(&place, c, self_t->attr, len, 28 + dh, REL_CANVAS, TRUE)

#define FINISH() \
  if (!ABSY(place)) { \
    canvas->cx += place.w; \
    canvas->cy = place.y; \
    canvas->endx = canvas->cx; \
    canvas->endy = max(canvas->endy, place.y + place.h); \
  } \
  if (ck == cStack) { \
    canvas->cx = CPX(canvas); \
    canvas->cy = canvas->endy; \
  }

//
// Macros for setting up drawing
//
#define SETUP_DRAWING(self_type, rel, dw, dh) \
  self_type *self_t; \
  shoes_place place; \
  shoes_canvas *canvas; \
  Data_Get_Struct(self, self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; \
  shoes_place_decide(&place, c, self_t->attr, dw, dh, rel, REL_COORDS(rel) == REL_CANVAS)

#define EVENT_COMMON(ele, est, sym) \
  VALUE \
  shoes_##ele##_##sym(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE str = Qnil, blk = Qnil; \
    GET_STRUCT(est, self_t); \
  \
    rb_scan_args(argc, argv, "01&", &str, &blk); \
    if (NIL_P(self_t->attr)) self_t->attr = rb_hash_new(); \
    rb_hash_aset(self_t->attr, ID2SYM(s_##sym), NIL_P(blk) ? str : blk ); \
    return self; \
  }

//
// Common methods
//

#define CLASS_COMMON(ele) \
  VALUE \
  shoes_##ele##_style(int argc, VALUE *argv, VALUE self) \
  { \
    rb_arg_list args; \
    GET_STRUCT(ele, self_t); \
    switch (rb_parse_args(argc, argv, "h,", &args)) { \
      case 1: \
        if (NIL_P(self_t->attr)) self_t->attr = rb_hash_new(); \
        rb_funcall(self_t->attr, s_update, 1, args.a[0]); \
        shoes_canvas_repaint_all(self_t->parent); \
      break; \
      case 2: return rb_obj_freeze(rb_obj_dup(self_t->attr)); \
    } \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_displace(VALUE self, VALUE x, VALUE y) \
  { \
    GET_STRUCT(ele, self_t); \
    ATTRSET(self_t->attr, displace_left, x); \
    ATTRSET(self_t->attr, displace_top, y); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_move(VALUE self, VALUE x, VALUE y) \
  { \
    GET_STRUCT(ele, self_t); \
    ATTRSET(self_t->attr, left, x); \
    ATTRSET(self_t->attr, top, y); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

#define CLASS_COMMON2(ele) \
  VALUE \
  shoes_##ele##_hide(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    ATTRSET(self_t->attr, hidden, Qtrue); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_show(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    ATTRSET(self_t->attr, hidden, Qfalse); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_toggle(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    ATTRSET(self_t->attr, hidden, ATTR(self_t->attr, hidden) == Qtrue ? Qfalse : Qtrue); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  \
  VALUE \
  shoes_##ele##_is_hidden(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    if (RTEST(ATTR(self_t->attr, hidden))) \
      return ATTR(self_t->attr, hidden); \
    else return Qfalse; \
  } \
  CLASS_COMMON(ele); \
  EVENT_COMMON(ele, ele, change); \
  EVENT_COMMON(ele, ele, click); \
  EVENT_COMMON(ele, ele, release); \
  EVENT_COMMON(ele, ele, hover); \
  EVENT_COMMON(ele, ele, leave);
  
#define PLACE_COMMON(ele) \
  VALUE \
  shoes_##ele##_get_parent(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    return self_t->parent; \
  } \
  \
  VALUE \
  shoes_##ele##_get_left(VALUE self) \
  { \
    shoes_canvas *canvas = NULL; \
    GET_STRUCT(ele, self_t); \
    if (!NIL_P(self_t->parent)) { \
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas); \
    } else { \
      Data_Get_Struct(self, shoes_canvas, canvas); \
    } \
    return INT2NUM(self_t->place.x - CPX(canvas)); \
  } \
  \
  VALUE \
  shoes_##ele##_get_top(VALUE self) \
  { \
    shoes_canvas *canvas = NULL; \
    GET_STRUCT(ele, self_t); \
    if (!NIL_P(self_t->parent)) { \
      Data_Get_Struct(self_t->parent, shoes_canvas, canvas); \
    } else { \
      Data_Get_Struct(self, shoes_canvas, canvas); \
    } \
    return INT2NUM(self_t->place.y - CPY(canvas)); \
  } \
  \
  VALUE \
  shoes_##ele##_get_height(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    return INT2NUM(self_t->place.h); \
  } \
  \
  VALUE \
  shoes_##ele##_get_width(VALUE self) \
  { \
    GET_STRUCT(ele, self_t); \
    return INT2NUM(self_t->place.w); \
  }
  
#define REPLACE_COMMON(ele) \
  VALUE \
  shoes_##ele##_replace(int argc, VALUE *argv, VALUE self) \
  { \
    long i; \
    shoes_textblock *block_t; \
    VALUE texts, attr, block; \
    GET_STRUCT(ele, self_t); \
    attr = Qnil; \
    texts = rb_ary_new(); \
    for (i = 0; i < argc; i++) \
    { \
      if (rb_obj_is_kind_of(argv[i], rb_cHash)) \
        attr = argv[i]; \
      else \
        rb_ary_push(texts, argv[i]); \
    } \
    self_t->texts = texts; \
    if (!NIL_P(attr)) self_t->attr = attr; \
    block = shoes_find_textblock(self); \
    Data_Get_Struct(block, shoes_textblock, block_t); \
    shoes_textblock_uncache(block_t, TRUE); \
    shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }
  
//
// Transformations
//
#define TRANS_COMMON(ele, repaint) \
  VALUE \
  shoes_##ele##_transform(VALUE self, VALUE _m) \
  { \
    GET_STRUCT(ele, self_t); \
    ID m = SYM2ID(_m); \
    if (m == s_center || m == s_corner) \
    { \
      self_t->st = shoes_transform_detach(self_t->st); \
      self_t->st->mode = m; \
    } \
    else \
    { \
      rb_raise(rb_eArgError, "transform must be called with either :center or :corner."); \
    } \
    return self; \
  } \
  VALUE \
  shoes_##ele##_translate(VALUE self, VALUE _x, VALUE _y) \
  { \
    double x, y; \
    GET_STRUCT(ele, self_t); \
    x = NUM2DBL(_x); \
    y = NUM2DBL(_y); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_translate(&self_t->st->tf, x, y); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_rotate(VALUE self, VALUE _deg) \
  { \
    double rad; \
    GET_STRUCT(ele, self_t); \
    rad = NUM2DBL(_deg) * SHOES_RAD2PI; \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_rotate(&self_t->st->tf, -rad); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_scale(int argc, VALUE *argv, VALUE self) \
  { \
    VALUE _sx, _sy; \
    double sx, sy; \
    GET_STRUCT(ele, self_t); \
    rb_scan_args(argc, argv, "11", &_sx, &_sy); \
    sx = NUM2DBL(_sx); \
    if (NIL_P(_sy)) sy = sx; \
    else            sy = NUM2DBL(_sy); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_scale(&self_t->st->tf, sx, sy); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  } \
  VALUE \
  shoes_##ele##_skew(int argc, VALUE *argv, VALUE self) \
  { \
    cairo_matrix_t matrix; \
    VALUE _sx, _sy; \
    double sx, sy; \
    GET_STRUCT(ele, self_t); \
    rb_scan_args(argc, argv, "11", &_sx, &_sy); \
    sx = NUM2DBL(_sx) * SHOES_RAD2PI; \
    sy = 0.0; \
    if (!NIL_P(_sy)) sy = NUM2DBL(_sy) * SHOES_RAD2PI; \
    cairo_matrix_init(&matrix, 1.0, sy, sx, 1.0, 0.0, 0.0); \
    self_t->st = shoes_transform_detach(self_t->st); \
    cairo_matrix_multiply(&self_t->st->tf, &self_t->st->tf, &matrix); \
    if (repaint) shoes_canvas_repaint_all(self_t->parent); \
    return self; \
  }

int shoes_px(VALUE, int, int, int);
int shoes_px2(VALUE, ID, ID, int, int, int);
VALUE shoes_hash_set(VALUE, ID, VALUE);
VALUE shoes_hash_get(VALUE, ID);
int shoes_hash_int(VALUE, ID, int);
double shoes_hash_dbl(VALUE, ID, double);
char *shoes_hash_cstr(VALUE, ID, char *);
VALUE rb_str_to_pas(VALUE);
void shoes_place_exact(shoes_place *, VALUE, int, int);
void shoes_place_decide(shoes_place *, VALUE, VALUE, int, int, unsigned char, int);
unsigned char shoes_is_element(VALUE);
unsigned char shoes_is_any(VALUE);
void shoes_extras_remove_all(shoes_canvas *);
void shoes_ele_remove_all(VALUE);
void shoes_cairo_rect(cairo_t *, double, double, double, double, double);
void shoes_cairo_arc(cairo_t *, double, double, double, double, double, double);

#define SYMBOL_DEFS(f) f(bind); f(gsub); f(keys); f(update); f(merge); f(new); f(URI); f(now); f(debug); f(info); f(warn); f(error); f(run); f(to_a); f(to_ary); f(to_f); f(to_i); f(to_int); f(to_s); f(to_str); f(align); f(angle); f(angle1); f(angle2); f(arrow); f(autoplay); f(begin); f(body); f(cancel); f(call); f(center); f(change); f(choose); f(click); f(corner); f(curve); f(distance); f(displace_left); f(displace_top); f(downcase); f(draw); f(emphasis); f(end); f(family); f(fill); f(finish); f(font); f(fraction); f(fullscreen); f(group); f(hand); f(headers); f(hidden); f(host); f(hover); f(href); f(insert); f(inner); f(items); f(justify); f(kerning); f(keydown); f(keypress); f(keyup); f(match); f(method); f(motion); f(leading); f(leave); f(ok); f(outer); f(path); f(points); f(port); f(redirect); f(release); f(request_uri); f(rise); f(scheme); f(save); f(size); f(state); f(wheel); f(scroll); f(stretch); f(strikecolor); f(strikethrough); f(stroke); f(start); f(attach); f(title); f(top); f(right); f(bottom); f(left); f(up); f(down); f(height); f(minheight); f(remove); f(resizable); f(strokewidth); f(cap); f(widget); f(width); f(minwidth); f(marker); f(margin); f(margin_left); f(margin_right); f(margin_top); f(margin_bottom); f(radius); f(secret); f(blur); f(glow); f(shadow); f(arc); f(rect); f(oval); f(line); f(star); f(project); f(round); f(square); f(undercolor); f(underline); f(variant); f(weight); f(wrap); f(dash); f(nodot); f(onedot); f(donekey); f(volume); f(bg_color); f(decorated); f(opacity)
#define SYMBOL_INTERN(name) s_##name = rb_intern("" # name)
#define SYMBOL_ID(name) ID s_##name
#define SYMBOL_EXTERN(name) extern ID s_##name

SYMBOL_DEFS(SYMBOL_EXTERN);

// TODO: temporary extern until refactoring proper component, i.e. text should move with TextEditBox in native/gtk
SYMBOL_EXTERN(text);
SYMBOL_EXTERN(link);

#define CANVAS_DEFS(f) \
  f(".close", close, 0); \
  f(".gutter", get_gutter_width, 0); \
  f(".nostroke", nostroke, 0); \
  f(".stroke", stroke, -1); \
  f(".strokewidth", strokewidth, 1); \
  f(".dash", dash, 1); \
  f(".cap", cap, 1); \
  f(".nofill", nofill, 0); \
  f(".fill", fill, -1); \
  f("+arc", arc, -1); \
  f("+rect", rect, -1); \
  f("+oval", oval, -1); \
  f("+line", line, -1); \
  f("+arrow", arrow, -1); \
  f("+star", star, -1); \
  f("+background", background, -1); \
  f("+border", border, -1); \
  f(".blur", blur, -1); \
  f(".glow", glow, -1); \
  f(".shadow", shadow, -1); \
  f("+image", image, -1); \
  f(".imagesize", imagesize, 1); \
  f(".move_to", move_to, 2); \
  f(".line_to", line_to, 2); \
  f(".curve_to", curve_to, 6); \
  f(".arc_to", arc_to, 6); \
  f(".transform", transform, 1); \
  f(".translate", translate, 2); \
  f(".rotate", rotate, 1); \
  f(".scale", scale, -1); \
  f(".skew", skew, -1); \
  f(".push", push, 0); \
  f(".pop", pop, 0); \
  f(".reset", reset, 0); \
  f(".app", get_app, 0); \
  f("+after", after, -1); \
  f("+before", before, -1); \
  f("+append", append, -1); \
  f("+prepend", prepend, -1); \
  f("+flow", flow, -1); \
  f("+stack", stack, -1); \
  f("+mask", mask, -1); \
  f("+widget", widget, -1); \
  f(".start", start, -1); \
  f(".finish", finish, -1); \
  f(".hover", hover, -1); \
  f(".leave", leave, -1); \
  f(".click", click, -1); \
  f(".release", release, -1); \
  f(".motion", motion, -1); \
  f(".keydown", keydown, -1); \
  f(".keypress", keypress, -1); \
  f(".keyup", keyup, -1); \
  f("+clear", clear_contents, -1); \
  f(".visit", goto, 1); \
  f(".mouse", mouse, 0); \
  f(".cursor", get_cursor, 0); \
  f(".cursor=", set_cursor, 1); \
  f(".clipboard", get_clipboard, 0); \
  f(".clipboard=", set_clipboard, 1); \
  f(".owner", owner, 0); \
  f(".window", window, -1); \
  f(".dialog", dialog, -1); \
  f(".window_plain", window_plain, 0); \
  f(".dialog_plain", dialog_plain, 0); \
  f("._snapshot", snapshot, -1); \

#endif
