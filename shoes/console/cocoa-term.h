
#import <Cocoa/Cocoa.h>
#import <AppKit/NSFontCollection.h>
#include "tesi.h"
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
  NSTextView *termview;
}
@end

@interface ConsoleContents : NSTextView
{

}
@end
