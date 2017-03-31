//
// shoes/ruby.c
// Just little bits of Ruby I've become accustomed to.
//
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/version.h"
#include "shoes/types/types.h"
#include <math.h>

VALUE cShoes, cApp, cDialog, cTypes, cShoesWindow, cMouse, cCanvas, cFlow, cStack, cMask, cWidget, cProgress, cColor, cResponse, ssNestSlot;
VALUE eImageError, eInvMode, eNotImpl;
VALUE reHEX_SOURCE, reHEX3_SOURCE, reRGB_SOURCE, reRGBA_SOURCE, reGRAY_SOURCE, reGRAYA_SOURCE, reLF;
VALUE symAltQuest, symAltSlash, symAltDot, symAltEqual, symAltSemiColon;
ID s_perc, s_fraction, s_aref, s_mult, s_donekey;

SYMBOL_DEFS(SYMBOL_ID);

//
// Mauricio's instance_eval hack (he bested my cloaker back in 06 Jun 2006)
//
VALUE instance_eval_proc;

VALUE mfp_instance_eval(VALUE obj, VALUE block) {
    return rb_funcall(instance_eval_proc, s_call, 2, obj, block);
}

//
// From Guy Decoux [ruby-talk:144098]
//
static VALUE ts_each(VALUE *tmp) {
    return rb_funcall2(tmp[0], (ID)tmp[1], (int)tmp[2], (VALUE *)tmp[3]);
}

VALUE ts_funcall2(VALUE obj, ID meth, int argc, VALUE *argv) {
    VALUE tmp[4];
    if (!rb_block_given_p())
        return rb_funcall2(obj, meth, argc, argv);
    tmp[0] = obj;
    tmp[1] = (VALUE)meth;
    tmp[2] = (VALUE)argc;
    tmp[3] = (VALUE)argv;
    return rb_iterate((VALUE(*)(VALUE))ts_each, (VALUE)tmp, CASTHOOK(rb_yield), 0);
}

