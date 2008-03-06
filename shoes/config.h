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

#define SHOES_EXTERN
#define SHOES_EXTERN_VAR extern
#define SHOES_INIT_ARGS void

typedef struct {
  GtkWidget *box, *canvas;
  GdkEventExpose *expose;
  int scrolly;
} shoes_slot_gtk, SHOES_SLOT_OS;

typedef struct {
  GtkWidget *window;
} shoes_app_gtk, SHOES_APP_OS;

typedef struct {
  int nada;
} shoes_world_gtk, SHOES_WORLD_OS;

#define SHOES_CONTROL_REF GtkWidget *
#define DC(slot) slot.canvas
#define HAS_DRAWABLE(slot) GTK_LAYOUT(slot.canvas)->bin_window != 0
#define DRAWABLE(ref) GDK_DRAWABLE_XID(GTK_LAYOUT(ref)->bin_window)
#define APP_WINDOW(app) (app == NULL ? NULL : GTK_WINDOW(app->os.window))
#endif

//
// quartz (osx) window struct
//
#ifdef SHOES_QUARTZ
// hacks to prevent T_DATA conflict between Ruby and Carbon headers
# define __OPENTRANSPORT__
# define __OPENTRANSPORTPROTOCOL__
# define __OPENTRANSPORTPROVIDERS__
#include <Carbon/Carbon.h>
#include <cairo-quartz.h>

#define SHOES_HELP_MANUAL 3044
#define SHOES_CONTROL1    3045
#define SHOES_EXTERN
#define SHOES_EXTERN_VAR extern
#define SHOES_INIT_ARGS void

typedef struct {
  HIViewRef scrollview, view;
  VALUE focus;
  VALUE controls;
  CGContextRef context;
  cairo_surface_t *surface;
  int scrolly;
} shoes_slot_quartz, SHOES_SLOT_OS;

typedef struct {
  WindowRef window;
  HIViewRef view;
} shoes_app_quartz, SHOES_APP_OS;

typedef struct {
  TECObjectRef converter;
  PasteboardRef clip;
} shoes_world_quartz, SHOES_WORLD_OS;

#define kShoesViewClassID CFSTR("org.hackety.ShoesView")
#define kShoesBoundEvent  'Boun'
#define kShoesSlotData    'SLOT'

#define SHOES_CONTROL_REF ControlRef
#define DC(slot) slot.view
#define HAS_DRAWABLE(slot) slot.context != NULL
#define DRAWABLE(ref) ref

OSStatus shoes_slot_quartz_register(void);
OSStatus shoes_slot_quartz_create(VALUE, SHOES_SLOT_OS *, int, int, int, int);
VALUE shoes_cf2rb(CFStringRef cf);

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

#define SHOES_CONTROL1  3045
#define SHOES_EXTERN extern "C" __declspec(dllimport)
#define SHOES_EXTERN_VAR SHOES_EXTERN
#define SHOES_INIT_ARGS HINSTANCE inst, int style

typedef struct {
  HDC dc;
  HWND window;
  VALUE focus;
  VALUE controls;
  cairo_surface_t *surface;
  int scrolly;
} shoes_slot_win32, SHOES_SLOT_OS;

typedef struct {
  BOOL ctrlkey, altkey, shiftkey;
} shoes_app_win32, SHOES_APP_OS;

typedef struct {
  HINSTANCE instance;
  int style;
  WNDCLASSEX classex, slotex, vlclassex;
} shoes_world_win32, SHOES_WORLD_OS;

#define SHOES_CONTROL_REF HWND
#define DC(slot) slot.window
#define HAS_DRAWABLE(slot) slot.window != NULL
#define DRAWABLE(ref) (libvlc_drawable_t)ref
#define APP_WINDOW(app) (app == NULL ? NULL : app->slot.window)

#endif

#define KEY_STATE(sym) \
  { \
    VALUE str = rb_str_new2("" # sym "_"); \
    rb_str_append(str, rb_funcall(v, s_to_s, 0)); \
    v = rb_str_intern(str); \
  }

#endif
