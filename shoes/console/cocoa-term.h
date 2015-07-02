
#import <Cocoa/Cocoa.h>
#import <AppKit/NSFontCollection.h>
#include "tesi.h"
@interface ConsoleWindow : NSWindow
{
@public
  struct tesiObject* tobj;
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
