//
// shoes/http/winhttp.h
// the shoes_winhttp function, used by platform/msw/stub.c.
//
#ifndef SHOES_HTTP_WINHTTP_H
#define SHOES_HTTP_WINHTTP_H

#include <stdio.h>
#include <windows.h>
#include <wchar.h>
#include <wininet.h>
#include <shellapi.h>
#include "shoes/http/common.h"

#define HTTP_HANDLER(x) reinterpret_cast<shoes_http_handler>(x)

void shoes_winhttp(LPCWSTR, LPCWSTR, INTERNET_PORT, LPCWSTR, LPCWSTR, LPCWSTR, LPVOID, DWORD, TCHAR **, ULONG, HANDLE, LPDWORD, UCHAR, shoes_http_handler, void *);

#endif
