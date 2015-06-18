#include "tesi.h"
// tesInterpreter
/*
Interprets TESI escape sequences.
See terminfo definition
*/

//#define DEBUG 1

/*
handleInput
	- reads data from file descriptor into buffer
	-

interpretSequence
	- once sequence has been completely read from buffer, it's passed here
	- it is interpreted and makes calls to appropriate callbacks
*/

int tesi_handleInput(struct tesiObject *to) {
	char input[128];
	char *pointer, c;
	int lengthRead;
	int i, j, sequenceLength;
	FILE *f;
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

	//f = fopen("output", "a+");
	//fwrite(input, lengthRead, 1, f);
	//fclose(f);

	pointer = input;
	for(i = 0; i < lengthRead; i++, pointer++) {
		c = *pointer;
		if(c == 0) { // skip NULL for unicode?
#ifdef DEBUG
			fprintf(stderr, "Skipped a NULL character\n");
#endif
			continue;
		}

		if((c >= 1 && c <= 31) || c == 127) {
			tesi_handleControlCharacter(to, c);
			continue;
		}

		if(to->partialSequence) {
			// keep track of the sequence type. some
			to->sequence[ to->sequenceLength++ ] = c;
			to->sequence[ to->sequenceLength ] = 0;
			if(
				(c >= 'a' && c <= 'z') ||
				(c >= 'A' && c <= 'Z')
			) // done with sequence
				to->partialSequence = 0;

			if(to->partialSequence == 0) {
#ifdef DEBUG
				fprintf(stderr, "Sequence: %s\n", to->sequence);
#endif
				tesi_interpretSequence(to);
				to->sequenceLength = 0;
				to->parametersLength = 0;
			}
		} else { // standard text, not part of any escape sequence
			if(to->partialSequence == 0) {
				// newlines aren't in visible range, so they shouldn't be a problem
				if(to->insertMode == 0 && to->callback_printCharacter) {
					to->callback_printCharacter(to->pointer, c, to->x, to->y);
					to->x++;
					tesi_limitCursor(to, 1);
				}
				if(to->insertMode == 1 && to->callback_insertCharacter) {
					to->callback_insertCharacter(to->pointer, c, to->x, to->y);
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
			if(to->callback_moveCursor)
				to->callback_moveCursor(to->pointer, to->x, to->y);
			break;

		case '\n':  // line feed ('J' - '@'). Move cursor down line and to first column.
#ifdef DEBUG
			fprintf(stderr, "Newline\n");
#endif
			to->y++;
			//if(to->insertMode == 0 && to->linefeedMode == 1)
				to->x = 0;
			
			if(i == 1 && to->callback_scrollUp)
				to->callback_scrollUp(to->pointer);
			
			tesi_limitCursor(to, 1);
			break;

		case '\t': // ht - horizontal tab, ('I' - '@')
#ifdef DEBUG
			fprintf(stderr, "Tab. %d,%d (x,y)\n", to->x, to->y);
#endif
			j = 8 - (to->x % 8);		
			if(j == 0)
				j = 8;
			for(i = 0; i < j; i++, to->x++) {
				if(tesi_limitCursor(to, 1))
					break;
				if(to->callback_moveCursor)
					to->callback_moveCursor(to->pointer, to->x, to->y);
				if(to->callback_printCharacter)
					to->callback_printCharacter(to->pointer, ' ', to->x, to->y);
			}
#ifdef DEBUG
			fprintf(stderr, "End of Tab processing\n");
#endif
	 		break;

		case '\a': // bell ('G' - '@')
	 		// do nothing for now... maybe a visual bell would be nice?
			if(to->callback_bell)
				to->callback_bell(to->pointer);
			break;

	 	case 8: // backspace cub1 cursor back 1 ('H' - '@')
	 		// what do i do about wrapping back up to previous line?
	 		// where should that be handled
	 		// just move cursor, don't print space
			tesi_limitCursor(to, 1);
			if(to->callback_eraseCharacter)
				to->callback_eraseCharacter(to->pointer, to->x, to->y);
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
 * This skips the [ present in most sequences
 */
void tesi_interpretSequence(struct tesiObject *to) {
	char *p = to->sequence;
	char *q; // another pointer
	char c;
	char *secondChar = to->sequence; // used to test for ? after [
	char operation = to->sequence[to->sequenceLength - 1];
	int i,j;

#ifdef DEBUG
	//fprintf(stderr, "Operation: %c\n", operation);
#endif

	// preliminary reset of parameters
	for(i=0; i<6; i++)
		to->parameters[i] = 0;
	to->parametersLength = 0;

	if(*secondChar == '?')
		p++;

	// parse numeric parameters
	q = p;
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
				tesi_processAttributes(to);
				to->parameters[0] = 1;
				tesi_processAttributes(to);
				// scroll regions, colors, etc.
				break;
			case 'C': // clear screen
#ifdef DEBUG
				fprintf(stderr, "Clear screen\n");
#endif

				for(i = 0; i < to->height; i++) {
					if(to->callback_moveCursor)
						to->callback_moveCursor(to->pointer, 0, i);
					if(to->callback_eraseLine)
						to->callback_eraseLine(to->pointer, i);
				}
				to->x = to->y = 0;
				if(to->callback_moveCursor)
						to->callback_moveCursor(to->pointer, to->x, to->y);
				break;

			// LINE-RELATED
			case 'c': // clear line
				if(to->callback_eraseLine)
					to->callback_eraseLine(to->pointer, to->y);
				break;
			case 'L': // insert line
				if(to->callback_insertLine)
					to->callback_insertLine(to->pointer, to->y);
				break;

			// ATTRIBUTES AND MODES
			case 'a': // change output attributes
				tesi_processAttributes(to);
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
						to->callback_scrollRegion(to->pointer, i,j);
				} else {
					//0, 0
				}
				break;
			case 'D': // scroll down
				if(to->callback_scrollDown)
					to->callback_scrollDown(to->pointer);
				break;
			case 'U': // scroll up
				if(to->callback_scrollUp)
					to->callback_scrollUp(to->pointer);
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


void tesi_processAttributes(struct tesiObject *to) {
	//short bold, underline, blink, reverse, foreground, background, charset, i;

	//standout, underline, reverse, blink, dim, bold, foreground, background
	// no need for invisible or dim, right?
	switch(to->parameters[0]) {
		case 0: // all off
			to->attributes[0] = to->attributes[1] = to->attributes[2] = to->attributes[3] = to->attributes[4] = to->attributes[5] = to->attributes[6] = to->attributes[7] = 0;
			//bold = underline = blink = reverse = foreground = background = charset = 0;
			break;
		case 1: // standout
			to->attributes[4] = to->parameters[1];
			break;
		case 2: // underline
			to->attributes[1] = to->parameters[1];
			break;
		case 3: // reverse
			to->attributes[2] = 1;
			break;
		case 4: // blink
			to->attributes[3] = 1;
			break;
		case 5: // bold
			to->attributes[4] = 1;
			break;
		case 6: // foreground color
			to->attributes[5] = to->parameters[1];
			// setf
			// black blue green cyan red magenta yellow white
			// setaf
			// black, red green yellow blue magenta cyan white
			break;
		case 7: // background color
			to->attributes[6] = to->parameters[1];
			break;
	}

	if(to->callback_attributes)
		to->callback_attributes(to->pointer, to->attributes[4], to->attributes[3], to->attributes[2], to->attributes[1], to->attributes[5], to->attributes[6], 0);
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
int tesi_limitCursor(struct tesiObject *to, int moveCursorRegardless) {
	// create some local variables for speed
	int width = to->width;
	int height = to->height;
	int a, x;
	int b, y;

	a = x = to->x;
	b = y = to->y;

	if(x == width) {
		x = 0;
		y++;
	}
	if(x < 0)
		x = 0;
#ifdef DEBUG
	if(a != x)
		fprintf(stderr, "Cursor out of bounds in X direction: %d\n",a);
#endif

	if(y == height) {
		y = height - 1;
		if(to->callback_scrollUp)
			to->callback_scrollUp(to->pointer);
	}
	if(y < 0)
		y = 0;
#ifdef DEBUG
	if(b != y)
		fprintf(stderr, "Cursor out of bounds in Y direction: %d\n",b);
#endif
	if(moveCursorRegardless || a != x || b != y) {
		if(to->callback_moveCursor)
			to->callback_moveCursor(to->pointer, x, y);
		to->x = x;
		to->y = y;
		return 1;
	}
	return 0;
}


struct tesiObject* newTesiObject(char *command, int width, int height) {
	struct tesiObject *to;
	struct winsize ws;
	char message[256];
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

	to->callback_printCharacter = NULL;
	to->callback_printString = NULL;
	to->callback_insertCharacter = NULL;
	to->callback_eraseLine = NULL;
	to->callback_eraseCharacter = NULL;
	to->callback_moveCursor = NULL;
	to->callback_attributes = NULL;
	to->callback_scrollUp = NULL;
	to->callback_scrollDown = NULL;
	to->callback_bell = NULL;
	to->callback_invertColors = NULL;

	to->command[0] = to->command[1] = to->command[2] = NULL;
	to->pid = 0;
	to->ptyMaster = posix_openpt(O_RDWR|O_NOCTTY);

	to->fd_activity = to->ptyMaster; // descriptor to check whether the process has sent output
	to->fd_input = to->ptyMaster; // descriptor for sending input to child process

	// welcome message
	/*
	sprintf(message, "echo \"This %d x %d terminal is controlled by TESI...\"\n", width, height);
	message[255] = 0;
	write(to->fd_input, message, strlen(message));
	*/

	grantpt(to->ptyMaster);
	unlockpt(to->ptyMaster);
	/* for shoes we don't fork. pty slave will be used for this process
	 * stdin/out/err  which replaces what was there in this process.
	 * This may be a bad idea or not work or leave zombie processses
	*/
#ifndef OLD_PTY_CODE
	ptySlave = ptsname(to->ptyMaster);
	to->ptySlave = open(ptySlave, O_RDWR);
	dup2(to->ptySlave, fileno(stdin));
	dup2(to->ptySlave, fileno(stdout));
	dup2(to->ptySlave, fileno(stderr));

#else
	to->pid = fork();
	//setpgid(to->pid, to->pid); // set new process group id for easy killing of shell children
	if(to->pid == 0) { // inside child
		ptySlave = ptsname(to->ptyMaster);
		to->ptySlave = open(ptySlave, O_RDWR);
		// dup fds to stdin and stdout, or something like, then exec /bin/bash
		dup2(to->ptySlave, fileno(stdin));
		dup2(to->ptySlave, fileno(stdout));
		dup2(to->ptySlave, fileno(stderr));

		//clearenv(); // don't clear because we want to keep all other variables
		setenv("TERM","tesi",1); // vt102
		sprintf(message, "%d", width);
		setenv("COLUMNS", message, 1);
		sprintf(message, "%d", height);
		setenv("LINES", message, 1);

		//fflush(stdout); // flush output now because it will be cleared upon execv

		//if(execl("/bin/bash", "/bin/bash", "--noediting", NULL) == -1)
		if(execl("/bin/bash", "/bin/bash", NULL) == -1)
		//if(execl("/bin/cat", "/bin/cat", NULL) == -1)
			exit(EXIT_FAILURE); // exit and become zombie until parent cares....
	}
#endif
	return to;
}
/*
 * Why does this take a void pointer?
 * So that you don't have to cast before you pass the parameter
 * */
void deleteTesiObject(void *p) {
	struct tesiObject *to = (struct tesiObject*) p;

	//kill(-(getpgid(to->pid)), SIGTERM); // kill all with this process group id
	kill(to->pid, SIGTERM); // probably don't need this line
	waitpid(to->pid);

	close(to->ptyMaster);

	free(to->sequence);
	free(to->command[0]);
	if(to->command[1])
		free(to->command[1]);
	free(to);
}
