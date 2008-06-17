#include <stdio.h>
#include <windows.h>

#define BUFSIZE 512

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
{
  LPBYTE data = NULL;
  HANDLE payload = CreateFile(arg, GENERIC_READ, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (payload != INVALID_HANDLE_VALUE)
  {
    DWORD rlen = 0;
    TCHAR outPath[BUFSIZE];
    DWORD len = GetFileSize(payload, NULL);
    data = (LPBYTE)LocalAlloc(LPTR, len + 1);
    SetFilePointer(payload, 0, 0, FILE_BEGIN);
    ReadFile(payload, data, len, &rlen, NULL);
    CloseHandle(payload);

    LPTSTR fname = strrchr(arg, '\\');
    if (fname == NULL)
      fname = arg;
    else
      fname += 1;

    strcpy(outPath, arg);
    LPSTR ext = strrchr(outPath, '.');
    if (ext != NULL)
      *ext = '\0';
    strcat(outPath, ".exe");

    CopyFile("shoes-stub.exe", outPath, FALSE);
    HANDLE exe = BeginUpdateResource(outPath, FALSE);
    UpdateResource(exe, RT_STRING, "SHOES_FILENAME", 
      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), fname, strlen(fname));
    UpdateResource(exe, RT_RCDATA, "SHOES_PAYLOAD", 
      MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), data, rlen);

    payload = CreateFile("shoes-setup.exe", GENERIC_READ, FILE_SHARE_READ,
      NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (payload != INVALID_HANDLE_VALUE)
    {
      LPBYTE setup = NULL;
      DWORD setuplen = 0;
      len = GetFileSize(payload, NULL);
      setup = (LPBYTE)LocalAlloc(LPTR, len + 1);
      SetFilePointer(payload, 0, 0, FILE_BEGIN);
      ReadFile(payload, setup, len, &setuplen, NULL);
      CloseHandle(payload);
      UpdateResource(exe, RT_RCDATA, "SHOES_SETUP", 
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), setup, setuplen);
    }
    EndUpdateResource(exe, FALSE);
    LocalFree(data);
    LocalFree(fname);
  }
}
