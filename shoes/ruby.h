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
extern ID s_aref, s_new, s_run, s_to_s, s_arrow, s_call, s_center, s_change, s_click, s_corner, s_draw, s_font, s_hand, s_hidden, s_insert, s_items, s_match, s_text, s_top, s_right, s_bottom, s_left, s_height, s_width, s_margin, s_margin_left, s_margin_right, s_margin_top, s_margin_bottom;
extern VALUE instance_eval_proc, exception_proc, exception_alert_proc;

VALUE mfp_instance_eval(VALUE, VALUE);
long rb_ary_index_of(VALUE, VALUE);
VALUE rb_ary_insert_at(VALUE, long, int, VALUE);
VALUE shoes_safe_block(VALUE, VALUE, VALUE);
void shoes_ruby_init(void);

//
// Exception handling strings for eval
//
#define SHOES_META \
  "(class << Shoes; self; end).instance_eval do;"
#define EXC_ALERT \
  "proc do; alert %{#{@exc.message}\\n#{@exc.backtrace.map { |x| %{\\n  * #{x}}}}}; end"
#define EXC_MARKUP \
  "text %%{<span size='larger'>#{Shoes.escape(e.message)}</span>#{e.backtrace.map { |x| %%{\\n  * #{Shoes.escape(x)}} }}};"
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
#define ATTR(attr, n)                  shoes_hash_get(attr, s_##n)
#define PX(attr, n, dn, pn)            shoes_px(attr, s_##n, dn, pn)
#define PX2(attr, n1, n2, dn, dr, pn)  shoes_px2(attr, s_##n1, s_##n2, dn, dr, pn)
#define ATTR2(typ, attr, n, dn)        shoes_hash_##typ(attr, s_##n, dn)
#define ATTRSET(attr, k, v)            shoes_hash_set(attr, s_##k, v)
#define ATTR_MARGINS(attr, dm) \
  int margin = ATTR2(int, attr, margin, dm); \
  int lmargin = ATTR2(int, attr, margin_left, margin); \
  int rmargin = ATTR2(int, attr, margin_right, margin); \
  int tmargin = ATTR2(int, attr, margin_top, margin); \
  int bmargin = ATTR2(int, attr, margin_bottom, margin)

#define REL_WINDOW  1
#define REL_CANVAS  2
#define REL_CURSOR  3
#define REL_TILE    4

int shoes_px(VALUE, ID, int, int);
int shoes_px2(VALUE, ID, ID, int, int, int);
VALUE shoes_hash_set(VALUE, ID, VALUE);
VALUE shoes_hash_get(VALUE, ID);
int shoes_hash_int(VALUE, ID, int);
double shoes_hash_dbl(VALUE, ID, double);
char *shoes_hash_cstr(VALUE, ID, char *);
VALUE rb_str_to_pas(VALUE);
void shoes_place_decide(shoes_place *, VALUE, VALUE, int, int, char);
