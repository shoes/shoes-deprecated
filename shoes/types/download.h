#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/http.h"

#ifndef SHOES_DOWNLOAD_TYPE_H
#define SHOES_DOWNLOAD_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

typedef struct {
    SHOES_CONTROL_REF ref;
    VALUE download;
} shoes_doth_data;

/* each widget should have its own init function */
void shoes_download_init();

// ruby (download)
void shoes_http_mark(shoes_http_klass *dl);
void shoes_http_free(shoes_http_klass *dl);
VALUE shoes_http_new(VALUE klass, VALUE parent, VALUE attr);
VALUE shoes_http_alloc(VALUE klass);
VALUE shoes_http_remove(VALUE self);
VALUE shoes_http_abort(VALUE self);
int shoes_message_download(VALUE self, void *data);

int shoes_doth_handler(shoes_http_event *de, void *data);
void shoes_http_request_free(shoes_http_request *req);
VALUE shoes_http_threaded(VALUE self, VALUE url, VALUE attr);
VALUE shoes_http_length(VALUE self);
VALUE shoes_http_percent(VALUE self);
VALUE shoes_http_response(VALUE self);
VALUE shoes_http_transferred(VALUE self);

// ruby (response)
VALUE shoes_response_new(VALUE klass, int status);
VALUE shoes_response_body(VALUE self);
VALUE shoes_response_headers(VALUE self);
VALUE shoes_response_status(VALUE self);
int shoes_catch_message(unsigned int name, VALUE obj, void *data);

// canvas
VALUE shoes_canvas_download(int, VALUE *, VALUE);

#endif
