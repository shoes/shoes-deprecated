#include "shoes/types/list_box.h"

// ruby
VALUE cListBox;

FUNC_M("+list_box", list_box, -1);

void shoes_list_box_init() {
    cListBox  = rb_define_class_under(cTypes, "ListBox", cNative);
    rb_define_method(cListBox, "text", CASTHOOK(shoes_list_box_text), 0);
    rb_define_method(cListBox, "draw", CASTHOOK(shoes_list_box_draw), 2);
    rb_define_method(cListBox, "choose", CASTHOOK(shoes_list_box_choose), 1);
    rb_define_method(cListBox, "change", CASTHOOK(shoes_control_change), -1);
    rb_define_method(cListBox, "items", CASTHOOK(shoes_list_box_items_get), 0);
    rb_define_method(cListBox, "items=", CASTHOOK(shoes_list_box_items_set), 1);
    rb_define_method(cListBox, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cListBox, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    RUBY_M("+list_box", list_box, -1);
}

VALUE shoes_list_box_choose(VALUE self, VALUE item) {
    VALUE items = Qnil;

    GET_STRUCT(control, self_t);
    ATTRSET(self_t->attr, choose, item);
    if (self_t->ref == NULL) return self;

    items = ATTR(self_t->attr, items);
    shoes_native_list_box_set_active(self_t->ref, items, item);

    return self;
}

VALUE shoes_list_box_text(VALUE self) {
    GET_STRUCT(control, self_t);
    if (self_t->ref == NULL) return Qnil;
    return shoes_native_list_box_get_active(self_t->ref, ATTR(self_t->attr, items));
}

VALUE shoes_list_box_items_get(VALUE self) {
    GET_STRUCT(control, self_t);
    return ATTR(self_t->attr, items);
}

VALUE shoes_list_box_items_set(VALUE self, VALUE items) {
    VALUE opt = shoes_list_box_text(self);

    GET_STRUCT(control, self_t);
    if (!rb_obj_is_kind_of(items, rb_cArray))
        rb_raise(rb_eArgError, "ListBox items must be an array.");
    ATTRSET(self_t->attr, items, items);
    if (self_t->ref != NULL) shoes_native_list_box_update(self_t->ref, items);
    if (!NIL_P(opt)) shoes_list_box_choose(self, opt);

    return items;
}

VALUE shoes_list_box_draw(VALUE self, VALUE c, VALUE actual) {
    SETUP_CONTROL(0, 0, TRUE);

#ifdef SHOES_QUARTZ
    place.h += 4;
    place.ih += 4;
#endif
    if (RTEST(actual)) {
        if (self_t->ref == NULL) {
            VALUE items = ATTR(self_t->attr, items);
            self_t->ref = shoes_native_list_box(self, canvas, &place, self_t->attr, msg);

            if (!NIL_P(items)) {
                shoes_list_box_items_set(self, items);
                shoes_control_check_styles(self_t);
                if (!NIL_P(ATTR(self_t->attr, choose)))
                    shoes_native_list_box_set_active(self_t->ref, items, ATTR(self_t->attr, choose));
            }

#ifdef SHOES_WIN32
            shoes_native_control_position_no_pad(self_t->ref, &self_t->place, self, canvas, &place);
#else
            shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
#endif
        } else
#ifdef SHOES_WIN32
            shoes_native_control_repaint_no_pad(self_t->ref, &self_t->place, canvas, &place);
#else
            shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
#endif
    }

    FINISH();

    return self;
}

// canvas
VALUE shoes_canvas_list_box(int argc, VALUE *argv, VALUE self) {
    rb_arg_list args;
    VALUE list_box;

    SETUP_CANVAS();

    rb_parse_args(argc, argv, "|h&", &args);

    if (!NIL_P(args.a[1]))
        ATTRSET(args.a[0], change, args.a[1]);

    list_box = shoes_control_new(cListBox, args.a[0], self);
    shoes_add_ele(canvas, list_box);

    return list_box;
}