#define SET_ARG(o) args->a[n] = o, n = n + 1
#define CHECK_ARG(mac, t, d, cond, condarg) \
{ \
  if ((!x || m) && n < argc) \
  { \
    if (mac(argv[n]) == t) \
      SET_ARG(argv[n]); \
    else if (cond) \
      SET_ARG(condarg); \
    else \
    { \
      if (m) m = 0; \
      x = 1; \
    } \
  } \
  else if (m) \
    SET_ARG(d); \
  else \
    x = 1; \
}
#define CHECK_ARG_TYPE(t, d) CHECK_ARG(TYPE, t, d, 0, Qnil)
#define CHECK_ARG_NOT_NIL()  CHECK_ARG(NIL_P, 0, Qnil, 0, Qnil)
#define CHECK_ARG_COERCE(t, meth) \
  CHECK_ARG(TYPE, t, Qnil, rb_respond_to(argv[n], s_##meth), rb_funcall(argv[n], s_##meth, 0))
#define CHECK_ARG_DATA(func) CHECK_ARG(TYPE, T_DATA, Qnil, func(argv[n]), argv[n])

//
// rb_parse_args
// - a rb_scan_args replacement, designed to assist in typecasting, since
//   use of RSTRING_* and RARRAY_* macros are so common in Shoes.
//
// returns 0 if no match.
// returns 1 and up, the arg list matched.
// (args.n is set to the arg count, args.a is the args)
//
static int rb_parse_args_p(unsigned char rais, int argc, const VALUE *argv, const char *fmt, rb_arg_list *args) {
    int i = 1, m = 0, n = 0, nmin = 0, x = 0;
    const char *p = fmt;
    args->n = 0;

    do {
        if (*p == ',') {
            if ((x && !m) || n < argc) {
                i++;
                x = 0;
                if (nmin == 0 || nmin > n) {
                    nmin = n;
                }
                n = 0;
            } else break;
        } else if (*p == '|') {
            if (!x) m = i;
        } else if (*p == 's') {
            CHECK_ARG_COERCE(T_STRING, to_str)
        } else if (*p == 'S') {
            CHECK_ARG_COERCE(T_STRING, to_s)
        } else if (*p == 'i') {
            CHECK_ARG_COERCE(T_FIXNUM, to_int)
        } else if (*p == 'I') {
            CHECK_ARG_COERCE(T_FIXNUM, to_i)
        } else if (*p == 'f') {
            CHECK_ARG_TYPE(T_FLOAT, Qnil)
        } else if (*p == 'F') {
            CHECK_ARG_COERCE(T_FLOAT, to_f)
        } else if (*p == 'a') {
            CHECK_ARG_COERCE(T_ARRAY, to_ary)
        } else if (*p == 'A') {
            CHECK_ARG_COERCE(T_ARRAY, to_a)
        } else if (*p == 'k') {
            CHECK_ARG_TYPE(T_CLASS, Qnil)
        } else if (*p == 'h') {
            CHECK_ARG_TYPE(T_HASH, Qnil)
        }  else if (*p == 'o') {
            CHECK_ARG_NOT_NIL()
        } else if (*p == '&') {
            if (rb_block_given_p())
                SET_ARG(rb_block_proc());
            else
                SET_ARG(Qnil);
        }

        //
        // shoes-specific structures
        //
        else if (*p == 'e') {
            CHECK_ARG_DATA(shoes_is_element)
        } else if (*p == 'E') {
            CHECK_ARG_DATA(shoes_is_any)
        } else break;
    } while (p++);

    if (!x && n >= argc)
        m = i;
    if (m)
        args->n = n;

    // printf("rb_parse_args(%s): %d %d (%d)\n", fmt, m, n, x);

    if (!m && rais) {
        if (argc < nmin)
            rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)", argc, nmin);
        else
            rb_raise(rb_eArgError, "bad arguments");
    }

    return m;
}

int rb_parse_args(int argc, const VALUE *argv, const char *fmt, rb_arg_list *args) {
    return rb_parse_args_p(1, argc, argv, fmt, args);
}

int rb_parse_args_allow(int argc, const VALUE *argv, const char *fmt, rb_arg_list *args) {
    return rb_parse_args_p(0, argc, argv, fmt, args);
}

long rb_ary_index_of(VALUE ary, VALUE val) {
    long i;

    for (i=0; i<RARRAY_LEN(ary); i++) {
        if (rb_equal(RARRAY_PTR(ary)[i], val))
            return i;
    }

    return -1;
}

VALUE rb_ary_insert_at(VALUE ary, long index, int len, VALUE ary2) {
    rb_funcall(ary, s_aref, 3, LONG2NUM(index), INT2NUM(len), ary2);
    return ary;
}

//
// from ruby's eval.c
//
inline VALUE call_cfunc(HOOK func, VALUE recv, int len, int argc, VALUE *argv) {
    if (len >= 0 && argc != len) {
        rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)",
                 argc, len);
    }

    switch (len) {
        case -2:
            return (*func)(recv, rb_ary_new4(argc, argv));
        case -1:
            return (*func)(argc, argv, recv);
        case 0:
            return (*func)(recv);
        case 1:
            return (*func)(recv, argv[0]);
        case 2:
            return (*func)(recv, argv[0], argv[1]);
        case 3:
            return (*func)(recv, argv[0], argv[1], argv[2]);
        case 4:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3]);
        case 5:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4]);
        case 6:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5]);
        case 7:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6]);
        case 8:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7]);
        case 9:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8]);
        case 10:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9]);
        case 11:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);
        case 12:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9],
                           argv[10], argv[11]);
        case 13:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
                           argv[11], argv[12]);
        case 14:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
                           argv[11], argv[12], argv[13]);
        case 15:
            return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
                           argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
                           argv[11], argv[12], argv[13], argv[14]);
        default:
            rb_raise(rb_eArgError, "too many arguments (%d)", len);
            break;
    }
    return Qnil;        /* not reached */
}

typedef struct {
    VALUE canvas;
    VALUE block;
    VALUE args;
} safe_block;

static VALUE shoes_safe_block_call(VALUE rb_sb) {
    int i;
    VALUE vargs[10];
    safe_block *sb = (safe_block *)rb_sb;
    for (i = 0; i < RARRAY_LEN(sb->args); i++)
        vargs[i] = rb_ary_entry(sb->args, i);
    return rb_funcall2(sb->block, s_call, (int)RARRAY_LEN(sb->args), vargs);
}

static VALUE shoes_safe_block_exception(VALUE rb_sb, VALUE e) {
    safe_block *sb = (safe_block *)rb_sb;
    shoes_canvas_error(sb->canvas, e);
    return Qnil;
}

VALUE shoes_safe_block(VALUE self, VALUE block, VALUE args) {
    safe_block sb;
    VALUE v;

    sb.canvas = shoes_find_canvas(self);
    sb.block = block;
    sb.args = args;
    rb_gc_register_address(&args);

    v = rb_rescue2(CASTHOOK(shoes_safe_block_call), (VALUE)&sb,
                   CASTHOOK(shoes_safe_block_exception), (VALUE)&sb, rb_cObject, 0);
    rb_gc_unregister_address(&args);
    return v;
}


/* get a dimension, in pixels, given a string, float, int or nil
**      "90%" or 0.9 or actual dimension as integer or
**      amount of pixel to substract from base to compute value
**      or nil
** int dv : default value
** int pv : a base dimension to process
** int nv : switch (boolean) to substract or not computed value from base dimension
*/

