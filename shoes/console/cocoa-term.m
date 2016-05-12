/* cocoa-term is a very minimal and dumb terminal emulator
   for use in Shoes, mostly for logging purposes.
   Gui wise it is a Window with a small panel on top that has a static message
   and two buttons (copy and clear) and the a larger scrollable panel of
   text lines. There is a minimal keyboard support so keypresses are sent to
   a pty and characters are read from that pty and displayed in the window.
   see tesi.c and tesi.h.
   
   Things are even more confusing with Stdout since OSX pretends Gui
   apps don't have it so it's a bitbucket. So, before we create the pty
   we have to create something useful for stdout (a pipe). We setup a handler to
   read the pipe and pass the chars to tesi like it was read from the pty.
   (yes that makes the pty almost useless on OSX) Almost.

   NOTE: Use of NSLog will cause confusion &  misbehaviour. Don't use it if
   console is showing.
*/
#include "cocoa-term.h"

/* there can only be one termina so only one tesi_object.
 I'll keep a global Obj-C ref to tesi
*/
static struct tesiObject* shadow_tobj;

void console_haveChar(struct tesiObject *tobj, char c); // forward ref

@implementation ConsoleTermView // It's a NSTextView
- (void)initView: (ConsoleWindow *)cw withFont: (NSFont *)fixedfont
{
  cwin = cw;
  tobj = cw->tobj;  // is this Obj-C ugly? Probably
  font = fixedfont;
  //attrs = [NSMutableDictionary dictionary];
  //[attrs setObject:font forKey:NSFontAttributeName];
  attrs = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];
  [self setEditable: YES];
  [self setRichText: false];
  [[self textStorage] setFont: font]; // doesn't work
  [self setFont: font]; //doesn't work
  [self setTypingAttributes: attrs]; // doesn't hang so thats good. doesn't work either
  // [[self textStorage] setTypingAttributes: attrs]; // no such selector
}
- (void)keyDown: (NSEvent *)e
{
  // works but I do not like it.
  NSString *str = [e charactersIgnoringModifiers];
  char *utf8 = [str UTF8String];
  if (strlen(utf8)==1) {
    write(tobj->fd_input, utf8, strlen(utf8));
  } else {
    // this sends the key event (back?) to the responder chain
    [self interpretKeyEvents:[NSArray arrayWithObject:e]];
  }
}

// not called?
- (void)insertTab:(id)sender {
   if ([[self window] firstResponder] == self) {
     write(tobj->fd_input,"TAB",3);
   }
}

- (void)writeChr:(char)c
{
  char buff[4];
  buff[0] = c;
  buff[1] = 0;
  NSString *cnvbfr = [[NSString alloc] initWithCString: buff encoding: NSUTF8StringEncoding];
  //Create a AttributeString using the font.
  NSAttributedString *attrStr = [[NSAttributedString alloc] initWithString: cnvbfr attributes: attrs];
  //[[self textStorage] appendAttributedString: [[NSMutableAttributedString alloc] initWithString: cnvbfr attributes: attrs]];
  [[self textStorage] appendAttributedString: attrStr];
  [self scrollRangeToVisible:NSMakeRange([[self string] length], 0)];

  // TODO: Am I leaking memory ? The C programmer in me says "Oh hell yes!"
  // [obj release] to match the alloc's ?
}

- (void)deleteChar
{
  // Remeber, this a \b char in a printf/puts not a key event (although one might get here)
  int length = (int)[[self textStorage] length];
  [[self textStorage] deleteCharactersInRange:NSMakeRange(length-1, 1)];
  [self scrollRangeToVisible:NSMakeRange([[self string] length], 0)];
  //TODO: constrain y so it doesn't crawl up the screen
}

@end

