
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
  // For stdout
  NSPipe *outPipe;
  NSFileHandle *outReadHandle;
  NSFileHandle *outWriteHandle;
  // Stderr 
  NSPipe *errPipe;
  NSFileHandle *errReadHandle;
}
@end
// C level declares
extern void terminal_visAscii(struct tesiObject *, char, int, int );
extern void terminal_return(struct tesiObject *, int, int);
extern void terminal_newline(struct tesiObject *, int, int);
extern void terminal_backspace(struct tesiObject *, int, int);
extern void terminal_tab(struct tesiObject *, int, int);
extern void terminal_attreset(struct tesiObject *);
extern int terminal_hook(void *, const char *, int);
