// cocoa-term is a very minimal and dumb terminal emulator
// for use in Shoes, mostly for logging purposes.
// Gui wise it is a Window with a small panel on top that has a static message
// and two buttons (copy and clear) and the a larger scrollable panel of
// text lines. There is a minimal keyboard support so keypresses are sent to
// a pty and characters are read from that pty and displayed in the window.
// see tesi.c and tesi.h. Cocoa version of gtk-terminal.

#include "cocoa-term.h"
#define ENABLE_OUTPUT 1 // need to get they key handling code working first
void console_haveChar(void *p, char c); // forward ref
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

  clrbtn = [[NSButton alloc] initWithFrame: NSMakeRect(400, 2, 60, 28)];
  [clrbtn setButtonType: NSMomentaryPushInButton];
  [clrbtn setBezelStyle: NSRoundedBezelStyle];
  [clrbtn setTitle: @"Clear"];
  [clrbtn setTarget: self];
  [clrbtn setAction: @selector(handleClear:)];

  cpybtn = [[NSButton alloc] initWithFrame: NSMakeRect(500, 2, 60, 28)];
  [cpybtn setButtonType: NSMomentaryPushInButton];
  [cpybtn setBezelStyle: NSRoundedBezelStyle];
  [cpybtn setTitle: @"Copy"];
  [cpybtn setTarget: self];
  [cpybtn setAction: @selector(handleCopy:)];

  [btnpnl addSubview: clrbtn];
  [btnpnl addSubview: cpybtn];
  // init termpnl and textview here.
  // Note NSTextView is subclass of NSText so there are MANY methods to learn
  // not to mention delagates and protocols
  termview = [[NSTextView alloc]  initWithFrame: NSMakeRect(0, 0, width, 468-PNLH)];

  termpnl = [[NSScrollView alloc] initWithFrame: NSMakeRect(0, 0, width, 468-PNLH)];
  [termpnl setHasVerticalScroller: YES];
  [termpnl setDocumentView: termview];

  // Put the panels in the Window
  cntview = [[NSView alloc] initWithFrame: NSMakeRect(0, 0 ,width, 468)];
  [cntview setAutoresizesSubviews: YES];
  [cntview addSubview: btnpnl];
  [cntview addSubview: termpnl];
  [self setContentView: cntview];

  // Now init the Tesi object - NOTE tesi callbacks are C,  which calls Objective-C
#ifdef ENABLE_OUTPUT
  tobj = newTesiObject("/bin/bash", 80, 24); // first arg not used
  tobj->pointer = (void *)self;
  tobj->callback_haveCharacter = &console_haveChar;
#endif
  /* cjc - my handler short circuts much (all?) of these callbacks:
  t->callback_printCharacter = &tesi_printCharacter;
  t->callback_eraseCharacter = &tesi_eraseCharacter;
  t->callback_moveCursor = &tesi_moveCursor;
  t->callback_insertLine = &tesi_insertLine;
  t->callback_eraseLine = &tesi_eraseLine;
  t->callback_scrollUp = &tesi_scrollUp;
  */
  // try inserting some text.
  //[termview insertText: @"First Line!"];
  // need to get the handleInput started
  // OSX timer res less than 0.1 second not likely
#ifdef ENABLE_OUTPUT
  cnvbfr = [[NSMutableString alloc] initWithCapacity: 4];
  pollTimer = [NSTimer scheduledTimerWithTimeInterval:0.1
                            target: self selector:@selector(readStdout:)
                            userInfo: self repeats:YES];
#endif
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
  NSLog(@"Key %c", [e keyCode]);
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

-(void)readStdout: (NSTimer *)t
{
  // do tesi.handleInput
  tesi_handleInput(tobj);
}
- (void)writeStr:(NSString*)text
{
  [termview setString:[NSString stringWithFormat:@"%@\n%@", text]];
#ifdef BE_CLEVER
    dispatch_async(dispatch_get_main_queue(), ^{
        NSAttributedString* attr = [[NSAttributedString alloc] initWithString:text];

        [[termview textStorage] appendAttributedString: attr];
        [termview scrollRangeToVisible:NSMakeRange([[termview string] length], 0)];
    });
#endif
}

