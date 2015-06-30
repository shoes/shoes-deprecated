// cocoa-term is a very minimal and dumb terminal emulator
// for use in Shoes, mostly for logging purposes.
// Gui wise it is a Window with a small panel on top that has a static message
// and two buttons (copy and clear) and the a larger scrollable panel of
// text lines. There is a minimal keyboard support so keypresses are sent to
// a pty and characters are read from that pty and displayed in the window.
// see tesi.c and tesi.h. Cocoa version of gtk-terminal.

#include "cocoa-term.h"

// Implement
@implementation ConsoleWindow
- (void)consoleInit
{
  //app = a;
  int width = 600; // window
  int height = 468;
#define PNLH 40
  [self setTitle: @"(New) Shoes Console"];
  //[self center];
  [self makeKeyAndOrderFront: self];
  [self setAcceptsMouseMovedEvents: YES];
  [self setAutorecalculatesKeyViewLoop: YES];
  [self setDelegate: (id <NSWindowDelegate>)self];
  // setup the copy and clear buttons (yes command key handling would be better)
  btnpnl = [[NSBox alloc] initWithFrame: NSMakeRect(0,height-PNLH,width,PNLH)];
  [btnpnl setTitlePosition: NSNoTitle ];
  [btnpnl setAutoresizingMask: NSViewWidthSizable|NSViewMinYMargin];

  clrbtn = [[NSButton alloc] initWithFrame: NSMakeRect(400,2, 60, 28)];
  [clrbtn setButtonType: NSMomentaryPushInButton];
  [clrbtn setBezelStyle: NSRoundedBezelStyle];
  [clrbtn setTitle: @"Clear"];
  [clrbtn setTarget: self];
  [clrbtn setAction: @selector(handleClear:)];

  cpybtn = [[NSButton alloc] initWithFrame: NSMakeRect(500,2, 60, 28)];
  [cpybtn setButtonType: NSMomentaryPushInButton];
  [cpybtn setBezelStyle: NSRoundedBezelStyle];
  [cpybtn setTitle: @"Copy"];
  [cpybtn setTarget: self];
  [cpybtn setAction: @selector(handleCopy:)];

  [btnpnl addSubview: clrbtn];
  [btnpnl addSubview: cpybtn];
  // init termpnl here.
  cntview = [[NSView alloc] initWithFrame: NSMakeRect(0,0,width,468)];
  [cntview setAutoresizesSubviews: YES];
  [cntview addSubview: btnpnl];

  // add termpnl
  [self setContentView: cntview];

}
-(IBAction)handleClear: (id)sender
{
  NSLog(@"Clear button pressed");
}

-(IBAction)handleCopy: (id)sender
{
  NSLog(@"Copy button pressed");
}

- (void)disconnectApp
{
  //app = Qnil;
}
- (void)keyDown: (NSEvent *)e
{
  // lots to do here
}
- (BOOL)canBecomeKeyWindow
{
  return YES;
}
- (BOOL)canBecomeMainWindow
{
  return YES;
}
- (void)windowWillClose: (NSNotification *)n
{

}

@end


int shoes_native_console()
{
  NSLog(@"Console starting");
  ConsoleWindow *window;
  unsigned int mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  NSRect rect = NSMakeRect(0, 0, 600, 468);
  //NSSize size = {app->minwidth, app->minheight};

  //if (app->resizable)
    mask |= NSResizableWindowMask;
  window = [[ConsoleWindow alloc] initWithContentRect: rect
    styleMask: mask backing: NSBackingStoreBuffered defer: NO];
  //if (app->minwidth > 0 || app->minheight > 0)
  //  [window setContentMinSize: size];
  [window consoleInit];
  // Fire up console window, switch stdin..
  printf("mac\010c\t console \t\tcreated\n"); //test \b \t in string
  return 1;
}
