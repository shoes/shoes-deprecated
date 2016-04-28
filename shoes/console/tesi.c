#include "tesi.h"
/* 
 * This is a state machine to parse a subset of xterm[-256] escape sequences 
 * that arrive on a pty (slave side) and call back into gtk/cocoa functions
 * to implement that sequence 'clear_screen' or 'setcolor' or ...
 * 
 * Those call back functions have nothing to do with Shoes. The window they
 * draw to is invisible to Shoes scripts and we won't call any Shoes or Ruby 
 * internal code 
 * 
 * The code expects row & colunm numbers in escape sequences (1,1) is the 
 * top left for escape sequences and width and height of 80 and 24 or 25
 * are common. The gtk/cocoa backends expect (0,0) so this code is a fine
 * source of 'off by one' errors and confusion. Internally, the main struct
 * tesiObject is 0,0 based for x,y
 * 
 * If you don't setup a console_haveChar() callback (the short-circuit)
 * then you need to implement the following call backs in gtk & cocoa
 * 
 * Required 
 *  callback_printChar          Required
 *  callback_handleNL           Required for Shoes
 *  callback_handleBS           Required for Shoes
 *  cakkback_handleTAB          Required for Shoes (set to printChar)
 *  callback_insertCharacter    optional, be careful of x, y
 *  callback_printString        not needed for Shoes?
 *  callback_insertCharacter    optional
 *  callback_eraseLine          optional
 *  callback_eraseCharacter     optional
 * 	callback_moveCursor         be very careful with scrolled buffers.
 *	callback_scrollUp           Dragons Here!
 *  callback_scrollDown         Dragons Here!
 *  callback_bell               Dragons Here!
 *  callback_invertColors       God Forbid. 
 * 
*/
//#define DEBUG 1

// forward declares - these are private to tesi.c - they may simulate behaviour
// or do nothing at all. 
static void tesi_seqED(struct tesiObject *to); // Erase Display
static void tesi_seqEL(struct tesiObject *to); // Erase Line
/*
handleInput
	- reads data from file descriptor into buffer
*/

