#include <stdio.h>
#include <windows.h>
#include <shellapi.h>

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
{
  HRSRC res;
  res = FindResource(inst, "SHOES_PAYLOAD", RT_RCDATA);
  if (res != NULL)
  {
    HANDLE payload = CreateFile("app.rb", GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    DWORD rlen = 0;
    DWORD len = SizeofResource(inst, res);
    HGLOBAL resdata = LoadResource(inst, res);
    LPVOID data = NULL;
    data = LockResource(resdata);
    SetFilePointer(payload, 0, 0, FILE_BEGIN);
    WriteFile(payload, (LPBYTE)data, len, &rlen, NULL);
    CloseHandle(payload);

    ShellExecute(NULL, "open", "C:\\Program Files\\Common Files\\Shoes\\shoes.bat", "app.rb", NULL, SW_SHOWNORMAL);
  }
}
