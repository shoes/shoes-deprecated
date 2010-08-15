#include <stdio.h>
#include <stdlib.h>

#define LONG_LONG long long
#define SHOES_TIME DWORD
#define SHOES_DOWNLOAD_ERROR DWORD

#include "shoes/version.h"
#include "shoes/internal.h"
#include "shoes/http/winhttp.h"
#include "stub32.h"

#define BUFSIZE 512

HWND dlg;
BOOL http_abort = FALSE;

int
StubDownloadingShoes(shoes_http_event *event, void *data)
{
  TCHAR msg[512];
  if (http_abort) return SHOES_DOWNLOAD_HALT;
  if (event->stage == SHOES_HTTP_TRANSFER)
  {
    sprintf(msg, "Shoes is downloading. (%d%% done)", event->percent);
    SetDlgItemText(dlg, IDSHOE, msg);
    SendMessage(GetDlgItem(dlg, IDPROG), PBM_SETPOS, event->percent, 0L);
  }
  return SHOES_DOWNLOAD_CONTINUE;
}

void
CenterWindow(HWND hwnd)
{
  RECT rc;
  
  GetWindowRect (hwnd, &rc);
  
  SetWindowPos(hwnd, 0, 
    (GetSystemMetrics(SM_CXSCREEN) - rc.right)/2,
    (GetSystemMetrics(SM_CYSCREEN) - rc.bottom)/2,
     0, 0, SWP_NOZORDER|SWP_NOSIZE );
}

BOOL CALLBACK
stub_win32proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      CenterWindow(hwnd);
      return TRUE;

    case WM_COMMAND:
      if (LOWORD(wParam) == IDCANCEL)
      {
        http_abort = TRUE;
        EndDialog(hwnd, LOWORD(wParam));
        return TRUE;
      }
      break;

    case WM_CLOSE:
      http_abort = TRUE;
      EndDialog(hwnd, 0);
      return FALSE;
  }
  return FALSE;
}

void
shoes_silent_install(TCHAR *path)
{
  SHELLEXECUTEINFO shell = {0};
  SetDlgItemText(dlg, IDSHOE, "Setting up Shoes...");
  shell.cbSize = sizeof(SHELLEXECUTEINFO);
  shell.fMask = SEE_MASK_NOCLOSEPROCESS;
  shell.hwnd = NULL;
  shell.lpVerb = NULL;
  shell.lpFile = path;
  shell.lpParameters = "/S"; 
  shell.lpDirectory = NULL;
  shell.nShow = SW_SHOW;
  shell.hInstApp = NULL; 
  ShellExecuteEx(&shell);
  WaitForSingleObject(shell.hProcess,INFINITE);
}

char *setup_exe = "shoes-setup.exe";

