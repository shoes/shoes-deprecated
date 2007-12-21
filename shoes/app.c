//
// shoes/app.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/world.h"
#include "shoes/dialogs.h"
#include "node.h"

static void
shoes_app_mark(shoes_app *app)
{
#ifndef SHOES_GTK
  rb_gc_mark_maybe(app->slot.controls);
#endif
#ifdef SHOES_WIN32
  rb_gc_mark_maybe(app->slot.focus);
#endif
  rb_gc_mark_maybe(app->location);
  rb_gc_mark_maybe(app->canvas);
  rb_gc_mark_maybe(app->nesting);
  rb_gc_mark_maybe(app->timers);
  rb_gc_mark_maybe(app->styles);
}

static void
shoes_app_free(shoes_app *app)
{
  RUBY_CRITICAL(free(app));
}

VALUE
shoes_app_alloc(VALUE klass)
{
  shoes_app *app = SHOE_ALLOC(shoes_app);
  SHOE_MEMZERO(app, shoes_app, 1);
  app->location = Qnil;
  app->canvas = shoes_canvas_new(cShoes, app);
  app->nesting = rb_ary_new();
  app->timers = rb_ary_new();
  app->styles = Qnil;
  app->title = Qnil;
  app->width = SHOES_APP_WIDTH;
  app->height = SHOES_APP_HEIGHT;
  app->resizable = TRUE;
#ifdef SHOES_WIN32
  app->slot.window = NULL;
#else
  app->os.window = NULL;
#endif
  app->self = Data_Wrap_Struct(klass, shoes_app_mark, shoes_app_free, app);
  return app->self;
}

VALUE
shoes_app_new()
{
  shoes_world->app = shoes_app_alloc(cApp);
  rb_ary_push(shoes_world->apps, shoes_world->app);
  return shoes_world->app;
}

#ifdef SHOES_GTK
static gboolean
shoes_app_gtk_idle(gpointer data)
{
  rb_eval_string("sleep(0.001)");
  return TRUE;
}

