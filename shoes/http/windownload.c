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
#include "shoes/native/native.h"
#include <shellapi.h>
#include <wchar.h>
#include <time.h>
#ifdef SHOES_GTK_WIN32
#include <winhttp.h>
#endif

void
shoes_download(shoes_http_request *req)
{
  HANDLE file = INVALID_HANDLE_VALUE;
  INTERNET_PORT _port = req->port;
  LPWSTR _method = NULL, _body = NULL;
  LPWSTR _scheme = SHOE_ALLOC_N(WCHAR, MAX_PATH);
  LPWSTR _host = SHOE_ALLOC_N(WCHAR, MAX_PATH);
  LPWSTR _path = SHOE_ALLOC_N(WCHAR, MAX_PATH);
  DWORD _size;
  MultiByteToWideChar(CP_UTF8, 0, req->scheme, -1, _scheme, MAX_PATH);
  MultiByteToWideChar(CP_UTF8, 0, req->host, -1, _host, MAX_PATH);
  MultiByteToWideChar(CP_UTF8, 0, req->path, -1, _path, MAX_PATH);
  if (req->method != NULL)
  {
    _method = SHOE_ALLOC_N(WCHAR, MAX_PATH);
    MultiByteToWideChar(CP_UTF8, 0, req->method, -1, _method, MAX_PATH);
  }

  if (req->mem == NULL && req->filepath != NULL)
    file = CreateFile(req->filepath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  shoes_winhttp(_scheme, _host, _port, _path, _method, req->headers, (LPVOID)req->body, req->bodylen,
               &req->mem, req->memlen, file, &_size, req->flags, req->handler, req->data);
  req->size = _size;
  SHOE_FREE(_scheme);
  SHOE_FREE(_host);
  SHOE_FREE(_path);
  if (_method != NULL) SHOE_FREE(_method);
}

DWORD WINAPI
shoes_download2(LPVOID data)
{
  shoes_http_request *req = (shoes_http_request *)data;
  shoes_download(req);
  shoes_http_request_free(req);
  free(req);
  return TRUE;
}

void
shoes_queue_download(shoes_http_request *req)
{
  DWORD tid;
  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)shoes_download2, (void *)req, 0, &tid);
}

VALUE
shoes_http_err(SHOES_DOWNLOAD_ERROR code)
{
  TCHAR msg[1024];
  DWORD msglen;
  if (code > 12000 && code <= 12174)
  {
    msglen = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
      GetModuleHandle("WINHTTP.DLL"), code, 0, msg, sizeof(msg), NULL);
  }
  else
  {
    msglen = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, code, 0, msg, sizeof(msg), NULL);
  }
  msg[msglen] = '\0';
  return rb_str_new2(msg);
}

SHOES_DOWNLOAD_HEADERS
shoes_http_headers(VALUE hsh)
{
  long i;
  LPWSTR hdrs = NULL;
  VALUE keys = rb_funcall(hsh, s_keys, 0);
  if (RARRAY_LEN(keys) > 0)
  {
    VALUE headers = rb_str_new2("");
    //for (i = 0; i < RARRAY(keys)->as.heap.len; i++ )
    for (i = 0; i < RARRAY_LEN(keys); i++ )
    {
      VALUE key = rb_ary_entry(keys, i);
      rb_str_append(headers, key);
      rb_str_cat2(headers, ": ");
      rb_str_append(headers, rb_hash_aref(hsh, key));
      rb_str_cat2(headers, "\n");
    }

    hdrs = SHOE_ALLOC_N(WCHAR, RSTRING_LEN(headers) + 1);
    SHOE_MEMZERO(hdrs, WCHAR, RSTRING_LEN(headers) + 1);
    MultiByteToWideChar(CP_UTF8, 0, RSTRING_PTR(headers), -1, hdrs, RSTRING_LEN(headers) + 1);
  }
  return hdrs;
}

void
shoes_http_headers_free(SHOES_DOWNLOAD_HEADERS headers)
{
  SHOE_FREE(headers);
}
