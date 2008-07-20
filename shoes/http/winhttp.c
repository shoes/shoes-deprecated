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
  WCHAR _host[MAX_PATH];
  WCHAR _path[MAX_PATH];
  DWORD _size;
  MultiByteToWideChar(CP_UTF8, 0, req->host, -1, _host, MAX_PATH);
  MultiByteToWideChar(CP_UTF8, 0, req->path, -1, _path, MAX_PATH);

  if (req->mem == NULL && req->filepath != NULL)
    file = CreateFile(req->filepath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  shoes_winhttp(_host, _port, _path, req->mem, file, &_size, req->handler, req->data);
  req->size = _size;
  if (file != INVALID_HANDLE_VALUE)
    CloseHandle(file);
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
  HANDLE th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)shoes_download2, (void *)req, 0, &tid);
  if (th != NULL)
  {
    MSG msg;
    while (WaitForSingleObject(th, 10) != WAIT_OBJECT_0)
    {
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    CloseHandle(th);
  }
}

void
shoes_winhttp(LPCWSTR host, INTERNET_PORT port, LPCWSTR path, TCHAR *mem, HANDLE file,
  LPDWORD size, shoes_download_handler handler, void *data)
{
  DWORD len = 0, rlen = 0, status = 0;
  TCHAR buf[SHOES_BUFSIZE];
  WCHAR uagent[SHOES_BUFSIZE];
  HINTERNET sess = NULL, conn = NULL, req = NULL;
  SHOES_TIME last = 0;

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

  if (status != 200)
    goto done;

  len = sizeof(buf);
  if (!WinHttpQueryHeaders(req, WINHTTP_QUERY_CONTENT_LENGTH,
    NULL, buf, &len, NULL))
    goto done;

  *size = _wtoi((wchar_t *)buf);
  HTTP_EVENT(handler, SHOES_HTTP_CONNECTED, last, 0, 0, *size, data, goto done);

  if (mem != NULL)
  {
    WinHttpReadData(req, mem, SHOES_BUFSIZE, &len);
    mem[len] = '\0';
  }

  if (file != INVALID_HANDLE_VALUE)
  {
    TCHAR fbuf[SHOES_CHUNKSIZE];
    DWORD flen = 0, total = *size * 100;
    rlen = *size;
    while (rlen > 0)
    {
      WinHttpReadData(req, fbuf, SHOES_CHUNKSIZE, &len);
      WriteFile(file, (LPBYTE)fbuf, len, &flen, NULL);

      HTTP_EVENT(handler, SHOES_HTTP_TRANSFER, last, (int)((total - (rlen * 100)) / *size),
                 *size - rlen, *size, data, break);
      rlen -= len;
    }
  }

  HTTP_EVENT(handler, SHOES_HTTP_COMPLETED, last, 100, *size, *size, data, goto done);

done:
  if (req)
    WinHttpCloseHandle(req);

  if (conn)
    WinHttpCloseHandle(conn);

  if (sess)
    WinHttpCloseHandle(sess);
}
