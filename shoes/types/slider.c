#include "shoes/types/native.h"
#include "shoes/types/slider.h"

// ruby
VALUE cSlider;

FUNC_M("+slider", slider, -1);

void shoes_slider_init() {
    cSlider  = rb_define_class_under(cTypes, "Slider", cNative);
    rb_define_method(cSlider, "draw", CASTHOOK(shoes_slider_draw), 2);
    rb_define_method(cSlider, "fraction", CASTHOOK(shoes_slider_get_fraction), 0);
    rb_define_method(cSlider, "fraction=", CASTHOOK(shoes_slider_set_fraction), 1);
    rb_define_method(cSlider, "change", CASTHOOK(shoes_control_change), -1);
    rb_define_method(cSlider, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cSlider, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+slider", slider, -1);
}

VALUE shoes_slider_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 0, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_slider(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            if (RTEST(ATTR(self_t->attr, fraction))) shoes_native_slider_set_fraction(self_t->ref, NUM2DBL(ATTR(self_t->attr, fraction)));
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_slider_get_fraction(VALUE self) {
    double perc = 0.;
    GET_STRUCT(control, self_t);
    if (self_t->ref != NULL)
        perc = shoes_native_slider_get_fraction(self_t->ref);

    return rb_float_new(perc);
}

VALUE shoes_slider_set_fraction(VALUE self, VALUE _perc) {
    double perc = min(max(NUM2DBL(_perc), 0.0), 1.0);

    GET_STRUCT(control, self_t);
    if (self_t->ref != NULL)
        shoes_native_slider_set_fraction(self_t->ref, perc);

    return self;
}

// canvas
VALUE shoes_canvas_slider(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE slider;

    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h&", &args);

    if (!NIL_P(args.a[1]))
        ATTRSET(args.a[0], change, args.a[1]);

    slider = shoes_control_new(cSlider, args.a[0], self);
    shoes_add_ele(canvas, slider);

    return slider;
}