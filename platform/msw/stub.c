#include <stdio.h>
#include <windows.h>
#include <shellapi.h>

#define BUFSIZE 512

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
{
  HRSRC res;
  DWORD len = 0, rlen = 0;
  LPVOID data = NULL;
  TCHAR tempPath[BUFSIZE];

  GetTempPath(BUFSIZE, tempPath);
  res = FindResource(inst, "SHOES_FILENAME", RT_STRING);
  if (res == NULL)
    return 1;
  data = LoadResource(inst, res);
  len = SizeofResource(inst, res);
  strncat(tempPath, (LPTSTR)data, len);

  res = FindResource(inst, "SHOES_PAYLOAD", RT_RCDATA);
  if (res == NULL)
    return 1;

  HANDLE payload = CreateFile(tempPath, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  len = SizeofResource(inst, res);
  HGLOBAL resdata = LoadResource(inst, res);
  data = LockResource(resdata);
  SetFilePointer(payload, 0, 0, FILE_BEGIN);
  WriteFile(payload, (LPBYTE)data, len, &rlen, NULL);
  CloseHandle(payload);

  ShellExecute(NULL, "open", "C:\\Program Files\\Common Files\\Shoes\\shoes.bat", tempPath, NULL, 0);
  return 0;
}
