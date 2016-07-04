
#import <Cocoa/Cocoa.h>
#ifndef OLD_OSX
#import <AppKit/NSFontCollection.h>
#endif
#include "tesi.h"

// Sadly, for keyDown to work we have to subclass NSTextView
// so that uglifies (TM) things.

@interface DisplayView : NSTextView
{
@public
}

- (void)keyDown: (NSEvent *)e;
@end 

// a 'clever' hack for STDOUT
@interface StdoutBridge : NSObject
{
@public
  // For stdout
  NSFileHandle *oldHandle;
  NSPipe *outPipe;
  NSFileHandle *outReadHandle;
  NSFileHandle *outWriteHandle;
}

@end

@interface TerminalWindow : NSWindow
{
@public
  struct tesiObject* tobj;
  NSFont *monoFont;
  NSFont *monoBold;
  //NSMutableString *cnvbfr;  // for char to NSString conversion
  //NSTimer *pollTimer;		// no longer used in 3.3.2?
  NSBox *btnpnl;
  NSButton *clrbtn;
  NSButton *cpybtn;
  NSButton *rawbtn;
  // args to shoes_native_terminal are saved here if needed
  NSString *reg_dir;
  int req_cols;
  int req_rows;
  int req_mode;
  int req_fontsize;
  NSMutableDictionary *colorTable; // terminal defaults
  NSMutableArray *colorAttr; 
  NSColor *defaultBgColor;
  NSColor *defaultFgColor;
  // attrs is the CURRENT attributes for drawing a character (font, colors, ...)
  // They get added, deleted, and changed.
  NSMutableDictionary *attrs;
  // bold attributes appear to be need applied after writing to the textstorage
  // kind of like how gtk-terminal does 'tags' only simpler. Maybe.
  int boldActive;
  int boldStart;   //this will be a textStorage position
  NSView *cntview;
  NSScrollView *termpnl;
  NSTextStorage *termStorage;
  NSLayoutManager *termLayout;
  NSTextContainer *termContainer;
  //NSTextView *termView;
  DisplayView *termView;
  NSFileHandle *outReadHandle; 
  // Stderr 
  NSPipe *errPipe;
  NSFileHandle *errReadHandle;
  // Just in case you think nothing it too Weird
  char *lineBuffer;
  int linePos;
  NSMutableData *rawBuffer;
}

- (void)displayChar:(char)c ;
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
extern void terminal_setfg256(struct tesiObject *, int);
extern void terminal_setbg256(struct tesiObject *, int);
extern int terminal_hook(void *, const char *, int);
extern void rb_eval_string(char *);
