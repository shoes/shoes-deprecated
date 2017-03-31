#include "shoes/types/native.h"
#include "shoes/types/edit_box.h"

// ruby
VALUE cEditBox;

FUNC_M("+edit_box", edit_box, -1);

void shoes_edit_box_init() {
    cEditBox  = rb_define_class_under(cTypes, "EditBox", cNative);
    rb_define_method(cEditBox, "text", CASTHOOK(shoes_edit_box_get_text), 0);
    rb_define_method(cEditBox, "text=", CASTHOOK(shoes_edit_box_set_text), 1);
    rb_define_method(cEditBox, "draw", CASTHOOK(shoes_edit_box_draw), 2);
    rb_define_method(cEditBox, "change", CASTHOOK(shoes_control_change), -1);
    rb_define_method(cEditBox, "append", CASTHOOK(shoes_edit_box_append), 1);
    rb_define_method(cEditBox, "scroll_to_end", CASTHOOK(shoes_edit_box_scroll_to_end), 0);
    rb_define_method(cEditBox, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cEditBox, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+edit_box", edit_box, -1);
}

VALUE shoes_edit_box_get_text(VALUE self) {
    GET_STRUCT(control, self_t);
    if (self_t->ref == NULL) return Qnil;
    return shoes_native_edit_box_get_text(self_t->ref);
}

VALUE shoes_edit_box_set_text(VALUE self, VALUE text) {
    char *msg = "";

    GET_STRUCT(control, self_t);
    if (!NIL_P(text)) {
        text = shoes_native_to_s(text);
        ATTRSET(self_t->attr, text, text);
        msg = RSTRING_PTR(text);
    }
    if (self_t->ref != NULL) shoes_native_edit_box_set_text(self_t->ref, msg);

    return text;
}

VALUE shoes_edit_box_append(VALUE self, VALUE text) {
    char *msg = "";

    GET_STRUCT(control, self_t);
    if (!NIL_P(text)) {
        text = shoes_native_to_s(text);
        ATTRSET(self_t->attr, text, text);
        msg = RSTRING_PTR(text);
    }
    if (self_t->ref != NULL) shoes_native_edit_box_append(self_t->ref, msg);

    return Qnil;
}

VALUE shoes_edit_box_scroll_to_end(VALUE self) {
    GET_STRUCT(control, self_t);
    if (self_t->ref != NULL) shoes_native_edit_box_scroll_to_end(self_t->ref);
    return Qnil;
}

VALUE shoes_edit_box_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(80, 0, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_edit_box(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

// canvas
VALUE shoes_canvas_edit_box(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE phrase = Qnil, attr = Qnil, edit_box;

    SETUP_CANVAS();

    switch (rb_parse_args(argc, argv, "h,S|h,", &args)) {
        case 1:
            attr = args.a[0];
            break;

        case 2:
            phrase = args.a[0];
            attr = args.a[1];
            break;
    }

    if (!NIL_P(phrase))
        ATTRSET(attr, text, phrase);

    if (rb_block_given_p())
        ATTRSET(attr, change, rb_block_proc());

    edit_box = shoes_control_new(cEditBox, attr, self);
    shoes_add_ele(canvas, edit_box);

    return edit_box;
}