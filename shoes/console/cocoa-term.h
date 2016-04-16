
#import <Cocoa/Cocoa.h>
#ifndef OLD_OSX
#import <AppKit/NSFontCollection.h>
#endif
#include "tesi.h"

@interface ConsoleTermView : NSTextView
{
   void *cwin;  //  points to below
   struct tesiObject* tobj;
   NSFont *font;
   NSDictionary *attrs;
}
@end

@interface ConsoleWindow : NSWindow
{
@public
  struct tesiObject* tobj;
  NSFont *monoFont;
  NSMutableString *cnvbfr;  // for char to NSString conversion
  NSTimer *pollTimer;
  NSBox *btnpnl;
  NSButton *clrbtn;
  NSButton *cpybtn;
  NSView *cntview;
  NSScrollView *termpnl;
  NSTextStorage *termStorage;
  NSLayoutManager *termLayout;
  NSTextContainer *termContainer;
  ConsoleTermView *termView;
  // Trick for stdout
  NSPipe *pipe;
  NSFileHandle *pipeReadHandle;
}
@end
