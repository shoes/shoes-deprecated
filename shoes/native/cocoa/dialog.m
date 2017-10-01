// Shoes RadioButton subclasses NSButton.

#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native/native.h"
#include "shoes/types/types.h"
#include "shoes/native/cocoa/dialog.h"
extern VALUE cTimer;

//#import <Carbon/Carbon.h>

#define HEIGHT_PAD 6

#define INIT    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init]
#define RELEASE [pool release]
#define COCOA_DO(statements) do {\
  INIT; \
  @try { statements; } \
  @catch (NSException *e) { ; } \
  RELEASE; \
} while (0)

// ---- Cocoa Object side ----

@implementation ShoesDialogAsk
- (id)init
{
  if ((self = [super initWithContentRect: NSMakeRect(0, 0, 340, 140)
    styleMask: NSTitledWindowMask backing: NSBackingStoreBuffered defer: NO]))
  {
    answer = FALSE;
    [self setDelegate: (id<NSWindowDelegate>)self];
  }
  return self;
}
-(IBAction)cancelClick: (id)sender
{
  [[NSApplication sharedApplication] stopModal];
}
-(IBAction)okClick: (id)sender
{
  answer = TRUE;
  [[NSApplication sharedApplication] stopModal];
}
- (void)windowWillClose: (NSNotification *)n
{
  [[NSApplication sharedApplication] stopModal];
}
- (BOOL)accepted
{
  return answer;
}
@end

// ---- Ruby calls C/obj->C here ----

VALUE
shoes_native_dialog_color(shoes_app *app)
{
  return shoes_native_window_color(app);
}

VALUE
shoes_dialog_alert(int argc, VALUE *argv, VALUE self)
{
  //GLOBAL_APP(app);
  //ACTUAL_APP(app);
  OSX_APP_VAR(app);
  //NSString *appstr = [[NSString alloc] initWithUTF8String: RSTRING_PTR(app->title)];
  NSString *appstr = [[NSString alloc] initWithUTF8String: title_app];
  rb_arg_list args;
  rb_parse_args(argc, argv, "S|h", &args);
  VALUE msg;
  COCOA_DO({
    msg = shoes_native_to_s(args.a[0]);
    // replace with styles if needed when we have one
    NSString *deftitle =  [appstr stringByAppendingString: @" says:"];
    if (argc > 1)
    {
      if (RTEST(ATTR(args.a[1], title)))
      {
        VALUE tmpstr = shoes_native_to_s(ATTR(args.a[1], title));
        //char *tmptitle = RSTRING_PTR(ATTR(args.a[1], title));
        //NSLog(@"getting title string %s", tmptitle);
        deftitle = [[NSString alloc] initWithUTF8String: RSTRING_PTR(tmpstr)];
      }
      else
      {
        deftitle = [[NSString alloc] initWithUTF8String: ""];
      }
    }
    //below form of alert is deprecated in 10.10
    //NSAlert *alert = [NSAlert alertWithMessageText: deftitle
    //  defaultButton: @"OK" alternateButton: nil otherButton: nil
    //  informativeTextWithFormat: [NSString stringWithUTF8String: RSTRING_PTR(msg)]];
    NSAlert *alert = [[NSAlert alloc] init];
		[alert setMessageText: deftitle];
		[alert setInformativeText: [NSString stringWithUTF8String: RSTRING_PTR(msg)]];

    [alert runModal];
  });
  return Qnil;
}

