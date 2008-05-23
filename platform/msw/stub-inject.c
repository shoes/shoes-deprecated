#include <stdio.h>
#include <windows.h>

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
{
  LPBYTE data = NULL;
  HANDLE payload = CreateFile("app.rb", GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  DWORD rlen = 0;
  DWORD len = GetFileSize(payload, NULL);
  data = (LPBYTE)LocalAlloc(LPTR, len + 1);
  SetFilePointer(payload, 0, 0, FILE_BEGIN);
  ReadFile(payload, data, len, &rlen, NULL);
  CloseHandle(payload);

  MessageBox(NULL, "OK", "OK", MB_ICONINFORMATION);
  HANDLE exe = BeginUpdateResource("shoes-stub.exe", FALSE);
  UpdateResource(exe, RT_RCDATA, "SHOES_PAYLOAD", MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
    data, rlen);
  EndUpdateResource(exe, FALSE);
  LocalFree(data);
}
