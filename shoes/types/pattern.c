#include "shoes/types/color.h"
#include "shoes/types/pattern.h"

// ruby
VALUE cPattern, cBorder, cBackground;

CLASS_COMMON2(pattern);

FUNC_M("+background", background, -1);
FUNC_M("+border", border, -1);

void shoes_pattern_init() {
    cPattern = rb_define_class_under(cTypes, "Pattern", rb_cObject);

    rb_define_alloc_func(cPattern, shoes_pattern_alloc);

    rb_define_method(cPattern, "displace", CASTHOOK(shoes_pattern_displace), 2);
    rb_define_method(cPattern, "move", CASTHOOK(shoes_pattern_move), 2);
    rb_define_method(cPattern, "remove", CASTHOOK(shoes_basic_remove), 0);
    rb_define_method(cPattern, "to_pattern", CASTHOOK(shoes_pattern_self), 0);
    rb_define_method(cPattern, "style", CASTHOOK(shoes_pattern_style), -1);
    rb_define_method(cPattern, "fill", CASTHOOK(shoes_pattern_get_fill), 0);
    rb_define_method(cPattern, "fill=", CASTHOOK(shoes_pattern_set_fill), 1);
    rb_define_method(cPattern, "hide", CASTHOOK(shoes_pattern_hide), 0);
    rb_define_method(cPattern, "show", CASTHOOK(shoes_pattern_show), 0);
    rb_define_method(cPattern, "toggle", CASTHOOK(shoes_pattern_toggle), 0);

    cBackground = rb_define_class_under(cTypes, "Background", cPattern);
    rb_define_method(cBackground, "draw", CASTHOOK(shoes_background_draw), 2);
    cBorder = rb_define_class_under(cTypes, "Border", cPattern);
    rb_define_method(cBorder, "draw", CASTHOOK(shoes_border_draw), 2);

    rb_define_method(rb_mKernel, "pattern", CASTHOOK(shoes_pattern_method), 1);

    RUBY_M("+background", background, -1);
    RUBY_M("+border", border, -1);
}

// ruby

void shoes_pattern_mark(shoes_pattern *pattern) {
    rb_gc_mark_maybe(pattern->source);
    rb_gc_mark_maybe(pattern->parent);
    rb_gc_mark_maybe(pattern->attr);
}

void shoes_pattern_free(shoes_pattern *pattern) {
    if (pattern->pattern != NULL)
        cairo_pattern_destroy(pattern->pattern);
    RUBY_CRITICAL(free(pattern));
}

void shoes_pattern_gradient(shoes_pattern *pattern, VALUE r1, VALUE r2, VALUE attr) {
    double angle = ATTR2(dbl, attr, angle, 0.);
    double rads = angle * SHOES_RAD2PI;
    double dx = sin(rads);
    double dy = cos(rads);
    double edge = (fabs(dx) + fabs(dy)) * 0.5;

    if (rb_obj_is_kind_of(r1, rb_cString))
        r1 = shoes_color_parse(cColor, r1);
    if (rb_obj_is_kind_of(r2, rb_cString))
        r2 = shoes_color_parse(cColor, r2);

    VALUE radius = ATTR(attr, radius);
    if (!NIL_P(radius)) {
        double r = 0.001;
        if (rb_obj_is_kind_of(r, rb_cFloat)) r = NUM2DBL(radius);
        pattern->pattern = cairo_pattern_create_radial(0.5, 0.5, r, edge / 2., edge / 2., edge / 2.);
    } else {
        pattern->pattern = cairo_pattern_create_linear(0.5 + (-dx * edge), 0.5 + (-dy * edge),
                           0.5 + (dx * edge), 0.5 + (dy * edge));
    }
    shoes_color_grad_stop(pattern->pattern, 0.0, r1);
    shoes_color_grad_stop(pattern->pattern, 1.0, r2);
}