VALUE
shoes_dialog_ask(int argc, VALUE *argv, VALUE self)
{
  rb_arg_list args;
  VALUE answer = Qnil;
  //GLOBAL_APP(app);
  //ACTUAL_APP(app);
  OSX_APP_VAR(app);
  //char *rbcTitle = RSTRING_PTR(app->title);
  //char *rbcTitle = title_app;
  //NSString *appstr = [[NSString alloc] initWithCString: rbcTitle encoding: NSUTF8StringEncoding];
  NSString *appstr = [[NSString alloc] initWithCString: title_app encoding: NSUTF8StringEncoding];
  rb_parse_args(argc, argv, "S|h", &args);
  COCOA_DO({
    // replace with styles if needed when we have one
    //NSString *deftitle =  @"Shoes says:";
    NSString *deftitle =  [appstr stringByAppendingString: @" asks:"];
    if (argc > 1)
    {
      if (RTEST(ATTR(args.a[1], title)))
      {
        VALUE tmpstr = shoes_native_to_s(ATTR(args.a[1], title));
        //char *tmptitle = RSTRING_PTR(ATTR(args.a[1], title));
        //NSLog(@"getting title string %s", tmptitle);
        deftitle = [[NSString alloc] initWithUTF8String: RSTRING_PTR(tmpstr)];
      }
      else
      {
        deftitle = [[NSString alloc] initWithUTF8String: ""];
      }
    }

    ShoesDialogAsk *alert = [[ShoesDialogAsk alloc] init];
    NSRect iconPanelRect = NSMakeRect(0,0, 80, 300);
    NSView *iconPanelView = [[NSView alloc] initWithFrame: iconPanelRect];
    NSRect ctlPanelRect = NSMakeRect(81, 0, 220, 300);
    NSView *ctlPanelView = [[NSView alloc] initWithFrame: ctlPanelRect];
    [[alert contentView] addSubview: iconPanelView];
    [[alert contentView] addSubview: ctlPanelView];

    NSApplication *NSApp = [NSApplication sharedApplication];
    NSImage *icon = [NSApp applicationIconImage];
    NSRect iconRect = NSMakeRect(10,50,64,64);
    NSImageView *ictl = [[NSImageView alloc] initWithFrame: iconRect];
    [ictl setImage: icon];
    [ictl setEditable: false];
    [iconPanelView addSubview: ictl];

    NSButton *okButton = [[[NSButton alloc] initWithFrame:
      NSMakeRect(244, 10, 88, 30)] autorelease];
    NSButton *cancelButton = [[[NSButton alloc] initWithFrame:
      NSMakeRect(156, 10, 88, 30)] autorelease];
    NSTextField *text = [[[NSTextField alloc] initWithFrame:
      NSMakeRect(20, 110, 260, 18)] autorelease];
    NSTextField *input;
    if (RTEST(ATTR(args.a[1], secret)))
      input = [[NSSecureTextField alloc] initWithFrame:NSMakeRect(20, 72, 300, 24)];
    else
      input = [[NSTextField alloc] initWithFrame:NSMakeRect(20, 72, 300, 24)];

    [alert setTitle: deftitle];
    [text setStringValue: [NSString stringWithUTF8String: RSTRING_PTR(shoes_native_to_s(args.a[0]))]];
    [text setBezeled: NO];
    [text setBackgroundColor: [NSColor windowBackgroundColor]];
    [text setEditable: NO];
    [text setSelectable: NO];
    //[[alert contentView] addSubview: text];
    [ctlPanelView addSubview: text];
    [input setStringValue:@""];
    //[[alert contentView] addSubview: input];
    [ctlPanelView addSubview: input];
    [okButton setTitle: @"OK"];
    [okButton setBezelStyle: 1];
    [okButton setTarget: alert];
    [okButton setAction: @selector(okClick:)];
    [[alert contentView] addSubview: okButton];
    //[ctlPanelView addSubview: okButton];
    [cancelButton setTitle: @"Cancel"];
    [cancelButton setBezelStyle: 1];
    [cancelButton setTarget: alert];
    [cancelButton setAction: @selector(cancelClick:)];
    [[alert contentView] addSubview: cancelButton];
    //[ctlPanelView addSubview: cancelButton];
    [alert setDefaultButtonCell: (NSButtonCell *)okButton];
    [NSApp runModalForWindow: alert];
    if ([alert accepted])
      answer = rb_str_new2([[input stringValue] UTF8String]);
    [alert close];
  });
  return answer;
}