- (void)writeChr:(char)c
{
  char buff[4];
  buff[0] = c;
  buff[1] = 0;
  NSString *cnvbfr = [[NSString alloc] initWithCString: buff encoding: NSUTF8StringEncoding];
  [self writeStr: cnvbfr];
  // TODO: Am I leaking memory ? The C programmer in me says "Oh hell yes!"
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

// C Callbacks

/* these move to Obj-C keyDown, and button handling
static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*)data;
    char *c = ((GdkEventKey*)event)->string;
	char s = *c;
	if (event->keyval == GDK_BackSpace) {
		s = 010;
    }
	write(tobj->fd_input, &s, 1);
	return TRUE;
}

static gboolean clear_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *newbuf = gtk_text_buffer_new(NULL);
	gtk_text_view_set_buffer(view, newbuf);
	// set a mark to the end? get focus
	gtk_widget_grab_focus(view);
	return TRUE;
}

static gboolean copy_console(GtkWidget *widget, GdkEvent *event, gpointer data) {
	struct tesiObject *tobj = (struct tesiObject*) data;
	GtkTextView *view = GTK_TEXT_VIEW(tobj->pointer);
	GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
	GtkTextIter iter_s, iter_e;
    gtk_text_buffer_get_bounds(buffer, &iter_s, &iter_e);
	gchar *bigstr = gtk_text_buffer_get_slice(buffer, &iter_s, &iter_e, TRUE);
	GtkClipboard *primary = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(primary, bigstr, strlen(bigstr));
	return TRUE;
}
*/

/*
 * This is called to handle characters received from the pty
 * in response to a puts/printf/write from Shoes,Ruby, & C
 * I don't manage escape seq, x,y or deal with width and height.
 * Just write to the end of the buffer and let the cocoa textview manage it.
*/
void console_haveChar(void *p, char c) {
	struct tesiObject *tobj = (struct tesiObject*)p;
  ConsoleWindow * cwin = (ConsoleWindow *)tobj->pointer;
  NSTextView *tv = cwin->termview; //Be careful!

  char in[8];
  int lcnt;

	int i, j;
	//snprintf(in, 7, "%c", c);
	if (c >= 32 && c != 127) {
	    //buffer = gtk_text_view_get_buffer(view);
	    //gtk_text_buffer_insert_at_cursor(buffer, in, 1);
      NSLog(@"%c", c);
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
		    //gtk_text_buffer_insert_at_cursor(buffer, in, 1);
        NSLog(@"\\n");
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
		    // textview can handle tabs - it claims.
	      //  gtk_text_buffer_insert_at_cursor(buffer, in, 1);
        NSLog(@"\\t");
	 		break;

		case '\a': // bell ('G' - '@')
	 		// do nothing for now... maybe a visual bell would be nice?
			break;

	 	case 8: // backspace cub1 cursor back 1 ('H' - '@')
            //gtk_text_buffer_get_end_iter (buffer, &iter_e);
            //gtk_text_buffer_backspace(buffer, &iter_e, 1, 1);
			break;

		default:
#ifdef DEBUG
			fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
#endif
			break;
	}
	// tell the view to show the newest position
	// from http://www.gtkforums.com/viewtopic.php?t=1307
	//gtk_text_buffer_get_end_iter (buffer, &iter_e);
	//insert_mark = gtk_text_buffer_get_insert (buffer);
	//gtk_text_buffer_place_cursor(buffer, &iter_e);
  //  gtk_text_view_scroll_to_mark( GTK_TEXT_VIEW (view),
  //          insert_mark, 0.0, TRUE, 0.0, 1.0);
}

void tesi_printCharacter(void *p, char c, int x, int y) {
}
void tesi_eraseCharacter(void *p, int x, int y) {
}

void tesi_scrollUp(void *p) {
}

void tesi_moveCursor(void *p, int x, int y) {
}

void tesi_insertLine(void *p) {
}

void tesi_eraseLine(void *p) {
}