VALUE shoes_pattern_set_fill(VALUE self, VALUE source) {
    shoes_pattern *pattern;
    Data_Get_Struct(self, shoes_pattern, pattern);

    if (pattern->pattern != NULL)
        cairo_pattern_destroy(pattern->pattern);
    pattern->pattern = NULL;

    if (rb_obj_is_kind_of(source, rb_cRange)) {
        VALUE r1 = rb_funcall(source, s_begin, 0);
        VALUE r2 = rb_funcall(source, s_end, 0);
        shoes_pattern_gradient(pattern, r1, r2, pattern->attr);
    } else {
        if (!rb_obj_is_kind_of(source, cColor)) {
            VALUE rgb = shoes_color_parse(cColor, source);
            if (!NIL_P(rgb)) source = rgb;
        }

        if (rb_obj_is_kind_of(source, cColor)) {
            pattern->pattern = shoes_color_pattern(source);
        } else {
            pattern->cached = shoes_load_image(pattern->parent, source, 
                (shoes_cache_setting ? Qtrue: Qnil));
            if (pattern->cached != NULL && pattern->cached->pattern == NULL)
                pattern->cached->pattern = cairo_pattern_create_for_surface(pattern->cached->surface);
        }
        cairo_pattern_set_extend(PATTERN(pattern), CAIRO_EXTEND_REPEAT);
    }

    pattern->source = source;
    return source;
}

VALUE shoes_pattern_get_fill(VALUE self) {
    shoes_pattern *pattern;
    Data_Get_Struct(self, shoes_pattern, pattern);
    return pattern->source;
}

VALUE shoes_pattern_self(VALUE self) {
    return self;
}

VALUE shoes_pattern_args(int argc, VALUE *argv, VALUE self) {
    VALUE source, attr;
    rb_scan_args(argc, argv, "11", &source, &attr);
    CHECK_HASH(attr);
    return shoes_pattern_new(cPattern, source, attr, Qnil);
}

VALUE shoes_pattern_new(VALUE klass, VALUE source, VALUE attr, VALUE parent) {
    shoes_pattern *pattern;
    VALUE obj = shoes_pattern_alloc(klass);
    Data_Get_Struct(obj, shoes_pattern, pattern);
    pattern->source = Qnil;
    pattern->attr = attr;
    pattern->parent = parent;
    shoes_pattern_set_fill(obj, source);
    return obj;
}

VALUE shoes_pattern_method(VALUE klass, VALUE source) {
    return shoes_pattern_new(cPattern, source, Qnil, Qnil);
}

VALUE shoes_pattern_alloc(VALUE klass) {
    VALUE obj;
    shoes_pattern *pattern = SHOE_ALLOC(shoes_pattern);
    SHOE_MEMZERO(pattern, shoes_pattern, 1);
    obj = Data_Wrap_Struct(klass, shoes_pattern_mark, shoes_pattern_free, pattern);
    pattern->source = Qnil;
    pattern->attr = Qnil;
    pattern->parent = Qnil;
    return obj;
}

  
// This crawls up the parent tree at draw time to learn if there is
// an active scroll bar 'above'. Returns the width of the gutter
// TODO: Breaks manual (help.rb) so it's not used.
int is_slot_scrolled(shoes_canvas *canvas) {
   int gutterw = 0;
   shoes_canvas *cvs = canvas;
   gutterw = shoes_native_slot_gutter(cvs->slot);
   while (gutterw == 0 && (! NIL_P(cvs->parent)) ) {
     //fprintf(stderr, "Backgound Climb up\n");
     Data_Get_Struct(cvs->parent, shoes_canvas, cvs);
     gutterw = shoes_native_slot_gutter(cvs->slot);
   } 
   return gutterw;
}

// background is treated differently from other patterns.
VALUE shoes_background_draw(VALUE self, VALUE c, VALUE actual) {
    cairo_matrix_t matrix1, matrix2;
    double r = 0., sw = 1.;
    int expand = 0;
    SETUP_DRAWING(shoes_pattern, REL_TILE, PATTERN_DIM(self_t, width), PATTERN_DIM(self_t, height));
    r = ATTR2(dbl, self_t->attr, curve, 0.); 
    VALUE ev = shoes_hash_get(self_t->attr, s_scroll);
    if (!NIL_P(ev) && (ev == Qtrue))
      expand = 1;
    
    if (RTEST(actual)) {
        cairo_t *cr = CCR(canvas);
        cairo_save(cr);
        if (expand == 0) {
            cairo_translate(cr, place.ix + place.dx, place.iy + place.dy);
            PATTERN_SCALE(self_t, place, sw);
            cairo_new_path(cr);
            shoes_cairo_rect(cr, 0, 0, place.iw, place.ih, r);
       } else {
            // new option with 3.3.3
            int scrollwidth = is_slot_scrolled(canvas);
            int top = INT2NUM(canvas->slot->scrolly);
            //fprintf(stderr, "inside y: %d, iy: %d, dy: %d\n", place.y, place.iy, place.dy );
            //fprintf(stderr, "       h: %d, ih: %d, top: %d\n", place.h, place.ih, top);
            cairo_translate(cr, place.ix + place.dx - scrollwidth, place.iy + place.dy + top);
            PATTERN_SCALE(self_t, place, sw);
            cairo_new_path(cr);
            shoes_cairo_rect(cr, 0, 0 - top, place.iw, place.ih + top, r);
        }
        
        cairo_set_source(cr, PATTERN(self_t));
        cairo_fill(cr);
        cairo_restore(cr);
        PATTERN_RESET(self_t);
    }

    self_t->place = place;
    //INFO("BACKGROUND: (%d, %d), (%d, %d)\n", place.x, place.y, place.w, place.h);
    return self;
}

