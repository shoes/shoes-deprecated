#include "shoes/types/text_view.h"

// text_edit_box is new with 3.2.25
// text_edit_box has been renamed text_view with 3.3.4

// ruby
VALUE cTextView;

FUNC_M("+text_view", text_view, -1);

void shoes_text_view_init() {
    cTextView  = rb_define_class_under(cTypes, "TextView", cNative);
    rb_define_method(cTextView, "text", CASTHOOK(shoes_text_view_get_text), 0);
    rb_define_method(cTextView, "text=", CASTHOOK(shoes_text_view_set_text), 1);
    rb_define_method(cTextView, "draw", CASTHOOK(shoes_text_view_draw), 2);
    rb_define_method(cTextView, "change", CASTHOOK(shoes_control_change), -1);
    rb_define_method(cTextView, "append", CASTHOOK(shoes_text_view_append), 1);
    rb_define_method(cTextView, "insert", CASTHOOK(shoes_text_view_insert), -1);
    rb_define_method(cTextView, "delete", CASTHOOK(shoes_text_view_delete), 2);
    rb_define_method(cTextView, "get_from", CASTHOOK(shoes_text_view_get), 2);
    rb_define_method(cTextView, "new_insertion", CASTHOOK(shoes_text_view_create_insertion), 2);
    rb_define_method(cTextView, "currrent_insertion", CASTHOOK(shoes_text_view_current_insertion), 0);
    rb_define_method(cTextView, "scroll_to_insertion", CASTHOOK(shoes_text_view_scroll_to_insertion), 1);
    rb_define_method(cTextView, "scroll_to_end", CASTHOOK(shoes_text_view_scroll_to_end), 0);
    rb_define_method(cTextView, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cTextView, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+text_view", text_view, -1);
}

// ruby
VALUE shoes_text_view_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(80, 0, FALSE);

    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            self_t->ref = shoes_native_text_view(self, canvas, &place, self_t->attr, msg);
            shoes_control_check_styles(self_t);
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        } else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }

    FINISH();

    return self;

}

VALUE shoes_text_view_get_text(VALUE self) {
    GET_STRUCT(control, self_t);
    if (self_t->ref == NULL) return Qnil;
    return shoes_native_text_view_get_text(self_t->ref);
}

VALUE shoes_text_view_set_text(VALUE self, VALUE text) {
    char *msg = "";
    GET_STRUCT(control, self_t);
    if (!NIL_P(text)) {
        text = shoes_native_to_s(text);
        ATTRSET(self_t->attr, text, text);
        msg = RSTRING_PTR(text);
    }
    if (self_t->ref != NULL) shoes_native_text_view_set_text(self_t->ref, msg);
    return text;
}

VALUE shoes_text_view_append (VALUE self, VALUE text) {
    char *msg = "";
    VALUE ret;
    GET_STRUCT(control, self_t);
    if (!NIL_P(text)) {
        text = shoes_native_to_s(text);
        ATTRSET(self_t->attr, text, text);
        msg = RSTRING_PTR(text);
    }
    if (self_t->ref != NULL)
        ret = shoes_native_text_view_append(self_t->ref, msg);
    else
        ret = text;
    return ret; //TODO: should return updated internal insertion point
}

VALUE shoes_text_view_insert (VALUE self, VALUE args) {
    // parse args
    return Qnil;
}

VALUE shoes_text_view_delete( VALUE self, VALUE args) {
    return args;
}

VALUE shoes_text_view_get(VALUE self, VALUE args) {
    return args;
}

VALUE shoes_text_view_create_insertion(VALUE self, VALUE args) {
    return args;
}

VALUE shoes_text_view_current_insertion(VALUE self) {
    return Qnil;
}

VALUE shoes_text_view_scroll_to_insertion(VALUE seff, VALUE insert_pt) {
    return insert_pt; // TODO: wrong
}

VALUE shoes_text_view_scroll_to_end (VALUE self) {
    return self; // TODO: Not even wrong
}

// canvas
VALUE shoes_canvas_text_view(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE phrase = Qnil, attr = Qnil, text_view;
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

    text_view = shoes_control_new(cTextView, attr, self);
    shoes_add_ele(canvas, text_view);
    return text_view;
}
