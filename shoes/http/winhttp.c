//
// shoes/http/winhttp.c
// the downloader routines using windows' winhttp lib.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/http.h"
#include "shoes/version.h"
#include <shellapi.h>
#include <wchar.h>

void
shoes_download(char *host, int port, char *path, char *mem, char *filepath,
  unsigned long long *size, shoes_download_handler handler, void *data)
{
  HANDLE file = INVALID_HANDLE_VALUE;
  INTERNET_PORT _port = port;
  WCHAR _host[MAX_PATH];
  WCHAR _path[MAX_PATH];
  DWORD _size;
  MultiByteToWideChar(CP_UTF8, 0, host, -1, _host, MAX_PATH);
  MultiByteToWideChar(CP_UTF8, 0, path, -1, _path, MAX_PATH);

  if (mem == NULL && filepath != NULL)
    file = CreateFile(filepath, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  shoes_winhttp(_host, _port, _path, mem, file, &_size, handler, data);
  *size = _size;
  if (file != INVALID_HANDLE_VALUE)
    CloseHandle(file);
}

void
shoes_winhttp(LPCWSTR host, INTERNET_PORT port, LPCWSTR path, TCHAR *mem, HANDLE file,
  LPDWORD size, shoes_download_handler handler, void *data)
{
  shoes_download_event event;
  DWORD len = 0, rlen = 0, status = 0;
  TCHAR buf[SHOES_BUFSIZE];
  WCHAR uagent[SHOES_BUFSIZE];
  HINTERNET sess = NULL, conn = NULL, req = NULL;

  _snwprintf(uagent, SHOES_BUFSIZE, L"ShoeStub/0.r%d (%S) %S/%d", SHOES_REVISION, SHOES_PLATFORM,
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

  if (mem != NULL)
  {
    TCHAR *nl;
    WinHttpReadData(req, mem, SHOES_BUFSIZE, &len);
    mem[min(SHOES_BUFSIZE - 1, *size)] = '\0';
    nl = strstr(mem, "\n");
    if (nl) nl[0] = '\0';
  }

  if (file != NULL)
  {
    TCHAR fbuf[SHOES_CHUNKSIZE];
    DWORD flen = 0, total = *size * 100;
    rlen = *size;
    while (rlen > 0)
    {
      WinHttpReadData(req, fbuf, SHOES_CHUNKSIZE, &len);
      WriteFile(file, (LPBYTE)fbuf, len, &flen, NULL);

      event.percent = (int)((total - (rlen * 100)) / *size);
      event.total = *size;
      event.transferred = *size - rlen;
      if (handler != NULL && (handler(&event, data) & SHOES_DOWNLOAD_HALT))
        break;

      rlen -= SHOES_CHUNKSIZE;
    }
  }
done:
  if (req)
    WinHttpCloseHandle(req);

  if (conn)
    WinHttpCloseHandle(conn);

  if (sess)
    WinHttpCloseHandle(sess);
}