static gboolean
shoes_app_gtk_motion(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{ 
  GdkModifierType state;
  shoes_app *app = (shoes_app *)data;
  if (!event->is_hint)
  {
    state = (GdkModifierType)event->state;
    shoes_app_motion(app, (int)event->x, (int)event->y);
  }
  return TRUE;
}

static gboolean
shoes_app_gtk_button(GtkWidget *widget, GdkEventButton *event, gpointer data)
{ 
  shoes_app *app = (shoes_app *)data;
  if (event->type == GDK_BUTTON_PRESS)
  {
    shoes_app_click(app, event->button, event->x, event->y);
  }
  else if (event->type == GDK_BUTTON_RELEASE)
  {
    shoes_app_release(app, event->button, event->x, event->y);
  }
  return TRUE;
}

static void
shoes_app_gtk_paint_children(GtkWidget *widget, gpointer data)
{
  shoes_app *app = (shoes_app *)data;
  gtk_container_propagate_expose(GTK_CONTAINER(app->os.window), widget, app->slot.expose);
}

static void
shoes_app_gtk_paint (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{ 
  shoes_app *app = (shoes_app *)data;
  gtk_window_get_size(GTK_WINDOW(app->os.window), &app->width, &app->height);
  shoes_canvas_size(app->canvas, app->width, app->height);
  app->slot.expose = event;
  gtk_container_forall(GTK_CONTAINER(app->os.window), shoes_app_gtk_paint_children, app);
  app->slot.expose = NULL;
}

#define KEY_SYM(name, sym) \
  else if (event->keyval == GDK_##name) \
    v = ID2SYM(rb_intern("" # sym))

static gboolean
shoes_app_gtk_keypress (GtkWidget *widget, GdkEventKey *event, gpointer data)
{ 
  VALUE v = Qnil;
  guint modifiers;
  shoes_app *app = (shoes_app *)data;
  if (event->length > 0)
  {
    if (event->string[0] == '\r' && event->length == 1)
      v = rb_str_new2("\n");
    else
      v = rb_str_new(event->string, event->length);
  }
  KEY_SYM(BackSpace, backspace);
  KEY_SYM(Tab, tab);
  KEY_SYM(Page_Up, page_up);
  KEY_SYM(Page_Down, page_down);
  KEY_SYM(Home, home);
  KEY_SYM(End, end);
  KEY_SYM(Left, left);
  KEY_SYM(Up, up);
  KEY_SYM(Right, right);
  KEY_SYM(Down, down);
  KEY_SYM(F1, f1);
  KEY_SYM(F2, f2);
  KEY_SYM(F3, f3);
  KEY_SYM(F4, f4);
  KEY_SYM(F5, f5);
  KEY_SYM(F6, f6);
  KEY_SYM(F7, f7);
  KEY_SYM(F8, f8);
  KEY_SYM(F9, f9);
  KEY_SYM(F10, f10);
  KEY_SYM(F11, f11);
  KEY_SYM(F12, f12);

  if (SYMBOL_P(v))
  {
    if (event->state & GDK_MOD1_MASK)
      KEY_STATE(alt);
    if (event->state & GDK_SHIFT_MASK)
      KEY_STATE(shift);
    if (event->state & GDK_CONTROL_MASK)
      KEY_STATE(control);
  }
  else
  {
    if (event->state & GDK_MOD1_MASK)
      KEY_STATE(alt);
  }

  if (v != Qnil)
    shoes_app_keypress(app, v);
  return FALSE;
}
#endif

#ifdef SHOES_QUARTZ
pascal void shoes_app_quartz_redraw(EventLoopTimerRef theTimer, void* userData);
pascal OSStatus shoes_app_quartz_handler(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData);
pascal OSStatus shoes_slot_quartz_handler(EventHandlerCallRef inCallRef, EventRef inEvent, void* inUserData);

typedef struct
{
  HIViewRef view;
  VALUE canvas;
} shoes_qdata;

pascal OSStatus
shoes_slot_quartz_handler(
  EventHandlerCallRef inCallRef,
  EventRef inEvent,
  void* inUserData)
{
  OSStatus err        = eventNotHandledErr;
  UInt32 eventKind    = GetEventKind(inEvent);
  UInt32 eventClass   = GetEventClass(inEvent);
  shoes_qdata *qdata  = (shoes_qdata *)inUserData;
  
  switch (eventClass)
  {
    case kEventClassHIObject:
      switch (eventKind)
      {
        case kEventHIObjectConstruct:
          INFO("kEventHIObjectConstruct\n", 0);
          qdata = (shoes_qdata *)malloc(sizeof(shoes_qdata));
          qdata->canvas = Qnil;
          err = GetEventParameter(inEvent, kEventParamHIObjectInstance,
            typeHIObjectRef, NULL, sizeof(HIObjectRef), NULL,
            (HIObjectRef *)&qdata->view);
          if (err != noErr) break;
          err = SetEventParameter(inEvent, kEventParamHIObjectInstance,
            typeVoidPtr, sizeof(shoes_qdata *), &qdata);
        break;

        case kEventHIObjectInitialize:
        {
          INFO("kEventHIObjectInitialize\n", 0);
          HIRect bounds;
          err = CallNextEventHandler(inCallRef, inEvent);
          if (err != noErr) break;
          err = GetEventParameter(inEvent, kShoesBoundEvent, typeHIRect, NULL, sizeof(HIRect), NULL, &bounds);
          if (err != noErr) break;
          HIViewSetFrame(qdata->view, &bounds);
        }
        break;

        case kEventHIObjectDestruct:
          INFO("kEventHIObjectDestruct\n", 0);
          free(qdata);
        break;
      }
    break;

    
    case kEventClassScrollable:
      switch (eventKind)
      {
        case kEventScrollableGetInfo:
        {
          HIRect bounds;
          shoes_canvas *canvas;
          Data_Get_Struct(qdata->canvas, shoes_canvas, canvas);
          HIViewGetBounds(canvas->slot.view, &bounds);
          SetEventParameter(inEvent, kEventParamImageSize, typeHISize, sizeof(bounds.size), &bounds.size);
          SetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, sizeof(bounds.origin), &bounds.origin);
          HIViewGetBounds(canvas->slot.scrollview, &bounds);
          SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
          SetEventParameter(inEvent, kEventParamLineSize, typeHISize, sizeof(bounds.size), &bounds.size);
          err = noErr;
        }
        break;

        case kEventScrollableScrollTo:
        {
          HIPoint where;
          shoes_canvas *canvas;
          Data_Get_Struct(qdata->canvas, shoes_canvas, canvas);
          GetEventParameter(inEvent, kEventParamOrigin, typeHIPoint, NULL, sizeof(where), NULL, &where);
          HIViewSetBoundsOrigin(canvas->slot.view, where.x, where.y);
          // canvas->scrolly = (where.y < 0.0) ? 0.0 : where.y;
          HIViewSetNeedsDisplay(canvas->slot.scrollview, true);
        }
        break;
      }
    break;

    case kEventClassControl:
      switch (eventKind)
      {
        case kEventControlInitialize:
          INFO("kEventHIControlInitialize\n", 0);
          err = CallNextEventHandler(inCallRef, inEvent);
          if (err) break;

          UInt32 features = 0;
          err = GetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, NULL, sizeof(features), NULL, &features);
          if (err == noErr)
            features |= kControlSupportsEmbedding;
          else
            features = kControlSupportsEmbedding;

          err = SetEventParameter(inEvent, kEventParamControlFeatures, typeUInt32, sizeof features, &features);
        break;

        case kEventControlDraw:
        {
          INFO("kEventHIControlDraw\n", 0);
          shoes_canvas *canvas;
          Data_Get_Struct(qdata->canvas, shoes_canvas, canvas);
          INFO("Getting context\n", 0);
          GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef,
            NULL, sizeof(CGContextRef), NULL, &canvas->slot.context);
          INFO("Got context: %lu\n", canvas->slot.context);
          shoes_canvas_paint(qdata->canvas);
          INFO("Painted!\n", 0);
          err = eventNotHandledErr;
        }
        break;

        case kEventControlGetData:
        {
          OSType tag;
          Ptr ptr;
          Size outSize;

          INFO("kEventControlGetData\n", 0);
          GetEventParameter(inEvent, kEventParamControlDataTag, typeEnumeration,
            NULL, sizeof(OSType), NULL, &tag);

          GetEventParameter(inEvent, kEventParamControlDataBuffer, typePtr,
            NULL, sizeof(Ptr), NULL, &ptr);

          if (tag == kShoesSlotData)
          {
            ptr = qdata->canvas;
            outSize = sizeof(VALUE);
          }
          else
            err = errDataNotSupported;

          if (err == noErr)
          {
            SetEventParameter(inEvent, kEventParamControlDataBufferSize, typeLongInteger,
              sizeof(Size), &outSize);
          }
        }
        break;

        case kEventControlSetData:
        {
          Ptr ptr;
          OSType tag;

          INFO("kEventControlSetData\n", 0);
          GetEventParameter(inEvent, kEventParamControlDataTag, typeEnumeration,
            NULL, sizeof(OSType), NULL, &tag);

          GetEventParameter(inEvent, kEventParamControlDataBuffer, typePtr,
            NULL, sizeof(Ptr), NULL, &ptr);

          if (tag == kShoesSlotData)
            qdata->canvas = (VALUE)ptr;
          else
            err = errDataNotSupported;
        }
        break;
      }
    break;
  }

  INFO("End of window proc\n", 0);
  return err;
}

OSStatus
shoes_slot_quartz_register(void)
{
  OSStatus err = noErr;
  static HIObjectClassRef objcls = NULL;

  if (objcls == NULL) {
    EventTypeSpec eventList[] = {
      { kEventClassHIObject, kEventHIObjectConstruct },
      { kEventClassHIObject, kEventHIObjectInitialize },
      { kEventClassHIObject, kEventHIObjectDestruct },

      { kEventClassScrollable, kEventScrollableGetInfo },
      { kEventClassScrollable, kEventScrollableScrollTo },

      { kEventClassControl, kEventControlDraw },
      { kEventClassControl, kEventControlInitialize },
      { kEventClassControl, kEventControlGetData },
      { kEventClassControl, kEventControlSetData }
    };

    err = HIObjectRegisterSubclass(kShoesViewClassID, kHIViewClassID,
      0, shoes_slot_quartz_handler, GetEventTypeCount(eventList), eventList,
      NULL, &objcls);
  }
  return err;
}

OSStatus
shoes_slot_quartz_create(VALUE self, SHOES_SLOT_OS *parent, int x, int y, int w, int h)
{
  HIRect rect;
  OSStatus err;
  EventRef event;
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(self, shoes_canvas, canvas);
  slot = &canvas->slot;

  //
  // Create the scroll view
  //
  HIScrollViewCreate(kHIScrollViewOptionsVertScroll | kHIScrollViewOptionsAllowGrow, &slot->scrollview);
  HIScrollViewSetScrollBarAutoHide(slot->scrollview, true);
  rect.origin.x = x * 1.;
  rect.origin.y = y * 1.;
  rect.size.width = (double)w;
  rect.size.height = (double)h;
  HIViewSetFrame(slot->scrollview, &rect);

  //
  // Create the content view
  //
  CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize,
    GetCurrentEventTime(), 0, &event);

  rect.origin.x = 0.0;
  rect.origin.y = 0.0;
  rect.size.width = w * 1.;
  rect.size.height = (double)h * 2;
  SetEventParameter(event, kShoesBoundEvent, typeHIRect, sizeof(HIRect), &rect);
  
  HIObjectCreate(kShoesViewClassID, event, (HIObjectRef *)&slot->view);

  SetControlData(slot->view, 1, kShoesSlotData, sizeof(VALUE), self);
  HIViewAddSubview(slot->scrollview, slot->view);
  HIViewAddSubview(parent->view, slot->scrollview);
  HIViewSetVisible(slot->view, true);
  HIViewSetVisible(slot->scrollview, true);
  return noErr;
}

#define VK_TAB   48
#define VK_BS    51
#define VK_PRIOR 116
#define VK_NEXT  121
#define VK_HOME  115
#define VK_END   119
#define VK_LEFT  123
#define VK_UP    126
#define VK_RIGHT 124
#define VK_DOWN  125
#define VK_F1    122
#define VK_F2    120
#define VK_F3     99
#define VK_F4    118
#define VK_F5     96
#define VK_F6     97
#define VK_F7     98
#define VK_F8    100
#define VK_F9    101
#define VK_F10   109
#define VK_F11   103
#define VK_F12   111

#define KEYPRESS(name, sym) \
  if (key == VK_##name) { \
    VALUE v = ID2SYM(rb_intern("" # sym)); \
    if (modifier & cmdKey) \
      KEY_STATE(alt); \
    if (modifier & shiftKey) \
      KEY_STATE(shift); \
    if (modifier & controlKey) \
      KEY_STATE(control); \
    shoes_app_keypress(app, v); \
  } else

pascal OSStatus
shoes_app_quartz_handler(
  EventHandlerCallRef inCallRef,
  EventRef inEvent,
  void* inUserData)
{
  OSStatus	err		= eventNotHandledErr;
  UInt32	eventKind       = GetEventKind(inEvent);
  UInt32	eventClass      = GetEventClass(inEvent);
  shoes_app *app      = (shoes_app *)inUserData;
  EventMouseButton button;
  Point	mouseLoc;
  Rect bounds;
  char *text;
  UInt32 len;
  
  switch (eventClass)
  {
    case kEventClassWindow:
      switch (eventKind)
      {
        case kEventWindowBoundsChanged:
        {
          UInt32 attributes;
          GetEventParameter(inEvent, kEventParamAttributes, typeUInt32, NULL,
              sizeof(UInt32), NULL, &attributes);

          if(attributes & kWindowBoundsChangeSizeChanged)
          {
            HIRect hr;
            GetEventParameter(inEvent, kEventParamCurrentBounds,
              typeHIRect, NULL, sizeof(HIRect), NULL, &hr);
            app->width = hr.size.width;
            app->height = hr.size.height;
            shoes_canvas_size(app->canvas, app->width, app->height);
            HIViewSetNeedsDisplay(app->slot.view, true);
            err = noErr;
          }
        }
        break;
      }
    break;

    case kEventClassCommand:
      INFO("kEventClassCommand: %d+%d\n", eventKind, eventClass);
      if (!NIL_P(app->slot.controls))
      {
        HICommand aCommand;
        GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
        if (aCommand.commandID >= SHOES_CONTROL1 && aCommand.commandID < SHOES_CONTROL1 + RARRAY_LEN(app->slot.controls))
        {
          VALUE control = rb_ary_entry(app->slot.controls, aCommand.commandID - SHOES_CONTROL1);
          if (!NIL_P(control))
          {
            if (rb_respond_to(control, s_change))
              shoes_control_send(control, s_change);
            else
              shoes_control_send(control, s_click);
          }
        }
      }
    break;

    case kEventClassTextInput:
      INFO("kEventClassTextInput\n", 0);
      switch (eventKind)
      {
        case kEventTextInputUnicodeForKeyEvent:
        {
          UInt32 key, modifier;
          EventRef origEvent;
          GetEventParameter(inEvent, kEventParamTextInputSendKeyboardEvent,
            typeEventRef, NULL, sizeof(origEvent), NULL, &origEvent);
          GetEventParameter(origEvent, kEventParamKeyCode,
              typeUInt32, NULL, sizeof(key), NULL, &key);
          GetEventParameter(origEvent, kEventParamKeyModifiers,
              typeUInt32, NULL, sizeof(modifier), NULL, &modifier);

          KEYPRESS(TAB, tab)
          KEYPRESS(BS, backspace)
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
          {
            GetEventParameter(inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, 0, &len, NULL);
            if (len)
            {
              OSStatus status;
              int nread, nwrite;
              char *text8;

              VALUE v;
              text = SHOE_ALLOC_N(char, len);
              GetEventParameter(inEvent, kEventParamTextInputSendText, typeUnicodeText, NULL, len, NULL, text);

              text8 = SHOE_ALLOC_N(char, len);
              status = TECConvertText(shoes_world->os.converter, text, len, &nread, text8, len, &nwrite);

              if (nwrite == 1 && text8[0] == '\r') 
                v = rb_str_new2("\n");
              else
              {
                v = rb_str_new(text8, (int)nwrite);
                if (modifier & cmdKey)
                  KEY_STATE(alt);
              }

              shoes_app_keypress(app, v);
              free(text);
              free(text8);
            }
          }
          err = noErr;
        }
        break;
      }
    break;

    case kEventClassMouse:
      INFO("kEventClassMouse\n", 0);
      GetMouse(&mouseLoc);
      GetWindowBounds(app->os.window, kWindowContentRgn, &bounds);
      if (mouseLoc.h < bounds.left || mouseLoc.v < bounds.top) break;
      GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton, 0, sizeof(EventMouseButton), 0, &button);
      switch (eventKind)
      {
        case kEventMouseMoved:
        case kEventMouseDragged:
          shoes_app_motion(app, mouseLoc.h - bounds.left, mouseLoc.v - bounds.top);
        break;

        case kEventMouseDown:
          shoes_app_click(app, button, mouseLoc.h - bounds.left, mouseLoc.v - bounds.top);
        break;

        case kEventMouseUp:
          shoes_app_release(app, button, mouseLoc.h - bounds.left, mouseLoc.v - bounds.top);
        break;
      }
    break;
  }

  INFO("End of main window proc\n", 0);
  return err;
}

pascal void
shoes_app_quartz_redraw(
  EventLoopTimerRef theTimer,
  void* userData)
{
  Rect windowRect;
  shoes_app *app = (shoes_app *)userData;
  
  SetRect(&windowRect, 0, 0, app->width, app->height);
  InvalWindowRect(app->os.window, &windowRect);
}

static pascal OSErr
shoes_app_quartz_open(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
  short i;
  AEDesc fileDesc;
  OSErr err;
  AEKeyword ignoredKeyWord;
  DescType ignoredType;
  Size ignoredSize;
  long numberOFiles;
  FSRef fr;
  char _path[SHOES_BUFSIZE], bootup[SHOES_BUFSIZE];

  if (!(err = AEGetParamDesc(appleEvt, keyDirectObject, typeAEList, &fileDesc)))
  {
    if ((err = AECountItems(&fileDesc, &numberOFiles)) == noErr) 
    {
      for (i = 1; i <= numberOFiles; ++i)
      {
        // Get a pointer to selected file
        err = AEGetNthPtr(&fileDesc, i, typeFSRef, NULL,
          NULL, &fr, sizeof(FSRef), NULL);
        if (!err)
        {
          FSRefMakePath(&fr, &_path, SHOES_BUFSIZE);
          shoes_load(RSTRING(_path), "/");
        }
      }
    }
    err = AEDisposeDesc(&fileDesc);
  }

  return err;
}

static pascal OSErr
shoes_app_quartz_quit(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
  QuitApplicationEventLoop();
  return 128;
}
#endif

#ifdef SHOES_WIN32
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
      PostQuitMessage(0);
    return 0; 

    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
    {
      RECT rect;
      GetClientRect(app->slot.window, &rect);
      app->width = rect.right;
      app->height = rect.bottom;
      shoes_canvas_size(app->canvas, app->width, app->height);
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
      else if (w >= 'A' && w <= 'Z') {
        VALUE v;
        char letter = w + 32;
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
        shoes_anim_call(timer);
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
#endif

#ifdef SHOES_WIN32
#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif
#endif

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
#ifdef SHOES_GTK
  if (app->os.window == NULL || app->os.window->window == NULL || app->cursor == cursor)
    goto done;

  GdkCursor *c;
  if (cursor == s_hand)
  {
    c = gdk_cursor_new(GDK_HAND2);
  }
  else if (cursor == s_arrow)
  {
    c = gdk_cursor_new(GDK_ARROW);
  }
  else
    goto done;

  gdk_window_set_cursor(app->os.window->window, c);
#endif

#ifdef SHOES_QUARTZ
  if (app->os.window == NULL || app->cursor == cursor)
    goto done;

  if (cursor == s_hand)
  {
    SetThemeCursor(kThemePointingHandCursor);
  }
  else if (cursor == s_arrow)
  {
    SetThemeCursor(kThemeArrowCursor);
  }
  else
    goto done;

#endif

#ifdef SHOES_WIN32
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
#endif

  app->cursor = cursor;

done:
  return SHOES_OK;
}

shoes_code
shoes_app_resize(shoes_app *app, int width, int height)
{
  app->width = width;
  app->height = height;

#ifdef SHOES_GTK
  if (app->os.window != NULL)
    gtk_widget_set_size_request(app->os.window, app->width, app->height);
#endif

#ifdef SHOES_QUARTZ
  Rect gRect;
  GetWindowBounds(app->os.window, kWindowContentRgn, &gRect);
  gRect.right = app->width + gRect.left;
  gRect.bottom = app->height + gRect.top;
  SetWindowBounds(app->os.window, kWindowContentRgn, &gRect);
#endif

#ifdef SHOES_WIN32
  if (app->slot.window != NULL)
  {
    RECT r;
    GetWindowRect(app->slot.window, &r);
    r.right = r.left + app->width;
    r.bottom = r.top + app->height;
    AdjustWindowRect(&r, WINDOW_STYLE, FALSE);
    MoveWindow(app->slot.window, r.left, r.top, r.right - r.left, r.bottom - r.top, TRUE);
  }
#endif

  return SHOES_OK;
}

VALUE
shoes_app_main(int argc, VALUE *argv, VALUE self)
{
  VALUE attr, block;
  GLOBAL_APP(app);

  rb_scan_args(argc, argv, "01&", &attr, &block);
  rb_iv_set(self, "@main_app", block);

  app->title = ATTR(attr, title);
  app->resizable = (ATTR(attr, resizable) != Qfalse);
  shoes_app_resize(app, ATTR2(int, attr, width, SHOES_APP_WIDTH), ATTR2(int, attr, height, SHOES_APP_HEIGHT));
  shoes_canvas_init(app->canvas, app->slot, attr, app->width, app->height);
  return self;
}

void
shoes_app_title(shoes_app *app, VALUE title)
{
  char *msg;
  app->title = rb_str_new2(SHOES_APPNAME);
  if (!NIL_P(title))
  {
    rb_str_cat2(app->title, " - ");
    rb_str_append(app->title, title);
  }
  msg = RSTRING_PTR(app->title);

#ifdef SHOES_GTK
  gtk_window_set_title(GTK_WINDOW(app->os.window), _(msg));
#endif

#ifdef SHOES_QUARTZ
  CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
  SetWindowTitleWithCFString(app->os.window, cfmsg);
#endif

#ifdef SHOES_WIN32
  SetWindowText(app->slot.window, msg);
#endif
}

shoes_code
shoes_app_start(VALUE appobj, char *uri)
{
  shoes_code code;
  shoes_app *app;
  Data_Get_Struct(appobj, shoes_app, app);

  code = shoes_app_open(app);
  if (code != SHOES_OK)
    return code;

  code = shoes_app_loop(app, uri);
  if (code != SHOES_OK)
    return code;

  return shoes_app_close(app);
}

shoes_code
shoes_app_open(shoes_app *app)
{
  shoes_code code = SHOES_OK;

#ifdef SHOES_GTK
  shoes_app_gtk *gk = &app->os;
  shoes_slot_gtk *gs = &app->slot;

  gtk_init(NULL, NULL);

  gtk_window_set_default_icon_from_file("static/shoes-icon.png", NULL);
  gk->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  if (!app->resizable)
    gtk_window_set_resizable(GTK_WINDOW(gk->window), FALSE);
  g_signal_connect(G_OBJECT(gk->window), "expose-event",
                    G_CALLBACK(shoes_app_gtk_paint), app);
  g_signal_connect(G_OBJECT(gk->window), "motion-notify-event",
                   G_CALLBACK(shoes_app_gtk_motion), app);
  g_signal_connect(G_OBJECT(gk->window), "button-press-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "button-release-event",
                   G_CALLBACK(shoes_app_gtk_button), app);
  g_signal_connect(G_OBJECT(gk->window), "delete-event",
                   G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(G_OBJECT(gk->window), "key-press-event",
                   G_CALLBACK(shoes_app_gtk_keypress), app);
  app->slot.canvas = gk->window;
#endif

#ifdef SHOES_QUARTZ
  const EventTypeSpec  windowEvents[]  =   {   
    { kEventClassCommand,   kEventCommandProcess },
    { kEventClassTextInput, kEventTextInputUpdateActiveInputArea },
    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
    { kEventClassMouse,    kEventMouseMoved },
    { kEventClassMouse,    kEventMouseDragged },
    { kEventClassMouse,    kEventMouseDown },
    { kEventClassMouse,    kEventMouseUp },
    { kEventClassWindow,   kEventWindowBoundsChanged }
  };

  Rect                   gRect;
  static EventHandlerUPP gTestWindowEventProc = NULL;
  OSStatus               err;
  EventLoopTimerRef      redrawTimer;

  app->slot.controls = Qnil;
  SetRect(&gRect, 100, 100, app->width + 100, app->height + 100);

  INFO("Draw QUARTZ window.\n", 0);
  err = CreateNewWindow(kDocumentWindowClass,
      kWindowCompositingAttribute
    // | kWindowLiveResizeAttribute
    | kWindowStandardDocumentAttributes
    | kWindowStandardHandlerAttribute
    ^ (app->resizable ? kWindowResizableAttribute : 0),
    &gRect,
    &app->os.window);

  if (err != noErr)
  {
    QUIT("Couldn't make a new window.", 0);
  }

  InitCursor();

  err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
    NewAEEventHandlerUPP(shoes_app_quartz_quit), 0, false);
  if (err != noErr)
  {
    QUIT("Out of memory.", 0);
  }

  err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, 
    NewAEEventHandlerUPP(shoes_app_quartz_open), 0, false);
  if (err != noErr)
  {
    QUIT("Out of memory.", 0);
  }

  gTestWindowEventProc = NewEventHandlerUPP(shoes_app_quartz_handler);
  if (gTestWindowEventProc == NULL)
  {
    QUIT("Out of memory.", 0);
  }

  INFO("Event handler.\n", 0);
  err = InstallWindowEventHandler(app->os.window,
    gTestWindowEventProc, GetEventTypeCount(windowEvents),
    windowEvents, app, NULL);

  HIViewFindByID(HIViewGetRoot(app->os.window), kHIViewWindowContentID, &app->slot.view);

  if (PasteboardCreate(kPasteboardClipboard, &shoes_world->os.clip) != noErr) {
    INFO("Apple Pasteboard create failed.\n", 0);
  }
#endif

#ifdef SHOES_WIN32
  RECT rect;

  app->slot.controls = Qnil;
  app->slot.focus = Qnil;
  app->os.ctrlkey = false;
  app->os.altkey = false;
  app->os.shiftkey = false;
  app->os.classex.hInstance = shoes_world->os.instance;
  app->os.classex.lpszClassName = SHOES_SHORTNAME;
  app->os.classex.lpfnWndProc = shoes_app_win32proc;
  app->os.classex.style = CS_HREDRAW | CS_VREDRAW;
  app->os.classex.cbSize = sizeof(WNDCLASSEX);
  app->os.classex.hIcon = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  app->os.classex.hIconSm = LoadIcon(shoes_world->os.instance, IDI_APPLICATION);
  app->os.classex.hCursor = LoadCursor(NULL, IDC_ARROW);
  app->os.classex.lpszMenuName = NULL;
  app->os.classex.cbClsExtra = 0;
  app->os.classex.cbWndExtra = 0;
  app->os.classex.hbrBackground = 0;

  if (!RegisterClassEx(&app->os.classex))
  {
    QUIT("Couldn't register WIN32 window class.");
  }

  app->os.vlclassex.hInstance = app->os.slotex.hInstance = shoes_world->os.instance;
  app->os.vlclassex.lpszClassName = SHOES_VLCLASS;
  app->os.slotex.lpszClassName = SHOES_SLOTCLASS;
  app->os.vlclassex.style = app->os.slotex.style = CS_NOCLOSE;
  app->os.vlclassex.lpfnWndProc = DefWindowProc;
  app->os.slotex.lpfnWndProc = shoes_slot_win32proc;
  app->os.vlclassex.cbSize = app->os.slotex.cbSize = sizeof(WNDCLASSEX);
  app->os.vlclassex.hIcon = app->os.slotex.hIcon = NULL;
  app->os.vlclassex.hIconSm = app->os.slotex.hIconSm = NULL;
  app->os.vlclassex.hCursor = app->os.slotex.hCursor = LoadCursor(NULL, IDC_ARROW);
  app->os.vlclassex.lpszMenuName = app->os.slotex.lpszMenuName = NULL;
  app->os.vlclassex.cbClsExtra = app->os.slotex.cbClsExtra = 0;
  app->os.vlclassex.cbWndExtra = app->os.slotex.cbWndExtra = 0;
  app->os.vlclassex.hbrBackground = app->os.slotex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

  if (!RegisterClassEx(&app->os.slotex) || !RegisterClassEx(&app->os.vlclassex))
  {
    QUIT("Couldn't register VLC window class.");
  }

  // remove the menu
  rect.left = 0;
  rect.top = 0;
  rect.right = app->width;
  rect.bottom = app->height;
  AdjustWindowRect(&rect, WINDOW_STYLE, FALSE);

  app->slot.window = CreateWindowEx(
    WS_EX_CLIENTEDGE, SHOES_SHORTNAME, SHOES_APPNAME,
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
#endif

  shoes_app_title(app, app->title);

quit:
  return code;
}

shoes_code
shoes_app_loop(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;
#ifndef SHOES_GTK
  app->slot.controls = rb_ary_new();
  rb_gc_register_address(&app->slot.controls);
#endif
  shoes_slot_init(app->canvas, &app->slot, 0, 0, app->width, app->height, TRUE);
  code = shoes_app_goto(app, path);
  if (code != SHOES_OK)
    return code;
  INFO("RUNNING LOOP.\n", 0);

#ifdef SHOES_QUARTZ
  ShowWindow(app->os.window);
  TextEncoding utf8Encoding, unicodeEncoding;
  utf8Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicodeUTF8Format);
  unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicode16BitFormat);
  TECCreateConverter(&shoes_world->os.converter, unicodeEncoding, utf8Encoding);
  RunApplicationEventLoop();
#endif

#ifdef SHOES_GTK
  gtk_widget_show_all(app->os.window);
  g_idle_add(shoes_app_gtk_idle, app);
  gtk_main();
#endif

#ifdef SHOES_WIN32
  MSG msgs;
  ShowWindow(app->slot.window, SW_SHOWNORMAL);
  while (WM_QUIT != msgs.message)
  {
    BOOL msg = PeekMessage(&msgs, NULL, 0, 0, PM_REMOVE);
    if (msg)
    {
      if (msgs.message == WM_KEYDOWN || msgs.message == WM_KEYUP)
      {
        if (RARRAY_LEN(app->slot.controls) > 0)
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
      if (msg) msg = IsDialogMessage(app->slot.window, &msgs);
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
#endif

quit:
  return SHOES_OK;
}

typedef struct
{
  shoes_app *app;
  VALUE canvas;
  VALUE block;
  char ieval;
  VALUE args;
} shoes_exec;

struct METHOD {
    VALUE klass, rklass;
    VALUE recv;
    ID id, oid;
    int safe_level;
    NODE *body;
};

static VALUE
rb_unbound_get_class(VALUE method)
{
  struct METHOD *data;
  Data_Get_Struct(method, struct METHOD, data);
  return data->rklass;
}

static VALUE
shoes_app_run(VALUE rb_exec)
{
  shoes_exec *exec = (shoes_exec *)rb_exec;
  rb_ary_push(exec->app->nesting, exec->canvas);
  if (exec->ieval)
  {
    VALUE obj;
    obj = mfp_instance_eval(exec->app->self, exec->block);
    return obj;
  }
  else
  {
    int i;
    VALUE vargs[10];
    for (i = 0; i < RARRAY_LEN(exec->args); i++)
      vargs[i] = rb_ary_entry(exec->args, i);
    return rb_funcall2(exec->block, s_call, RARRAY_LEN(exec->args), vargs);
  }
}

static VALUE
shoes_app_exception(VALUE rb_exec, VALUE e)
{
  shoes_exec *exec = (shoes_exec *)rb_exec;
  rb_ary_clear(exec->app->nesting);
  rb_iv_set(exec->canvas, "@exc", e);
  return mfp_instance_eval(exec->canvas, exception_proc);
}

shoes_code
shoes_app_visit(shoes_app *app, char *path)
{
  long i;
  shoes_exec exec;
  shoes_canvas *canvas;
  VALUE meth;
  VALUE ary = rb_ary_dup(app->timers);
  Data_Get_Struct(app->canvas, shoes_canvas, canvas);

#ifdef SHOES_WIN32
  canvas->slot.scrolly = 0;
#endif
#ifndef SHOES_GTK
  rb_ary_clear(app->slot.controls);
#endif
  for (i = 0; i < RARRAY_LEN(ary); i++) 
  {
    VALUE timer = rb_ary_entry(ary, i);
    if (!NIL_P(timer))
      rb_funcall(timer, s_remove, 0);
  }

  shoes_canvas_clear(app->canvas);
  shoes_app_reset_styles(app);
  meth = rb_funcall(cShoes, s_run, 1, app->location = rb_str_new2(path));
  if (NIL_P(rb_ary_entry(meth, 0)))
  {
    VALUE script = shoes_dialog_open(app->canvas);
    if (NIL_P(script))
      return SHOES_QUIT;
    rb_funcall(cShoes, rb_intern("load"), 1, script);
    meth = rb_funcall(cShoes, s_run, 1, app->location);
  }

  exec.app = app;
  exec.block = rb_ary_entry(meth, 0);
  exec.args = rb_ary_entry(meth, 1);
  if (rb_obj_is_kind_of(exec.block, rb_cUnboundMethod)) {
    VALUE klass = rb_unbound_get_class(exec.block);
    exec.canvas = shoes_slot_new(klass, Qnil, app->canvas);
    exec.block = rb_funcall(exec.block, s_bind, 1, exec.canvas);
    exec.ieval = 0;
    rb_ary_push(canvas->contents, exec.canvas);
  } else {
    exec.canvas = app->canvas;
    exec.ieval = 1;
  }
  rb_rescue2(CASTHOOK(shoes_app_run), (VALUE)&exec, CASTHOOK(shoes_app_exception), (VALUE)&exec, rb_cObject, 0);
  rb_ary_clear(exec.app->nesting);
  return SHOES_OK;
}

shoes_code
shoes_app_paint(shoes_app *app)
{
  shoes_canvas_paint(app->canvas);
  return SHOES_OK;
}

shoes_code
shoes_app_motion(shoes_app *app, int x, int y)
{
  app->mousex = x; app->mousey = y;
  shoes_canvas_send_motion(app->canvas, x, y, Qnil);
  return SHOES_OK;
}

shoes_code
shoes_app_click(shoes_app *app, int button, int x, int y)
{
  shoes_canvas_send_click(app->canvas, button, x, y);
  return SHOES_OK;
}

shoes_code
shoes_app_release(shoes_app *app, int button, int x, int y)
{
  shoes_canvas_send_release(app->canvas, button, x, y);
  return SHOES_OK;
}

shoes_code
shoes_app_keypress(shoes_app *app, VALUE key)
{
  shoes_canvas_send_keypress(app->canvas, key);
  return SHOES_OK;
}

shoes_code
shoes_app_close(shoes_app *app)
{
  return SHOES_OK;
}

VALUE
shoes_sys(char *cmd, int detach)
{
  if (detach)
    return rb_funcall(rb_mKernel, rb_intern("system"), 1, rb_str_new2(cmd));
  else
    return rb_funcall(rb_mKernel, '`', 1, rb_str_new2(cmd));
}

void
shoes_browser_open(char *url)
{
#ifdef SHOES_GTK
  VALUE browser = rb_str_new2("/etc/alternatives/x-www-browser '");
  rb_str_cat2(browser, url);
  rb_str_cat2(browser, "' 2>/dev/null &");
  shoes_sys(RSTRING_PTR(browser), 1);
#endif

#ifdef SHOES_QUARTZ
  VALUE browser = rb_str_new2("open ");
  rb_str_cat2(browser, url);
  shoes_sys(RSTRING_PTR(browser), 1);
#endif

#ifdef SHOES_WIN32
  ShellExecute(0, "open", url, 0, 0, 0);
#endif
}

shoes_code
shoes_app_goto(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;
  const char http_scheme[] = "http://";
  if (strlen(path) > strlen(http_scheme) && strncmp(http_scheme, path, strlen(http_scheme)) == 0) {
    shoes_browser_open(path);
  } else {
    code = shoes_app_visit(app, path);
    if (code == SHOES_OK)
    {
      shoes_app_motion(app, app->mousex, app->mousey);
      shoes_slot_repaint(&app->slot);
    }
  }
  return code;
}

shoes_code
shoes_slot_repaint(SHOES_SLOT_OS *slot)
{
#ifdef SHOES_GTK
  gtk_widget_queue_draw(slot->canvas);
#endif
#ifdef SHOES_QUARTZ
  HIViewSetNeedsDisplay(slot->view, true);
#endif
#ifdef SHOES_WIN32
  InvalidateRgn(slot->window, NULL, TRUE);
  UpdateWindow(slot->window);
#endif
  return SHOES_OK;
}

static void
shoes_style_set(VALUE styles, VALUE klass, VALUE k, VALUE v)
{
  VALUE hsh = rb_hash_aref(styles, klass);
  if (NIL_P(hsh))
    rb_hash_aset(styles, klass, hsh = rb_hash_new());
  rb_hash_aset(hsh, k, v);
}

#define STYLE(klass, k, v) \
  shoes_style_set(app->styles, klass, \
    ID2SYM(rb_intern("" # k)), rb_str_new2("" # v))

void
shoes_app_reset_styles(shoes_app *app)
{
  app->styles = rb_hash_new();
  STYLE(cBanner,      size, 48);
  STYLE(cTitle,       size, 34);
  STYLE(cSubtitle,    size, 26);
  STYLE(cTagline,     size, 18);
  STYLE(cCaption,     size, 14);
  STYLE(cPara,        size, 12);
  STYLE(cInscription, size, 10);

  STYLE(cCode,        family, monospace);
  STYLE(cDel,         strikethrough, single);
  STYLE(cEm,          emphasis, italic);
  STYLE(cIns,         underline, single);
  STYLE(cLink,        underline, single);
  STYLE(cLink,        stroke, #06E);
  STYLE(cLinkHover,   underline, single);
  STYLE(cLinkHover,   stroke, #039);
  STYLE(cLinkHover,   fill,   #EEE);
  STYLE(cStrong,      weight, bold);
  STYLE(cSup,         rise,   10);
  STYLE(cSup,         size,   x-small);
  STYLE(cSub,         rise,   -10);
  STYLE(cSub,         size,   x-small);
}

void
shoes_app_style(shoes_app *app, VALUE klass, VALUE hsh)
{
  long i;
  VALUE keys = rb_funcall(hsh, s_keys, 0);
  for ( i = 0; i < RARRAY(keys)->len; i++ )
  {
    VALUE key = rb_ary_entry(keys, i);
    VALUE val = rb_hash_aref(hsh, key);
    if (!SYMBOL_P(key)) key = rb_str_intern(key);
    shoes_style_set(app->styles, klass, key, val);
  }
}

VALUE
shoes_app_location(VALUE self)
{
  shoes_app *app;
  Data_Get_Struct(self, shoes_app, app);
  return app->location;
}

VALUE
shoes_app_quit(VALUE self)
{
#ifdef SHOES_GTK
  gtk_main_quit();
#endif
#ifdef SHOES_QUARTZ
  QuitApplicationEventLoop();
#endif
#ifdef SHOES_WIN32
  PostQuitMessage(0);
#endif
  return self;
}