int shoes_px(VALUE obj, int dv, int pv, int nv) {
    int px;
    if (TYPE(obj) == T_STRING) {
        char *ptr = RSTRING_PTR(obj);
        int len = (int)RSTRING_LEN(obj);
        obj = rb_funcall(obj, s_to_i, 0);
        if (len > 1 && ptr[len - 1] == '%') {
            obj = rb_funcall(obj, s_mult, 1, rb_float_new(0.01));
        }
    }
    if (rb_obj_is_kind_of(obj, rb_cFloat)) {
        px = (int)((double)pv * NUM2DBL(obj));
    } else {
        if (NIL_P(obj))
            px = dv;
        else
            px = NUM2INT(obj);
        if (px < 0 && nv == 1)
            px += pv;
    }
    return px;
}

/* get a coordinate, in pixels, given bounds (left/right or top/bottom)
** int dv : default value
** int pv : a base dimension to process
** int dr : a delta to substract if working with :right or :bottom
*/

int shoes_px2(VALUE attr, ID k1, ID k2, int dv, int dr, int pv) {
    int px;
    VALUE obj = shoes_hash_get(attr, k2);
    if (!NIL_P(obj)) {
        px = shoes_px(obj, 0, pv, 0);
        px = (pv - dr) - px;
    } else {
        px = shoes_px(shoes_hash_get(attr, k1), dv, pv, 0);
    }
    return px;
}

VALUE shoes_hash_set(VALUE hsh, ID key, VALUE val) {
    if (NIL_P(hsh))
        hsh = rb_hash_new();
    rb_hash_aset(hsh, ID2SYM(key), val);
    return hsh;
}

VALUE shoes_hash_get(VALUE hsh, ID key) {
    VALUE v;

    if (TYPE(hsh) == T_HASH) {
        v = rb_hash_aref(hsh, ID2SYM(key));
        if (!NIL_P(v)) return v;
    }

    return Qnil;
}

int shoes_hash_int(VALUE hsh, ID key, int dn) {
    VALUE v = shoes_hash_get(hsh, key);
    if (!NIL_P(v)) return NUM2INT(v);
    return dn;
}

double shoes_hash_dbl(VALUE hsh, ID key, double dn) {
    VALUE v = shoes_hash_get(hsh, key);
    if (!NIL_P(v)) return NUM2DBL(v);
    return dn;
}

char * shoes_hash_cstr(VALUE hsh, ID key, char *dn) {
    VALUE v = shoes_hash_get(hsh, key);
    if (!NIL_P(v)) return RSTRING_PTR(v);
    return dn;
}

VALUE rb_str_to_pas(VALUE str) {
    VALUE str2;
    char slen[2];
    slen[0] = RSTRING_LEN(str);
    slen[1] = '\0';
    str2 = rb_str_new2(slen);
    rb_str_append(str2, str);
    return str2;
}

void shoes_place_exact(shoes_place *place, VALUE attr, int ox, int oy) {
    int r;
    VALUE x;
    place->dx = ATTR2(int, attr, displace_left, 0);
    place->dy = ATTR2(int, attr, displace_top, 0);
    place->flags = FLAG_ABSX | FLAG_ABSY;
    place->ix = place->x = ATTR2(int, attr, left, 0) + ox;
    place->iy = place->y = ATTR2(int, attr, top, 0) + oy;
    r = ATTR2(int, attr, radius, 0) * 2;
    place->iw = place->w = ATTR2(int, attr, width, r);
    place->ih = place->h = ATTR2(int, attr, height, place->w);
    x = ATTR(attr, right);
    if (!NIL_P(x)) place->iw = place->w = (NUM2INT(x) + ox) - place->x;
    x = ATTR(attr, bottom);
    if (!NIL_P(x)) place->ih = place->h = (NUM2INT(x) + oy) - place->y;

    if (RTEST(ATTR(attr, center))) {
        place->ix = place->x = place->x - (place->w / 2);
        place->iy = place->y = place->y - (place->h / 2);
    }
}

