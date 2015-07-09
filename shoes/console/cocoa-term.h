
#import <Cocoa/Cocoa.h>
#import <AppKit/NSFontCollection.h>
#include "tesi.h"

@interface ConsoleTermView : NSTextView
{
   void *cwin;  //  points to below
   struct tesiObject* tobj;
}
@end

@interface ConsoleWindow : NSWindow
{
@public
  struct tesiObject* tobj;
  NSMutableString *cnvbfr;  // for char to NSString conversion
  NSTimer *pollTimer;
  NSBox *btnpnl;
  NSButton *clrbtn;
  NSButton *cpybtn;
  NSView *cntview;
  NSScrollView *termpnl;
  //NSTextView *termview;
  ConsoleTermView *termView;
}
@end
