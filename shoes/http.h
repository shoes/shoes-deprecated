//
// shoes/http.h
// the shoes downloader, which uses platform code and threads
// to achieve much faster and brain-dead http access.
//
#define SHOES_CHUNKSIZE 16384

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
      shoes_download_event event; \
      event.stage = SHOES_HTTP_HEADER; \
      event.hkey = ptr; \
      event.hkeylen = colon - ptr; \
      event.hval = val; \
      event.hvallen = (end - val) + 1; \
      if (handler != NULL) handler(&event, data); \
    } \
  }

#define HTTP_EVENT(handler, s, last, perc, trans, tot, dat, bd, abort) \
{ SHOES_TIME ts; \
  shoes_get_time(&ts); \
  unsigned long elapsed = shoes_diff_time(&(last), &ts); \
  if (s != SHOES_HTTP_TRANSFER || elapsed > 600 ) { \
    shoes_download_event event; \
    event.stage = s; \
    if (s == SHOES_HTTP_COMPLETED) event.stage = SHOES_HTTP_TRANSFER; \
    event.percent = perc; \
    event.transferred = trans;\
    event.total = tot; \
    last = ts; \
    if (handler != NULL && (handler(&event, dat) & SHOES_DOWNLOAD_HALT)) \
    { abort; } \
    if (s == SHOES_HTTP_COMPLETED) { event.stage = s; \
      event.body = bd; \
      if (handler != NULL && (handler(&event, dat) & SHOES_DOWNLOAD_HALT)) \
      { abort; } \
    } \
  } }

typedef struct {
  unsigned char stage;
  unsigned long status;
  char *hkey, *hval, *body;
  unsigned long hkeylen, hvallen;
  unsigned LONG_LONG total;
  unsigned LONG_LONG transferred;
  unsigned long percent;
  SHOES_DOWNLOAD_ERROR error;
} shoes_download_event;

typedef struct {
  unsigned long status;
  char *filepath, *uripath, *etag;
  char hexdigest[41];
  VALUE slot;
} shoes_image_download_event;

typedef int (*shoes_download_handler)(shoes_download_event *, void *);

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
} shoes_download_request;

void shoes_download(shoes_download_request *req);
void shoes_queue_download(shoes_download_request *req);
VALUE shoes_http_error(SHOES_DOWNLOAD_ERROR error);
SHOES_DOWNLOAD_HEADERS shoes_http_headers(VALUE hsh);

#ifdef SHOES_WIN32
#include <stdio.h>
#include <windows.h>
#include <winhttp.h>
void shoes_winhttp(LPCWSTR, INTERNET_PORT, LPCWSTR, TCHAR **, ULONG, HANDLE, LPDWORD, shoes_download_handler, void *);
#define HTTP_HANDLER(x) reinterpret_cast<shoes_download_handler>(x)
#else
#define HTTP_HANDLER(x)
#endif
