//
// shoes/http/winhttp.c
// the central download routine
// (isolated so it can be reused in platform/msw/stub.c)
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/config.h"
#include "shoes/version.h"
#include "shoes/http/common.h"
#include "shoes/http/wininet.h"

void shoes_get_time(SHOES_TIME *ts)
{
  *ts = GetTickCount();
}

unsigned long shoes_diff_time(SHOES_TIME *start, SHOES_TIME *end)
{
  return *end - *start;
}

void
shoes_winhttp_headers(HINTERNET req, shoes_http_handler handler, void *data)
{ 
  return;
}

void
shoes_winhttp(LPCWSTR scheme, LPCWSTR host, INTERNET_PORT port, LPCWSTR path, LPCWSTR method,
  LPCWSTR headers, LPVOID body, DWORD bodylen, TCHAR **mem, ULONG memlen, HANDLE file,
  LPDWORD size, UCHAR flags, shoes_http_handler handler, void *data)
{
  LPWSTR proxy;
  DWORD len = 0, rlen = 0, status = 0, complete = 0, flen = 0, total = 0, written = 0;
  LPTSTR buf = SHOE_ALLOC_N(TCHAR, SHOES_BUFSIZE);
  LPTSTR fbuf = SHOE_ALLOC_N(TCHAR, SHOES_CHUNKSIZE);
  LPWSTR uagent = SHOE_ALLOC_N(WCHAR, SHOES_BUFSIZE);
  HINTERNET sess = NULL, conn = NULL, req = NULL;
  SHOES_TIME last = 0;
  LPCTSTR szFilterTypes[] = {L"*/*", NULL};

  if (buf == NULL || fbuf == NULL || uagent == NULL)
    goto done;

  _snwprintf(uagent, SHOES_BUFSIZE, L"Shoes/0.r%d (%S) %S/%d", SHOES_REVISION, SHOES_PLATFORM,
    SHOES_RELEASE_NAME, SHOES_BUILD_DATE);
  sess = InternetOpen(uagent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
  if (sess == NULL)
    goto done;

  conn = InternetConnect(sess, host, port, "", "", INTERNET_SERVICE_HTTP, 0, 0);
  if (conn == NULL)
    goto done;

  if (method == NULL) method = L"GET";
  req = HttpOpenRequest(conn, method, path,
    NULL, NULL, szFilterTypes, INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_PRAGMA_NOCACHE, 0);
  if (req == NULL)
    goto done;
/*
  proxy = _wgetenv(L"http_proxy");
  if (proxy != NULL)
  {
    //FIXME -- Needs to be moved to wininet instead of winhttp
    WINHTTP_PROXY_INFO proxy_info;
    proxy_info.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
    proxy_info.lpszProxy = proxy;
    proxy_info.lpszProxyBypass = NULL;
    WinHttpSetOption(req, WINHTTP_OPTION_PROXY, &proxy_info, sizeof(proxy_info));
  }

  if (!(flags & SHOES_DL_REDIRECTS))
  {
    DWORD options = WINHTTP_DISABLE_REDIRECTS;
    WinHttpSetOption(req, WINHTTP_OPTION_DISABLE_FEATURE, &options, sizeof(options));
  }

  if (headers != NULL)
    WinHttpAddRequestHeaders(req, headers, -1L, WINHTTP_ADDREQ_FLAG_ADD | WINHTTP_ADDREQ_FLAG_REPLACE);
*/
  if (!HttpSendRequest(req, NULL, 0, (LPVOID)body, bodylen))
    goto done;

//  if (!WinHttpReceiveResponse(req, NULL))
//    goto done;

  len = sizeof(DWORD);
  if (!HttpQueryInfo(req, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status, &len, NULL))
    goto done;
  else if (handler != NULL)
  {
    shoes_http_event *event = SHOE_ALLOC(shoes_http_event);
    SHOE_MEMZERO(event, shoes_http_event, 1);
    event->stage = SHOES_HTTP_STATUS;
    event->status = status;
    handler(event, data);
    SHOE_FREE(event);
  }

//  if (handler != NULL) shoes_winhttp_headers(req, handler, data);

  *size = 0;
  len = sizeof(buf);
  if (HttpQueryInfo(req, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
    size, &len, NULL));
  {
    if (*mem != NULL && *size > memlen)
    {
      SHOE_REALLOC_N(*mem, char, (memlen = *size));
      if (*mem == NULL) goto done;
    }
  }

  HTTP_EVENT(handler, SHOES_HTTP_CONNECTED, last, 0, 0, *size, data, NULL, goto done);

  total = *size * 100;
  rlen = *size;
  while (1)
  {
    len = 0;
    InternetReadFile(req, fbuf, SHOES_CHUNKSIZE, &len);
    if (len <= 0)
      break;

    if (*mem != NULL)
    {
      if (written + len > memlen)
      {
        while (written + len > memlen)
          memlen += SHOES_BUFSIZE;
        SHOE_REALLOC_N(*mem, char, memlen);
        if (*mem == NULL) goto done;
      }
      SHOE_MEMCPY(*mem + written, fbuf, char, len);
    }
    if (file != INVALID_HANDLE_VALUE)
      WriteFile(file, (LPBYTE)fbuf, len, &flen, NULL);
    written += len;

    if (*size == 0) total = written * 100;
    if (total > 0)
    {
      HTTP_EVENT(handler, SHOES_HTTP_TRANSFER, last, (int)((total - (rlen * 100)) / (total / 100)),
                 (total / 100) - rlen, (total / 100), data, NULL, break);
    }

    if (rlen > len) rlen -= len;
  }

  *size = written;

  if (file != INVALID_HANDLE_VALUE)
  {
    CloseHandle(file);
    file = INVALID_HANDLE_VALUE;
  }

  HTTP_EVENT(handler, SHOES_HTTP_COMPLETED, last, 100, *size, *size, data, *mem, goto done);
  complete = 1;

done:
  if (buf != NULL)    SHOE_FREE(buf);
  if (fbuf != NULL)   SHOE_FREE(fbuf);
  if (uagent != NULL) SHOE_FREE(uagent);

  if (!complete)
  {
    shoes_http_event *event = SHOE_ALLOC(shoes_http_event);
    SHOE_MEMZERO(event, shoes_http_event, 1);
    event->stage = SHOES_HTTP_ERROR;
    event->error = GetLastError();
    int hx = handler(event, data);
    SHOE_FREE(event);
    if (hx & SHOES_DOWNLOAD_HALT) goto done;
  }

  if (file != INVALID_HANDLE_VALUE)
    CloseHandle(file);

  if (req)
    InternetCloseHandle(req);

  if (conn)
    InternetCloseHandle(conn);

  if (sess)
    InternetCloseHandle(sess);
}

