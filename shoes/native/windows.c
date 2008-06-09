//
// shoes/native-carbon.c
// Windows-specific code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

int
shoes_win32_cmdvector(const char *cmdline, char ***argv)
{
  return rb_w32_cmdvector(cmdline, argv);
}

void shoes_native_init()
{
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&InitCtrlEx);
  shoes_classex_init();
}

void shoes_native_cleanup(shoes_world_t *world)
{
}

void shoes_native_quit()
{
  PostQuitMessage(0);
}

void shoes_slot_mark(SHOES_SLOT_OS *slot)
{
  rb_gc_mark_maybe(slot->controls);
  rb_gc_mark_maybe(slot->focus);
}

void shoes_native_slot_reset(SHOES_SLOT_OS *slot)
{
  slot->controls = rb_ary_new();
  rb_gc_register_address(&slot->controls);
}

void shoes_native_slot_clear(SHOES_SLOT_OS *slot)
{
  rb_ary_clear(slot->controls);
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
  RedrawWindow(slot->window, NULL, NULL, RDW_INVALIDATE|RDW_ALLCHILDREN);
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
  SetScrollPos(slot->window, SB_VERT, slot->scrolly, TRUE);
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  return GetSystemMetrics(SM_CXVSCROLL);
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot)
{
  if (c)
  {
    i = rb_ary_index_of(slot->controls, item);
    if (i >= 0)
      rb_ary_insert_at(slot->controls, i, 1, Qnil);
  }
}

//
// Window-level events
//
#define WINDOW_STYLE WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX

#define WM_POINTS() \
  x = LOWORD(l); \
  y = HIWORD(l)

