//
// shoes/config.h
// Most platform-specific definitions.  Window handles and GUI elements for GTK,
// OSX and Win32.
//
// About SHOES_APP_OS and SHOES_SLOT_OS:
// =========================
//
// Okay, so I should mention why these two are split up.  Obviously, these structures
// contain anything which is unique to a platform.  So, on GTK, you'll see a bunch of
// GtkWidget pointers in these two structures.  I try to isolate anything of that nature
// into SHOES_APP_OS or SHOES_SLOT_OS.  Of course, for native widgets, I just go ahead and put it
// in the widget structures, to fit that all in here would be too pedantic.
//
// SHOES_APP_OS covers the toplevel window.  So, there should only be one SHOES_APP_OS structure
// floating around.  In fact, the `global_app` variable in shoes/app.c should always
// point to that one struct.  Still, this struct has a pretty low visibility.  I don't
// like to use `global_app`, except to get around platform limitations.  So the SHOES_APP_OS
// struct is low-viz, it's only touched in the app-level API and in event handlers.
//
// SHOES_SLOT_OS travels down through nested windows, nested canvases.  It's always handy at
// any level in the canvas stack.  But, keep in mind, one is allocated per window or
// canvas.  I guess I think of each drawing layer as a "slot".  Each slot copies its
// parent slot.  So, it's possible that the bottom slot will simply reference pointers
// that are kept in the top slot.  But, in the case of nested fixed canvases (similar
// to a browser's IFRAMEs,) the slot will point to new window handles and pass that
// on to its children.
//
// Anyway, ultimately the idea is to occassionally use the native environment's window
// nesting, because these operating systems offer scrollbars (and some offer compositing)
// which would be wasteful to try to emulate.
//
#ifndef SHOES_CONFIG_H
#define SHOES_CONFIG_H

#define SHOES_BUFSIZE    4096

//
// gtk window struct
//
#ifdef SHOES_GTK
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>

#define SHOES_SIGNAL
#define SHOES_INIT_ARGS void
#define SHOES_EXTERN

typedef struct {
  GtkWidget *vscroll, *canvas;
  GdkEventExpose *expose;
  int scrolly, scrollh, scrollw;
  void *owner;
} shoes_slot_gtk, SHOES_SLOT_OS;

typedef struct {
  GtkWidget *layout;
  GtkWidget *radios;
} shoes_group_gtk, SHOES_GROUP_OS;

typedef struct {
  GtkWidget *window;
} shoes_app_gtk, SHOES_APP_OS;

typedef struct {
  int nada;
} shoes_world_gtk, SHOES_WORLD_OS;

#define USTR(str) str
#define SHOES_CONTROL_REF GtkWidget *
#define SHOES_SURFACE_REF GtkWidget *
#define SHOES_BOOL gboolean
#define SHOES_TIMER_REF guint
#define DC(slot) slot->canvas
#define HAS_DRAWABLE(slot) slot->canvas->window != 0
#define DRAWABLE(ref) GDK_DRAWABLE_XID(ref->window)
#define APP_WINDOW(app) (app == NULL ? NULL : GTK_WINDOW(app->os.window))
#define SHOES_TIME struct timespec
#define SHOES_DOWNLOAD_HEADERS struct curl_slist *
#define SHOES_DOWNLOAD_ERROR CURLcode
#endif

//
// quartz (osx) window struct
//
#ifdef SHOES_QUARTZ
// hacks to prevent T_DATA conflict between Ruby and Carbon headers
# define __OPENTRANSPORT__
# define __OPENTRANSPORTPROTOCOL__
# define __OPENTRANSPORTPROVIDERS__
#include <Cocoa/Cocoa.h>
#include "shoes/native/cocoa.h"
#include <cairo-quartz.h>

#define SHOES_SIGNAL
#define SHOES_HELP_MANUAL 3044
#define SHOES_CONTROL1    3045
#define SHOES_INIT_ARGS void
#define SHOES_EXTERN

typedef struct {
  NSView *view;
  NSScroller *vscroll;
  CGContextRef context;
  cairo_surface_t *surface;
  VALUE controls;
  int scrolly;
  void *owner;
} shoes_slot_quartz, SHOES_SLOT_OS;

typedef struct {
  char none;
} shoes_group_quartz, SHOES_GROUP_OS;

typedef struct {
  ShoesWindow *window;
  NSView *view;
} shoes_app_quartz, SHOES_APP_OS;

typedef struct {
  ShoesEvents *events;
} shoes_world_quartz, SHOES_WORLD_OS;

#define kShoesViewClassID CFSTR("org.hackety.ShoesView")
#define kShoesBoundEvent  'Boun'
#define kShoesSlotData    'SLOT'

#define USTR(str) str
#define SHOES_CONTROL_REF NSControl *
#define SHOES_SURFACE_REF CGrafPtr
#define SHOES_BOOL BOOL
#define SHOES_TIMER_REF ShoesTimer *
#define DC(slot) slot->view
#define HAS_DRAWABLE(slot) slot->context != NULL
#define DRAWABLE(ref) ref
#define SHOES_TIME struct timeval
#define SHOES_DOWNLOAD_HEADERS NSDictionary *
#define SHOES_DOWNLOAD_ERROR NSError *

#endif

#ifdef SHOES_WIN32
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <cairo-win32.h>
#include "win32/win32.h"

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
#ifndef SPI_GETWHEELSCROLLLINES
#define SPI_GETWHEELSCROLLLINES 104
#endif
#ifndef WHEEL_PAGESCROLL
#define WHEEL_PAGESCROLL UINT_MAX
#endif

#define SHOES_CONTROL1   3045
#define SHOES_WM_MESSAGE (WM_APP + 3045)
#define SHOES_INIT_ARGS  HINSTANCE inst, int style
#define SHOES_EXTERN     __declspec(dllimport)

typedef struct {
  PAINTSTRUCT ps;
  HDC dc, dc2;
  HWND window;
  VALUE focus;
  VALUE controls;
  cairo_surface_t *surface;
  int scrolly;
  char vscroll;
  void *owner;
} shoes_slot_win32, SHOES_SLOT_OS;

typedef struct {
  char none;
} shoes_group_win32, SHOES_GROUP_OS;

typedef struct {
  BOOL ctrlkey, altkey, shiftkey;
} shoes_app_win32, SHOES_APP_OS;

typedef struct {
  HINSTANCE instance;
  int style;
  HWND hidden;
  WNDCLASSEX classex, slotex, vlclassex, hiddenex;
  ATOM classatom;
} shoes_world_win32, SHOES_WORLD_OS;

#define USTR(str) (const char *)L##str
#define SHOES_CONTROL_REF HWND
#define SHOES_SURFACE_REF HWND
#define SHOES_BOOL BOOL
#define SHOES_TIMER_REF long
#define DC(slot) slot->window
#define HAS_DRAWABLE(slot) slot->window != NULL
#define DRAWABLE(ref) (libvlc_drawable_t)ref
#define APP_WINDOW(app) (app == NULL ? NULL : app->slot->window)
#define SHOES_TIME DWORD
#define SHOES_DOWNLOAD_HEADERS LPWSTR
#define SHOES_DOWNLOAD_ERROR DWORD

#endif

#define KEY_STATE(sym) \
  { \
    VALUE str = rb_str_new2("" # sym "_"); \
    rb_str_append(str, rb_funcall(v, s_to_s, 0)); \
    v = rb_str_intern(str); \
  }

#endif
