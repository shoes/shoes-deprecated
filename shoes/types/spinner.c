#include "shoes/types/native.h"
#include "shoes/types/spinner.h"

// ruby
VALUE cSpinner;

FUNC_M("+spinner", spinner, -1);

void shoes_spinner_init() {
    cSpinner  = rb_define_class_under(cTypes, "Spinner", cNative);
    rb_define_method(cSpinner, "draw", CASTHOOK(shoes_spinner_draw), 2);
    rb_define_method(cSpinner, "start", CASTHOOK(shoes_spinner_start), 0);
    rb_define_method(cSpinner, "stop", CASTHOOK(shoes_spinner_stop),0);
    rb_define_method(cSpinner, "started?", CASTHOOK(shoes_spinner_started), 0);
    rb_define_method(cSpinner, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cSpinner, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+spinner", spinner, -1);
}

// ruby
VALUE shoes_spinner_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 0, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_spinner(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_spinner_start(VALUE self) {
    GET_STRUCT(control, self_t);
    shoes_native_spinner_start(self_t->ref);
    return self;
}

VALUE shoes_spinner_stop(VALUE self) {
    GET_STRUCT(control, self_t);
    shoes_native_spinner_stop(self_t->ref);
    return self;
}

VALUE shoes_spinner_started(VALUE self) {
    GET_STRUCT(control, self_t);
    return (shoes_native_spinner_started(self_t->ref) ? Qtrue : Qfalse);
}

// canvas
VALUE shoes_canvas_spinner(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE spinner;

    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h", &args);

    spinner = shoes_control_new(cSpinner, args.a[0], self);
    shoes_add_ele(canvas, spinner);

    return spinner;
}
