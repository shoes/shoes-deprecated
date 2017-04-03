#include "shoes/types/native.h"
#include "shoes/types/check.h"

// ruby
VALUE cCheck;

FUNC_M("+check", check, -1);

void shoes_check_init() {
    cCheck  = rb_define_class_under(cTypes, "Check", cNative);
    rb_define_method(cCheck, "draw", CASTHOOK(shoes_check_draw), 2);
    rb_define_method(cCheck, "checked?", CASTHOOK(shoes_check_is_checked), 0);
    rb_define_method(cCheck, "checked=", CASTHOOK(shoes_check_set_checked), 1);
    rb_define_method(cCheck, "click", CASTHOOK(shoes_control_click), -1);
#ifdef GTK
    rb_define_method(cCheck, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cCheck, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);
#endif

    RUBY_M("+check", check, -1);
}

// ruby
VALUE shoes_check_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 20, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_check(self, canvas, &place, self_t->attr, msg);
            if (RTEST(ATTR(self_t->attr, checked))) shoes_native_check_set(self_t->ref, Qtrue);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_check_is_checked(VALUE self) {
    GET_STRUCT(control, self_t);
    return shoes_native_check_get(self_t->ref);
}

VALUE shoes_check_set_checked(VALUE self, VALUE on) {
    GET_STRUCT(control, self_t);
    ATTRSET(self_t->attr, checked, on);
    if (self_t->ref != NULL)
        shoes_native_check_set(self_t->ref, RTEST(on));
    return on;
}

// canvas
VALUE shoes_canvas_check(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE check;
    
    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h", &args);

    if (rb_block_given_p())
        ATTRSET(args.a[0], click, rb_block_proc());

    check = shoes_control_new(cCheck, args.a[0], self);
    shoes_add_ele(canvas, check);
    
    return check;
}