VALUE
shoes_dialog_confirm(int argc, VALUE *argv, VALUE self)
{
  char *msg;
  VALUE quiz;
  VALUE answer = Qnil;
  //GLOBAL_APP(app);
  //ACTUAL_APP(app);
  //char *rbcTitle = RSTRING_PTR(app->title);
  //NSString *appstr = [[NSString alloc] initWithCString: rbcTitle encoding: NSUTF8StringEncoding];
  OSX_APP_VAR(app);
  NSString *appstr = [[NSString alloc] initWithCString: title_app encoding: NSUTF8StringEncoding];
  rb_arg_list args;
  rb_parse_args(argc, argv, "S|h", &args);
  COCOA_DO({
    // replace with styles if needed when we have one
    //NSString *deftitle =  @"Shoes says:";
    NSString *deftitle =  [appstr stringByAppendingString: @" asks:"];
    if (argc > 1)
    {
      if (RTEST(ATTR(args.a[1], title)))
      {
        VALUE tmpstr = shoes_native_to_s(ATTR(args.a[1], title));
        //char *tmptitle = RSTRING_PTR(ATTR(args.a[1], title));
        //NSLog(@"getting title string %s", tmptitle);
        deftitle = [[NSString alloc] initWithUTF8String: RSTRING_PTR(tmpstr)];
      }
      else
      {
        deftitle = [[NSString alloc] initWithUTF8String: ""];
      }
    }
    quiz = args.a[0];
    quiz = shoes_native_to_s(quiz);
    msg = RSTRING_PTR(quiz);
    /*
    NSAlert *alert = [NSAlert alertWithMessageText: deftitle
      defaultButton: @"OK" alternateButton: @"Cancel" otherButton:nil
      informativeTextWithFormat: [NSString stringWithUTF8String: msg]];
    answer = ([alert runModal] == NSOKButton ? Qtrue : Qfalse);
    });
    return answer;
    */
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText: deftitle];
    [alert setInformativeText: [NSString stringWithUTF8String: msg]];
    [alert addButtonWithTitle: @"OK"];
    [alert addButtonWithTitle: @"Cancel"];
    answer = ([alert runModal] == NSAlertFirstButtonReturn ? Qtrue : Qfalse);
    });
    return answer;
}

// ---- Color ----
VALUE
shoes_dialog_color(VALUE self, VALUE title)
{
#ifdef CARBON_COLOR
  Point where;
  RGBColor colwh = { 0xFFFF, 0xFFFF, 0xFFFF };
  RGBColor _color;
  VALUE color = Qnil;
  GLOBAL_APP(app);

  where.h = where.v = 0;
  //title = shoes_native_to_s(title);
  ConstStr255Param defTitle = (ConstStr255Param) RSTRING_PTR(title);
  if (GetColor(where, defTitle, &colwh, &_color))
  {
    color = shoes_color_new(_color.red/256, _color.green/256, _color.blue/256, SHOES_COLOR_OPAQUE);
  }
  return color;
#else
  /*
   * New dialog for Shoes 3.3.2 - doesn't use Carbon
   * Implements Alpha selection
  */
  //GLOBAL_APP(app);
  //NSString *defTitle = [NSString stringWithUTF8String: RSTRING_PTR(title)];
  OSX_APP_VAR(app);
  NSString *defTitle = [NSString stringWithUTF8String: title_app];
  VALUE color = Qnil;
  NSInteger returnCode;
  NSColor *nscolor;
  NSColorPanel *colorPanel = [NSColorPanel sharedColorPanel];
  [colorPanel setTitle: defTitle];
  [colorPanel orderOut:NSApp];
  [colorPanel setContinuous:NO];
  [colorPanel setBecomesKeyOnlyIfNeeded:NO];
  [colorPanel setShowsAlpha: YES];
  // Turn off a warning message: laugh or cry?
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wobjc-method-access"
  [colorPanel _setUseModalAppearance:YES]; // must do but undocumented!!
#pragma clang diagnostic pop
  returnCode = [NSApp runModalForWindow: colorPanel];
  if (returnCode == NSOKButton) {
	  nscolor = [[colorPanel color] colorUsingColorSpace:
		        [NSColorSpace genericRGBColorSpace]];
	  CGFloat components[4];
	  components[3] = [colorPanel alpha];
      [nscolor getComponents:components];
      color = shoes_color_new(components[0] * 255, components[1] * 255, 
          components[2] * 255, components[3] * 255);
      return color;
    }
    return Qnil;
#endif
}
