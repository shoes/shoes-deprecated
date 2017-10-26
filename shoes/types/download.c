#include "shoes/types/download.h"

// ruby
VALUE cDownload, cResponse;
FUNC_M("+download", download, -1);

EVENT_COMMON(http, http_klass, start);
EVENT_COMMON(http, http_klass, progress);
EVENT_COMMON(http, http_klass, finish);
EVENT_COMMON(http, http_klass, error);

void shoes_download_init() {
    cDownload   = rb_define_class_under(cTypes, "Download", rb_cObject);
    rb_define_alloc_func(cDownload, shoes_http_alloc);
    rb_define_method(cDownload, "abort", CASTHOOK(shoes_http_abort), 0);
    rb_define_method(cDownload, "finish", CASTHOOK(shoes_http_finish), -1);
    rb_define_method(cDownload, "remove", CASTHOOK(shoes_http_remove), 0);
    rb_define_method(cDownload, "length", CASTHOOK(shoes_http_length), 0);
    rb_define_method(cDownload, "percent", CASTHOOK(shoes_http_percent), 0);
    rb_define_method(cDownload, "progress", CASTHOOK(shoes_http_progress), -1);
    rb_define_method(cDownload, "response", CASTHOOK(shoes_http_response), 0);
    rb_define_method(cDownload, "size", CASTHOOK(shoes_http_length), 0);
    rb_define_method(cDownload, "start", CASTHOOK(shoes_http_start), -1);
    rb_define_method(cDownload, "transferred", CASTHOOK(shoes_http_transferred), 0);

    cResponse   = rb_define_class_under(cDownload, "Response", rb_cObject);
    rb_define_method(cResponse, "body", CASTHOOK(shoes_response_body), 0);
    rb_define_method(cResponse, "headers", CASTHOOK(shoes_response_headers), 0);
    rb_define_method(cResponse, "status", CASTHOOK(shoes_response_status), 0);
    rb_define_method(cResponse, "text", CASTHOOK(shoes_response_body), 0);

    RUBY_M("+download", download, -1);
}

// ruby (download)

void shoes_http_mark(shoes_http_klass *dl) {
    rb_gc_mark_maybe(dl->parent);
    rb_gc_mark_maybe(dl->attr);
    rb_gc_mark_maybe(dl->response);
}

void shoes_http_free(shoes_http_klass *dl) {
    RUBY_CRITICAL(free(dl));
}

VALUE shoes_http_new(VALUE klass, VALUE parent, VALUE attr) {
    shoes_http_klass *dl;
    VALUE obj = shoes_http_alloc(klass);
    Data_Get_Struct(obj, shoes_http_klass, dl);
    dl->parent = parent;
    dl->attr = attr;
    return obj;
}

VALUE shoes_http_alloc(VALUE klass) {
    VALUE obj;
    shoes_http_klass *dl = SHOE_ALLOC(shoes_http_klass);
    SHOE_MEMZERO(dl, shoes_http_klass, 1);
    obj = Data_Wrap_Struct(klass, shoes_http_mark, shoes_http_free, dl);
    dl->parent = Qnil;
    dl->attr = Qnil;
    dl->response = Qnil;
    return obj;
}

VALUE shoes_http_remove(VALUE self) {
    GET_STRUCT(http_klass, self_t);
    self_t->state = SHOES_DOWNLOAD_HALT;
    shoes_canvas_remove_item(self_t->parent, self, 0, 1);
    return self;
}

VALUE shoes_http_abort(VALUE self) {
    GET_STRUCT(http_klass, self_t);
    self_t->state = SHOES_DOWNLOAD_HALT;
    return self;
}

