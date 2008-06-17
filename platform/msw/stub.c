#include <stdio.h>
#include <windows.h>
#include <winhttp.h>
#include <shellapi.h>
#include <wchar.h>
#include "stub32.h"

#define BUFSIZE 512
#define CHUNKSIZE 16384

HWND dlg;

BOOL CALLBACK
stub_win32proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDCANCEL)
      {
        return TRUE;
      }
      break;

    case WM_CLOSE:
      return FALSE;
  }
  return FALSE;
}

void
ShoesWinHttp(LPCWSTR host, INTERNET_PORT port, LPCWSTR path, TCHAR *mem, HANDLE file, LPDWORD size)
{
  DWORD len = 0, rlen = 0, status = 0;
  TCHAR buf[BUFSIZE];
  HINTERNET sess = NULL, conn = NULL, req = NULL;

  sess = WinHttpOpen( L"ShoeStub/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
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
    WinHttpReadData(req, mem, BUFSIZE, &len);
    mem[min(BUFSIZE - 1, *size)] = '\0';
    nl = strstr(mem, "\n");
    if (nl) nl[0] = '\0';
  }

  if (file != NULL)
  {
    TCHAR fbuf[CHUNKSIZE], msg[512];
    DWORD flen = 0, total = *size * 100;
    int perc;
    rlen = *size;
    while (rlen > 0)
    {
      WinHttpReadData(req, fbuf, CHUNKSIZE, &len);
      WriteFile(file, (LPBYTE)fbuf, len, &flen, NULL);
      perc = (int)((total - (rlen * 100)) / *size);

      sprintf(msg, "Shoes is downloading. (%d%% done)", perc);
      SetDlgItemText(dlg, IDSHOE, msg);
      SendMessage(GetDlgItem(dlg, IDPROG), PBM_SETPOS, perc, 0L);
      rlen -= CHUNKSIZE;
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

DWORD WINAPI
shoes_download(IN DWORD mid, IN WPARAM w, LPARAM &l, IN LPVOID data)
{
  DWORD len = 0;
  WCHAR path[BUFSIZE];
  TCHAR buf[BUFSIZE];
  HANDLE file;
  SHELLEXECUTEINFO shell = {0};

  ShoesWinHttp(L"hackety.org", 53045, L"/shoes.txt", buf, NULL, &len);
  if (len == 0)
    return 0;

  len = 0;
  MultiByteToWideChar(CP_ACP, 0, buf, -1, path, BUFSIZE);
  file = CreateFile("setup.exe", GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  ShoesWinHttp(L"code.whytheluckystiff.net", 80, path, NULL, file, &len);
  CloseHandle(file);

  SetDlgItemText(dlg, IDSHOE, "Setting up Shoes...");
  shell.cbSize = sizeof(SHELLEXECUTEINFO);
  shell.fMask = SEE_MASK_NOCLOSEPROCESS;
  shell.hwnd = NULL;
  shell.lpVerb = NULL;
  shell.lpFile = "setup.exe";    
  shell.lpParameters = "/S"; 
  shell.lpDirectory = NULL;
  shell.nShow = SW_SHOW;
  shell.hInstApp = NULL; 
  ShellExecuteEx(&shell);
  WaitForSingleObject(shell.hProcess,INFINITE);

  return 0;
}

static BOOL
reg_s(HKEY key, char* sub_key, char* val, LPBYTE data, LPDWORD data_len) {
  HKEY hkey;
  BOOL ret = FALSE;
  LONG retv;

  retv = RegOpenKeyEx(key, sub_key, 0, KEY_QUERY_VALUE, &hkey);
  if (retv == ERROR_SUCCESS)
  {
    retv = RegQueryValueEx(hkey, val, NULL, NULL, data, data_len);
    if (retv == ERROR_SUCCESS)
      return TRUE;
  }
  return FALSE;
}

int WINAPI
WinMain(HINSTANCE inst, HINSTANCE inst2, LPSTR arg, int style)
{
  HRSRC res;
  DWORD len = 0, rlen = 0, tid = 0;
  LPVOID data = NULL;
  TCHAR buf[BUFSIZE], path[BUFSIZE], cmd[BUFSIZE];
  HKEY hkey;
  BOOL shoes;
  DWORD plen;
  HANDLE payload, th;
  MSG msg;
  char *key = "SOFTWARE\\Hackety.org\\Shoes";

  plen = sizeof(path);
  if (!(shoes = reg_s((hkey=HKEY_LOCAL_MACHINE), key, "", (LPBYTE)&path, &plen)))
    shoes = reg_s((hkey=HKEY_CURRENT_USER), key, "", (LPBYTE)&path, &plen);

  if(!shoes)
  {
    INITCOMMONCONTROLSEX InitCtrlEx;
    InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&InitCtrlEx);

    dlg = CreateDialog(inst, MAKEINTRESOURCE(ASKDLG), NULL, stub_win32proc);
    ShowWindow(dlg, SW_SHOW);

    if (!(th = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)shoes_download, NULL, 0, &tid)))
      return 0;

    while (WaitForSingleObject(th, 10) != WAIT_OBJECT_0)   
    {       
        while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))         
        {            
            TranslateMessage(&msg);           
            DispatchMessage(&msg);         
        }        
    }
    CloseHandle(th);

    if (!(shoes = reg_s((hkey=HKEY_LOCAL_MACHINE), key, "", (LPBYTE)&path, &plen)))
      shoes = reg_s((hkey=HKEY_CURRENT_USER), key, "", (LPBYTE)&path, &plen);
  }

  if (shoes)
  {
    GetTempPath(BUFSIZE, buf);
    res = FindResource(inst, "SHOES_FILENAME", RT_STRING);
    if (res == NULL)
      return 1;
    data = LoadResource(inst, res);
    len = SizeofResource(inst, res);
    strncat(buf, (LPTSTR)data, len);

    res = FindResource(inst, "SHOES_PAYLOAD", RT_RCDATA);
    if (res == NULL)
      return 1;

    payload = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    len = SizeofResource(inst, res);
    HGLOBAL resdata = LoadResource(inst, res);
    data = LockResource(resdata);
    SetFilePointer(payload, 0, 0, FILE_BEGIN);
    WriteFile(payload, (LPBYTE)data, len, &rlen, NULL);
    CloseHandle(payload);

    sprintf(cmd, "%s\\..\\shoes.bat", path);
    ShellExecute(NULL, "open", cmd, buf, NULL, 0);
  }

  return 0;
}
