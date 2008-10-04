//
// shoes/http/windownload.c
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
  shoes_winhttp(_host, _port, _path, &req->mem, req->memlen, file, 
               &_size, req->flags, req->handler, req->data);
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
