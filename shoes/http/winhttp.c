//
// shoes/http/winhttp.c
// the downloader routines using windows' winhttp lib.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include <shellapi.h>
#include <wchar.h>
#include <time.h>

void
shoes_download(shoes_download_request *req)
{
  HANDLE file = INVALID_HANDLE_VALUE;
  INTERNET_PORT _port = req->port;
  LPWSTR _host = SHOE_ALLOC_N(WCHAR, MAX_PATH);
  LPWSTR _path = SHOE_ALLOC_N(WCHAR, MAX_PATH);
  DWORD _size;
  MultiByteToWideChar(CP_UTF8, 0, req->host, -1, _host, MAX_PATH);
  MultiByteToWideChar(CP_UTF8, 0, req->path, -1, _path, MAX_PATH);

  if (req->mem == NULL && req->filepath != NULL)
    file = CreateFile(req->filepath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  shoes_winhttp(_host, _port, _path, &req->mem, req->memlen, file, &_size, req->handler, req->data);
  req->size = _size;
  SHOE_FREE(_host);
  SHOE_FREE(_path);
}

DWORD WINAPI
shoes_download2(LPVOID data)
{
  shoes_download_request *req = (shoes_download_request *)data;
  shoes_download(req);
  if (req->mem != NULL) free(req->mem);
  if (req->filepath != NULL) free(req->filepath);
  free(req->data);
  free(req);
  return TRUE;
}

void
shoes_queue_download(shoes_download_request *req)
{
  DWORD tid;
  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)shoes_download2, (void *)req, 0, &tid);
}

void
shoes_winhttp_headers(HINTERNET req, shoes_download_handler handler, void *data)
{ 
  DWORD size;
  WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS,
    WINHTTP_HEADER_NAME_BY_INDEX, NULL, &size, WINHTTP_NO_HEADER_INDEX);

  if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    int whdrlen = 0, hdrlen = 0;
    LPCWSTR whdr;
    LPSTR hdr = SHOE_ALLOC_N(CHAR, MAX_PATH);
    LPCWSTR hdrs = SHOE_ALLOC_N(WCHAR, size/sizeof(WCHAR));
    BOOL res = WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS,
      WINHTTP_HEADER_NAME_BY_INDEX, (LPVOID)hdrs, &size, WINHTTP_NO_HEADER_INDEX);
    if (res)
    {
      for (whdr = hdrs; whdr - hdrs < size / sizeof(WCHAR); whdr += whdrlen)
      {
        WideCharToMultiByte(CP_UTF8, 0, whdr, -1, hdr, MAX_PATH, NULL, NULL);
        hdrlen = strlen(hdr);
        HTTP_HEADER(hdr, hdrlen, handler, data);
        whdrlen = wcslen(whdr) + 1;
      }
    }
    SHOE_FREE(hdrs);
    SHOE_FREE(hdr);
  }
}

void
shoes_winhttp(LPCWSTR host, INTERNET_PORT port, LPCWSTR path, TCHAR **mem, ULONG memlen, HANDLE file,
  LPDWORD size, shoes_download_handler handler, void *data)
{
  DWORD len = 0, rlen = 0, status = 0, complete = 0, flen = 0, total = 0, written = 0;
  LPTSTR buf = SHOE_ALLOC_N(TCHAR, SHOES_BUFSIZE);
  LPTSTR fbuf = SHOE_ALLOC_N(TCHAR, SHOES_CHUNKSIZE);
  LPWSTR uagent = SHOE_ALLOC_N(WCHAR, SHOES_BUFSIZE);
  HINTERNET sess = NULL, conn = NULL, req = NULL;
  SHOES_TIME last = 0;

  if (buf == NULL || fbuf == NULL || uagent == NULL)
    goto done;

  _snwprintf(uagent, SHOES_BUFSIZE, L"Shoes/0.r%d (%S) %S/%d", SHOES_REVISION, SHOES_PLATFORM,
    SHOES_RELEASE_NAME, SHOES_BUILD_DATE);
  sess = WinHttpOpen(uagent, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
    WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
  if (sess == NULL)
    goto done;

  conn = WinHttpConnect(sess, host, port, 0);
  if (conn == NULL)
    goto done;

  req = WinHttpOpenRequest(conn, L"GET", path,
    NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
  if (req == NULL)
    goto done;

  if (!WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
    NULL, 0, 0, 0))
    goto done;

  if (!WinHttpReceiveResponse(req, NULL))
    goto done;

  len = sizeof(DWORD);
  if (!WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
    NULL, &status, &len, NULL))
    goto done;
  else
  {
    shoes_download_event event;
    event.stage = SHOES_HTTP_STATUS;
    event.status = status;
    if (handler != NULL) handler(&event, data);
  }

  shoes_winhttp_headers(req, handler, data);

  *size = 0;
  len = sizeof(buf);
  if (WinHttpQueryHeaders(req, WINHTTP_QUERY_CONTENT_LENGTH, NULL, buf, &len, NULL))
  {
    *size = _wtoi((wchar_t *)buf);
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
    WinHttpReadData(req, fbuf, SHOES_CHUNKSIZE, &len);
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
    shoes_download_event event;
    event.stage = SHOES_HTTP_ERROR;
    event.error = GetLastError();
    if (handler != NULL) handler(&event, data);
  }

  if (file != INVALID_HANDLE_VALUE)
    CloseHandle(file);

  if (req)
    WinHttpCloseHandle(req);

  if (conn)
    WinHttpCloseHandle(conn);

  if (sess)
    WinHttpCloseHandle(sess);
}

VALUE
shoes_http_error(SHOES_DOWNLOAD_ERROR code)
{
  TCHAR msg[1024];
  DWORD msglen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, code, 0, msg, sizeof(msg), NULL);
  msg[msglen] = '\0';
  return rb_str_new2(msg);
}

SHOES_DOWNLOAD_HEADERS
shoes_http_headers(VALUE hsh)
{
  return NULL;
}