VALUE shoes_border_draw(VALUE self, VALUE c, VALUE actual) {
    cairo_matrix_t matrix1, matrix2;
    ID cap = s_rect;
    ID dash = s_nodot;
    double r = 0., sw = 1.;
    SETUP_DRAWING(shoes_pattern, REL_TILE, PATTERN_DIM(self_t, width), PATTERN_DIM(self_t, height));
    r = ATTR2(dbl, self_t->attr, curve, 0.);
    sw = ATTR2(dbl, self_t->attr, strokewidth, 1.);
    if (!NIL_P(ATTR(self_t->attr, cap))) cap = SYM2ID(ATTR(self_t->attr, cap));
    if (!NIL_P(ATTR(self_t->attr, dash))) dash = SYM2ID(ATTR(self_t->attr, dash));

    place.iw -= (int)round(sw);
    place.ih -= (int)round(sw);
    place.ix += (int)round(sw / 2.);
    place.iy += (int)round(sw / 2.);

    if (RTEST(actual)) {
        cairo_t *cr = CCR(canvas);
        cairo_save(cr);
        cairo_translate(cr, place.ix + place.dx, place.iy + place.dy);
        PATTERN_SCALE(self_t, place, sw);
        cairo_set_source(cr, PATTERN(self_t));
        cairo_new_path(cr);
        shoes_cairo_rect(cr, 0, 0, place.iw, place.ih, r);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
        CAP_SET(cr, cap);
        DASH_SET(cr, dash);
        cairo_set_line_width(cr, sw);
        cairo_stroke(cr);
        cairo_restore(cr);
        PATTERN_RESET(self_t);
    }

    self_t->place = place;
    INFO("BORDER: (%d, %d), (%d, %d)\n", place.x, place.y, place.w, place.h);
    return self;
}

VALUE shoes_subpattern_new(VALUE klass, VALUE pat, VALUE parent) {
    shoes_pattern *back, *pattern;
    VALUE obj = shoes_pattern_alloc(klass);
    Data_Get_Struct(obj, shoes_pattern, back);
    Data_Get_Struct(pat, shoes_pattern, pattern);
    back->source = pattern->source;
    back->cached = pattern->cached;
    back->pattern = pattern->pattern;
    if (back->pattern != NULL) cairo_pattern_reference(back->pattern);
    back->attr = pattern->attr;
    back->parent = parent;
    return obj;
}

// canvas
VALUE shoes_canvas_background(int argc, VALUE *argv, VALUE self) {
    VALUE pat;
    SETUP_CANVAS();

    if (argc == 1 && rb_obj_is_kind_of(argv[0], cPattern))
        pat = argv[0];
    else
        pat = shoes_pattern_args(argc, argv, self);

    if (!NIL_P(pat)) {
        pat = shoes_subpattern_new(cBackground, pat, self);
        shoes_add_ele(canvas, pat);
    }

    return pat;
}

VALUE shoes_canvas_border(int argc, VALUE *argv, VALUE self) {
    VALUE pat;
    SETUP_CANVAS();

    if (argc == 1 && rb_obj_is_kind_of(argv[0], cPattern))
        pat = argv[0];
    else
        pat = shoes_pattern_args(argc, argv, self);

    if (!NIL_P(pat)) {
        pat = shoes_subpattern_new(cBorder, pat, self);
        shoes_add_ele(canvas, pat);
    }

    return pat;
}
