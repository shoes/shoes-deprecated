//
// shoes/config.h
// Most platform-specific definitions.  Window handles and GUI elements for GTK,
// OSX and Win32.
//
// About APPKIT and APPSLOT:
// =========================
//
// Okay, so I should mention why these two are split up.  Obviously, these structures
// contain anything which is unique to a platform.  So, on GTK, you'll see a bunch of
// GtkWidget pointers in these two structures.  I try to isolate anything of that nature
// into APPKIT or APPSLOT.  Of course, for native widgets, I just go ahead and put it
// in the widget structures, to fit that all in here would be too pedantic.
//
// APPKIT covers the toplevel window.  So, there should only be one APPKIT structure
// floating around.  In fact, the `global_app` variable in shoes/app.c should always
// point to that one struct.  Still, this struct has a pretty low visibility.  I don't
// like to use `global_app`, except to get around platform limitations.  So the APPKIT
// struct is low-viz, it's only touched in the app-level API and in event handlers.
//
// APPSLOT travels down through nested windows, nested canvases.  It's always handy at
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

typedef struct {
  GtkWidget *box, *canvas;
  GdkEventExpose *expose;
} shoes_slot_gtk, APPSLOT;

typedef struct {
  GtkWidget *window;
} shoes_app_gtk, APPKIT;

#define DC(slot) slot.canvas
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

#define SHOES_CONTROL1  3045

typedef struct {
  HIViewRef scrollview, view;
  VALUE controls;
  CGContextRef context;
  cairo_surface_t *surface;
} shoes_slot_quartz, APPSLOT;

typedef struct {
  WindowRef window;
  HIViewRef view;
  TECObjectRef converter;
  PasteboardRef clip;
} shoes_app_quartz, APPKIT;

#define kShoesViewClassID CFSTR("org.hackety.ShoesView")
#define kShoesBoundEvent  'Boun'
#define kShoesSlotData    'SLOT'

#define DC(slot) slot.view

OSStatus shoes_slot_quartz_create(VALUE, APPSLOT *, int, int);

#endif

#ifdef SHOES_WIN32
#include <windows.h>
#include <commctrl.h>
#include <cairo-win32.h>

#define SHOES_CONTROL1  3045

typedef struct {
  HDC dc;
  HWND window;
  VALUE controls;
  cairo_surface_t *surface;
} shoes_slot_win32, APPSLOT;

typedef struct {
  HINSTANCE instance;
  int style;
  WNDCLASSEX classex;
  BOOL ctrlkey, altkey, shiftkey;
} shoes_app_win32, APPKIT;

#define DC(slot) slot.window

#endif

#define KEY_STATE(sym) \
  { \
    VALUE str = rb_str_new2("" # sym "_"); \
    rb_str_append(str, rb_funcall(v, s_to_s, 0)); \
    v = rb_str_intern(str); \
  }

#endif
