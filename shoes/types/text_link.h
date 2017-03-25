#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

/*
   Named text_link{.c,.h} to ensure text{.c,.h} is loaded first in SHOES_TYPES_INIT
   because link is a subclass of TextClass
*/

#ifndef SHOES_TEXT_LINK_TYPE_H
#define SHOES_TEXT_LINK_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget, cTextClass;
extern shoes_app _shoes_app;

VALUE cLink, cLinkText, cLinkHover, cLinkUrl;

SYMBOL_ID(link);

typedef struct {
    int start;
    int end;
    VALUE ele;
} shoes_link;

/* each widget should have its own init function */
void shoes_text_link_init();

// ruby (link url)
void shoes_link_mark(shoes_link *link);
void shoes_link_free(shoes_link *link) ;
VALUE shoes_link_new(VALUE ele, int start, int end);
VALUE shoes_link_alloc(VALUE klass);
VALUE shoes_link_at(shoes_textblock *t, VALUE self, int index, int blockhover, VALUE *clicked, char *touch);

// canvas
VALUE shoes_canvas_link(int argc, VALUE *argv, VALUE self);

#endif
