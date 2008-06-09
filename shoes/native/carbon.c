//
// shoes/native-carbon.c
// Carbon-specific code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

void shoes_native_init()
{
  shoes_app_quartz_install();
  shoes_slot_quartz_register();
  if (PasteboardCreate(kPasteboardClipboard, &shoes_world->os.clip) != noErr) {
    INFO("Apple Pasteboard create failed.\n");
  }
}

void shoes_native_cleanup(shoes_world_t *world)
{
  CFRelease(world->os.clip);
  TECDisposeConverter(world->os.converter);
}

void shoes_native_quit()
{
  QuitApplicationEventLoop();
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot)
{
  rb_gc_mark_maybe(slot->controls);
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
  HIViewSetNeedsDisplay(slot->view, true);
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  int scrollwidth;
  GetThemeMetric(kThemeMetricScrollBarWidth, (SInt32 *)&scrollwidth);
  return scrollwidth;
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
static MenuRef HelpMenu;
pascal void shoes_app_quartz_redraw(EventLoopTimerRef theTimer, void* userData);
pascal void shoes_app_quartz_idle(EventLoopTimerRef theTimer, void* userData);
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
          INFO("kEventHIObjectConstruct\n");
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
          INFO("kEventHIObjectInitialize\n");
          HIRect bounds;
          err = CallNextEventHandler(inCallRef, inEvent);
          if (err != noErr) break;
          err = GetEventParameter(inEvent, kShoesBoundEvent, typeHIRect, NULL, sizeof(HIRect), NULL, &bounds);
          if (err != noErr) break;
          HIViewSetFrame(qdata->view, &bounds);
        }
        break;

        case kEventHIObjectDestruct:
          INFO("kEventHIObjectDestruct\n");
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
          HIViewGetBounds(canvas->slot.view, &bounds);
          SetEventParameter(inEvent, kEventParamViewSize, typeHISize, sizeof(bounds.size), &bounds.size);
          bounds.size.height = 50;
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
          canvas->slot.scrolly = (where.y < 0.0) ? 0 : (int)where.y;
          HIViewSetNeedsDisplay(canvas->slot.view, true);
          err = noErr;
        }
        break;
      }
    break;

    case kEventClassControl:
      switch (eventKind)
      {
        case kEventControlInitialize:
          INFO("kEventHIControlInitialize\n");
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
          INFO("kEventHIControlDraw\n");
          shoes_canvas *canvas;
          Data_Get_Struct(qdata->canvas, shoes_canvas, canvas);
          INFO("Getting context\n");
          GetEventParameter(inEvent, kEventParamCGContextRef, typeCGContextRef,
            NULL, sizeof(CGContextRef), NULL, &canvas->slot.context);
          INFO("Got context: %lu\n", canvas->slot.context);
          shoes_canvas_paint(qdata->canvas);
          INFO("Painted!\n");
          err = noErr;
        }
        break;

        case kEventControlGetData:
        {
          OSType tag;
          Ptr ptr;
          Size outSize;

          INFO("kEventControlGetData\n");
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

          INFO("kEventControlSetData\n");
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

  INFO("End of window proc\n");
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

pascal void
shoes_slot_scroll_action(ControlRef vscroll, short part)
{
}

OSStatus
shoes_slot_quartz_create(VALUE self, SHOES_SLOT_OS *parent, int x, int y, int w, int h, int s)
{
  HIRect rect;
  OSStatus err;
  EventRef event;
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Rect bounds = {0, w - 16, h, w};
  Data_Get_Struct(self, shoes_canvas, canvas);
  slot = &canvas->slot;

  //
  // Create the content view
  //
  CreateEvent(NULL, kEventClassHIObject, kEventHIObjectInitialize,
    GetCurrentEventTime(), 0, &event);

  rect.origin.x = (double)x;
  rect.origin.y = (double)y;
  rect.size.width = (double)w;
  rect.size.height = (double)h * 2;
  SetEventParameter(event, kShoesBoundEvent, typeHIRect, sizeof(HIRect), &rect);
  HIObjectCreate(kShoesViewClassID, event, (HIObjectRef *)&slot->view);
  
  slot->vscroll = NULL;
  if (s) {
    CreateScrollBarControl(slot->view, &bounds, 0, 0, 1, h, true, NewControlActionUPP(shoes_slot_scroll_action), &slot->vscroll);
    HIViewAddSubview(slot->view, slot->vscroll);
    HIViewSetVisible(slot->vscroll, true);
  }

  SetControlData(slot->view, 1, kShoesSlotData, sizeof(VALUE), self);
  HIViewAddSubview(parent->view, slot->view);
  HIViewSetVisible(slot->view, true);
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

        case kEventWindowClose:
        {
          shoes_app_remove(app);
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
        if (aCommand.commandID == SHOES_HELP_MANUAL)
        {
          rb_eval_string("Shoes.show_manual"); 
        }
        else if (aCommand.commandID >= SHOES_CONTROL1 && aCommand.commandID < SHOES_CONTROL1 + RARRAY_LEN(app->slot.controls))
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
      INFO("kEventClassTextInput\n");
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
    {
      shoes_canvas *canvas;
      GetMouse(&mouseLoc);
      GetWindowBounds(app->os.window, kWindowContentRgn, &bounds);
      if (mouseLoc.h < bounds.left || mouseLoc.v < bounds.top) break;
      GetEventParameter(inEvent, kEventParamMouseButton, typeMouseButton, 0, sizeof(EventMouseButton), 0, &button);
      Data_Get_Struct(app->canvas, shoes_canvas, canvas);
      switch (eventKind)
      {
        case kEventMouseMoved:
        case kEventMouseDragged:
          shoes_app_motion(app, mouseLoc.h - bounds.left, (mouseLoc.v - bounds.top) + canvas->slot.scrolly);
        break;

        case kEventMouseDown:
          shoes_app_click(app, button, mouseLoc.h - bounds.left, (mouseLoc.v - bounds.top) + canvas->slot.scrolly);
        break;

        case kEventMouseUp:
          shoes_app_release(app, button, mouseLoc.h - bounds.left, (mouseLoc.v - bounds.top) + canvas->slot.scrolly);
        break;
      }
    }
    break;
  }

  INFO("End of main window proc\n");
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

pascal void
shoes_app_quartz_idle(
  EventLoopTimerRef theTimer,
  void* userData)
{
  rb_eval_string("sleep(0.001)");
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
  char _path[SHOES_BUFSIZE];

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
          printf("Opening %s\n", _path);
          shoes_load(_path);
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

void 
shoes_app_quartz_install()
{
  AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, 
    NewAEEventHandlerUPP(shoes_app_quartz_quit), 0, false);

  AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, 
    NewAEEventHandlerUPP(shoes_app_quartz_open), 0, false);
}

static pascal void
shoes_quartz_cancel_app_loop(EventLoopTimerRef timer, void* userData)
{
  QuitEventLoop(GetCurrentEventLoop());
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
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

  app->cursor = cursor;

done:
  return SHOES_OK;
}

void
shoes_native_app_resized(shoes_app *app)
{
  Rect gRect;
  GetWindowBounds(app->os.window, kWindowContentRgn, &gRect);
  gRect.right = app->width + gRect.left;
  gRect.bottom = app->height + gRect.top;
  SetWindowBounds(app->os.window, kWindowContentRgn, &gRect);
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
  CFStringRef cfmsg = CFStringCreateWithCString(NULL, msg, kCFStringEncodingUTF8);
  SetWindowTitleWithCFString(app->os.window, cfmsg);
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;

  const EventTypeSpec  windowEvents[]  =   {   
    { kEventClassCommand,   kEventCommandProcess },
    { kEventClassTextInput, kEventTextInputUpdateActiveInputArea },
    { kEventClassTextInput, kEventTextInputUnicodeForKeyEvent },
    { kEventClassMouse,    kEventMouseMoved },
    { kEventClassMouse,    kEventMouseDragged },
    { kEventClassMouse,    kEventMouseDown },
    { kEventClassMouse,    kEventMouseUp },
    { kEventClassWindow,   kEventWindowBoundsChanged },
    { kEventClassWindow,   kEventWindowClose }
  };

  Rect                   gRect;
  static EventHandlerUPP gTestWindowEventProc = NULL;
  OSStatus               err;
  EventLoopTimerRef      rubyTimer;

  app->slot.controls = Qnil;
  SetRect(&gRect, 100, 100, app->width + 100, app->height + 100);

  INFO("Draw QUARTZ window.\n");
  err = CreateNewWindow(kDocumentWindowClass,
      kWindowCompositingAttribute
    | kWindowStandardHandlerAttribute
    | (app->resizable ? (kWindowLiveResizeAttribute | kWindowStandardDocumentAttributes) :
      kWindowStandardFloatingAttributes),
    &gRect,
    &app->os.window);

  if (err != noErr)
  {
    QUIT("Couldn't make a new window.");
  }

  InitCursor();

  gTestWindowEventProc = NewEventHandlerUPP(shoes_app_quartz_handler);
  if (gTestWindowEventProc == NULL)
  {
    QUIT("Out of memory.");
  }

  INFO("Event handler.\n");
  err = InstallWindowEventHandler(app->os.window,
    gTestWindowEventProc, GetEventTypeCount(windowEvents),
    windowEvents, app, NULL);

  err = InstallEventLoopIdleTimer(GetMainEventLoop(),
   kEventDurationNoWait, 10 * kEventDurationMillisecond,
   NewEventLoopIdleTimerUPP(shoes_app_quartz_idle),
   0, &rubyTimer);

  HIViewFindByID(HIViewGetRoot(app->os.window), kHIViewWindowContentID, &app->slot.view);

quit:
  return code;
}

void
shoes_native_app_show(shoes_app *app)
{
  ShowWindow(app->os.window);
}

void
shoes_native_loop()
{
  TextEncoding utf8Encoding, unicodeEncoding;
  utf8Encoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicodeUTF8Format);
  unicodeEncoding = CreateTextEncoding(kTextEncodingUnicodeDefault,
    kUnicodeNoSubset, kUnicode16BitFormat);
  TECCreateConverter(&shoes_world->os.converter, unicodeEncoding, utf8Encoding);

  CreateNewMenu(202, 0, &HelpMenu);
  SetMenuTitleWithCFString(HelpMenu, CFSTR("Help"));
  InsertMenu(HelpMenu, 0);
  AppendMenuItemTextWithCFString(HelpMenu, CFSTR("Manual"), 0, SHOES_HELP_MANUAL, 0);

  RunApplicationEventLoop();
}

