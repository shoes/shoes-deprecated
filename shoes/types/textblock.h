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

typedef struct {
    int pos, x, y, hi;
} shoes_textcursor;

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    VALUE texts;
    VALUE links;
    shoes_textcursor *cursor;
    PangoLayout *layout;
    PangoAttrList *pattr;
    GString *text;
    guint len;
    char cached, hover;
    shoes_transform *st;
} shoes_textblock;

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

#define GET_STYLE(name) \
  attr = NULL; \
  str = Qnil; \
  if (!NIL_P(oattr)) str = rb_hash_aref(oattr, ID2SYM(s_##name)); \
  if (!NIL_P(hsh) && NIL_P(str)) str = rb_hash_aref(hsh, ID2SYM(s_##name))
     
#define APPLY_ATTR() \
  if (attr != NULL) { \
    attr->start_index = start_index; \
    attr->end_index = end_index; \
    pango_attr_list_insert_before(block->pattr, attr); \
    attr = NULL; \
  }

#define APPLY_STYLE_COLOR(name, func) \
  GET_STYLE(name); \
  if (!NIL_P(str)) \
  { \
    if (TYPE(str) == T_STRING) \
      str = shoes_color_parse(cColor, str); \
    if (rb_obj_is_kind_of(str, cColor)) \
    { \
      shoes_color *color; \
      Data_Get_Struct(str, shoes_color, color); \
      attr = pango_attr_##func##_new(color->r * 255, color->g * 255, color-> b * 255); \
    } \
    APPLY_ATTR(); \
  }

#endif
