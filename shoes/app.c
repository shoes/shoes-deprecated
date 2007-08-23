//
// shoes/app.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "node.h"

shoes_app *global_app = NULL;

shoes_app *
shoes_app_new()
{
  shoes_app *app = SHOE_ALLOC(shoes_app);
  app->canvas = shoes_canvas_new(cCanvas, app);
  rb_gc_register_address(&app->canvas);
  app->timers = rb_ary_new();
  rb_gc_register_address(&app->timers);
  app->width = SHOES_APP_WIDTH;
  app->height = SHOES_APP_HEIGHT;
#ifdef SHOES_WIN32
  app->slot.window = NULL;
#else
  app->kit.window = NULL;
#endif
  return app;
}

void
shoes_app_free(shoes_app *app)
{
#ifdef SHOES_QUARTZ
  CFRelease(app->kit.clip);
  TECDisposeConverter(app->kit.converter);
#endif
  rb_gc_unregister_address(&app->canvas);
  rb_gc_unregister_address(&app->timers);
  if (app != NULL)
    SHOE_FREE(app);
}

#ifdef SHOES_GTK
static gboolean
shoes_app_gtk_idle(gpointer data)
{
  rb_eval_string("sleep(0.01)");
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
  gtk_container_propagate_expose(GTK_CONTAINER(app->kit.window), widget, app->slot.expose);
}

static void
shoes_app_gtk_paint (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{ 
  shoes_app *app = (shoes_app *)data;
  gtk_window_get_size(GTK_WINDOW(app->kit.window), &app->width, &app->height);
  shoes_canvas_size(app->canvas, app->width, app->height);
  app->slot.expose = event;
  gtk_container_forall(GTK_CONTAINER(app->kit.window), shoes_app_gtk_paint_children, app);
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

        case kEventControlHitTest:
          INFO("kEventControlHitTest\n", 0);
          err = noErr;
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

        case kEventControlClick:
          INFO("kEventControlClick\n", 0);
          err = noErr;
        break;
      }
    break;
  }

  INFO("End of window proc\n", 0);
  return err;
}

static OSStatus
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
      { kEventClassControl, kEventControlHitTest },
      { kEventClassControl, kEventControlInitialize },
      { kEventClassControl, kEventControlGetData },
      { kEventClassControl, kEventControlSetData },
      { kEventClassControl, kEventControlClick }
    };

    err = HIObjectRegisterSubclass(kShoesViewClassID, kHIViewClassID,
      0, shoes_slot_quartz_handler, GetEventTypeCount(eventList), eventList,
      NULL, &objcls);
  }
  return err;
}

