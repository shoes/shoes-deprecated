#include "tesi.h"
/* 
 * This is a state machine to parse a subset of xterm-256 escape sequences 
 * that arrive on a pty (slave side) and call back into gtk/cocoa functions
 * to implement that sequence 'clear_screen' or 'setcolor' or ...
 * 
 * Those call back functions have nothing to do with Shoes. The window they
 * draw to is invisible to Shoes scripts and won't call any shoes internal code
 * 
 * NOTE: the terminal may have x, y (e.g width 80, height 24). Gtk/cocoa
 * code maintains a large scroll back buffer,  so x=1,y=24 might mean
 * the first character of the last line shown if the last line of the
 * scrollback buffer is visible. Also possible the x,y a zero relative
 * so becareful.
 * 
 * Some keypad keys the user presses are sent to yet more callbacks and 
 * some are sent as 'escape' sequences
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
 *  callback_attributes         Recommended for Shoes
 *	callback_scrollUp           Dragons Here!
 *  callback_scrollDown         Dragons Here!
 *  callback_bell               Dragons Here!
 *  callback_invertColors       God Forbid. 
 * 
*/
//#define DEBUG 1

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
				tesi_interpretSequence(to); // we've got everything in th esc sequence
				to->sequenceLength = 0;
				to->parametersLength = 0;
			}
		} else { // standard text, not part of any escape sequence
      if(to->partialSequence == 0) {
				// newlines aren't in visible range, so they shouldn't be a problem
				if(to->insertMode == 0 && to->callback_printCharacter) {
					to->callback_printCharacter(to, c, to->x, to->y);
					to->x++;
					tesi_limitCursor(to, 1);
				}
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
#ifdef DEBUG
			fprintf(stderr, "Carriage return\n");
#endif
			to->x = 0;
			if(to->callback_handleRTN)
				to->callback_handleRTN(to, to->x, to->y);
			break;

		case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
#ifdef DEBUG
			fprintf(stderr, "Newline\n");
#endif
			to->y++;
			//if(to->insertMode == 0 && to->linefeedMode == 1)
				to->x = 0;

			//if(i == 1 && to->callback_scrollUp)
			//	to->callback_scrollUp(to->pointer);
			tesi_limitCursor(to, 1);
      if (to->callback_handleNL)
        to->callback_handleNL(to, to->x, to->y);
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
#ifdef DEBUG
			fprintf(stderr, "Tab. %d,%d (x,y)\n", to->x, to->y);
#endif
      if (to->callback_handleTAB) {
        to->callback_handleTAB(to, to->x, to->y);
        break; // This prevents tesi from writing spaces
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
#ifdef DEBUG
			fprintf(stderr, "End of Tab processing\n");
#endif
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
			tesi_limitCursor(to, 1);
			if(to->callback_handleBS)
				to->callback_handleBS(to, to->x, to->y);
			if (to->x > 0)
				to->x--;
			break;

		default:
#ifdef DEBUG
			fprintf(stderr, "Unrecognized control char: %d (^%c)\n", c, c + '@');
#endif
			return false;
			break;
	}
	return true;
}

/*
 * once an escape sequence has been completely read from buffer, it's passed here
 * it is interpreted and makes calls to appropriate callbacks
 * This skips the [ present in most sequences
 */
void tesi_interpretSequence(struct tesiObject *to) {
	char *p = to->sequence;
	char *q; // another pointer
	char c;
	char *secondChar = to->sequence; // used to test for ? after [
	char operation = to->sequence[to->sequenceLength - 1];
	int i,j;

	// preliminary reset of parameters
	for(i=0; i<6; i++)
		to->parameters[i] = 0;
	to->parametersLength = 0;

	if(*secondChar == '?')   //ignore it
		p++;
  // in ecma 48 terms, we are in a CSI and we have all the chars.
  // is 'm' at the end (SGR?)
  int endptr = strlen(to->sequence);
  if (to->sequence[endptr-1] == 'm') {
    p++;  // past the '['
    c = *p;
    int accum = 0;
    while ((c >= '0' && c <= '9') ||  c == ';' || c == 'm')  {
      if (c == ';') {
        tesi_processAttributes(to, accum);
        accum = 0;
      } else if (c == 'm') {
        tesi_processAttributes(to, accum);
        accum = 0;
        return;
      } else {
        accum = accum * 10;
        accum += c - '0';
     }
      p++;
      c = *p;
    }
    return;
  }
	// parse numeric parameters
	q = p++; //cjc: add ++
	c = *p;
	while ((c >= '0' && c <= '9') || c == ';') {
		if (c == ';') {
			//if (to->parametersLength >= MAX_CSI_ES_PARAMS)
				//return;
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
			case 'R': // defaults
				to->parameters[0] = 0;
				tesi_processAttributes(to, to->parameters[0]) ; // FIXME:
				to->parameters[0] = 1;
				tesi_processAttributes(to, to->parameters[0]); // FIXME:
				// scroll regions, colors, etc.
				break;
			case 'C': // clear screen
#ifdef DEBUG
				fprintf(stderr, "Clear screen\n");
#endif
        if (to->callback_clearScreen) {
          to->callback_clearScreen(to);
          break; // don't do ugly clear
        }

				for(i = 0; i < to->height; i++) {
					if(to->callback_moveCursor)
						to->callback_moveCursor(to, 0, i);
					if(to->callback_eraseLine)
						to->callback_eraseLine(to, i);
				}
				to->x = to->y = 0;
				if(to->callback_moveCursor)
						to->callback_moveCursor(to, to->x, to->y);
				break;

			// LINE-RELATED
			case 'c': // clear line
				if(to->callback_eraseLine)
					to->callback_eraseLine(to, to->y);
				break;
			case 'L': // insert line
				if(to->callback_insertLine)
					to->callback_insertLine(to, to->y);
				break;

			// ATTRIBUTES AND MODES
			case 'a': // change output attributes
				tesi_processAttributes(to, 0); // FIXME
				break;
      case 'm':  // what really works; 
				tesi_processAttributes(to, 0); // FIXME
				break;
			case 'I': // enter/exit insert mode
				break;

			// CURSOR RELATED
			case 'M': // cup. move to col, row, cursor to home
#ifdef DEBUG
				//fprintf(stderr, "Move cursor (x,y): %d %d\n", to->x, to->y);
#endif
				// parameters should be 1 more than the real value
				if(to->parametersLength == 0)
					to->x = to->y = 0;
				else {
					to->x = to->parameters[1];
					to->y = to->parameters[0];
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

			// INPUT RELATED
			/*
			case 'h': // left arrow
				if(to->x > 0) {
				       to->x--;
				}
				break;
			case 'l': // right arrow
				if(to->x < to->width) {
					to->x++;
				}
				break;
			case 'k': // up arrow
				if(to->y > 0) {
					to->y--;
				}
				break;
			case 'j': // down arrow
				if(to->y < to->height) {
					to->y++;
				}
				break;
			*/
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

/*
void tesi_bufferPush(struct tesiObject *to, char c) {
	if(to->outputBufferLength == TESI_OUTPUT_BUFFER_LENGTH) {
		// PRINT
	}
	to->outputBuffer[ to->outputBufferLength++ ] = c;
	//to->sequence[ to->sequenceLength ] = 0; // don't use null as terminator
}
*/

// Returns 1 if cursor was out of bounds
// however, there are no calls to limitCursor that DON'T want the callback_moveCursor invoked
// so the parameter is probably not necessary
int tesi_limitCursor(struct tesiObject *tobj, int moveCursorRegardless) {
	// create some local variables for speed // cjc bad idea not mine
	//int width = to->width;
	//int height = to->height;
	int a; //, x;
	int b; //, y;

	a = tobj->x;
	b = tobj->y;

	if(tobj->x >= tobj->width) {
		tobj->x = 0;
		tobj->y = tobj->y + 1;
	}
	if(tobj->x < 0)
		tobj->x = 0;
#ifdef DEBUG
	if(a != x)
		fprintf(stderr, "Cursor out of bounds in X direction: %d\n",a);
#endif

	if(tobj->y >= tobj->height) {
		//tobj->y = tobj->height - 1; //width,height are 1 based, x,y 0 based
		tobj->height++;  //wacky but Shoes likes it.
		if(tobj->callback_scrollUp) {
		  tobj->callback_scrollUp(tobj);
		  tobj->x = 0;
		}
	}
	if(tobj->y < 0)
		tobj->y = 0;
#ifdef DEBUG
	if(b != y)
		fprintf(stderr, "Cursor out of bounds in Y direction: %d\n",b);
#endif
	if(moveCursorRegardless || a != tobj->x || b != tobj->y) {
		if(tobj->callback_moveCursor)
			tobj->callback_moveCursor(tobj, tobj->x, tobj->y);
		//to->x = x;
		//to->y = y;
		return 1;
	}
	return 0;
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
  
  //kill(-(getpgid(to->pid)), SIGTERM); // kill all with this process group id
  
//  This freezes Shoes when closing console ! unecessary ? FIXME, looks like to->pid == 0, not used
//  kill(to->pid, SIGTERM); // probably don't need this line
  
  //waitpid(to->pid); // don't need this on OSX if never called

  close(to->ptyMaster);

  free(to->sequence);
  free(to->command[0]);
  if(to->command[1])
    free(to->command[1]);
  free(to);
// FIXME Is all this enough ?
}