DWORD WINAPI
shoes_auto_setup(IN DWORD mid, IN WPARAM w, LPARAM &l, IN LPVOID vinst)
{
  HINSTANCE inst = (HINSTANCE)vinst;
  TCHAR setup_path[BUFSIZE];
  GetTempPath(BUFSIZE, setup_path);
  strncat(setup_path, setup_exe, strlen(setup_exe));

  HANDLE install = CreateFile(setup_path, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
  HRSRC setupres = FindResource(inst, "SHOES_SETUP", RT_RCDATA);
  DWORD len = 0, rlen = 0;
  LPVOID data = NULL;
  len = SizeofResource(inst, setupres);
  if (GetFileSize(install, NULL) != len)
  {
    HGLOBAL resdata = LoadResource(inst, setupres);
    data = LockResource(resdata);
    SetFilePointer(install, 0, 0, FILE_BEGIN);
    SetEndOfFile(install);
    WriteFile(install, (LPBYTE)data, len, &rlen, NULL);
  }
  CloseHandle(install);
  SendMessage(GetDlgItem(dlg, IDPROG), PBM_SETPOS, 50, 0L);

  shoes_silent_install(setup_path);
  return 0;
}

DWORD WINAPI
shoes_http_thread(IN DWORD mid, IN WPARAM w, LPARAM &l, IN LPVOID data)
{
  DWORD len = 0;
  WCHAR path[BUFSIZE];
  TCHAR *buf = SHOE_ALLOC_N(TCHAR, BUFSIZE);
  TCHAR *empty = NULL;
  HANDLE file;
  TCHAR *nl;
  TCHAR setup_path[BUFSIZE];
  GetTempPath(BUFSIZE, setup_path);
  strncat(setup_path, setup_exe, strlen(setup_exe));

  shoes_winhttp(NULL, L"www.rin-shun.com", 80, L"/pkg/win32/shoes",
    NULL, NULL, NULL, 0, &buf, BUFSIZE,
    INVALID_HANDLE_VALUE, &len, SHOES_DL_DEFAULTS, NULL, NULL);
  if (len == 0)
    return 0;

  nl = strstr(buf, "\n");
  if (nl) nl[0] = '\0';

  len = 0;
  MultiByteToWideChar(CP_ACP, 0, buf, -1, path, BUFSIZE);
  file = CreateFile(setup_path, GENERIC_READ | GENERIC_WRITE,
    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  shoes_winhttp(NULL, L"www.rin-shun.com", 80, path,
    NULL, NULL, NULL, 0, &empty, 0, file, &len,
    SHOES_DL_DEFAULTS, HTTP_HANDLER(StubDownloadingShoes), NULL);
  CloseHandle(file);

  shoes_silent_install(setup_path);
  return 0;
}

static BOOL
file_exists(char *fname)
{
  WIN32_FIND_DATA data;
  if (FindFirstFile(fname, &data) != INVALID_HANDLE_VALUE)
    return !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
  return FALSE;
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
  HRSRC nameres, shyres, setupres;
  DWORD len = 0, rlen = 0, tid = 0;
  LPVOID data = NULL;
  TCHAR buf[BUFSIZE], path[BUFSIZE], cmd[BUFSIZE];
  HKEY hkey;
  BOOL shoes;
  DWORD plen;
  HANDLE payload, th;
  MSG msg;
  char *key = "SOFTWARE\\Hackety.org\\Shoes";

  nameres = FindResource(inst, "SHOES_FILENAME", RT_STRING);
  shyres = FindResource(inst, "SHOES_PAYLOAD", RT_RCDATA);
  if (nameres == NULL || shyres == NULL)
  {
    MessageBox(NULL, "This is an empty Shoes stub.", "shoes!! feel yeah!!", MB_OK);
    return 0;
  }

  setupres = FindResource(inst, "SHOES_SETUP", RT_RCDATA);
  plen = sizeof(path);
  if (!(shoes = reg_s((hkey=HKEY_LOCAL_MACHINE), key, "", (LPBYTE)&path, &plen)))
    shoes = reg_s((hkey=HKEY_CURRENT_USER), key, "", (LPBYTE)&path, &plen);

  if (shoes)
  {
    sprintf(cmd, "%s\\shoes.exe", path);
    if (!file_exists(cmd)) shoes = FALSE;
    memset(cmd, 0, BUFSIZE);
  }

  if (!shoes)
  {
    LPTHREAD_START_ROUTINE back_action = (LPTHREAD_START_ROUTINE)shoes_auto_setup;

    INITCOMMONCONTROLSEX InitCtrlEx;
    InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&InitCtrlEx);

    dlg = CreateDialog(inst, MAKEINTRESOURCE(ASKDLG), NULL, stub_win32proc);
    ShowWindow(dlg, SW_SHOW);

    if (setupres == NULL)
      back_action = (LPTHREAD_START_ROUTINE)shoes_http_thread;

    if (!(th = CreateThread(0, 0, back_action, inst, 0, &tid)))
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
    data = LoadResource(inst, nameres);
    len = SizeofResource(inst, nameres);
    strncat(buf, (LPTSTR)data, len);

    payload = CreateFile(buf, GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    len = SizeofResource(inst, shyres);
    if (GetFileSize(payload, NULL) != len)
    {
      HGLOBAL resdata = LoadResource(inst, shyres);
      data = LockResource(resdata);
      SetFilePointer(payload, 0, 0, FILE_BEGIN);
      SetEndOfFile(payload);
      WriteFile(payload, (LPBYTE)data, len, &rlen, NULL);
    }
    CloseHandle(payload);

    sprintf(cmd, "%s\\..\\shoes.bat", path);
    ShellExecute(NULL, "open", cmd, buf, NULL, 0);
  }

  return 0;
}
