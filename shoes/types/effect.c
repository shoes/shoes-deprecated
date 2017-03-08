#include "shoes/types/effect.h"
#include "shoes/effects.h"

// ruby
VALUE cEffect;

void shoes_effect_init() {
    cEffect   = rb_define_class_under(cTypes, "Effect", rb_cObject);
    rb_define_alloc_func(cEffect, shoes_effect_alloc);
    rb_define_method(cEffect, "draw", CASTHOOK(shoes_effect_draw), 2);
    rb_define_method(cEffect, "remove", CASTHOOK(shoes_basic_remove), 0);
}

VALUE shoes_effect_new(ID name, VALUE attr, VALUE parent) {
    shoes_effect *fx;
    shoes_canvas *canvas;
    VALUE obj = shoes_effect_alloc(cEffect);

    Data_Get_Struct(obj, shoes_effect, fx);
    Data_Get_Struct(parent, shoes_canvas, canvas);
    fx->parent = parent;
    fx->attr = attr;
    fx->filter = shoes_effect_for_type(name);

    return obj;
}

VALUE shoes_effect_alloc(VALUE klass) {
    VALUE obj;
    shoes_effect *fx = SHOE_ALLOC(shoes_effect);

    SHOE_MEMZERO(fx, shoes_effect, 1);
    obj = Data_Wrap_Struct(klass, shoes_effect_mark, shoes_effect_free, fx);
    fx->attr = Qnil;
    fx->parent = Qnil;

    return obj;
}

VALUE shoes_effect_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_DRAWING(shoes_effect, REL_TILE, canvas->width, canvas->height);

    if (RTEST(actual) && self_t->filter != NULL)
        self_t->filter(CCR(canvas), self_t->attr, &self_t->place);

    self_t->place = place;
    return self;
}

void shoes_effect_mark(shoes_effect *fx) {
    rb_gc_mark_maybe(fx->parent);
    rb_gc_mark_maybe(fx->attr);
}

void shoes_effect_free(shoes_effect *fx) {
    RUBY_CRITICAL(free(fx));
}

shoes_effect_filter shoes_effect_for_type(ID name) {
    if (name == s_blur)
        return &shoes_gaussian_blur_filter;
    else if (name == s_shadow)
        return &shoes_shadow_filter;
    else if (name == s_glow)
        return &shoes_glow_filter;
    return NULL;
}

// Canvas
VALUE shoes_add_effect(VALUE self, ID name, VALUE attr) {
    if (rb_obj_is_kind_of(self, cImage)) {
        shoes_effect_filter filter = shoes_effect_for_type(name);
        SETUP_IMAGE();
        if (filter == NULL) return self;
        filter(image->cr, attr, &place);
        return self;
    }

    SETUP_CANVAS();

    return shoes_add_ele(canvas, shoes_effect_new(name, attr, self));
}