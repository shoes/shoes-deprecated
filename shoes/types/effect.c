#include "shoes/types/color.h"
#include "shoes/types/image.h"
#include "shoes/types/pattern.h"
#include "shoes/types/shape.h"
#include "shoes/types/effect.h"

// ruby
VALUE cEffect;

void shoes_effect_init() {
    cEffect   = rb_define_class_under(cTypes, "Effect", rb_cObject);
    rb_define_alloc_func(cEffect, shoes_effect_alloc);
    rb_define_method(cEffect, "draw", CASTHOOK(shoes_effect_draw), 2);
    rb_define_method(cEffect, "remove", CASTHOOK(shoes_basic_remove), 0);
}

// ruby
VALUE shoes_effect_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_DRAWING(shoes_effect, REL_TILE, canvas->width, canvas->height);

    if (RTEST(actual) && self_t->filter != NULL)
        self_t->filter(CCR(canvas), self_t->attr, &self_t->place);

    self_t->place = place;
    return self;
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

VALUE shoes_canvas_blur(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 1, s_radius);
    return shoes_add_effect(self, s_blur, attr);
}

VALUE shoes_canvas_glow(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 1, s_radius);
    return shoes_add_effect(self, s_glow, attr);
}

VALUE shoes_canvas_shadow(int argc, VALUE *argv, VALUE self) {
    VALUE attr = shoes_shape_attr(argc, argv, 2, s_distance, s_radius);
    return shoes_add_effect(self, s_shadow, attr);
}

// shoes/effect.c

static unsigned char *box_run(unsigned int size) {
    int i;
    unsigned char *tmp = SHOE_ALLOC_N(unsigned char, size * 256);
    for (i = 0; i < 256; i++)
        memset(tmp + i * size, i, size);
    return tmp;
}

#define BOX_H 1
#define BOX_V 2

static void box_blur(unsigned char *in, unsigned char *out,
                     int stride, shoes_place *place,
                     unsigned int edge1, unsigned int edge2,
                     const unsigned char *run, int dir) {
    int i, j1, j2, l = 0, l2, l3, l4, lx = 0, c1, c2, c3, c4, start;
    int boxSize = edge1 + edge2 + 1;
    if (dir == BOX_H) {
        c1 = place->y;
        c2 = place->y + place->h;
        c3 = place->x;
        c4 = place->x + place->w;
    } else {
        c1 = place->x;
        c2 = place->x + place->w;
        c3 = place->y;
        c4 = place->y + place->h;
    }

    start = c3 - edge1;
    for (j1 = c1; j1 < c2; j1++) {
        unsigned int sums[4] = {0, 0, 0, 0};
        if (dir == BOX_H)
            l = stride * j1;
        else
            lx = j1 << 2;
        for (i = 0; i < boxSize; i++) {
            int pos = start + i;
            pos = max(pos, c3);
            pos = min(pos, c4 - 1);
            if (dir == BOX_V)
                l = stride * pos + lx;
            sums[0] += in[l];
            sums[1] += in[l + 1];
            sums[2] += in[l + 2];
            sums[3] += in[l + 3];
        }
        for (j2 = c3; j2 < c4; j2++) {
            if (dir == BOX_H)
                l2 = l + (j2 << 2);
            else
                l2 = stride * j2 + lx;
            out[l2] = run[(int)sums[0]];
            out[l2 + 1] = run[(int)sums[1]];
            out[l2 + 2] = run[(int)sums[2]];
            out[l2 + 3] = run[(int)sums[3]];

            int tmp = j2 - edge1;
            int last = max(tmp, c3);
            int next = min(tmp + boxSize, c4 - 1);
            if (dir == BOX_H) {
                l3 = l + (next << 2);
                l4 = l + (last << 2);
            } else {
                l3 = stride * next + lx;
                l4 = stride * last + lx;
            }

            sums[0] += in[l3] - in[l4];
            sums[1] += in[l3 + 1] - in[l4 + 1];
            sums[2] += in[l3 + 2] - in[l4 + 2];
            sums[3] += in[l3 + 3] - in[l4 + 3];
        }
    }
}