@implementation ConsoleWindow
- (void)consoleInitWithFont: (NSFont *) font
{
  monoFont = font;
  //NSRect winRect = [[self contentView] frame]; // doesn't do what I think
  NSSize charSize = [monoFont maximumAdvancement];
  float fw = charSize.width;
  float fh = [monoFont pointSize]+2.0;
  int width = (int)(fw * (80.0+8));
  int height = (int)(fh * 24);
  int btnPanelH = 40;
//#define PNLH 40
  [self setTitle: @"(New) Shoes Console"];
  //[self center]; // there is a bug report about centering.
  [self makeKeyAndOrderFront: self];
  [self setAcceptsMouseMovedEvents: YES];
  [self setAutorecalculatesKeyViewLoop: YES];
  [self setDelegate: (id <NSWindowDelegate>)self];
  // setup the copy and clear buttons (yes command key handling would be better)
  //btnpnl = [[NSBox alloc] initWithFrame: NSMakeRect(0,height-PNLH,width,PNLH)];
  btnpnl = [[NSBox alloc] initWithFrame: NSMakeRect(0,height,width,btnPanelH)];
  [btnpnl setTitlePosition: NSNoTitle ];
  [btnpnl setAutoresizingMask: NSViewWidthSizable|NSViewMinYMargin];
  // draw the icon
  NSApplication *NSApp = [NSApplication sharedApplication];
  NSImage *icon = [NSApp applicationIconImage];
  NSRect iconRect = NSMakeRect(20,-2,32,32); // -2 doesn't make sense but ...
  NSImageView *ictl = [[NSImageView alloc] initWithFrame: iconRect];
  [ictl setImage: icon];
  [ictl setEditable: false];

  NSTextField *labelWidget;
  labelWidget = [[NSTextField alloc] initWithFrame: NSMakeRect(80, -2, 200, 28)];
  [labelWidget setStringValue: @"Very Dumb Console"];
  [labelWidget setBezeled:NO];
  [labelWidget setDrawsBackground:NO];
  [labelWidget setEditable:NO];
  [labelWidget setSelectable:NO];
  NSFont *labelFont = [NSFont fontWithName:@"Helvetica" size:18.0];
  [labelWidget setFont: labelFont];

  clrbtn = [[NSButton alloc] initWithFrame: NSMakeRect(300, -2, 60, 28)];
  [clrbtn setButtonType: NSMomentaryPushInButton];
  [clrbtn setBezelStyle: NSRoundedBezelStyle];
  [clrbtn setTitle: @"Clear"];
  [clrbtn setTarget: self];
  [clrbtn setAction: @selector(handleClear:)];

  cpybtn = [[NSButton alloc] initWithFrame: NSMakeRect(400, -2, 60, 28)];
  [cpybtn setButtonType: NSMomentaryPushInButton];
  [cpybtn setBezelStyle: NSRoundedBezelStyle];
  [cpybtn setTitle: @"Copy"];
  [cpybtn setTarget: self];
  [cpybtn setAction: @selector(handleCopy:)];

  [btnpnl addSubview: ictl];
  [btnpnl addSubview: labelWidget];
  [btnpnl addSubview: clrbtn];
  [btnpnl addSubview: cpybtn];
  // init termpnl and textview here.
  // Note NSTextView is subclass of NSText so there are MANY methods to learn
  // not to mention delagates and protocols

  // compute the Size of the window for the font and size
  NSRect textViewBounds = NSMakeRect(0, 0, width, height);

  // setup internals for NSTextView - there are many
  termStorage = [[NSTextStorage alloc] init];
  [termStorage setFont: monoFont];
  termLayout = [[NSLayoutManager alloc] init];
  [termStorage addLayoutManager:termLayout];
  termContainer = [[NSTextContainer alloc] initWithContainerSize:textViewBounds.size];
  [termLayout addTextContainer:termContainer];

  //termView = [[ConsoleTermView alloc]  initWithFrame: textViewBounds];
  //termView = [[ConsoleTermView alloc]  initWithFrame: NSMakeRect(0, height, width, height-btnPanelH)];
  termView = [[ConsoleTermView alloc]  initWithFrame: NSMakeRect(0, 0, width, height)];

  termpnl = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, width, height)];
  [termpnl setHasVerticalScroller: YES];
  //causes btnpnl to vanish. Fixes many resizing issues though:
  [termpnl setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
  [termpnl setDocumentView: termView];

  // Put the panels in the Window
  cntview = [[NSView alloc] initWithFrame: NSMakeRect(0,height+btnPanelH,width,height+btnPanelH)];
  [self setContentView: cntview];
  [cntview setAutoresizesSubviews: YES];
  [cntview addSubview: btnpnl];
  [cntview addSubview: termpnl];
  [self makeFirstResponder:termView];
  
  // This works nicely for stderr
  errPipe = [NSPipe pipe];
  errReadHandle = [errPipe fileHandleForReading];
  if (dup2([[errPipe fileHandleForWriting] fileDescriptor], fileno(stderr)) != -1) {
    [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(stdErrDataAvail:)
                                        name: NSFileHandleDataAvailableNotification
                                        object: errReadHandle];
    [errReadHandle waitForDataInBackgroundAndNotify];
  }
  // Somehow we need to create an fd for stdout and read it.
  // OSX Stdout is weird. Lets do some discovery to print later to stderrnewfd
  //int outfd = fileno(stdout);
  int pipewrfd, piperdfd;
  int dup2fail = 0;
  int rtn = 0;

#define TRY_STDOUT 2
#if (TRY_STDOUT == 0) // one pipe two fd's - doesn't work
   if (dup2(fileno(stderr), fileno(stdout)) == -1) {
     // failed
     dup2fail = errno;
  }

#else // methods that use a different pipe for stdout
  outPipe = [NSPipe pipe];
  outReadHandle = [outPipe fileHandleForReading] ;
  outWriteHandle = [outPipe fileHandleForWriting];
  pipewrfd = [[outPipe fileHandleForWriting] fileDescriptor];
  piperdfd = [[outPipe fileHandleForReading] fileDescriptor];
  // fclose(stdout) // Don't do this!!
  rtn = dup2(pipewrfd, fileno(stdout));
  // do some checking on dup2 and stdout
  //char *pipeWrStr = "Explicit Write to Pipe FileWriteHandle\n";
  //NSData *pipeMsgData = [NSData dataWithBytes: pipeWrStr length: strlen(pipeWrStr)];
  if (rtn != -1) { 
#if (TRY_STDOUT == 2) 
    [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(stdOutDataAvail:)
                                        name: NSFileHandleDataAvailableNotification
                                        object: outReadHandle];
    [outReadHandle waitForDataInBackgroundAndNotify];
#elif (TRY_STDOUT == 3)
   [[NSNotificationCenter defaultCenter] addObserver: self
                                        selector: @selector(handleNotification:)
                                        name: NSFileHandleReadCompletionNotification
                                        object: outReadHandle];
   [outReadHandle readInBackgroundAndNotify] ;
#endif
    /*
    // issue some test messages for fd, file*, FileHandle*
    char *foo = "Explicit write() to pipe wrfd\n";
    write(pipewrfd,  foo, strlen(foo)); 
    char *foo1 ="Explicit write() to stdout fd\n";
    write(fileno(stdout), foo1, strlen(foo1));
    // above results in one call to stdOutDataAvail
    
    stdout = fdopen(pipewrfd, "w");
    if (setlinebuf(stdout) != 0) {
      fprintf(stderr, "failed setlinebuffer()\n");
    }
    fflush(stdout);  // doesn't work
    printf("Hello from FILE* stdout\n"); // Yay!!
    
    [outWriteHandle writeData: pipeMsgData];
    */
    // now convince Ruby to use the new stdout - sadly it's not line buffered
    // BEWARE the monkey patch!
    char evalstr[256];
    //sprintf(evalstr, "$stdout.reopen(IO.new(%d, 'w'))",pipewrfd);
    //sprintf(evalstr, "$stdout = IO.new(1, 'w')); $stdout.flush"); 
    strcpy(evalstr, "class IO \n\
          def puts args \n\
            super args \n\
            self.flush if self.fileno < 3 \n\
          end \n\
        end\n");
    rb_eval_string(evalstr);
  } else {
    dup2fail = errno;
  }
#endif 
  // Now init the Tesi object - NOTE tesi callbacks are C,  which calls Objective-C
  tobj = newTesiObject("/bin/bash", 80, 24); // first arg is not used
  shadow_tobj = tobj; 
  tobj->pointer = (void *)self;
  //tobj->callback_haveCharacter = &console_haveChar; // short circuit - to be fixed
  tobj->callback_handleNL = &terminal_newline;
  tobj->callback_handleRTN = NULL; // &terminal_return;
  tobj->callback_handleBS = &terminal_backspace;
  tobj->callback_handleTAB = &terminal_tab; 
  tobj->callback_handleBEL = NULL;
  tobj->callback_printCharacter = &terminal_visAscii;
  tobj->callback_attreset = &terminal_attreset;
  tobj->callback_charattr = NULL; // terminal_charattr;
  tobj->callback_setfgcolor= NULL; // &terminal_setfgcolor;
  tobj->callback_setbgcolor = NULL; //&terminal_setbgcolor;
  // that's the minimum set of call backs;
  tobj->callback_setdefcolor = NULL;
  tobj->callback_deleteLines = NULL;
  tobj->callback_insertLines = NULL;
  tobj->callback_attributes = NULL; // old tesi - not used? 
  
  tobj->callback_clearScreen = NULL;  //&terminal_clearscreen;
  tobj->callback_eraseCharacter = NULL; // &console_eraseCharacter;
  tobj->callback_moveCursor = NULL; //&terminal_moveCursor; 
  tobj->callback_insertLines = NULL; //&console_insertLine;
  tobj->callback_eraseLine = NULL; // &terminal_eraseLine;
  tobj->callback_scrollUp = NULL; // &console_scrollUp;

  // try inserting some text.
  //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: @"First Line!\n"]];
  
  [termView initView: self withFont: monoFont]; // tell ConsoleTermView what font to use
#if 0
  // need to get the handleInput started
  // OSX timer resolution less than 0.1 second unlikely?
  cnvbfr = [[NSMutableString alloc] initWithCapacity: 4];
  pollTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                            target: self selector:@selector(readStdout:)
                            userInfo: self repeats:YES];
  // debug
