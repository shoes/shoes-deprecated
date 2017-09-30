#include "shoes/types/textblock.h"
#include "shoes/types/text.h"

// ruby
VALUE cTextClass, cSpan, cDel, cStrong, cSub, cSup, cCode, cEm, cIns;

FUNC_M(".code", code, -1);
FUNC_M(".del", del, -1);
FUNC_M(".em", em, -1);
FUNC_M(".ins", ins, -1);
FUNC_M(".span", span, -1);
FUNC_M(".strong", strong, -1);
FUNC_M(".sub", sub, -1);
FUNC_M(".sup", sup, -1);

CLASS_COMMON(text);
REPLACE_COMMON(text);

MARKUP_DEF(code, INLINE, cCode);
MARKUP_DEF(del, INLINE, cDel);
MARKUP_DEF(em, INLINE, cEm);
MARKUP_DEF(ins, INLINE, cIns);
MARKUP_DEF(span, INLINE, cSpan);
MARKUP_DEF(strong, INLINE, cStrong);
MARKUP_DEF(sub, INLINE, cSub);
MARKUP_DEF(sup, INLINE, cSup);

void shoes_text_init() {
    cTextClass = rb_define_class_under(cTypes, "Text", rb_cObject);

    rb_define_alloc_func(cTextClass, shoes_text_alloc);
    rb_define_method(cTextClass, "app", CASTHOOK(shoes_canvas_get_app), 0);
    rb_define_method(cTextClass, "contents", CASTHOOK(shoes_text_children), 0);
    rb_define_method(cTextClass, "children", CASTHOOK(shoes_text_children), 0);
    rb_define_method(cTextClass, "parent", CASTHOOK(shoes_text_parent), 0);
    rb_define_method(cTextClass, "style", CASTHOOK(shoes_text_style), -1);
    rb_define_method(cTextClass, "to_s", CASTHOOK(shoes_text_to_s), 0);
    rb_define_method(cTextClass, "text", CASTHOOK(shoes_text_children), 0);
    rb_define_method(cTextClass, "text=", CASTHOOK(shoes_text_replace), -1);
    rb_define_method(cTextClass, "replace", CASTHOOK(shoes_text_replace), -1);

    cCode      = rb_define_class_under(cTypes, "Code", cTextClass);
    cDel       = rb_define_class_under(cTypes, "Del", cTextClass);
    cEm        = rb_define_class_under(cTypes, "Em", cTextClass);
    cIns       = rb_define_class_under(cTypes, "Ins", cTextClass);
    cSpan      = rb_define_class_under(cTypes, "Span", cTextClass);
    cStrong    = rb_define_class_under(cTypes, "Strong", cTextClass);
    cSub       = rb_define_class_under(cTypes, "Sub", cTextClass);
    cSup       = rb_define_class_under(cTypes, "Sup", cTextClass);

    RUBY_M(".code", code, -1);
    RUBY_M(".del", del, -1);
    RUBY_M(".em", em, -1);
    RUBY_M(".ins", ins, -1);
    RUBY_M(".span", span, -1);
    RUBY_M(".strong", strong, -1);
    RUBY_M(".sub", sub, -1);
    RUBY_M(".sup", sup, -1);
}

// ruby
void shoes_text_mark(shoes_text *text) {
    rb_gc_mark_maybe(text->texts);
    rb_gc_mark_maybe(text->attr);
    rb_gc_mark_maybe(text->parent);
}

void shoes_text_free(shoes_text *text) {
    RUBY_CRITICAL(free(text));
}

VALUE shoes_text_check(VALUE texts, VALUE parent) {
    long i;
    for (i = 0; i < RARRAY_LEN(texts); i++) {
        VALUE ele = rb_ary_entry(texts, i);
        if (rb_obj_is_kind_of(ele, cTextClass)) {
            shoes_text *text;
            Data_Get_Struct(ele, shoes_text, text);
            text->parent = parent;
        }
    }
    return texts;
}

VALUE shoes_text_to_s(VALUE self) {
    GET_STRUCT(textblock, self_t);
    return rb_funcall(self_t->texts, s_to_s, 0);
}

VALUE shoes_text_new(VALUE klass, VALUE texts, VALUE attr) {
    shoes_text *text;
    VALUE obj = shoes_text_alloc(klass);
    Data_Get_Struct(obj, shoes_text, text);
    text->hover = 0;
    text->texts = shoes_text_check(texts, obj);
    text->attr = attr;
    return obj;
}

VALUE shoes_text_alloc(VALUE klass) {
    VALUE obj;
    shoes_text *text = SHOE_ALLOC(shoes_text);
    SHOE_MEMZERO(text, shoes_text, 1);
    obj = Data_Wrap_Struct(klass, shoes_text_mark, shoes_text_free, text);
    text->texts = Qnil;
    text->attr = Qnil;
    text->parent = Qnil;
    return obj;
}

VALUE shoes_text_parent(VALUE self) {
    GET_STRUCT(text, text);
    return text->parent;
}

VALUE shoes_text_children(VALUE self) {
    GET_STRUCT(text, text);
    return text->texts;
}