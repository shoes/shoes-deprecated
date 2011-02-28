#include <ruby.h>

static VALUE hello_world(VALUE mod)
{
  return rb_str_new2("hello world");
}

void Init_shoes()
{
  VALUE cShoes = rb_define_class("Shoes", rb_cObject);
  VALUE cTypes = rb_define_class_under(cShoes, "Types", rb_cObject);
  VALUE cBackground = rb_define_class_under(cShoes, "Background", rb_cObject);
  VALUE cBorder = rb_define_class_under(cShoes, "Border", rb_cObject);
  VALUE cCanvas = rb_define_class_under(cShoes, "Canvas", rb_cObject);
  VALUE cCheck = rb_define_class_under(cShoes, "Check", rb_cObject);
  VALUE cRadio = rb_define_class_under(cShoes, "Radio", rb_cObject);
  VALUE cEditLine = rb_define_class_under(cShoes, "EditLine", rb_cObject);
  VALUE cEditBox = rb_define_class_under(cShoes, "EditBox", rb_cObject);
  VALUE cEffect = rb_define_class_under(cShoes, "Effect", rb_cObject);
  VALUE cImage = rb_define_class_under(cShoes, "Image", rb_cObject);
  VALUE cListBox = rb_define_class_under(cShoes, "ListBox", rb_cObject);
  VALUE cProgress = rb_define_class_under(cShoes, "Progress", rb_cObject);
  VALUE cShape = rb_define_class_under(cShoes, "Shape", rb_cObject);
  VALUE cTextBlock = rb_define_class_under(cShoes, "TextBlock", rb_cObject);
  VALUE cText = rb_define_class_under(cShoes, "Text", rb_cObject);
  rb_define_singleton_method(cShoes, "hello_world", hello_world, 0);
}

