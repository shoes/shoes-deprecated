//
// shoes/internal.c
// Internal debug functions.
//
#ifdef SHOES_WIN32
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <windows.h>

void odprintf(const char *format, ...)
{
  char    buf[4096], *p = buf;
  va_list args;
  int     n;

  va_start(args, format);
  n = _vsnprintf(p, sizeof buf - 3, format, args); // buf-3 is room for CR/LF/NUL
  va_end(args);

  p += (n < 0) ? sizeof buf - 3 : n;

  while(p > buf && isspace(p[-1]))
    *--p = '\0';

  *p++ = '\r';
  *p++ = '\n';
  *p   = '\0';

  OutputDebugString(buf);
}
#endif
