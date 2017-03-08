#include "shoes/types/edit_line.h"

// ruby
VALUE cEditLine;

FUNC_M("+edit_line", edit_line, -1);

void shoes_edit_line_init() {
    cEditLine  = rb_define_class_under(cTypes, "EditLine", cNative);
    rb_define_method(cEditLine, "text", CASTHOOK(shoes_edit_line_get_text), 0);
    rb_define_method(cEditLine, "text=", CASTHOOK(shoes_edit_line_set_text), 1);
    rb_define_method(cEditLine, "draw", CASTHOOK(shoes_edit_line_draw), 2);
    rb_define_method(cEditLine, "change", CASTHOOK(shoes_control_change), -1);
    rb_define_method(cEditLine, "finish=", CASTHOOK(shoes_edit_line_enterkey), 1);
    rb_define_method(cEditLine, "to_end", CASTHOOK(shoes_edit_line_cursor_to_end), 0);

    RUBY_M("+edit_line", edit_line, -1);
}

VALUE shoes_edit_line_get_text(VALUE self) {
    GET_STRUCT(control, self_t);
    if (self_t->ref == NULL) return Qnil;
    return shoes_native_edit_line_get_text(self_t->ref);
}

VALUE shoes_edit_line_set_text(VALUE self, VALUE text) {
    char *msg = "";
    GET_STRUCT(control, self_t);
    if (!NIL_P(text)) {
        text = shoes_native_to_s(text);
        ATTRSET(self_t->attr, text, text);
        msg = RSTRING_PTR(text);
    }
    if (self_t->ref != NULL) shoes_native_edit_line_set_text(self_t->ref, msg);
    return text;
}

// cjc: added in Shoes 3.2.15
VALUE shoes_edit_line_enterkey(VALUE self, VALUE proc) {
    // store the proc in the attr
    GET_STRUCT(control, self_t);
    if (!NIL_P(proc)) {
        ATTRSET(self_t->attr, donekey, proc);
    }
    return Qnil;
}

VALUE shoes_edit_line_cursor_to_end(VALUE self) {
    GET_STRUCT(control, self_t);
    shoes_native_edit_line_cursor_to_end(self_t->ref);
    return Qnil;
}

VALUE shoes_edit_line_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 0, FALSE);

#ifdef SHOES_QUARTZ
    // cjc 2015-03-15  only change h, ih
    place.h += 4;
    place.ih += 4;
#endif
    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_edit_line(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;
}

VALUE shoes_canvas_edit_line(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE phrase = Qnil, attr = Qnil, edit_line;
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

    edit_line = shoes_control_new(cEditLine, attr, self);
    shoes_add_ele(canvas, edit_line);
    return edit_line;
}