void shoes_gaussian_blur_filter(cairo_t *cr, VALUE attr, shoes_place *place) {
    double blur_d = ATTR2(dbl, attr, radius, 2.);
    double blur_x = blur_d, blur_y = blur_d;

    RAW_FILTER_START(place);
    if (blur_x < 0 || blur_y < 0)
        return;
    if (blur_x == 0 || blur_y == 0)
        memset(out, 0, len);
    unsigned int dX, dY;
    dX = (unsigned int) floor(blur_x * 3*sqrt(2*SHOES_PI)/4 + 0.5);
    dY = (unsigned int) floor((blur_y * 3*sqrt(2*SHOES_PI)/4) + 0.5);

    unsigned char *tmp = SHOE_ALLOC_N(unsigned char, len);

    if (dX & 1) {    // odd dX
        unsigned char *run = box_run(2 * (dX / 2) + 1);
        box_blur(in, tmp,  stride, place, dX/2, dX/2, run, BOX_H);
        box_blur(tmp, out, stride, place, dX/2, dX/2, run, BOX_H);
        box_blur(out, tmp, stride, place, dX/2, dX/2, run, BOX_H);
        SHOE_FREE(run);
    } else {        // even dX
        if (dX == 0) {
            memcpy(tmp, in, len);
        } else {
            unsigned char *run1 = box_run(2 * (dX / 2) + 1);
            unsigned char *run2 = box_run(2 * (dX / 2));
            box_blur(in, tmp,  stride, place, dX/2,     dX/2 - 1, run2, BOX_H);
            box_blur(tmp, out, stride, place, dX/2 - 1, dX/2,     run2, BOX_H);
            box_blur(out, tmp, stride, place, dX/2,     dX/2,     run1, BOX_H);
            SHOE_FREE(run1);
            SHOE_FREE(run2);
        }
    }
    if (dY & 1) {
        unsigned char *run = box_run(2 * (dY / 2) + 1);
        box_blur(tmp, out, stride, place, dY/2, dY/2, run, BOX_V);
        box_blur(out, tmp, stride, place, dY/2, dY/2, run, BOX_V);
        box_blur(tmp, out, stride, place, dY/2, dY/2, run, BOX_V);
        SHOE_FREE(run);
    } else {
        if (dY == 0) {
            memcpy(out, tmp, len);
        } else {
            unsigned char *run1 = box_run(2 * (dY / 2) + 1);
            unsigned char *run2 = box_run(2 * (dY / 2));
            box_blur(tmp, out, stride, place, dY/2,     dY/2 - 1, run2, BOX_V);
            box_blur(out, tmp, stride, place, dY/2 - 1, dY/2,     run2, BOX_V);
            box_blur(tmp, out, stride, place, dY/2,     dY/2,     run1, BOX_V);
            SHOE_FREE(run1);
            SHOE_FREE(run2);
        }
    }

    SHOE_FREE(tmp);
    RAW_FILTER_END();
}

static void shoes_layer_blur_filter(cairo_t *cr, VALUE attr, shoes_place *place,
                                    cairo_operator_t blur_op, cairo_operator_t merge_op, int distance) {
    cairo_surface_t *source = cairo_get_target(cr);
    int width  = cairo_image_surface_get_width(source);
    int height = cairo_image_surface_get_height(source);
    VALUE fill = ATTR(attr, fill);
    int dx = ATTR2(int, attr, displace_left, 0);
    int dy = ATTR2(int, attr, displace_top, 0);

    cairo_surface_t *target = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cr2 = cairo_create(target);
    if (dx != 0 || dy != 0)
        cairo_set_source_surface(cr2, source, dx, dy);
    else
        cairo_set_source_surface(cr2, source, distance, distance);
    cairo_paint(cr2);
    cairo_set_operator(cr2, blur_op);
    if (NIL_P(fill))
        cairo_set_source_rgb(cr2, 0., 0., 0.);
    else if (rb_obj_is_kind_of(fill, cColor)) {
        shoes_color *color;
        Data_Get_Struct(fill, shoes_color, color);
        cairo_set_source_rgba(cr2, color->r / 255., color->g / 255., color->b / 255., color->a / 255.);
    } else {
        shoes_pattern *pattern;
        Data_Get_Struct(fill, shoes_pattern, pattern);
        cairo_set_source(cr2, PATTERN(pattern));
    }
    cairo_rectangle(cr2, 0, 0, width, height);
    cairo_paint(cr2);
    shoes_gaussian_blur_filter(cr2, attr, place);
    cairo_set_operator(cr, merge_op);
    cairo_set_source_surface(cr, target, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr2);
}

void shoes_shadow_filter(cairo_t *cr, VALUE attr, shoes_place *place) {
    int distance = ATTR2(int, attr, distance, 4);
    shoes_layer_blur_filter(cr, attr, place, CAIRO_OPERATOR_IN, CAIRO_OPERATOR_DEST_OVER, distance);
}

void shoes_glow_filter(cairo_t *cr, VALUE attr, shoes_place *place) {
    cairo_operator_t blur_op = CAIRO_OPERATOR_IN;
    cairo_operator_t merge_op = CAIRO_OPERATOR_DEST_OVER;
    if (RTEST(ATTR(attr, inner))) {
        blur_op = CAIRO_OPERATOR_OUT;
        merge_op = CAIRO_OPERATOR_ATOP;
    }
    shoes_layer_blur_filter(cr, attr, place, blur_op, merge_op, 0);
}
