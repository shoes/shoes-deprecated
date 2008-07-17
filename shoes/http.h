//
// shoes/http.h
// the shoes downloader, which uses platform code and threads
// to achieve much faster and brain-dead http access.
//
#define SHOES_CHUNKSIZE 16384

#define SHOES_DOWNLOAD_CONTINUE 0
#define SHOES_DOWNLOAD_HALT 1

#define SHOES_HTTP_CONNECTED 5
#define SHOES_HTTP_TRANSFER  10
#define SHOES_HTTP_COMPLETED 15

#define HTTP_EVENT(handler, s, perc, trans, tot, dat, abort) \
{ shoes_download_event event; \
  event.stage = s; \
  event.percent = perc; \
  event.transferred = trans;\
  event.total = tot; \
  if (handler != NULL && (handler(&event, dat) & SHOES_DOWNLOAD_HALT)) \
  { abort; } }

typedef struct {
  unsigned char stage;
  unsigned long long total;
  unsigned long long transferred;
  unsigned int percent;
} shoes_download_event;

typedef int (*shoes_download_handler)(shoes_download_event *, void *);

void shoes_download(char *, int, char *, char *, char *, unsigned long long *, shoes_download_handler, void *);

#ifdef SHOES_WIN32
#include <stdio.h>
#include <windows.h>
#include <winhttp.h>
void shoes_winhttp(LPCWSTR, INTERNET_PORT, LPCWSTR, TCHAR *, HANDLE, LPDWORD, shoes_download_handler, void *);
#define HTTP_HANDLER(x) reinterpret_cast<shoes_download_handler>(x)
#else
#define HTTP_HANDLER(x)
#endif