void shoes_place_decide(shoes_place *place, VALUE c, VALUE attr, int dw, int dh, unsigned char rel, int padded) {
    shoes_canvas *canvas = NULL;
    if (!NIL_P(c)) Data_Get_Struct(c, shoes_canvas, canvas);
    VALUE ck = rb_obj_class(c);
    VALUE stuck = ATTR(attr, attach);

    // for image : we want to scale the image, given only one attribute :width or :height
    // get dw and dh, set width or height
    if (REL_FLAGS(rel) & REL_SCALE) {   // 8
        VALUE rw = ATTR(attr, width), rh = ATTR(attr, height);

        if (NIL_P(rw) && !NIL_P(rh)) {          // we have height
            // fetch height in pixels whatever the input (string, float, positive/negative int)
            int spx = shoes_px(rh, dh, CPH(canvas), 1);
            // compute width with image aspect ratio [(dh == dw) means a square ]
            dw = (dh == dw) ? spx : ROUND(((dh * 1.) / dw) * spx);
            dh = spx;                           // now re-init 'dh' for next calculations
            ATTRSET(attr, width, INT2NUM(dw));  // set calculated width
        } else if (NIL_P(rh) && !NIL_P(rw)) {
            int spx = shoes_px(rw, dw, CPW(canvas), 1);
            dh = (dh == dw) ? spx : ROUND(((dh * 1.) / dw) * spx);
            dw = spx;
            ATTRSET(attr, height, INT2NUM(dh));
        }
    }

    ATTR_MARGINS(attr, 0, canvas);
    if (padded || dh == 0) dh += tmargin + bmargin;
    if (padded || dw == 0) dw += lmargin + rmargin;

    int testw = dw;
    if (testw == 0) testw = lmargin + 1 + rmargin;

    if (!NIL_P(stuck)) {
        if (stuck == cShoesWindow)
            rel = REL_FLAGS(rel) | REL_WINDOW;
        else if (stuck == cMouse)
            rel = REL_FLAGS(rel) | REL_CURSOR;
        else
            rel = REL_FLAGS(rel) | REL_STICKY;
    }

    place->flags = rel;
    place->dx = place->dy = 0;
    if (canvas == NULL) {
        place->ix = place->x = 0;
        place->iy = place->y = 0;
        place->iw = place->w = dw;
        place->ih = place->h = dh;
    } else {
        int cx, cy, ox, oy, tw = dw, th = dh;

        switch (REL_COORDS(rel)) {
            case REL_WINDOW:
                cx = 0;
                cy = 0;
                ox = 0;
                oy = canvas->slot->scrolly;
                break;

            case REL_CANVAS:
                cx = canvas->cx - CPX(canvas);
                cy = canvas->cy - CPY(canvas);
                ox = CPX(canvas);
                oy = CPY(canvas);
                break;

            case REL_CURSOR:
                cx = 0;
                cy = 0;
                ox = canvas->app->mousex;
                oy = canvas->app->mousey;
                break;

            case REL_TILE:
                cx = 0;
                cy = 0;
                ox = CPX(canvas);
                oy = CPY(canvas);
                testw = dw = CPW(canvas);
                dh = max(canvas->height, CPH(canvas));
                // Fix #2 ?
                //dh = (max(canvas->height, canvas->fully - CPB(canvas)) - CPY(canvas));
                break;

            default:
                cx = 0;
                cy = 0;
                ox = canvas->cx;
                oy = canvas->cy;
                if ((REL_COORDS(rel) & REL_STICKY) && shoes_is_element(stuck)) {
                    shoes_element *element;
                    Data_Get_Struct(stuck, shoes_element, element);
                    ox = element->place.x;
                    oy = element->place.y;
                }
                break;
        }

        place->w = PX(attr, width, testw, CPW(canvas));
        if (dw == 0 && place->w + (int)canvas->cx > canvas->place.iw) {
            canvas->cx = canvas->endx = CPX(canvas);
            canvas->cy = canvas->endy;
            place->w = canvas->place.iw;
        }
        place->h = PX(attr, height, dh, CPH(canvas));

        if (REL_COORDS(rel) != REL_TILE) {
            tw = place->w;
            th = place->h;
        }
        place->x = PX2(attr, left, right, cx, tw, canvas->place.iw) + ox;
        place->y = PX2(attr, top, bottom, cy, th,
                       ORIGIN(canvas->place) ? canvas->height : canvas->fully) + oy;
        if (!ORIGIN(canvas->place)) {
            place->dx = canvas->place.dx;
            place->dy = canvas->place.dy;
        }
        place->dx += PXN(attr, displace_left, 0, CPW(canvas));
        place->dy += PXN(attr, displace_top, 0, CPH(canvas));

        place->flags |= NIL_P(ATTR(attr, left)) && NIL_P(ATTR(attr, right)) ? 0 : FLAG_ABSX;
        place->flags |= NIL_P(ATTR(attr, top)) && NIL_P(ATTR(attr, bottom)) ? 0 : FLAG_ABSY;
        if (REL_COORDS(rel) != REL_TILE && ABSY(*place) == 0 && (ck == cStack || place->x + place->w > CPX(canvas) + canvas->place.iw)) {
            canvas->cx = place->x = CPX(canvas);
            canvas->cy = place->y = canvas->endy;
        }
    }
    place->ix = place->x + lmargin;
    place->iy = place->y + tmargin;
    place->iw = place->w - (lmargin + rmargin);
    //place->iw = (RTEST(ATTR(attr, width))) ? place->w : place->w - (lmargin + rmargin);
    if (place->iw < 0) place->iw = 0;
    place->ih = place->h - (tmargin + bmargin);
    //place->ih = (RTEST(ATTR(attr, height))) ? place->h : place->h - (tmargin + bmargin);
    if (place->ih < 0) place->ih = 0;

    INFO("PLACE: (%d, %d), (%d: %d, %d: %d) [%d, %d] %x\n", place->x, place->y, place->w, place->iw, place->h, place->ih, ABSX(*place), ABSY(*place), place->flags);

}