int shoes_message_download(VALUE self, void *data) {
    VALUE proc = Qnil;
    shoes_http_event *de = (shoes_http_event *)data;
    GET_STRUCT(http_klass, dl);
    INFO("EVENT: %d (%d), %lu, %llu, %llu\n", (int)de->stage, (int)de->error, de->percent,
         de->transferred, de->total);

    switch (de->stage) {
        case SHOES_HTTP_STATUS:
            dl->response = shoes_response_new(cResponse, (int)de->status);
            return 0;

        case SHOES_HTTP_HEADER: {
                VALUE h = shoes_response_headers(dl->response);
                rb_hash_aset(h, rb_str_new(de->hkey, de->hkeylen), rb_str_new(de->hval, de->hvallen));
            }
            return 0;

        case SHOES_HTTP_ERROR:
            proc = ATTR(dl->attr, error);
            if (!NIL_P(proc))
                shoes_safe_block(dl->parent, proc, rb_ary_new3(2, self, shoes_http_err(de->error)));
            else
                shoes_canvas_error(self, shoes_http_err(de->error));
            return 0;

        case SHOES_HTTP_COMPLETED:
            if (de->body != NULL) rb_iv_set(dl->response, "body", rb_str_new(de->body, de->total));
    }

    dl->percent = de->percent;
    dl->total = de->total;
    dl->transferred = de->transferred;

    switch (de->stage) {
        case SHOES_HTTP_CONNECTED:
            proc = ATTR(dl->attr, start);
            break;
        case SHOES_HTTP_TRANSFER:
            proc = ATTR(dl->attr, progress);
            break;
        case SHOES_HTTP_COMPLETED:
            proc = ATTR(dl->attr, finish);
            break;
    }

    if (!NIL_P(proc))
        shoes_safe_block(dl->parent, proc, rb_ary_new3(1, self));
    return dl->state;
}

int shoes_doth_handler(shoes_http_event *de, void *data) {
    shoes_doth_data *doth = (shoes_doth_data *)data;
    shoes_http_event *de2 = SHOE_ALLOC(shoes_http_event);
    SHOE_MEMCPY(de2, de, shoes_http_event, 1);
    return shoes_throw_message(SHOES_THREAD_DOWNLOAD, doth->download, de2);
}

void shoes_http_request_free(shoes_http_request *req) {
    if (req->url != NULL) free(req->url);
    if (req->scheme != NULL) free(req->scheme);
    if (req->host != NULL) free(req->host);
    if (req->path != NULL) free(req->path);
    if (req->method != NULL) free(req->method);
    if (req->filepath != NULL) free(req->filepath);
    if (req->body != NULL) free(req->body);
    if (req->headers != NULL) shoes_http_headers_free(req->headers);
    if (req->mem != NULL) free(req->mem);
}

VALUE shoes_http_threaded(VALUE self, VALUE url, VALUE attr) {
    VALUE obj = shoes_http_new(cDownload, self, attr);
    GET_STRUCT(canvas, self_t);
    char *url_string = NULL;

    if (!rb_respond_to(url, s_host)) {
        url_string = strdup(RSTRING_PTR(url));
        url = rb_funcall(rb_mKernel, s_URI, 1, url);
    }

    VALUE scheme = rb_funcall(url, s_scheme, 0);
    VALUE host = rb_funcall(url, s_host, 0);
    VALUE port = rb_funcall(url, s_port, 0);
    VALUE path = rb_funcall(url, s_request_uri, 0);

    if (url_string == NULL) {
        url_string = SHOE_ALLOC_N(char, SHOES_BUFSIZE);
        char slash[2] = "/";
        if (RSTRING_PTR(path)[0] == '/') slash[0] = '\0';
//    sprintf(url_string, "%s://%s:%d%s%s", RSTRING_PTR(scheme), RSTRING_PTR(host),
        sprintf(url_string, "%s://%s:%s%s%s", RSTRING_PTR(scheme), RSTRING_PTR(host),
                RSTRING_PTR(port), slash, RSTRING_PTR(path));
    }

    shoes_http_request *req = SHOE_ALLOC(shoes_http_request);
    SHOE_MEMZERO(req, shoes_http_request, 1);
    req->url = url_string;
    req->scheme = strdup(RSTRING_PTR(scheme));
    req->host = strdup(RSTRING_PTR(host));
    req->port = NUM2INT(port);
    req->path = strdup(RSTRING_PTR(path));
    req->handler = shoes_doth_handler;
    req->flags = SHOES_DL_DEFAULTS;
    if (ATTR(attr, redirect) == Qfalse) req->flags ^= SHOES_DL_REDIRECTS;

    VALUE method = ATTR(attr, method);
    VALUE headers = ATTR(attr, headers);
    VALUE body = ATTR(attr, body);
    if (!NIL_P(body)) {
        req->body = strdup(RSTRING_PTR(body));
        req->bodylen = RSTRING_LEN(body);
    }
    if (!NIL_P(method))  req->method = strdup(RSTRING_PTR(method));
    if (!NIL_P(headers)) req->headers = shoes_http_headers(headers);

    VALUE save = ATTR(attr, save);
    if (req->method == NULL || strcmp(req->method, "HEAD") != 0) {
        if (NIL_P(save)) {
            req->mem = SHOE_ALLOC_N(char, SHOES_BUFSIZE);
            req->memlen = SHOES_BUFSIZE;
        } else {
            req->filepath = strdup(RSTRING_PTR(save));
        }
    }

    shoes_doth_data *data = SHOE_ALLOC(shoes_doth_data);
    data->download = obj;
    req->data = data;

    shoes_native_download(req);
    return obj;
}