#endif
  /*
  outfd = fileno(stdout);
  fprintf(stderr, "About C stdout after:\n");
  fprintf(stderr, "fd: %d, pipewrfd: %d, piperdfd: %d\n", outfd, pipewrfd, piperdfd);
  fprintf(stderr, "dup2 errorno: %d, %s\n", dup2fail, strerror(dup2fail));
  
  fprintf(stdout, "From C stdout\n");  // Nothing, Damn it!
  // printf("w = %d, h = %d winh = %d \n", width, height, winRect.size.height);
  */
}

-(IBAction)handleClear: (id)sender
{
  [termView setString: @""];
}

-(IBAction)handleCopy: (id)sender
{
  NSString *str = [termView string];
  //NSPasteboard *pboard;
  //printf("Copy button (%lu)\n", (unsigned long)[str length]);
  [[NSPasteboard generalPasteboard] declareTypes: [NSArray arrayWithObject: NSStringPboardType] owner: nil];
  [[NSPasteboard generalPasteboard] setString: str forType: NSStringPboardType];
  //pboard = [[NSPasteboard generalPasteboard];
  //[pboard clearContents];
  //[pboard setData: str forType: NSPasteboardTypeString];
}

- (void)disconnectApp
{
  //app = Qnil;
}

/*
- (void)keyDown: (NSEvent *)e
{
  // handle upper level keys like cmd and page_up/down?
}
*/

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