//
// shoes_basic routines
//
VALUE shoes_basic_remove(VALUE self) {
    GET_STRUCT(basic, self_t);
    shoes_canvas_remove_item(self_t->parent, self, 0, 0);
    shoes_canvas_repaint_all(self_t->parent);
    return self;
}

unsigned char shoes_is_element_p(VALUE ele, unsigned char any) {
    void *dmark;
    if (TYPE(ele) != T_DATA)
        return 0;
    dmark = RDATA(ele)->dmark;
    return (dmark == shoes_canvas_mark || dmark == shoes_shape_mark ||
            dmark == shoes_image_mark || dmark == shoes_effect_mark ||
            dmark == shoes_pattern_mark || dmark == shoes_textblock_mark ||
            dmark == shoes_control_mark ||
            (any && (dmark == shoes_http_mark || dmark == shoes_timer_mark ||
                     dmark == shoes_color_mark || dmark == shoes_link_mark ||
                     dmark == shoes_text_mark))
           );
}

unsigned char shoes_is_element(VALUE ele) {
    return shoes_is_element_p(ele, 0);
}

unsigned char shoes_is_any(VALUE ele) {
    return shoes_is_element_p(ele, 1);
}

void shoes_extras_remove_all(shoes_canvas *canvas) {
    int i;
    shoes_basic *basic;
    shoes_canvas *parent;
    if (canvas->app == NULL) return;
    for (i = (int)RARRAY_LEN(canvas->app->extras) - 1; i >= 0; i--) {
        VALUE ele = rb_ary_entry(canvas->app->extras, i);
        if (!NIL_P(ele)) {
            Data_Get_Struct(ele, shoes_basic, basic);
            Data_Get_Struct(basic->parent, shoes_canvas, parent);
            if (parent == canvas) {
                rb_funcall(ele, s_remove, 0);
                rb_ary_delete_at(canvas->app->extras, i);
            }
        }
    }
}

void shoes_ele_remove_all(VALUE contents) {
    if (!NIL_P(contents)) {
        long i;
        VALUE ary;
        ary = rb_ary_dup(contents);
        rb_gc_register_address(&ary);
        for (i = 0; i < RARRAY_LEN(ary); i++)
            if (!NIL_P(rb_ary_entry(ary, i)))
                rb_funcall(rb_ary_entry(ary, i), s_remove, 0);
        rb_gc_unregister_address(&ary);
        rb_ary_clear(contents);
    }
}

void shoes_cairo_arc(cairo_t *cr, double x, double y, double w, double h, double a1, double a2) {
    cairo_translate(cr, x, y);
    cairo_scale(cr, w / 2., h / 2.);
    cairo_arc(cr, 0., 0., 1., a1, a2);
}

void shoes_cairo_rect(cairo_t *cr, double x, double y, double w, double h, double r) {
    double rc = r * BEZIER;
    cairo_move_to(cr, x + r, y);
    cairo_rel_line_to(cr, w - 2 * r, 0.0);
    if (r != 0.) cairo_rel_curve_to(cr, rc, 0.0, r, rc, r, r);
    cairo_rel_line_to(cr, 0, h - 2 * r);
    if (r != 0.) cairo_rel_curve_to(cr, 0.0, rc, rc - r, r, -r, r);
    cairo_rel_line_to(cr, -w + 2 * r, 0);
    if (r != 0.) cairo_rel_curve_to(cr, -rc, 0, -r, -rc, -r, -r);
    cairo_rel_line_to(cr, 0, -h + 2 * r);
    if (r != 0.) cairo_rel_curve_to(cr, 0.0, -rc, r - rc, -r, r, -r);
    cairo_close_path(cr);
}

VALUE shoes_app_method_missing(int argc, VALUE *argv, VALUE self) {
    VALUE cname, canvas;
    GET_STRUCT(app, app);

    cname = argv[0];
    canvas = rb_ary_entry(app->nesting, RARRAY_LEN(app->nesting) - 1);
    if (!NIL_P(canvas) && rb_respond_to(canvas, SYM2ID(cname)))
        return ts_funcall2(canvas, SYM2ID(cname), argc - 1, argv + 1);
    return shoes_color_method_missing(argc, argv, self);
}

