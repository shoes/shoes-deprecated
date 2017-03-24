#include "shoes/types/radio.h"

// ruby
VALUE cRadio;

FUNC_M("+radio", radio, -1);

void shoes_radio_init() {
    cRadio  = rb_define_class_under(cTypes, "Radio", cNative);
    rb_define_method(cRadio, "draw", CASTHOOK(shoes_radio_draw), 2);
    rb_define_method(cRadio, "checked?", CASTHOOK(shoes_check_is_checked), 0);
    rb_define_method(cRadio, "checked=", CASTHOOK(shoes_check_set_checked_m), 1);
    rb_define_method(cRadio, "click", CASTHOOK(shoes_control_click), -1);
    rb_define_method(cRadio, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cRadio, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+radio", radio, -1);
}

// ruby
VALUE shoes_radio_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 20, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            VALUE group = ATTR(self_t->attr, group);
            if (NIL_P(group)) group = c;

            VALUE glist = shoes_hash_get(canvas->app->groups, group);
#ifdef SHOES_FORCE_RADIO // aka OSX - create group before realizing widget
            if (NIL_P(glist))
                canvas->app->groups = shoes_hash_set(canvas->app->groups, group, (glist = rb_ary_new3(1, self)));
            else
                rb_ary_push(glist, self);
            glist = shoes_hash_get(canvas->app->groups, group);
            self_t->ref = shoes_native_radio(self, canvas, &place, self_t->attr, glist);
#else
            self_t->ref = shoes_native_radio(self, canvas, &place, self_t->attr, glist);

            if (NIL_P(glist))
                canvas->app->groups = shoes_hash_set(canvas->app->groups, group, (glist = rb_ary_new3(1, self)));
            else
                rb_ary_push(glist, self);
#endif
            if (RTEST(ATTR(self_t->attr, checked))) shoes_native_check_set(self_t->ref, Qtrue);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_check_set_checked_m(VALUE self, VALUE on) {
#ifdef SHOES_FORCE_RADIO
    if (RTEST(on)) {
        VALUE glist = shoes_button_group(self);

        if (!NIL_P(glist)) {
            long i;
            for (i = 0; i < RARRAY_LEN(glist); i++) {
                VALUE ele = rb_ary_entry(glist, i);
                shoes_check_set_checked(ele, ele == self ? Qtrue : Qfalse);
            }
        } else {
            shoes_check_set_checked(self, on);
        }
        return on;
    }
#endif
    shoes_check_set_checked(self, on);
    return on;
}

#ifdef SHOES_FORCE_RADIO
void shoes_radio_button_click(VALUE control) {
    shoes_check_set_checked_m(control, Qtrue);
}
#endif

// canvas
VALUE shoes_canvas_radio(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE group = Qnil, attr = Qnil, radio;
    SETUP_CANVAS();

    switch (rb_parse_args(argc, argv, "h,o|h,", &args)) {
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