/*
- (void)readStdout: (NSTimer *)t
{
  // do tesi.Input
  tesi_handleInput(tobj);
}
*/
- (void)writeStr:(NSString*)text
{
  [[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: text]];
  [termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
}

// Hints are this stopped working in 10.7 - doesn't work for me on 10.10
- (void)handleNotification: (NSNotification *)notification
{
  NSData *data  = [[notification userInfo] objectForKey: NSFileHandleNotificationDataItem];
  int len = [data length];
  if (len) {
    char *s = (char *)[data bytes];
    s[len] = '\0';
    tesi_handleInput(tobj, s, len);
    
    //NSString *str = [[NSString alloc] initWithData: obj encoding: NSASCIIStringEncoding] ;
    //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: str]];
    //[termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    
    [outReadHandle readInBackgroundAndNotify] ;
  }
} 

- (void) stdOutDataAvail: (NSNotification *) notification
{
  NSFileHandle *fh = (NSFileHandle *) [notification object];
  NSData *data = [fh availableData];
  int len = [data length];
  if (len) {
    char *s = (char *)[data bytes];  // odds are high this is UTF16-LE
    s[len] = '\0';
    // feed  str to tesi and it will callback into other C code defined here.
    tesi_handleInput(tobj, s, len);
    
    //NSString *str = [[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding] ;
    //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: str]];
    //[termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    
    [outReadHandle waitForDataInBackgroundAndNotify];
  } else {
    // eof? close?
  }
}

- (void) stdErrDataAvail: (NSNotification *) notification
{
  NSFileHandle *fh = (NSFileHandle *) [notification object];
  NSData *data = [fh availableData];
  int len = [data length];
  if (len) {
    char *s = (char *)[data bytes];  // odds are high this is UTF16-LE
    s[len] = '\0';
    // feed  str to tesi and it will callback into other C code defined here.
    tesi_handleInput(tobj, s, len);
    
    //NSString *str = [[NSString alloc] initWithData: data encoding: NSASCIIStringEncoding] ;
    //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: str]];
    //[termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    
    [fh waitForDataInBackgroundAndNotify];
  } else {
    // eof? close?
  }
}

@end

// Called by Shoes via commandline arg or command Shoes::show_console
int shoes_native_console()
{
  //NSLog(@"Console starting");
  NSFont *font = [NSFont fontWithName:@"Menlo" size:11.0]; //menlo is monospace
  NSSize charSize = [font maximumAdvancement];
  float fw = charSize.width;
  float fh = [font pointSize]+2.0;
  int width = (int)(fw * 80.0);
  int height = (int)(fh * 24);
  int btnPanelH = 40; //TODO: dont hardcode this here
  ConsoleWindow *window;
  unsigned int mask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask;
  NSRect rect = NSMakeRect(0, 0, width, height+btnPanelH); //Screen co-ords?
  //NSSize size = {app->minwidth, app->minheight};

  //if (app->resizable)
    mask |= NSResizableWindowMask;
  window = [[ConsoleWindow alloc] initWithContentRect: rect
    styleMask: mask backing: NSBackingStoreBuffered defer: NO];
  //if (app->minwidth > 0 || app->minheight > 0)
  //  [window setContentMinSize: size];
  [window consoleInitWithFont: font];
  // Fire up console window, switch stdin..
  printf("Mak\010c\t console \t\tcreated\n"); //test \b \t in string
  fflush(stdout); // OSX pipes are not line buffered
  return 1;
}

void shoes_native_terminal() {
  shoes_native_console();
}

void terminal_visAscii (struct tesiObject *tobj, char c, int x, int y) {
  ConsoleWindow *cpanel = (ConsoleWindow *)tobj->pointer;
  ConsoleTermView *cwin = cpanel->termView;
  [cwin writeChr: c];
}

void terminal_attreset(struct tesiObject *tobj) {
}

void terminal_backspace(struct tesiObject *tobj, int x, int y) {
  ConsoleWindow *cpanel = (ConsoleWindow *)tobj->pointer;
  ConsoleTermView *cwin = cpanel->termView;
  [cwin deleteChar];
}

void terminal_newline (struct tesiObject *tobj, int x, int y) {
  ConsoleWindow *cpanel = (ConsoleWindow *)tobj->pointer;
  ConsoleTermView *cwin = cpanel->termView;
  [cwin writeChr: '\n'];
}

void terminal_tab(struct tesiObject *tobj, int x, int y) {
  return terminal_visAscii(tobj, '\t', x, y);
}

/*
 * This is called to handle characters received from the pty
 * in response to a puts/printf/write from Shoes,Ruby, & C
 * I don't manage escape seq, x,y or deal with width and height.
 * Just write to the end of the buffer and let cocoa textview manage it.
*/
void console_haveChar(struct tesiObject *tobj, char c) {
  ConsoleWindow *cpanel = (ConsoleWindow *)tobj->pointer;
  ConsoleTermView *cwin = cpanel->termView;

  //char in[8];
  //int lcnt;
	//int i, j;
	//snprintf(in, 7, "%c", c);
	if (c >= 32 && c != 127) {
      [cwin writeChr: c];
		return;
    }
	switch (c) {
		case '\x1B': // begin escape sequence (aborting previous one if any)
			//tobj->partialSequence = 1;
			// possibly flush buffer
			break;

		case '\r': // carriage return ('M' - '@'). Move cursor to first column.
			// odds are high this preceeds a \n. Move to the begining of
			// last line in buffer line.  What happens if we insert
			break;

		case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
		    // just insert '\n' into the buffer.
        [cwin writeChr: c];
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
		    // textview can handle tabs - it claims.
       [cwin writeChr: c];
	 		 break;

		case '\a': // bell ('G' - '@')
	 	  // do nothing for now... maybe a visual bell would be nice?
			break;

	 	case 8: // backspace cub1 cursor back 1 ('H' - '@')
      // TODO:
      [cwin deleteChar];
			break;

		default:
			break;
	}
}
