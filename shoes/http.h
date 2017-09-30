//
// shoes/http.h
// the shoes downloader, which uses platform code and threads
// to achieve much faster and brain-dead http access.
//
#ifndef SHOES_HTTP_H
#define SHOES_HTTP_H

#include "shoes/http/common.h"

typedef struct {
    char *url;
    char *scheme;
    char *host;
    int port;
    char *path;

    char *method, *body;
    unsigned long bodylen;
    SHOES_DOWNLOAD_HEADERS headers;

    char *mem;
    unsigned long memlen;
    char *filepath;
    unsigned LONG_LONG size;
    shoes_http_handler handler;
    void *data;
    unsigned char flags;
} shoes_http_request;

void shoes_download(shoes_http_request *req);
void shoes_queue_download(shoes_http_request *req);
VALUE shoes_http_err(SHOES_DOWNLOAD_ERROR error);
SHOES_DOWNLOAD_HEADERS shoes_http_headers(VALUE hsh);
void shoes_http_request_free(shoes_http_request *);
void shoes_http_headers_free(SHOES_DOWNLOAD_HEADERS);

#ifdef SHOES_WIN32
#include "shoes/http/winhttp.h"
#else
#define HTTP_HANDLER(x)
#endif

#endif