VALUE shoes_http_length(VALUE self) {
    GET_STRUCT(http_klass, dl);
    return rb_ull2inum(dl->total);
}

VALUE shoes_http_percent(VALUE self) {
    GET_STRUCT(http_klass, dl);
    return rb_uint2inum(dl->percent);
}

VALUE shoes_http_response(VALUE self) {
    GET_STRUCT(http_klass, dl);
    return dl->response;
}

VALUE shoes_http_transferred(VALUE self) {
    GET_STRUCT(http_klass, dl);
    return rb_ull2inum(dl->transferred);
}

// ruby (response)
VALUE shoes_response_new(VALUE klass, int status) {
    VALUE obj = rb_obj_alloc(cResponse);
    rb_iv_set(obj, "body", Qnil);
    rb_iv_set(obj, "headers", rb_hash_new());
    rb_iv_set(obj, "status", INT2NUM(status));
    return obj;
}

VALUE shoes_response_body(VALUE self) {
    return rb_iv_get(self, "body");
}

VALUE shoes_response_headers(VALUE self) {
    return rb_iv_get(self, "headers");
}

VALUE shoes_response_status(VALUE self) {
    return rb_iv_get(self, "status");
}

/*  TODO: check for C memory leaks 
 *  This a lot more than 'catch' - that's a poor name 
 *  
 */ 
int shoes_catch_message(unsigned int name, VALUE obj, void *data) {
    int ret = SHOES_DOWNLOAD_CONTINUE;
    switch (name) {
      case SHOES_THREAD_DOWNLOAD:
        ret = shoes_message_download(obj, data);
        free(data);
        break;
      case SHOES_IMAGE_DOWNLOAD: {
        VALUE hash, etag = Qnil, uri, uext, path, realpath;
        shoes_image_download_event *side = (shoes_image_download_event *)data;
        if (shoes_image_downloaded(side)) {
            shoes_canvas_repaint_all(side->slot);
            path = rb_str_new2(side->filepath);
            uri = rb_str_new2(side->uripath);
            hash = rb_str_new2(side->hexdigest);
            if (side->etag != NULL)
              etag = rb_str_new2(side->etag);
            if (shoes_cache_setting) {
              uext = rb_funcall(rb_cFile, rb_intern("extname"), 1, path);
              rb_funcall(rb_const_get(rb_cObject, rb_intern("DATABASE")),
                     rb_intern("notify_cache_of"), 3, uri, etag, hash);
              if (side->status != 304) {
                realpath = rb_funcall(cShoes, rb_intern("image_cache_path"), 2, hash, uext);
                rename(side->filepath, RSTRING_PTR(realpath));
              }
            }
        }

        free(side->filepath);
        free(side->uripath);
        if (side->etag != NULL)
          free(side->etag);
        free(data);
      }
      break;
  }
  return ret;
}


// canvas
VALUE shoes_canvas_download(int argc, VALUE *argv, VALUE self) {
    VALUE url, block, obj, attr = Qnil;
    
    SETUP_CANVAS();

    rb_scan_args(argc, argv, "11&", &url, &attr, &block);
    CHECK_HASH(attr);
    if (!NIL_P(block))
        ATTRSET(attr, finish, block);
    obj = shoes_http_threaded(self, url, attr);
    rb_ary_push(canvas->app->extras, obj);
    
    return obj;
}
