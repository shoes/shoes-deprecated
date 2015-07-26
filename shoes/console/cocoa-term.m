/* cocoa-term is a very minimal and dumb terminal emulator
   for use in Shoes, mostly for logging purposes.
   Gui wise it is a Window with a small panel on top that has a static message
   and two buttons (copy and clear) and the a larger scrollable panel of
   text lines. There is a minimal keyboard support so keypresses are sent to
   a pty and characters are read from that pty and displayed in the window.
   see tesi.c and tesi.h.

   NOTE: Use of NSLog will cause confusion &  misbehaviour. Don't use it if
   console is showing.
*/
#include "cocoa-term.h"

void console_haveChar(void *p, char c); // forward ref

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
    if (utf8[0] == '0x09') {
      write(tobj->fd_input,"TAB",3);  //never called
    }
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
  NSRect winRect = [[self contentView] frame]; // doesn't do what I think
  NSSize charSize = [monoFont maximumAdvancement];
  float fw = charSize.width;
  float fh = [monoFont pointSize]+2.0;
  int width = (int)(fw * 80.0);
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
  // Now init the Tesi object - NOTE tesi callbacks are C,  which calls Objective-C
  tobj = newTesiObject("/bin/bash", 80, 24); // first arg not used, 2 and 3 not either
  tobj->pointer = (void *)self;
  tobj->callback_haveCharacter = &console_haveChar;

  // try inserting some text.
  //[[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: @"First Line!\n"]];

  [termView initView: self withFont: monoFont]; // tell ConsoleTermView what font to use

  // need to get the handleInput started
  // OSX timer resolution less than 0.1 second unlikely?
  cnvbfr = [[NSMutableString alloc] initWithCapacity: 4];
  pollTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                            target: self selector:@selector(readStdout:)
                            userInfo: self repeats:YES];
  // debug
  // printf("w = %d, h = %d winh = %d \n", width, height, winRect.size.height);
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

-(void)readStdout: (NSTimer *)t
{
  // do tesi.handleInput
  tesi_handleInput(tobj);
}

- (void)writeStr:(NSString*)text
{
  [[termView textStorage] appendAttributedString: [[NSAttributedString alloc] initWithString: text]];
  [termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
#ifdef BE_CLEVER
    dispatch_async(dispatch_get_main_queue(), ^{
        NSAttributedString* attr = [[NSAttributedString alloc] initWithString:text];

        [[termView textStorage] appendAttributedString: attr];
        [termView scrollRangeToVisible:NSMakeRange([[termView string] length], 0)];
    });
#endif
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
  return 1;
}

/*
 * This is called to handle characters received from the pty
 * in response to a puts/printf/write from Shoes,Ruby, & C
 * I don't manage escape seq, x,y or deal with width and height.
 * Just write to the end of the buffer and let cocoa textview manage it.
*/
void console_haveChar(void *p, char c) {
	struct tesiObject *tobj = (struct tesiObject*)p;
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
#ifdef DEBUG
			fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
#endif
			break;
	}
}
