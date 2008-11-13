//
// shoes/http/common.h
// cross-platform structures common to Shoes and its stubs
//
#ifndef SHOES_HTTP_COMMON_H
#define SHOES_HTTP_COMMON_H

#define SHOES_CHUNKSIZE 16384

#define SHOES_DL_REDIRECTS 1
#define SHOES_DL_DEFAULTS  (SHOES_DL_REDIRECTS)

#define SHOES_DOWNLOAD_CONTINUE 0
#define SHOES_DOWNLOAD_HALT 1

#define SHOES_HTTP_STATUS    3
#define SHOES_HTTP_HEADER    4
#define SHOES_HTTP_CONNECTED 5
#define SHOES_HTTP_TRANSFER  10
#define SHOES_HTTP_COMPLETED 15
#define SHOES_HTTP_ERROR     20

#define HTTP_HEADER(ptr, realsize, handler, data) \
  { \
    char *colon, *val, *end; \
    for (colon = ptr; colon < ptr + realsize; colon++) \
      if (colon[0] == ':') \
        break; \
    for (val = colon + 1; val < ptr + realsize; val++) \
      if (val[0] != ' ') \
        break; \
    for (end = (ptr + realsize) - 1; end > ptr; end--) \
      if (end[0] != '\r' && end[0] != '\n' && end[0] != ' ') \
        break; \
    if (colon < ptr + realsize) \
    { \
      shoes_download_event *event = SHOE_ALLOC(shoes_download_event); \
      event->stage = SHOES_HTTP_HEADER; \
      event->hkey = ptr; \
      event->hkeylen = colon - ptr; \
      event->hval = val; \
      event->hvallen = (end - val) + 1; \
      if (handler != NULL) handler(event, data); \
      SHOE_FREE(event); \
    } \
  }

#define HTTP_EVENT(handler, s, last, perc, trans, tot, dat, bd, abort) \
{ SHOES_TIME ts; \
  shoes_get_time(&ts); \
  unsigned long elapsed = shoes_diff_time(&(last), &ts); \
  if (s != SHOES_HTTP_TRANSFER || elapsed > 600 ) { \
    shoes_download_event *event = SHOE_ALLOC(shoes_download_event); \
    event->stage = s; \
    if (s == SHOES_HTTP_COMPLETED) event->stage = SHOES_HTTP_TRANSFER; \
    event->percent = perc; \
    event->transferred = trans;\
    event->total = tot; \
    last = ts; \
    if (handler != NULL && (handler(event, dat) & SHOES_DOWNLOAD_HALT)) \
    { SHOE_FREE(event); event = NULL; abort; } \
    if (event != NULL && s == SHOES_HTTP_COMPLETED) { event->stage = s; \
      event->body = bd; \
      if (handler != NULL && (handler(event, dat) & SHOES_DOWNLOAD_HALT)) \
      { SHOE_FREE(event); event = NULL; abort; } \
    } \
    if (event != NULL) SHOE_FREE(event); \
  } }

typedef struct {
  unsigned char stage;
  unsigned long status;
  char *hkey, *hval, *body;
  unsigned long hkeylen, hvallen, bodylen;
  unsigned LONG_LONG total;
  unsigned LONG_LONG transferred;
  unsigned long percent;
  SHOES_DOWNLOAD_ERROR error;
} shoes_download_event;

typedef int (*shoes_download_handler)(shoes_download_event *, void *);

void shoes_get_time(SHOES_TIME *);
unsigned long shoes_diff_time(SHOES_TIME *, SHOES_TIME *);

#endif
