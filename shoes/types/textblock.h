#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_TEXTBLOCK_TYPE_H
#define SHOES_TEXTBLOCK_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

VALUE cTextBlock, cPara, cBanner, cTitle, cSubtitle, cTagline, cCaption, cInscription;

/* each widget should have its own init function */
void shoes_textblock_init();

// ruby
VALUE shoes_textblock_draw(VALUE self, VALUE c, VALUE actual);
VALUE shoes_textblock_string(VALUE self);
VALUE shoes_textblock_style_m(int argc, VALUE *argv, VALUE self);
void shoes_textblock_mark(shoes_textblock *text);
void shoes_textblock_uncache(shoes_textblock *text, unsigned char all);
void shoes_textblock_free(shoes_textblock *text);
VALUE shoes_textblock_new(VALUE klass, VALUE texts, VALUE attr, VALUE parent, shoes_transform *st);
VALUE shoes_textblock_alloc(VALUE klass);
VALUE shoes_textblock_children(VALUE self);
VALUE shoes_textblock_set_cursor(VALUE self, VALUE pos);
VALUE shoes_textblock_get_cursor(VALUE self);
VALUE shoes_textblock_cursorx(VALUE self);
VALUE shoes_textblock_cursory(VALUE self);
VALUE shoes_textblock_set_marker(VALUE self, VALUE pos);
VALUE shoes_textblock_get_marker(VALUE self);
VALUE shoes_textblock_get_highlight(VALUE self);
VALUE shoes_find_textblock(VALUE self);
VALUE shoes_textblock_send_hover(VALUE self, int x, int y, VALUE *clicked, char *t);
VALUE shoes_textblock_motion(VALUE self, int x, int y, char *t);
VALUE shoes_textblock_hit(VALUE self, VALUE _x, VALUE _y);
VALUE shoes_textblock_send_click(VALUE self, int button, int x, int y, VALUE *clicked);
void shoes_textblock_send_release(VALUE self, int button, int x, int y);

// canvas

#endif