OSStatus
shoes_slot_quartz_create(VALUE self, APPSLOT *parent, int w, int h)
{
  HIRect rect;
  OSStatus err;
  EventRef event;
  shoes_canvas *canvas;
  APPSLOT *slot;
  Data_Get_Struct(self, shoes_canvas, canvas);
  slot = &canvas->slot;

  //
  // Create the scroll view
  //
  HIScrollViewCreate(kHIScrollViewOptionsVertScroll | kHIScrollViewOptionsHorizScroll | kHIScrollViewOptionsAllowGrow, &slot->scrollview);
  HIScrollViewSetScrollBarAutoHide(slot->scrollview, true);
  rect.origin.x = 0.0;
  rect.origin.y = 0.0;
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
  rect.size.width = (double)w * 2;
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
    shoes_app_keypress(global_app, v); \
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
  UInt len;
  
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
      INFO("kEventClassCommand\n", 0);
      if (!NIL_P(app->slot.controls))
      {
        HICommand aCommand;
        GetEventParameter(inEvent, kEventParamDirectObject, typeHICommand, NULL, sizeof(aCommand), NULL, &aCommand);
        if (aCommand.commandID >= SHOES_CONTROL1 && aCommand.commandID < SHOES_CONTROL1 + RARRAY_LEN(app->slot.controls))
        {
          VALUE control = rb_ary_entry(app->slot.controls, aCommand.commandID - SHOES_CONTROL1);
          if (!NIL_P(control))
            shoes_control_send(control, s_click);
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
              status = TECConvertText(app->kit.converter, text, len, &nread, text8, len, &nwrite);

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
      GetWindowBounds(app->kit.window, kWindowContentRgn, &bounds);
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
  InvalWindowRect(app->kit.window, &windowRect);
}

static pascal OSErr
shoes_app_quartz_quit(const AppleEvent *appleEvt, AppleEvent* reply, long refcon)
{
#pragma unused (appleEvt, reply, refcon)
  QuitApplicationEventLoop();
  return 128;
}
#endif

#ifdef SHOES_WIN32
#define WINDOW_STYLE WS_OVERLAPPEDWINDOW

#define WM_POINTS() \
  x = LOWORD(l); \
  y = HIWORD(l)

#define KEY_SYM(sym)  shoes_app_keypress(global_app, ID2SYM(rb_intern("" # sym)))
#define KEYPRESS(name, sym) \
  else if (w == VK_##name) { \
    VALUE v = ID2SYM(rb_intern("" # sym)); \
    if (global_app->kit.altkey) \
      KEY_STATE(alt); \
    if (global_app->kit.shiftkey) \
      KEY_STATE(shift); \
    if (global_app->kit.ctrlkey) \
      KEY_STATE(control); \
    shoes_app_keypress(global_app, v); \
  }

LRESULT CALLBACK
shoes_app_win32proc(
  HWND win,
  UINT msg,
  WPARAM w,
  LPARAM l)
{
  int x = 0, y = 0;

  switch (msg)
  {
    case WM_DESTROY:
      PostQuitMessage(0);
    return 0; 

    case WM_PAINT:
    {
      RECT rect;
      GetClientRect(global_app->slot.window, &rect);
      global_app->width = rect.right;
      global_app->height = rect.bottom;
      shoes_canvas_size(global_app->canvas, global_app->width, global_app->height);
      shoes_app_paint(global_app);
    }
    break;

    case WM_LBUTTONDOWN:
      WM_POINTS();
      shoes_app_click(global_app, 1, x, y);
    break;

    case WM_RBUTTONDOWN:
      WM_POINTS();
      shoes_app_click(global_app, 2, x, y);
    break;

    case WM_MBUTTONDOWN:
      WM_POINTS();
      shoes_app_click(global_app, 3, x, y);
    break;

    case WM_LBUTTONUP:
      WM_POINTS();
      shoes_app_release(global_app, 1, x, y);
    break;

    case WM_RBUTTONUP:
      WM_POINTS();
      shoes_app_release(global_app, 2, x, y);
    break;

    case WM_MBUTTONUP:
      WM_POINTS();
      shoes_app_release(global_app, 3, x, y);
    break;

    case WM_MOUSEMOVE:
      WM_POINTS();
      shoes_app_motion(global_app, x, y);
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
          shoes_app_keypress(global_app, rb_str_new2("\n"));
        break;

        default:
        {
          VALUE v;
          WCHAR _str = w;
          CHAR str[10];
          DWORD len = WideCharToMultiByte(CP_UTF8, 0, &_str, 1, (LPSTR)str, 10, NULL, NULL);
          str[len] = '\0';
          v = rb_str_new(str, len);
          shoes_app_keypress(global_app, v);
        }
      }
    break;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      if (w == VK_CONTROL)
        global_app->kit.ctrlkey = true;
      else if (w == VK_MENU)
        global_app->kit.altkey = true;
      else if (w == VK_SHIFT)
        global_app->kit.shiftkey = true;
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
        if (global_app->kit.altkey) {
          KEY_STATE(alt);
          shoes_app_keypress(global_app, v);
        }
      }
    break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
      if (w == VK_CONTROL)
        global_app->kit.ctrlkey = false;
      else if (w == VK_MENU)
        global_app->kit.altkey = false;
      else if (w == VK_SHIFT)
        global_app->kit.shiftkey = false;
    break;

    case WM_TIMER:
    {
      int id = LOWORD(w);
      VALUE timer = rb_ary_entry(global_app->timers, id - SHOES_CONTROL1);
      if (!NIL_P(timer))
        shoes_anim_call(timer);
    }
    break;

    case WM_COMMAND:
      if ((HWND)l && HIWORD(w) == BN_CLICKED)
      {
        int id = LOWORD(w);
        VALUE control = rb_ary_entry(global_app->slot.controls, id - SHOES_CONTROL1);
        if (!NIL_P(control))
          shoes_control_send(control, s_click);
      }
    break;
  }
  return DefWindowProc(win, msg, w, l);
}
#endif

shoes_code
shoes_app_load(shoes_app *app, char *uri)
{
  char bootup[512];
  if (global_app == NULL)
    global_app = app;

  sprintf(bootup,
    "begin;"
      "DIR = File.expand_path(File.dirname(%%q<%s>));"
      "$:.replace([DIR+'/ruby/lib/'+PLATFORM, DIR+'/ruby/lib', DIR+'/lib']);"
      "require 'shoes';"
      "'OK';"
    "rescue Object => e;"
      SHOES_META
        "define_method :load do |path|; end;"
        EXC_RUN
      "end;"
      "e.message;"
    "end",
    app->path);
  VALUE str = rb_eval_string(bootup);
  StringValue(str);
  INFO("Bootup: %s\n", RSTRING(str)->ptr);

  if (uri != NULL)
  {
    sprintf(bootup,
      "begin;"
        "Shoes.load(%%q<%s>);"
      "rescue Object => e;"
        SHOES_META
          EXC_RUN
        "end;"
      "end;",
      uri);
    rb_eval_string(bootup);
  }

  return SHOES_OK;
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
#ifdef SHOES_GTK
  if (app->kit.window == NULL || app->kit.window->window == NULL || app->cursor == cursor)
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

  gdk_window_set_cursor(app->kit.window->window, c);
#endif

#ifdef SHOES_QUARTZ
  if (app->kit.window == NULL || app->cursor == cursor)
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
  if (app->kit.window != NULL)
    gtk_widget_set_size_request(app->kit.window, app->width, app->height);
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
  int width, height;
  VALUE attr, block;
  rb_scan_args(argc, argv, "01&", &attr, &block);
  rb_iv_set(self, "@main_app", block);
  shoes_app_resize(global_app, ATTR2(int, attr, width, SHOES_APP_WIDTH), ATTR2(int, attr, height, SHOES_APP_HEIGHT));
  shoes_canvas_init(global_app->canvas, global_app->slot, attr, global_app->width, global_app->height);
  return self;
}

shoes_code
shoes_app_open(shoes_app *app)
{
  shoes_code code = SHOES_OK;

#ifdef SHOES_GTK
  shoes_app_gtk *gk = &app->kit;
  shoes_slot_gtk *gs = &app->slot;

  gtk_init(NULL, NULL);

  gtk_window_set_default_icon_from_file("static/shoes-icon.png", NULL);
  gk->window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(gk->window), _(SHOES_APPNAME));
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
  CFStringRef            titleKey;
  CFStringRef            windowTitle;

  app->slot.controls = Qnil;
  SetRect(&gRect, 100, 100, app->width + 100, app->height + 100);

  INFO("Draw QUARTZ window.\n", 0);
  err = CreateNewWindow(kDocumentWindowClass,
      kWindowCompositingAttribute
    // | kWindowLiveResizeAttribute
    | kWindowStandardDocumentAttributes
    | kWindowStandardHandlerAttribute,
    &gRect,
    &app->kit.window);

  if (err != noErr)
  {
    QUIT("Couldn't make a new window.", 0);
  }

  titleKey = CFSTR(SHOES_APPNAME);
  windowTitle = CFCopyLocalizedString(titleKey, NULL);
  err = SetWindowTitleWithCFString(app->kit.window, windowTitle);
  if (err != noErr)
  {
    QUIT("Couldn't set the window title.", 0);
  }

  InitCursor();

  err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
    NewAEEventHandlerUPP(shoes_app_quartz_quit), 0, false);
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
  err = InstallWindowEventHandler(app->kit.window,
    gTestWindowEventProc, GetEventTypeCount(windowEvents),
    windowEvents, app, NULL);

  HIViewFindByID(HIViewGetRoot(app->kit.window), kHIViewWindowContentID, &app->slot.view);

  CFRelease(titleKey);
  CFRelease(windowTitle);

  if (PasteboardCreate(kPasteboardClipboard, &app->kit.clip) != noErr) {
    INFO("Apple Pasteboard create failed.\n", 0);
  }
#endif

#ifdef SHOES_WIN32
  RECT rect;

  app->slot.controls = Qnil;
  app->kit.ctrlkey = false;
  app->kit.altkey = false;
  app->kit.shiftkey = false;
  app->kit.classex.hInstance = app->kit.instance;
  app->kit.classex.lpszClassName = SHOES_SHORTNAME;
  app->kit.classex.lpfnWndProc = shoes_app_win32proc;
  app->kit.classex.style = CS_HREDRAW | CS_VREDRAW;
  app->kit.classex.cbSize = sizeof(WNDCLASSEX);
  app->kit.classex.hIcon = LoadIcon(app->kit.instance, IDI_APPLICATION);
  app->kit.classex.hIconSm = LoadIcon(app->kit.instance, IDI_APPLICATION);
  app->kit.classex.hCursor = LoadCursor(NULL, IDC_ARROW);
  app->kit.classex.lpszMenuName = NULL;
  app->kit.classex.cbClsExtra = 0;
  app->kit.classex.cbWndExtra = 0;
  app->kit.classex.hbrBackground = 0;

  if (!RegisterClassEx(&app->kit.classex))
  {
    QUIT("Couldn't register WIN32 window class.");
  }

  // remove the menu
  rect.left = 0;
  rect.top = 0;
  rect.right = app->width;
  rect.bottom = app->height;
  AdjustWindowRect(&rect, WINDOW_STYLE, FALSE);

  app->slot.window = CreateWindowEx(
    0, SHOES_SHORTNAME, SHOES_APPNAME,
    WINDOW_STYLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    rect.right-rect.left, rect.bottom-rect.top,
    HWND_DESKTOP,
    NULL,
    app->kit.instance,
    NULL);
#endif

quit:
  return code;
}