int tesi_handleInput(struct tesiObject *to) {
	char input[128];
	char *pointer, c;
	long lengthRead;
	int i; //, j, sequenceLength;
	//FILE *f;
	struct pollfd fds[1];

  // use sequenceLength as a local cache for faster ops?
	// avoid premature optimization ... wait until find bottlenecks

	// use poll for it's speed, allows us to call this function at regular intervals without first checking for input
	fds[0].fd = to->fd_activity;
	fds[0].events = POLLIN | POLLPRI;
	poll(fds, 1, 0);
	if((fds[0].revents & (POLLIN | POLLPRI)) == 0) {
		return 0;
	}

	lengthRead = read(to->ptyMaster, input, 128);

	pointer = input;
	for(i = 0; i < lengthRead; i++, pointer++) {
		c = *pointer;
		if(c == 0) { // skip NULL for unicode?
			continue;
		}
		// has Shoes put a short circuit in? Call it
		if (to->callback_haveCharacter) {
			to->callback_haveCharacter(to, c);
			continue;
    }

		if((c >= 1 && c <= 31) || c == 127) {
			tesi_handleControlCharacter(to, c);
			continue;
		}

		if(to->partialSequence) { // was set in tesi_handleControlCharacter()
			// keep track of the sequence type. some
			to->sequence[ to->sequenceLength++ ] = c;
			to->sequence[ to->sequenceLength ] = 0;
			if(
				(c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z')
			) // done with sequence
				to->partialSequence = 0;

			if(to->partialSequence == 0) {
				tesi_interpretSequence(to); // we've got everything in the esc sequence
				to->sequenceLength = 0;
				to->parametersLength = 0;
			}
		} else { // standard text, not part of any escape sequence
      if(to->partialSequence == 0) {
				// newlines aren't in visible range, so they shouldn't be a problem
				if(to->insertMode == 0 && to->callback_printCharacter) {
					to->callback_printCharacter(to, c, to->x, to->y);
					to->x++;
					//tesi_limitCursor(to, 1);
				}
        // cjc: we don't do insert mode
				if(to->insertMode == 1 && to->callback_insertCharacter) {
					to->callback_insertCharacter(to, c, to->x, to->y);
				}
			}
		}
	}
	return 1;
}

/*
 * tesi_handleControlCharacter
 * Handles carriage return, newline (line feed), backspace, tab and others
 * Also starts a new escape sequence when ASCII 27 is read
 * */
int tesi_handleControlCharacter(struct tesiObject *to, char c) {
	int i, j;
	// entire switch from Rote
	switch (c) {
		case '\x1B': // begin escape sequence (aborting previous one if any)
			to->partialSequence = 1;
			// possibly flush buffer
			break;

		case '\r': // carriage return ('M' - '@'). Move cursor to first column.
			to->x = 0;
			if(to->callback_handleRTN)
				to->callback_handleRTN(to, to->x, to->y);
			break;

		case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
			to->y++;
			//if(to->insertMode == 0 && to->linefeedMode == 1)
				to->x = 0;

			//if(i == 1 && to->callback_scrollUp)
			//	to->callback_scrollUp(to->pointer);
			//tesi_limitCursor(to, 1);
      if (to->callback_handleNL)
        to->callback_handleNL(to, to->x, to->y);
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
      if (to->callback_handleTAB) {
        to->callback_handleTAB(to, to->x, to->y);
        break; // This prevents tesi from writing spaces it's way
      }
			j = 8 - (to->x % 8);
			if(j == 0)
				j = 8;
			for(i = 0; i < j; i++, to->x++) {
				if(tesi_limitCursor(to, 1))
					break;
				if(to->callback_moveCursor)
					to->callback_moveCursor(to , to->x, to->y);
				if(to->callback_printCharacter)
					to->callback_printCharacter(to , ' ', to->x, to->y);
			}
	 		break;

		case '\a': // bell ('G' - '@')
	 		// do nothing for now... maybe a visual bell would be nice?
			if(to->callback_handleBEL)
				to->callback_handleBEL(to, to->x, to->y);
			break;

	 	case 8: // backspace cub1 cursor back 1 ('H' - '@')
	 		// what do i do about wrapping back up to previous line?
	 		// where should that be handled
	 		// just move cursor, don't print space
			//tesi_limitCursor(to, 1);
			if(to->callback_handleBS)
				to->callback_handleBS(to, to->x, to->y);
			if (to->x > 0)
				to->x--;
			break;

		default:
			return false;
			break;
	}
	return true;
}

/*
 * once an escape sequence has been completely read from buffer, it's passed here.
 * It is interpreted and makes calls to appropriate callbacks
 * This skips the [ present in most sequences, then collects the optional
 * parameters (ascii decimal characters), separated by ';' if more than one
 */
void tesi_interpretSequence(struct tesiObject *to) {
	char *p = to->sequence;
	char *q; // another pointer
	char c;
	char *secondChar = to->sequence; // used to test for ? after [
	char operation = to->sequence[to->sequenceLength - 1];
	int i,j;

	// init parameters to zero
	for(i=0; i<6; i++)
		to->parameters[i] = 0;
	to->parametersLength = 0;

	if(*secondChar == '?')   //ignore it
		p++;
	// parse numeric parameters
	q = p++; // move past esc [
	c = *p;
  
	while ((c >= '0' && c <= '9') || c == ';') {
		if (c == ';') {
			if (to->parametersLength > 6)
				return;  // too many - best to ignore.
			to->parametersLength++;
		} else {
			j = to->parameters[ to->parametersLength ];
			j = j * 10;
			j += c - '0';
			to->parameters[ to->parametersLength ] = j;
		}
		p++;
		c = *p;
	}
	if(p != q)
		to->parametersLength++;

	if( (operation >= 'A' && operation <= 'Z') || (operation >= 'a' && operation <= 'z')) {
		int j;
		switch(operation) {
			// RESET INITIALIZATIONS
			case 'l': // defaults
				to->parameters[0] = 0;
				tesi_processAttributes(to, to->parameters[0]) ; // FIXME:
				to->parameters[0] = 1;
				tesi_processAttributes(to, to->parameters[0]); // FIXME:
				// scroll regions, colors, etc.
				break;
			case 'J': // ED erase display
        tesi_seqED(to);
				break;

			// LINE-RELATED
			case 'K': // clear line -- 
        tesi_seqEL(to);
				break;
			case 'L': // insert line(s)
				if(to->callback_insertLines)
					to->callback_insertLines(to, to->parameters[0]);
				break;
      case 'M': // delelete line(s)
        if (to->callback_deleteLines)
          to->callback_deleteLines(to, to->parameters[0]);
        break;
        
			// ATTRIBUTES AND MODES
      case 'm':  // SGR attributes 
        for (i = 0; i < to->parametersLength; i++) 
				  tesi_processAttributes(to, to->parameters[i]);
				break;
			case 'h': // enter/exit insert mode
				break;

			// CURSOR RELATED
      case 'A': // CUU  Move cursor up the indicated # of rows.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->y -= j;
        tesi_limitCursor(to, 1);
        break;
      case 'B': // CUD Move cursor down the indicated # of rows.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->y += j;
        tesi_limitCursor(to, 1);
        break;
      case 'C': // CUF Move cursor right the indicated # of columns.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->x += j;
        tesi_limitCursor(to, 1);
        break;
      case 'D': // CUB Move cursor left the indicated # of columns.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->x -= j;
        tesi_limitCursor(to, 1);
        break;
      case 'E': // CNL Move cursor down the indicated # of rows, to column 1.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->y += j;
        to->x = 0;
        tesi_limitCursor(to, 1);
        break;
      case 'F': // CPL Move cursor up the indicated # of rows, to column 1.
        j = to->parameters[0];
        if (j == 0) j = 1;
        to->y -= j;
        to->x = 0;
        tesi_limitCursor(to, 1);
        break;
      case 'G': // CHA Move cursor to indicated column in current row.
        j = to->parameters[0];
        to->y = j - 1; // escape cursor address is 1..80, not 0..79
        tesi_limitCursor(to, 1);
        break;
      case 'f': // HVP
			case 'H': // CUP. move cursor to row, column
				// parameters should be 1 more than the real value
				if(to->parametersLength == 0)
					to->x = to->y = 0;
				else {
					to->x = to->parameters[1] - 1;
					to->y = to->parameters[0] - 1;
				}
				// limit cursor to boundaries
				tesi_limitCursor(to, 1);
				break;
      
			// SCROLLING RELATED
			case 'r': // change scrolling region
#ifdef DEBUG
				fprintf(stderr, "Change scrolling region from line %d to %d\n", i, j);
#endif
				if(to->parametersLength == 2) {
					i = to->parameters[0];
					j = to->parameters[1];
					to->scrollBegin = i;
					to->scrollEnd = j;
					if(to->callback_scrollRegion)
						to->callback_scrollRegion(to, i,j);
				} else {
					//0, 0
				}
				break;
#if 0 // no such thing in xterm 
			case 'D': // scroll down
				if(to->callback_scrollDown)
					to->callback_scrollDown(to);
				break;
			case 'U': // scroll up
				if(to->callback_scrollUp)
					to->callback_scrollUp(to);
				// cursor shouldn't change positions after a scroll
				// but this means the next output line (like a new prompt invoked after Enter on last line)
				// will be indented
				break;
#endif
  	}
	}
}


void tesi_processAttributes(struct tesiObject *to, int attr) {
  // cjc: modify for ECMA 48 SGR terminals. attributes in tesi
  // are useless. Maintain them in the caller as needed
  // http://man7.org/linux/man-pages/man4/console_codes.4.html
  if (attr == 0) {
    if (to->callback_attreset)
        to->callback_attreset(to);
  } else if (attr > 0 && attr <= 27) { // 1..27
     if (to->callback_charattr) 
       to->callback_charattr(to, attr); 
  } else if (attr >= 30 && attr <= 37) { // 30..37
      if (to->callback_setfgcolor)
        to->callback_setfgcolor(to, attr);
  } else if (attr >= 40 &&  attr <= 47) {
      if (to->callback_setbgcolor) 
        to->callback_setbgcolor(to, attr);
  } else if ((attr == 38) || (attr == 39) || (attr = 49)) {
      if (to->callback_setdefcolor)
        to->callback_setdefcolor(to, attr);
  } else {
      // ignored. 
  }
}

// Returns 1 if cursor was out of bounds
// however, there are no calls to limitCursor that DON'T want the callback_moveCursor invoked
// so the parameter is probably not necessary
// x, y are 0 based. height and width are 1 based
int tesi_limitCursor(struct tesiObject *tobj, int moveCursorRegardless) {
	int oldx = tobj->x;
	int oldy = tobj->y;

  // auto wrap - cjc: do not like
	if(tobj->x >= tobj->width) {
		tobj->x = 0;
		tobj->y = tobj->y + 1;
	}
	if(tobj->x < 0)
		tobj->x = 0;
    
	if(tobj->y >= tobj->height) {
		tobj->y = tobj->height - 1; //width,height are 1 based, x,y 0 based
		// tobj->height++;  //wacky but Shoes likes it.
		if(tobj->callback_scrollUp) {
		  tobj->callback_scrollUp(tobj);
		  tobj->x = 0;
		}
	}

	if(tobj->y < 0)
		tobj->y = 0;
    
	if(moveCursorRegardless || oldx != tobj->x || oldy != tobj->y) {
		if(tobj->callback_moveCursor)
			tobj->callback_moveCursor(tobj, tobj->x, tobj->y);
		return 1;
	}
	return 0;
}

/* 
 * Process those xterm sequences - each is quirky 
 * They could be implemented differently (conditional compile) for
 * Linux and osx or NOT IMPLEMENTED.
*/ 

// various forms of Erase Display
static void tesi_seqED(struct tesiObject *to) {
  int i;
  if ((! to->callback_eraseLine) || (! to->callback_moveCursor)) return;
  switch(to->parameters[0]) {
    case 0:  // from cursor to end of display
      to->callback_eraseLine(to, to->x, to->width, to->y);
      for (i = to->y+1; i < to->height; i++) {
        to->callback_eraseLine(to, 0, to->width, i);
      }
      // cursor doesn't change
      break;
    case 1:  // from start (1,1) to cursor
      for (i = 0; i < to->y; i++) {
        to->callback_eraseLine(to, 0, to->width, i);
      }
      to->callback_eraseLine(to, 0, to->x, to->y);
      break;
    case 2:  // whole display
    case 3:  // whole display and scrollback buffer (linux only?)
      if (to->callback_clearScreen) {
        to->callback_clearScreen(to, to->parameters[0] == 3);
      } else {
        for(i = 0; i < to->height; i++) {
          if(to->callback_moveCursor)
            to->callback_moveCursor(to, 0, i);
          if(to->callback_eraseLine)
            to->callback_eraseLine(to, 0, to->width, i);
        }
      }
      to->x = to->y = 0;
      if(to->callback_moveCursor)
        to->callback_moveCursor(to, to->x, to->y);
      break;
    default:
      break;
  }
}
// various forms of Erase Line (EL)
// you would expect that ' ' would replace the backing buffer and
// reflected visually
static void tesi_seqEL(struct tesiObject *to) {
  int arg = 0;
  switch(to->parameters[0]) {
    case 0: // erase line from cursor to end of line
      if(to->callback_eraseLine)
        to->callback_eraseLine(to, to->x, to->width, to->y);
      break;
    case 1: // erase line from start to cursor
      if(to->callback_eraseLine)
        to->callback_eraseLine(to, 0, to->x , to->y);
      break;
    case 2: // erase whole line
      if(to->callback_eraseLine)
        to->callback_eraseLine(to, 0, to->width, to->y);
      break;
  }
}

// ----  initialize 

struct tesiObject* newTesiObject(char *command, int width, int height) {
	struct tesiObject *to;
	//struct winsize ws;
	char message[32]; // really just a temp
	char *ptySlave;
	to = malloc(sizeof(struct tesiObject));
	if(to == NULL)
		return NULL;

	to->partialSequence = 0;
	to->sequence = malloc(sizeof(char) * 40); // escape sequence buffer
	to->sequence[0] = 0;
	to->sequenceLength = 0;
	//to->outputBuffer[0] = 0;
	//to->outputBufferLength = 0;
	to->parametersLength = 0;

	to->x = to->y = to->x2 = to->y2 = 0; // cursor x,y and window width,height
	to->width = width;
	to->height = height;
	to->scrollBegin = 0;
	to->scrollEnd = height - 1;
	to->insertMode = 0;
  // simple callbacks 
  to->callback_haveCharacter = NULL; // The shortcircuit to be replaced
  to->callback_handleRTN = NULL;
  to->callback_handleNL = NULL;
  to->callback_handleBS = NULL;
  to->callback_handleTAB = NULL;
	to->callback_handleBEL = NULL;
	to->callback_printCharacter = NULL;
  to->callback_printString = NULL;
  // more complex processing
  to->callback_attributes = NULL;
  to->callback_clearScreen = NULL;
	to->callback_insertCharacter = NULL;
	to->callback_eraseLine = NULL;
	to->callback_eraseCharacter = NULL;
	to->callback_moveCursor = NULL;
	to->callback_attributes = NULL;
	to->callback_scrollUp = NULL;
	to->callback_scrollDown = NULL;
	to->callback_invertColors = NULL;

	to->command[0] = to->command[1] = to->command[2] = NULL;
	to->pid = 0;
	to->ptyMaster = posix_openpt(O_RDWR|O_NOCTTY);

	to->fd_activity = to->ptyMaster; // descriptor to check whether the process has sent output
	to->fd_input = to->ptyMaster; // descriptor for sending input to child process

	grantpt(to->ptyMaster);
	unlockpt(to->ptyMaster);
	/* for shoes we don't fork. pty slave will be used for this process
	 * stdin/out/err  which replaces what was there in this process.
	 * This may be a bad idea or not work or leave zombie processses
	*/
  ptySlave = ptsname(to->ptyMaster);
  to->ptySlave = open(ptySlave, O_RDWR);
  // need to setup the terminal stuff for the slave side of the pty.
  struct termios slave_orig_term_settings; // Saved terminal settings
  struct termios new_term_settings; // Current terminal settings
  tcgetattr(to->ptySlave, &slave_orig_term_settings);
  new_term_settings = slave_orig_term_settings;
#if defined(SHOES_QUARTZ) || defined(SHOES_GTK_OSX)
  cfmakeraw (&new_term_settings);
#endif
  tcsetattr (to->ptySlave, TCSANOW, &new_term_settings);
  dup2(to->ptySlave, fileno(stdin));
  dup2(to->ptySlave, fileno(stdout));
  dup2(to->ptySlave, fileno(stderr));
#ifdef SHOES_QUARTZ
  setenv("TERM","xterm-256",1); 
#else
  setenv("TERM","xterm",1); 
#endif
  sprintf(message, "%d", width);
  setenv("COLUMNS", message, 1);
  sprintf(message, "%d", height);
  setenv("LINES", message, 1);
	return to;
}
/*
 * Why does this take a void pointer?
 * So that you don't have to cast before you pass the parameter
 * */
void deleteTesiObject(void *p) {
  struct tesiObject *to = (struct tesiObject*) p;
   close(to->ptyMaster);
  free(to->sequence);
  free(to->command[0]);
  if(to->command[1])
    free(to->command[1]);
  free(to);
// FIXME Is all ?
}

/* 
 *  stuff that may never be called or work
*/ 