void
shoes_native_app_close(shoes_app *app)
{
  DisposeWindow(app->os.window);
}

void
shoes_browser_open(char *url)
{
  VALUE browser = rb_str_new2("open ");
  rb_str_cat2(browser, url);
  shoes_sys(RSTRING_PTR(browser), 1);
}

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
  shoes_canvas *canvas;
  SHOES_SLOT_OS *slot;
  Data_Get_Struct(c, shoes_canvas, canvas);
  slot = &canvas->slot;

  slot->controls = parent->controls;
  shoes_slot_quartz_create(c, parent, x, y, width, height, scrolls);

  if (toplevel) shoes_canvas_size(c, width, height);
}

cairo_t *
shoes_cairo_create(SHOES_SLOT_OS *slot, shoes_canvas *parent, int width, int height, int toplevel)
{
  slot->surface = cairo_quartz_surface_create_for_cg_context(slot->context, width, height);
  return cairo_create(slot->surface);
}

void shoes_cairo_destroy(SHOES_SLOT_OS *)
{
  cairo_surface_destroy(slot->surface);
}

void
shoes_group_clear(SHOES_GROUP_OS *group)
{
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
  HIRect rect, rect2;
  rect.origin.x = (self_t->place.ix + self_t->place.dx) * 1.;
  rect.origin.y = ((self_t->place.iy + self_t->place.dy) * 1.) + 4;
  rect.size.width = (self_t->place.iw * 1.) + 4;
  rect.size.height = (self_t->place.ih * 1.) - 8;
  HIViewGetFrame(self_t->slot.view, &rect2);
  if (rect.origin.x != rect2.origin.x || rect.origin.y != rect2.origin.y ||
      rect.size.width != rect2.size.width || rect.size.height != rect2.size.height)
  {
    HIViewSetFrame(self_t->slot.view, &rect);
    if (self_t->slot.vscroll)
    {
      Rect scrollb = {0, self_t->width - 16, self_t->height, self_t->width};
      SInt16 baseLine;
      SetControlBounds(self_t->slot.vscroll, &scrollb);
      SetControlViewSize(self_t->slot.vscroll, self_t->height);
      int upper = GetControlMaximum(self_t->slot.vscroll);
      HIViewSetVisible(self_t->slot.vscroll, upper > self_t->height);
      HIViewSetNeedsDisplay(self_t->slot.vscroll, true);
    }
  }
}
