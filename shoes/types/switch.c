#include "shoes/types/native.h"
#include "shoes/types/switch.h"

// ruby
VALUE cSwitch;

FUNC_M("+switch", switch, -1);

EVENT_COMMON(control, control, active)

void shoes_switch_init() {
    cSwitch  = rb_define_class_under(cTypes, "Switch", cNative);
    rb_define_method(cSwitch, "draw", CASTHOOK(shoes_switch_draw), 2);
    rb_define_method(cSwitch, "active?", CASTHOOK(shoes_switch_get_active), 0);
    rb_define_method(cSwitch, "active=", CASTHOOK(shoes_switch_set_active),1);
    rb_define_method(cSwitch, "click", CASTHOOK(shoes_control_active), -1);
    rb_define_method(cSwitch, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cSwitch, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+switch", switch, -1);
}

VALUE shoes_switch_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 0, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_switch(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_switch_get_active(VALUE self) {
    GET_STRUCT(control, self_t);
    return shoes_native_switch_get_active(self_t->ref) ? Qtrue : Qfalse;
}

VALUE shoes_switch_set_active(VALUE self, VALUE activate) {
    GET_STRUCT(control, self_t);
    if (self_t->ref != NULL)
        shoes_native_switch_set_active(self_t->ref, activate);

    return self;
}

// canvas
VALUE shoes_canvas_switch(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE _switch;

    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h", &args);

    if (rb_block_given_p())
        ATTRSET(args.a[0], active, rb_block_proc());

    _switch = shoes_control_new(cSwitch, args.a[0], self);
    shoes_add_ele(canvas, _switch);

    return _switch;
}
