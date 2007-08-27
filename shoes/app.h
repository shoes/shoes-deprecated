//
// shoes/app.h
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
// This is really just a light wrapper around Cairo, which does most of the
// work anyway.  I'm not sure why they don't do this for ya.  Probably so I
// could do it in Shoes!!
//

#ifndef SHOES_APP_H
#define SHOES_APP_H

#include <cairo.h>
#include <ruby.h>

#include "shoes/canvas.h"
#include "shoes/code.h"
#include "shoes/config.h"

#define SHOES_APP_HEIGHT 400
#define SHOES_APP_WIDTH  400
#define SHOES_SHORTNAME  "shoes"
#define SHOES_APPNAME    "Shoes"

//
// abstract window struct
//
typedef struct _shoes_app {
  APPKIT kit;
  APPSLOT slot;
  int width, height, mousex, mousey;
  const char *path;
  VALUE canvas;
  VALUE timers;
  ID cursor;
  VALUE title;
} shoes_app;

extern shoes_app *global_app;

//
// function signatures
//
shoes_app *shoes_app_new(void);
void shoes_app_free(shoes_app *);
shoes_code shoes_app_load(shoes_app *);
shoes_code shoes_app_open(shoes_app *);
shoes_code shoes_app_loop(shoes_app *, char *);
shoes_code shoes_app_visit(shoes_app *, char *);
shoes_code shoes_app_paint(shoes_app *);
shoes_code shoes_app_cursor(shoes_app *, ID);
shoes_code shoes_app_motion(shoes_app *, int, int);
shoes_code shoes_app_click(shoes_app *, int, int, int);
shoes_code shoes_app_release(shoes_app *, int, int, int);
shoes_code shoes_app_keypress(shoes_app *, VALUE);
shoes_code shoes_app_close(shoes_app *);
shoes_code shoes_app_goto(shoes_app *, char *);
shoes_code shoes_slot_repaint(APPSLOT *);
VALUE shoes_app_quit(VALUE);
shoes_code shoes_init(void);

#endif