PLACE_COMMON(canvas)
TRANS_COMMON(canvas, 0);

void shoes_msg(ID typ, VALUE str) {
#ifndef RUBY_1_8
    ID func = rb_frame_this_func();
    rb_ary_push(shoes_world->msgs, rb_ary_new3(6,
                ID2SYM(typ), str, rb_funcall(rb_cTime, s_now, 0),
                func ? ID2SYM(func) : Qnil,
                rb_str_new2("<unknown>"), INT2NUM(0)));
#else
    ID func = rb_frame_last_func();
    rb_ary_push(shoes_world->msgs, rb_ary_new3(6,
                ID2SYM(typ), str, rb_funcall(rb_cTime, s_now, 0),
                func ? ID2SYM(func) : Qnil,
                rb_str_new2(ruby_sourcefile), INT2NUM(ruby_sourceline)));
#endif
}

#define DEBUG_TYPE(t) \
  VALUE \
  shoes_canvas_##t(VALUE self, VALUE str) \
  { \
    shoes_msg(s_##t, str); \
    return Qnil; \
  } \
  \
  void \
  shoes_##t(const char *fmt, ...) \
  { \
    va_list args; \
    char buf[BUFSIZ]; \
  \
    va_start(args,fmt); \
    vsnprintf(buf, BUFSIZ, fmt, args); \
    va_end(args); \
    shoes_msg(s_##t, rb_str_new2(buf)); \
  }

DEBUG_TYPE(info);
DEBUG_TYPE(debug);
DEBUG_TYPE(warn);
DEBUG_TYPE(error);

VALUE shoes_p(VALUE self, VALUE obj) {
    return shoes_canvas_debug(self, rb_inspect(obj));
}

VALUE shoes_log(VALUE self) {
    return shoes_world->msgs;
}

VALUE shoes_font(VALUE self, VALUE path) {
    StringValue(path);
    return shoes_load_font(RSTRING_PTR(path));
}

//
// See ruby.h for the complete list of App methods which redirect to Canvas.
//
CANVAS_DEFS(FUNC_M);

#define C(n, s) \
  re##n = rb_eval_string(s); \
  rb_const_set(cShoes, rb_intern("" # n), re##n);

VALUE progname;

//
// Everything exposed to Ruby is exposed here.
//
void shoes_ruby_init() {
    progname = rb_str_new2("(eval)");
    rb_define_variable("$0", &progname);
    rb_define_variable("$PROGRAM_NAME", &progname);

    instance_eval_proc = rb_eval_string("lambda{|o,b| o.instance_eval(&b)}");
    rb_gc_register_address(&instance_eval_proc);
    ssNestSlot = rb_eval_string("{:height => 1.0}");
    rb_gc_register_address(&ssNestSlot);

    s_aref = rb_intern("[]=");
    s_perc = rb_intern("%");
    s_mult = rb_intern("*");
    SYMBOL_DEFS(SYMBOL_INTERN);

    symAltQuest = ID2SYM(rb_intern("alt_?"));
    symAltSlash = ID2SYM(rb_intern("alt_/"));
    symAltEqual = ID2SYM(rb_intern("alt_="));
    symAltDot = ID2SYM(rb_intern("alt_."));
    symAltSemiColon = ID2SYM(rb_intern("alt_;"));

    //
    // I want all elements to be addressed Shoes::Name, but also available in
    // a separate mixin (cTypes), for inclusion in every Shoes.app block.
    //
    cTypes = rb_define_module("Shoes");
    rb_mod_remove_const(rb_cObject, rb_str_new2("Shoes"));

    cShoesWindow = rb_define_class_under(cTypes, "Window", rb_cObject);
    cMouse = rb_define_class_under(cTypes, "Mouse", rb_cObject);

    cCanvas = rb_define_class_under(cTypes, "Canvas", rb_cObject);
    rb_define_alloc_func(cCanvas, shoes_canvas_alloc);
    rb_define_method(cCanvas, "top", CASTHOOK(shoes_canvas_get_top), 0);
    rb_define_method(cCanvas, "left", CASTHOOK(shoes_canvas_get_left), 0);
    rb_define_method(cCanvas, "width", CASTHOOK(shoes_canvas_get_width), 0);
    rb_define_method(cCanvas, "height", CASTHOOK(shoes_canvas_get_height), 0);
    rb_define_method(cCanvas, "scroll_height", CASTHOOK(shoes_canvas_get_scroll_height), 0);
    rb_define_method(cCanvas, "scroll_max", CASTHOOK(shoes_canvas_get_scroll_max), 0);
    rb_define_method(cCanvas, "scroll_top", CASTHOOK(shoes_canvas_get_scroll_top), 0);
    rb_define_method(cCanvas, "scroll_top=", CASTHOOK(shoes_canvas_set_scroll_top), 1);
    rb_define_method(cCanvas, "displace", CASTHOOK(shoes_canvas_displace), 2);
    rb_define_method(cCanvas, "move", CASTHOOK(shoes_canvas_move), 2);
    rb_define_method(cCanvas, "style", CASTHOOK(shoes_canvas_style), -1);
    rb_define_method(cCanvas, "parent", CASTHOOK(shoes_canvas_get_parent), 0);
    rb_define_method(cCanvas, "contents", CASTHOOK(shoes_canvas_contents), 0);
    rb_define_method(cCanvas, "children", CASTHOOK(shoes_canvas_children), 0);
    rb_define_method(cCanvas, "draw", CASTHOOK(shoes_canvas_draw), 2);
    rb_define_method(cCanvas, "hide", CASTHOOK(shoes_canvas_hide), 0);
    rb_define_method(cCanvas, "show", CASTHOOK(shoes_canvas_show), 0);
    rb_define_method(cCanvas, "toggle", CASTHOOK(shoes_canvas_toggle), 0);
    rb_define_method(cCanvas, "remove", CASTHOOK(shoes_canvas_remove), 0);
    rb_define_method(cCanvas, "refresh_slot", CASTHOOK(shoes_canvas_refresh_slot), 0);
    rb_define_method(cCanvas, "refresh", CASTHOOK(shoes_canvas_refresh_slot), 0);
    rb_define_method(cCanvas, "cursor=", CASTHOOK(shoes_canvas_get_cursor), 0);
    rb_define_method(cCanvas, "cursor=", CASTHOOK(shoes_canvas_set_cursor), 1);

    cShoes = rb_define_class("Shoes", cCanvas);
    rb_include_module(cShoes, cTypes);
    rb_const_set(cShoes, rb_intern("Types"), cTypes);

    shoes_version_init();

    // other Shoes:: constants
    rb_const_set(cTypes, rb_intern("RAD2PI"), rb_float_new(SHOES_RAD2PI));
    rb_const_set(cTypes, rb_intern("TWO_PI"), rb_float_new(SHOES_PIM2));
    rb_const_set(cTypes, rb_intern("HALF_PI"), rb_float_new(SHOES_HALFPI));
    rb_const_set(cTypes, rb_intern("PI"), rb_float_new(SHOES_PI));

    cApp = rb_define_class_under(cTypes, "App", rb_cObject);
    rb_define_alloc_func(cApp, shoes_app_alloc);
    rb_define_method(cApp, "fullscreen", CASTHOOK(shoes_app_get_fullscreen), 0);
    rb_define_method(cApp, "fullscreen=", CASTHOOK(shoes_app_set_fullscreen), 1);
    rb_define_method(cApp, "name", CASTHOOK(shoes_app_get_title), 0);
    rb_define_method(cApp, "name=", CASTHOOK(shoes_app_set_title), 1);
    rb_define_method(cApp, "location", CASTHOOK(shoes_app_location), 0);
    rb_define_method(cApp, "started?", CASTHOOK(shoes_app_is_started), 0);
    rb_define_method(cApp, "width", CASTHOOK(shoes_app_get_width), 0);
    rb_define_method(cApp, "height", CASTHOOK(shoes_app_get_height), 0);
    rb_define_method(cApp, "slot", CASTHOOK(shoes_app_slot), 0);
    rb_define_method(cApp, "set_window_icon_path", CASTHOOK(shoes_app_set_icon), 1); // New in 3.2.19
    rb_define_method(cApp, "set_window_title", CASTHOOK(shoes_app_set_wtitle), 1); // New in 3.2.19
    rb_define_method(cApp, "opacity", CASTHOOK(shoes_app_get_opacity), 0);
    rb_define_method(cApp, "opacity=", CASTHOOK(shoes_app_set_opacity), 1);
    rb_define_method(cApp, "decorated", CASTHOOK(shoes_app_get_decoration), 0);
    rb_define_method(cApp, "decorated=", CASTHOOK(shoes_app_set_decoration), 1);
    rb_define_alias(cApp, "decorated?", "decorated");

    cDialog = rb_define_class_under(cTypes, "Dialog", cApp);

    eInvMode = rb_define_class_under(cTypes, "InvalidModeError", rb_eStandardError);
    eNotImpl = rb_define_class_under(cTypes, "NotImplementedError", rb_eStandardError);
    eImageError = rb_define_class_under(cTypes, "ImageError", rb_eStandardError);
    C(HEX_SOURCE, "/^(?:0x|#)?([0-9A-F]{2})([0-9A-F]{2})([0-9A-F]{2})$/i");
    C(HEX3_SOURCE, "/^(?:0x|#)?([0-9A-F])([0-9A-F])([0-9A-F])$/i");
    C(RGB_SOURCE, "/^rgb\\((\\d+), *(\\d+), *(\\d+)\\)$/i");
    C(RGBA_SOURCE, "/^rgb\\((\\d+), *(\\d+), *(\\d+), *(\\d+)\\)$/i");
    C(GRAY_SOURCE, "/^gray\\((\\d+)\\)$/i");
    C(GRAYA_SOURCE, "/^gray\\((\\d+), *(\\d+)\\)$/i");
    C(LF, "/\\r?\\n/");
    rb_eval_string(
        "def Shoes.escape(string);"
        "string.gsub(/&/n, '&amp;').gsub(/\\\"/n, '&quot;').gsub(/>/n, '&gt;').gsub(/</n, '&lt;');"
        "end"
    );

    rb_define_singleton_method(cShoes, "APPS", CASTHOOK(shoes_apps_get), 0);
    rb_define_singleton_method(cShoes, "app", CASTHOOK(shoes_app_main), -1);
    rb_define_singleton_method(cShoes, "p", CASTHOOK(shoes_p), 1);
    rb_define_singleton_method(cShoes, "log", CASTHOOK(shoes_log), 0);
    rb_define_singleton_method(cShoes, "show_console", CASTHOOK(shoes_app_console), 0); // New in 3.2.23
    rb_define_singleton_method(cShoes, "terminal", CASTHOOK(shoes_app_terminal), -1); // New in 3.3.2 replaces console
    rb_define_singleton_method(cShoes, "quit", CASTHOOK(shoes_app_quit), 0); // Shoes 3.3.2
    //rb_define_singleton_method(cShoes, "exit", CASTHOOK(shoes_app_quit), 0);

    //
    // Canvas methods
    // See ruby.h for the complete list of Canvas method signatures.
    // Macros are used to build App redirection methods, which should be
    // speedier than method_missing.
    //

    CANVAS_DEFS(RUBY_M);

    //
    // Shoes Kernel methods
    //
    //rb_define_method(rb_mKernel, "quit", CASTHOOK(shoes_app_quit), 0);
    //rb_define_method(rb_mKernel, "exit", CASTHOOK(shoes_app_quit), 0);
    rb_define_method(rb_mKernel, "secret_exit_hook", CASTHOOK(shoes_exit_setup),0); //unused?

    rb_define_method(rb_mKernel, "debug", CASTHOOK(shoes_canvas_debug), 1);
    rb_define_method(rb_mKernel, "info", CASTHOOK(shoes_canvas_info), 1);
    rb_define_method(rb_mKernel, "warn", CASTHOOK(shoes_canvas_warn), 1);
    rb_define_method(rb_mKernel, "error", CASTHOOK(shoes_canvas_error), 1);

    cFlow       = rb_define_class_under(cTypes, "Flow", cShoes);
    cStack      = rb_define_class_under(cTypes, "Stack", cShoes);
    cMask       = rb_define_class_under(cTypes, "Mask", cShoes);
    cWidget     = rb_define_class_under(cTypes, "Widget", cShoes);

    rb_define_method(cApp, "method_missing", CASTHOOK(shoes_app_method_missing), -1);

    rb_define_method(rb_mKernel, "alert", CASTHOOK(shoes_dialog_alert), -1);
    rb_define_method(rb_mKernel, "ask", CASTHOOK(shoes_dialog_ask), -1);
    rb_define_method(rb_mKernel, "confirm", CASTHOOK(shoes_dialog_confirm), -1);
    rb_define_method(rb_mKernel, "ask_color", CASTHOOK(shoes_dialog_color), 1);
    rb_define_method(rb_mKernel, "ask_open_file", CASTHOOK(shoes_dialog_open), -1);
    rb_define_method(rb_mKernel, "ask_save_file", CASTHOOK(shoes_dialog_save), -1);
    rb_define_method(rb_mKernel, "ask_open_folder", CASTHOOK(shoes_dialog_open_folder), -1);
    rb_define_method(rb_mKernel, "ask_save_folder", CASTHOOK(shoes_dialog_save_folder), -1);
    rb_define_method(rb_mKernel, "font", CASTHOOK(shoes_font), 1);
    
    SHOES_TYPES_INIT;
}

VALUE shoes_exit_setup(VALUE self) {
    rb_define_method(rb_mKernel, "quit", CASTHOOK(shoes_app_quit), 0);
    rb_define_method(rb_mKernel, "exit", CASTHOOK(shoes_app_quit), 0);
    return Qtrue;
}