shoes_code
shoes_app_loop(shoes_app *app, char *path)
{
#ifndef SHOES_GTK
  app->slot.controls = rb_ary_new();
#endif
  shoes_slot_init(app->canvas, &app->slot, app->width, app->height);
  shoes_app_goto(app, path);
  INFO("RUNNING LOOP.\n", 0);

#ifdef SHOES_QUARTZ
  ShowWindow(app->kit.window);
  TextEncoding utf8Encoding, unicodeEncoding;
  utf8Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicodeUTF8Format);
  unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicode16BitFormat);
  TECCreateConverter(&app->kit.converter, unicodeEncoding, utf8Encoding);
  RunApplicationEventLoop();
#endif

#ifdef SHOES_GTK
  gtk_widget_show_all(app->kit.window);
  // g_idle_add(shoes_app_gtk_idle, app);
  gtk_main();
#endif

#ifdef SHOES_WIN32
  MSG msgs;
  ShowWindow(app->slot.window, app->kit.style);
  while (GetMessage(&msgs, NULL, 0, 0))
  {
    TranslateMessage(&msgs);
    DispatchMessage(&msgs);
  }
#endif

quit:
  return SHOES_OK;
}

typedef struct
{
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
  if (exec->ieval)
  {
    return mfp_instance_eval(exec->canvas, exec->block);
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
  rb_iv_set(exec->canvas, "@exc", e);
  return mfp_instance_eval(exec->canvas, exception_proc);
}

shoes_code
shoes_app_visit(shoes_app *app, char *path)
{
  long i;
  shoes_exec exec;
  VALUE meth;
  VALUE ary = rb_ary_dup(app->timers);
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
  meth = rb_funcall(cShoes, s_run, 1, rb_str_new2(path));
  exec.block = rb_ary_entry(meth, 0);
  exec.args = rb_ary_entry(meth, 1);
  if (rb_obj_is_kind_of(exec.block, rb_cUnboundMethod)) {
    shoes_canvas *canvas;
    VALUE klass = rb_unbound_get_class(exec.block);
    Data_Get_Struct(app->canvas, shoes_canvas, canvas);
    exec.canvas = shoes_slot_new(klass, Qnil, app->canvas);
    exec.block = rb_funcall(exec.block, s_bind, 1, exec.canvas);
    exec.ieval = 0;
    rb_ary_push(canvas->contents, exec.canvas);
  } else {
    exec.canvas = app->canvas;
    exec.ieval = 1;
  }
  rb_rescue2(CASTHOOK(shoes_app_run), (VALUE)&exec, CASTHOOK(shoes_app_exception), (VALUE)&exec, rb_cObject, 0);
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

shoes_code
shoes_app_goto(shoes_app *app, char *path)
{
  shoes_app_visit(app, path);
  shoes_canvas_compute(app->canvas);
  shoes_slot_repaint(&app->slot);
  shoes_app_cursor(app, s_arrow);
  return SHOES_OK;
}

shoes_code
shoes_slot_repaint(APPSLOT *slot)
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

shoes_code
shoes_init()
{
  g_type_init();
#ifdef SHOES_WIN32
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&InitCtrlEx);
#endif
  ruby_init();
  shoes_ruby_init();
#ifdef SHOES_QUARTZ
  shoes_slot_quartz_register();
#endif
  return SHOES_OK;
}
