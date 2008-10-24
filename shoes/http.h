//
// shoes/http.h
// the shoes downloader, which uses platform code and threads
// to achieve much faster and brain-dead http access.
//
#ifndef SHOES_HTTP_H
#define SHOES_HTTP_H

#include "shoes/http/common.h"

typedef struct {
  char *host;
  int port;
  char *path;

  char *method, *body;
  SHOES_DOWNLOAD_HEADERS headers;

  char *mem;
  unsigned long memlen;
  char *filepath;
  unsigned LONG_LONG size;
  shoes_download_handler handler;
  void *data;
  unsigned char flags;
} shoes_download_request;

void shoes_download(shoes_download_request *req);
void shoes_queue_download(shoes_download_request *req);
VALUE shoes_http_error(SHOES_DOWNLOAD_ERROR error);
SHOES_DOWNLOAD_HEADERS shoes_http_headers(VALUE hsh);

#ifdef SHOES_WIN32
#include "shoes/http/winhttp.h"
#else
#define HTTP_HANDLER(x)
#endif

#endif
