//
// shoes/ruby.h
// Some defs for Ruby.
//
#include <ruby.h>

#if SHOES_WIN32
typedef VALUE (*HOOK)(...);
#define CASTHOOK(x) reinterpret_cast<HOOK>(x)
#else
#define CASTHOOK(x) x
#endif

#ifndef RARRAY_LEN
#define RARRAY_LEN(arr)  RARRAY(arr)->len
#define RSTRING_LEN(str) RSTRING(str)->len
#define RSTRING_PTR(str) RSTRING(str)->ptr
#endif

extern VALUE mShoes, cCanvas, cFlow, cStack, cPath, cImage, cBackground, cTextClass, cButton, cEditLine, cEditBox, cListBox, cProgress, cColor, cLink;
extern VALUE reRGB_SOURCE;
extern ID s_new, s_run, s_to_s, s_call, s_center, s_change, s_click, s_corner, s_draw, s_font, s_hidden, s_insert, s_match, s_x, s_y, s_height, s_width, s_margin, s_marginleft, s_marginright, s_margintop, s_marginbottom;
extern VALUE instance_eval_proc, exception_proc;

VALUE mfp_instance_eval(VALUE, VALUE);
void shoes_ruby_init(void);

//
// Exception handling strings for eval
//
#define SHOES_META \
  "(class << Shoes; self; end).instance_eval do;"
#define EXC_MARKUP \
  "markup %%{<span color='black'><span size='larger'>#{Shoes.escape(e.message)}</span>#{e.backtrace.map { |x| %%{\\n  * #{Shoes.escape(x)}} }}</span>};"
#define EXC_PROC \
  "proc do;" \
    EXC_MARKUP \
  "end;"
#define EXC_RUN \
  "define_method :run do |path|;" \
    EXC_PROC \
  "end;"

//
// Common funcs for dealing with attribute hashes
//
#define ATTR(n)           shoes_attr_find(s_##n, self_t->attr, Qnil)
#define ATTR2(n)          shoes_attr_find(s_##n, attr, self_t->attr)
#define ATTR_INT(n, dn)   shoes_attr_int(s_##n, attr, Qnil, dn)
#define ATTR2INT(n, dn)   shoes_attr_int(s_##n, attr, self_t->attr, dn)
#define ATTR2DBL(n, dn)   shoes_attr_dbl(s_##n, attr, self_t->attr, dn)
#define ATTR2CSTR(n, dn)  shoes_attr_cstr(s_##n, attr, self_t->attr, dn)
#define ATTRSET(k, v)     self_t->attr = shoes_attr_set(self_t->attr, s_##k, v)
#define ATTRSIZE(k, dv, p, m) \
  self_t->k = ATTR2INT(k, dv); \
  if (self_t->k < 0) self_t->k += p->k; \
  self_t->k -= m;
#define ATTRBASE() \
  ATTR_MARGINS(0); \
  ATTRSIZE(width, 1, parent, lmargin + rmargin); \
  if ((int)(parent->cx + self_t->width) > parent->width) { \
    parent->endx = parent->cx = self_t->endx = self_t->cx = parent->x; \
    parent->cy = self_t->cy = self_t->endy = parent->endy; \
  } \
  self_t->x = ATTR2DBL(x, parent->cx - parent->x) + parent->x + lmargin; \
  self_t->y = ATTR2DBL(y, parent->cy - parent->y) + parent->y + tmargin; \
  ATTRSIZE(width, parent->width - parent->cx, parent, lmargin + rmargin); \
  ATTRSIZE(height, parent->height, parent, tmargin + bmargin);

//
// Common properties
//
#define ATTR_MARGINS(dm) \
  int margin = ATTR2INT(margin, dm); \
  int lmargin = ATTR2INT(marginleft, margin); \
  int rmargin = ATTR2INT(marginright, margin); \
  int tmargin = ATTR2INT(margintop, margin); \
  int bmargin = ATTR2INT(marginbottom, margin)

VALUE shoes_attr_set(VALUE, ID, VALUE);
VALUE shoes_attr_find(ID, VALUE, VALUE);
int shoes_attr_int(ID, VALUE, VALUE, int);
double shoes_attr_dbl(ID, VALUE, VALUE, double);
char *shoes_attr_cstr(ID, VALUE, VALUE, char *);
VALUE rb_str_to_pas(VALUE);