#define KEY_SYM(sym)  shoes_app_keypress(app, ID2SYM(rb_intern("" # sym)))
#define KEYPRESS(name, sym) \
  else if (w == VK_##name) { \
    VALUE v = ID2SYM(rb_intern("" # sym)); \
    if (app->os.altkey) \
      KEY_STATE(alt); \
    if (app->os.shiftkey) \
      KEY_STATE(shift); \
    if (app->os.ctrlkey) \
      KEY_STATE(control); \
    shoes_app_keypress(app, v); \
  }

static void
shoes_canvas_win32_vscroll(shoes_canvas *canvas, int code, int pos)
{
  SCROLLINFO si;
  SHOE_MEMZERO(&si, SCROLLINFO, 1);
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
  GetScrollInfo(canvas->slot.window, SB_VERT, &si);

  switch (code)
  {
    case SB_LINEUP:
      si.nPos -= 16;
    break;
    case SB_LINEDOWN:
      si.nPos += 16;
    break;
    case SB_PAGEUP:
      si.nPos -= si.nPage - 32;
    break;
    case SB_PAGEDOWN:
      si.nPos += si.nPage - 32;
    break;
    case SB_THUMBTRACK:
      si.nPos = pos;
    break;
    default:
      return;
  }

  if (si.nPos < 0)
    si.nPos = 0;
  else if (si.nPos > (si.nMax - si.nPage))
    si.nPos = si.nMax - si.nPage;

  SetScrollInfo(canvas->slot.window, SB_VERT, &si, TRUE);
  canvas->slot.scrolly = si.nPos;
  InvalidateRect(canvas->slot.window, NULL, TRUE);
}

LRESULT CALLBACK
shoes_slot_win32proc(
  HWND win,
  UINT msg,
  WPARAM w,
  LPARAM l)
{
  shoes_canvas *canvas;
  VALUE c = (VALUE)GetWindowLong(win, GWL_USERDATA);

  if (c != NULL)
  {
    Data_Get_Struct(c, shoes_canvas, canvas);
    int x = 0, y = 0;

    switch (msg)
    {
      case WM_ERASEBKGND:
        return 1;

      case WM_PAINT:
        INFO("WM_PAINT(slot)\n");
        if (c != canvas->app->canvas)
          shoes_canvas_paint(c);
      return 1;

      case WM_VSCROLL:
        shoes_canvas_win32_vscroll(canvas, LOWORD(w), HIWORD(w));
      break;

      case WM_LBUTTONDOWN:
      {
        WM_POINTS();
        shoes_canvas_send_click(c, 1, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_RBUTTONDOWN:
      {
        WM_POINTS();
        shoes_canvas_send_click(c, 2, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_MBUTTONDOWN:
      {
        WM_POINTS();
        shoes_canvas_send_click(c, 3, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_LBUTTONUP:
      {
        WM_POINTS();
        shoes_canvas_send_release(c, 1, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_RBUTTONUP:
      {
        WM_POINTS();
        shoes_canvas_send_release(c, 2, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_MBUTTONUP:
      {
        WM_POINTS();
        shoes_canvas_send_release(c, 3, x, y + canvas->slot.scrolly);
      }
      break;

      case WM_MOUSEMOVE:
      {
        WM_POINTS();
        shoes_canvas_send_motion(c, x, y + canvas->slot.scrolly, Qnil);
      }
      break;

      case WM_ACTIVATE:
        if (LOWORD(w) == WA_INACTIVE)
        {
          int i;
          HWND newFocus = GetFocus();
          for (i = 0; i < RARRAY_LEN(canvas->slot.controls); i++)
          {
            VALUE ctrl = rb_ary_entry(canvas->slot.controls, i);
            if (rb_obj_is_kind_of(ctrl, cNative))
            {
              shoes_control *self_t;
              Data_Get_Struct(ctrl, shoes_control, self_t);
              if (self_t->ref == newFocus)
              {
                canvas->slot.focus = ctrl;
                break;
              }
            }
          }
        }
      break;

      case WM_SETFOCUS:
        if (!NIL_P(canvas->slot.focus))
        {
          shoes_control_focus(canvas->slot.focus);
        }
      break;

      case WM_COMMAND:
        if ((HWND)l)
        {
          switch (HIWORD(w))
          {
            case BN_CLICKED:
            {
              int id = LOWORD(w);
              VALUE control = rb_ary_entry(canvas->slot.controls, id - SHOES_CONTROL1);
              if (!NIL_P(control))
                shoes_control_send(control, s_click);
            }
            break;

            case CBN_SELCHANGE:
            case EN_CHANGE:
            {
              int id = LOWORD(w);
              VALUE control = rb_ary_entry(canvas->slot.controls, id - SHOES_CONTROL1);
              if (!NIL_P(control))
                shoes_control_send(control, s_change);
            }
            break;
          }
        }
      break;
    }
  }
  return DefWindowProc(win, msg, w, l);
}

LRESULT CALLBACK
shoes_app_win32proc(
  HWND win,
  UINT msg,
  WPARAM w,
  LPARAM l)
{
  shoes_app *app = (shoes_app *)GetWindowLong(win, GWL_USERDATA);
  int x = 0, y = 0;

  switch (msg)
  {
    case WM_DESTROY:
      if (shoes_app_remove(app))
        PostQuitMessage(0);
    return 0; 

    case WM_ERASEBKGND:
      return 1;

    //
    // On Windows, I have to ensure the scrollbar's width is added
    // to the client area width.  In Shoes, the toplevel slot size is
    // always obscured by the scrollbar when it appears, rather than
    // resizing the width of the slot.
    //
    case WM_PAINT:
    {
      RECT rect, wrect;
      int scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
      GetClientRect(app->slot.window, &rect);
      GetWindowRect(app->slot.window, &wrect);
      if (wrect.right - wrect.left > rect.right + scrollwidth)
        rect.right += scrollwidth;
      app->width = rect.right;
      app->height = rect.bottom;
      shoes_canvas_size(app->canvas, app->width, app->height);
      INFO("WM_PAINT(app)\n");
      shoes_app_paint(app);
    }
    break;

    case WM_LBUTTONDOWN:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 1, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_RBUTTONDOWN:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 2, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_MBUTTONDOWN:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_click(app, 3, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_LBUTTONUP:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 1, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_RBUTTONUP:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 2, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_MBUTTONUP:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_release(app, 3, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_MOUSEMOVE:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      WM_POINTS();
      shoes_app_motion(app, x, y + canvas->slot.scrolly);
    }
    break;

    case WM_CHAR:
      switch(w)
      {
        case 0x08:
          KEY_SYM(backspace);
        break;

        case 0x09:
          KEY_SYM(tab);
        break;

        case 0x0D:
          shoes_app_keypress(app, rb_str_new2("\n"));
        break;

        default:
        {
          VALUE v;
          WCHAR _str = w;
          CHAR str[10];
          DWORD len = WideCharToMultiByte(CP_UTF8, 0, &_str, 1, (LPSTR)str, 10, NULL, NULL);
          str[len] = '\0';
          v = rb_str_new(str, len);
          shoes_app_keypress(app, v);
        }
      }
    break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (w == VK_CONTROL)
        app->os.ctrlkey = true;
      else if (w == VK_MENU)
        app->os.altkey = true;
      else if (w == VK_SHIFT)
        app->os.shiftkey = true;
      KEYPRESS(PRIOR, page_up)
      KEYPRESS(NEXT, page_down)
      KEYPRESS(HOME, home)
      KEYPRESS(END, end)
      KEYPRESS(LEFT, left)
      KEYPRESS(UP, up)
      KEYPRESS(RIGHT, right)
      KEYPRESS(DOWN, down)
      KEYPRESS(F1, f1)
      KEYPRESS(F2, f2)
      KEYPRESS(F3, f3)
      KEYPRESS(F4, f4)
      KEYPRESS(F5, f5)
      KEYPRESS(F6, f6)
      KEYPRESS(F7, f7)
      KEYPRESS(F8, f8)
      KEYPRESS(F9, f9)
      KEYPRESS(F10, f10)
      KEYPRESS(F11, f11)
      KEYPRESS(F12, f12)
      else if ((w >= 'A' && w <= 'Z') || w == 191 || w == 190) {
        VALUE v;
        char letter = w;
        if (w == 191)
        {
          if (app->os.shiftkey)
            letter = '?';
          else
            letter = '/';
        }
        else if (w == 190)
        {
          if (app->os.shiftkey)
            letter = '>';
          else
            letter = '.';
        }
        else
        {
          if (!app->os.shiftkey)
            letter += 32;
        }
        v = rb_str_new(&letter, 1);
        if (app->os.altkey) {
          KEY_STATE(alt);
          shoes_app_keypress(app, v);
        }
      }
    break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (w == VK_CONTROL)
        app->os.ctrlkey = false;
      else if (w == VK_MENU)
        app->os.altkey = false;
      else if (w == VK_SHIFT)
        app->os.shiftkey = false;
    break;

    case WM_MOUSEWHEEL:
    {
      shoes_canvas *canvas;
      int lines = 0, scode = 0;
      int notch = ((int)w >> 16) / WHEEL_DELTA;
      SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &lines, 0);
      if (lines == WHEEL_PAGESCROLL)
        scode = (int)w < 0 ? SB_PAGEDOWN : SB_PAGEUP;
      else
      {
        scode = (int)w < 0 ? SB_LINEDOWN : SB_LINEUP;
        notch *= lines;
      }

      INFO("WM_MOUSEWHEEL: %d (%d, %d) %lu\n", w, scode, notch, lines);
      notch = abs(notch);
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      while (notch--)
        shoes_canvas_win32_vscroll(canvas, scode, 0);
    }
    break;

    case WM_VSCROLL:
    {
      shoes_canvas *canvas;
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      shoes_canvas_win32_vscroll(canvas, LOWORD(w), HIWORD(w));
    }
    break;

    case WM_TIMER:
    {
      int id = LOWORD(w);
      VALUE timer = rb_ary_entry(app->timers, id - SHOES_CONTROL1);
      if (!NIL_P(timer))
      {
        if (rb_obj_is_kind_of(timer, cTimer))
          KillTimer(win, id);
        shoes_timer_call(timer);
      }
    }
    break;

    case WM_ACTIVATE:
      if (LOWORD(w) == WA_INACTIVE)
      {
        int i;
        HWND newFocus = GetFocus();
        for (i = 0; i < RARRAY_LEN(app->slot.controls); i++)
        {
          VALUE ctrl = rb_ary_entry(app->slot.controls, i);
          if (rb_obj_is_kind_of(ctrl, cNative))
          {
            shoes_control *self_t;
            Data_Get_Struct(ctrl, shoes_control, self_t);
            if (self_t->ref == newFocus)
            {
              app->slot.focus = ctrl;
              break;
            }
          }
        }
      }
    break;

    case WM_SETFOCUS:
      if (!NIL_P(app->slot.focus))
      {
        shoes_control_focus(app->slot.focus);
      }
    break;

    case WM_COMMAND:
      if ((HWND)l)
      {
        switch (HIWORD(w))
        {
          case BN_CLICKED:
          {
            int id = LOWORD(w);
            VALUE control = rb_ary_entry(app->slot.controls, id - SHOES_CONTROL1);
            if (!NIL_P(control))
              shoes_control_send(control, s_click);
          }
          break;

          case CBN_SELCHANGE:
          case EN_CHANGE:
          {
            int id = LOWORD(w);
            VALUE control = rb_ary_entry(app->slot.controls, id - SHOES_CONTROL1);
            if (!NIL_P(control))
              shoes_control_send(control, s_change);
          }
          break;
        }
      }
    break;
  }
  return DefWindowProc(win, msg, w, l);
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
  HCURSOR c;
  if (cursor == s_hand)
  {
    c = LoadCursor(NULL, IDC_HAND);
  }
  else if (cursor == s_arrow)
  {
    c = LoadCursor(NULL, IDC_ARROW);
  }
  else
    goto done;

  SetCursor(c);

  app->cursor = cursor;

done:
  return SHOES_OK;
}

shoes_code
shoes_classex_init()
{
  shoes_code code = SHOES_OK;
  shoes_world->os.classex.hInstance = shoes_world->os.instance;
  shoes_world->os.classex.lpszClassName = SHOES_SHORTNAME;
  shoes_world->os.classex.lpfnWndProc = shoes_app_win32proc;
  shoes_world->os.classex.style = CS_HREDRAW | CS_VREDRAW;
  shoes_world->os.classex.cbSize = sizeof(WNDCLASSEX);
  shoes_world->os.classex.hIcon = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  shoes_world->os.classex.hIconSm = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  shoes_world->os.classex.hCursor = LoadCursor(NULL, IDC_ARROW);
  shoes_world->os.classex.lpszMenuName = NULL;
  shoes_world->os.classex.cbClsExtra = 0;
  shoes_world->os.classex.cbWndExtra = 0;
  shoes_world->os.classex.hbrBackground = (HBRUSH)COLOR_WINDOW;

  if (!RegisterClassEx(&shoes_world->os.classex))
  {
    QUIT("Couldn't register WIN32 window class.");
  }

  shoes_world->os.vlclassex.hInstance = shoes_world->os.slotex.hInstance = shoes_world->os.instance;
  shoes_world->os.vlclassex.lpszClassName = SHOES_VLCLASS;
  shoes_world->os.slotex.lpszClassName = SHOES_SLOTCLASS;
  shoes_world->os.vlclassex.style = shoes_world->os.slotex.style = CS_NOCLOSE;
  shoes_world->os.vlclassex.lpfnWndProc = DefWindowProc;
  shoes_world->os.slotex.lpfnWndProc = shoes_slot_win32proc;
  shoes_world->os.vlclassex.cbSize = shoes_world->os.slotex.cbSize = sizeof(WNDCLASSEX);
  shoes_world->os.vlclassex.hIcon = shoes_world->os.slotex.hIcon = NULL;
  shoes_world->os.vlclassex.hIconSm = shoes_world->os.slotex.hIconSm = NULL;
  shoes_world->os.vlclassex.hCursor = shoes_world->os.slotex.hCursor = LoadCursor(NULL, IDC_ARROW);
  shoes_world->os.vlclassex.lpszMenuName = shoes_world->os.slotex.lpszMenuName = NULL;
  shoes_world->os.vlclassex.cbClsExtra = shoes_world->os.slotex.cbClsExtra = 0;
  shoes_world->os.vlclassex.cbWndExtra = shoes_world->os.slotex.cbWndExtra = 0;
  shoes_world->os.vlclassex.hbrBackground = shoes_world->os.slotex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

  if (!RegisterClassEx(&shoes_world->os.slotex) || !RegisterClassEx(&shoes_world->os.vlclassex))
  {
    QUIT("Couldn't register VLC window class.");
  }

quit:
  return code;
}

void
shoes_native_app_resized(shoes_app *app)
{
  if (app->slot.window != NULL)
  {
    RECT r;
    GetWindowRect(app->slot.window, &r);
    r.right = r.left + app->width;
    r.bottom = r.top + app->height;
    AdjustWindowRect(&r, WINDOW_STYLE, FALSE);
    MoveWindow(app->slot.window, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
  }
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
  SetWindowText(app->slot.window, msg);
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;
  RECT rect;
  DWORD exStyle = dialog ? WS_EX_WINDOWEDGE : WS_EX_CLIENTEDGE;

  app->slot.controls = Qnil;
  app->slot.focus = Qnil;
  app->os.ctrlkey = false;
  app->os.altkey = false;
  app->os.shiftkey = false;

  // remove the menu
  rect.left = 0;
  rect.top = 0;
  rect.right = app->width;
  rect.bottom = app->height;
  AdjustWindowRectEx(&rect, WINDOW_STYLE, FALSE, exStyle);

  app->slot.window = CreateWindowEx(
    exStyle, SHOES_SHORTNAME, SHOES_APPNAME,
    WINDOW_STYLE | WS_CLIPCHILDREN |
      (app->resizable ? (WS_THICKFRAME | WS_MAXIMIZEBOX) : WS_DLGFRAME) |
      WS_VSCROLL | ES_AUTOVSCROLL,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rect.right-rect.left, rect.bottom-rect.top,
    HWND_DESKTOP,
    NULL,
    shoes_world->os.instance,
    NULL);

  SetWindowLong(app->slot.window, GWL_USERDATA, (long)app);

  SCROLLINFO si;
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMin = 0;
  si.nMax = 0; 
  si.nPage = 0;
  si.nPos = 0;
  SetScrollInfo(app->slot.window, SB_VERT, &si, TRUE);

quit:
  return code;
}

void
shoes_native_app_show(shoes_app *)
{
  // TODO: disable parent windows of dialogs
  // if (dialog && !NIL_P(app->owner))
  // {
  //   shoes_app *owner;
  //   Data_Get_Struct(app->owner, shoes_app, owner);
  //   EnableWindow(owner->slot.window, FALSE);
  // }
  ShowWindow(app->slot.window, SW_SHOWNORMAL);
}

void
shoes_native_loop()
{
  MSG msgs;
  while (msgs.message != WM_QUIT)
  {
    BOOL msg = PeekMessage(&msgs, NULL, 0, 0, PM_REMOVE);
    if (msg)
    {
      HWND focused = GetForegroundWindow();
      if (msgs.message == WM_KEYDOWN || msgs.message == WM_KEYUP)
      {
        shoes_app *appk = (shoes_app *)GetWindowLong(focused, GWL_USERDATA);
        if (appk != NULL && RARRAY_LEN(appk->slot.controls) > 0)
        {
          switch (msgs.wParam)
          {
            case VK_TAB: case VK_UP: case VK_LEFT: case VK_DOWN:
            case VK_RIGHT: case VK_PRIOR: case VK_NEXT:
              break;
            default:
              msg = false;
          }
        }
        else msg = false;
      }
      else if (msgs.message == WM_SYSCHAR || msgs.message == WM_CHAR)
        msg = false;
      if (msg)
        msg = IsDialogMessage(focused, &msgs);

      if (!msg)
      {
        TranslateMessage(&msgs);
        DispatchMessage(&msgs);
      }
    }
    else
    {
      rb_eval_string("sleep(0.001)");
    }
  }
}

void
shoes_native_app_close(shoes_app *app)
{
  SendMessage(APP_WINDOW(app), WM_CLOSE, 0, 0);
}

void
shoes_browser_open(char *url)
{
  ShellExecute(0, "open", url, 0, 0, 0);
}

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(c, shoes_canvas, canvas);
  slot = &canvas->slot;

  if (toplevel)
  {
    slot->dc = parent->dc;
    slot->window = parent->window;
    slot->controls = parent->controls;
  }
  else
  {
    slot->controls = rb_ary_new();
    slot->dc = NULL;
    slot->window = CreateWindowEx(0, SHOES_SLOTCLASS, "Shoes Slot Window",
      WS_CHILD | WS_CLIPCHILDREN | WS_TABSTOP | WS_VISIBLE,
      x, y, width, height, parent->window, NULL, 
      (HINSTANCE)GetWindowLong(parent->window, GWL_HINSTANCE), NULL);
    SetWindowLong(slot->window, GWL_USERDATA, (long)c);
  }
  if (toplevel) shoes_canvas_size(c, width, height);
}

cairo_t *
shoes_cairo_create(SHOES_SLOT_OS *slot, shoes_canvas *parent, int width, int height, int toplevel)
{
  if (slot->surface != NULL)
    return NULL;

  HBITMAP bitmap, bitold;
  HDC hdc = BeginPaint(slot->window, &slot->ps);
  if (slot->dc != NULL)
  {
    DeleteObject(GetCurrentObject(slot->dc, OBJ_BITMAP));
    DeleteDC(slot->dc);
  }
  slot->dc = CreateCompatibleDC(hdc);
  bitmap = CreateCompatibleBitmap(hdc, width, height);
  bitold = (HBITMAP)SelectObject(slot->dc, bitmap);
  DeleteObject(bitold);
  if (toplevel)
  {
    RECT rc;
    HBRUSH bg = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    GetClientRect(slot->window, &rc);
    FillRect(slot->dc, &rc, bg);
    DeleteObject(bg);
  }
  else
  {
    if (parent != NULL && parent->slot.dc != NULL)
    {
      RECT r;
      GetClientRect(slot->window, &r);
      BitBlt(slot->dc, 0, 0, r.right - r.left, r.bottom - r.top,
        parent->slot.dc, canvas->place.ix, canvas->place.iy, SRCCOPY);
    }
  }

  slot->surface = cairo_win32_surface_create(slot->dc);

  cairo_t *cr = cairo_create(slot->surface);
  cairo_translate(cr, 0, -slot->scrolly);
  return cr;
}

void shoes_cairo_destroy(SHOES_SLOT_OS *)
{
  BitBlt(hdc, 0, 0, width, height, slot->dc, 0, 0, SRCCOPY);
  cairo_surface_destroy(slot->surface);
  slot->surface = NULL;
  EndPaint(slot->window, &slot->ps);
}

void
shoes_group_clear(SHOES_GROUP_OS *group)
{
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
  RECT r;
  GetWindowRect(self_t->slot.window, &r);
  if (r.left != self_t->place.ix + self_t->place.dx || 
      r.top != (self_t->place.iy + self_t->place.dy) - pc->slot.scrolly ||
      r.right - r.left != self_t->place.iw ||
      r.bottom - r.top != self_t->place.ih)
  {
    MoveWindow(self_t->slot.window, self_t->place.ix + self_t->place.dx, 
      (self_t->place.iy + self_t->place.dy) - pc->slot.scrolly, self_t->place.iw, 
      self_t->place.ih, TRUE);
  }
}
