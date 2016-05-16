
#import <Cocoa/Cocoa.h>
#ifndef OLD_OSX
#import <AppKit/NSFontCollection.h>
#endif
#include "tesi.h"

@interface ConsoleTermView : NSTextView
{
@public
   // attrs is the CURRENT attributes for drawing a character (font, colors, ...)
   // They get added, deleted, and changed.
   NSMutableDictionary *attrs;
   void *cwin;  //  points to below
   struct tesiObject* tobj;
   NSFont *font;

}
@end

@interface ConsoleWindow : NSWindow
{
@public
  struct tesiObject* tobj;
  NSFont *monoFont;
  NSFont *monoBold;
  NSMutableString *cnvbfr;  // for char to NSString conversion
  NSTimer *pollTimer;		// no longer used in 3.3.2?
  NSBox *btnpnl;
  NSButton *clrbtn;
  NSButton *cpybtn;
  // args to shoes_native_terminal are saved here if needed
  NSString *reg_dir;
  int req_cols;
  int req_rows;
  int req_mode;
  int req_fontsize;
  NSMutableDictionary *colorTable; // terminal defaults
  NSArray *colorAttr; 
  NSColor *defaultBgColor;
  NSColor *defaultFgColor;
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
// C level declares to make the compiler happy
extern void terminal_visAscii(struct tesiObject *, char, int, int );
extern void terminal_return(struct tesiObject *, int, int);
extern void terminal_newline(struct tesiObject *, int, int);
extern void terminal_backspace(struct tesiObject *, int, int);
extern void terminal_tab(struct tesiObject *, int, int);
extern void terminal_attreset(struct tesiObject *);
extern void terminal_charattr(struct tesiObject *, int);
extern void terminal_setfgcolor(struct tesiObject *, int);
extern void terminal_setbgcolor(struct tesiObject *, int);
extern int terminal_hook(void *, const char *, int);
extern void rb_eval_string(char *);
