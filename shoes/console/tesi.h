#ifndef TESI_H
#define TESI_H

// _XOPEN_SOURCE for posix_openpt() from stdlin and fcntl
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <pty.h>
#include <utmp.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
//#include <glib-1.2/glib.h>

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif

#define TESI_OUTPUT_BUFFER_LENGTH 128

struct tesiObject;

/*
 * should I pass this to the callbacks instead?
 * It's nice to have the x,y info right there
struct tesiPointer {
	int *x;
	int *y;
	void *pointer;
}
*/

struct tesiObject {
	// file descriptor for  "select"ing whether there are terminal sequences to be processed
	int fd_activity;
	int fd_input;
	int partialSequence; // whether we've begun buffering an escape sequence
	char *sequence; // escape sequence buffer
	int sequenceLength;
	//char outputBuffer[129];
	//int outputBufferLength;
	void *pointer; // gtk_text_view

	int parameters[32];
	int parametersLength;
	char attributes[8]; // display attributes: bold, foreground, etc
	int insertMode;

	pid_t pid;
	int ptyMaster;
	int ptySlave;
	char *command[3];

	// callbacks
	void (*callback_clear)(void*); // clear canvas
	void (*callback_haveCharacter)(void*, char);
	void (*callback_printCharacter)(void*, char, int, int); // print character at x, y
	void (*callback_printString)(void*, char*, int, int, int); // print string of length at x, y
	void (*callback_insertCharacter)(void*, char, int, int); // insert character at x, y
	void (*callback_insertLine)(void*, int); // insert line at line y
	void (*callback_eraseLine)(void*, int); // erase line at line y
	void (*callback_eraseCharacter)(void*, int, int); // erase character at x, y
	//void (*callback_printString)(void*, char, x, y); // print string at x, y
	void (*callback_moveCursor)(void*, int, int);
	void (*callback_attributes)(void*, short, short, short, short, short, short, short); // bold, blink, inverse, underline, foreground, background, charset
	void (*callback_scrollRegion)(void*,int,int);
	void (*callback_scrollUp)(void*);
	void (*callback_scrollDown)(void*);
	void (*callback_bell)(void*);
	void (*callback_invertColors)(void*);

	int x, y, x2, y2, width, height, scrollBegin, scrollEnd; // cursor x,y and window width,height
	//int alternativeChar;
};

// PRIVATE
int tesi_handleControlCharacter(struct tesiObject*, char);
void tesi_interpretSequence(struct tesiObject*);
// limit cursor to terminal boundaries. return 1 if cursor out of bounds
// set second param to 1 to invoke moveCursor callback whether or not cursor is out of bounds
int tesi_limitCursor(struct tesiObject*, int);
void tesi_processAttributes(struct tesiObject*);

// PUBLIC
struct tesiObject* newTesiObject(char*, int, int);
void deleteTesiObject(void*);
int tesi_handleInput(struct tesiObject*);

#endif
