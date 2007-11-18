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
#define SHOES_VLCLASS    "Shoes VLC"

//
// abstract window struct
//
typedef struct _shoes_app {
  SHOES_APP_OS os;
  SHOES_SLOT_OS slot;
  int width, height, mousex, mousey, resizable;
  VALUE self;
  VALUE canvas;
  VALUE nesting;
  VALUE timers;
  VALUE styles;
  ID cursor;
  VALUE title;
  VALUE location;
} shoes_app;

//
// function signatures
//
VALUE shoes_app_alloc(VALUE);
VALUE shoes_app_new(void);
shoes_code shoes_app_start(VALUE, char *);
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
shoes_code shoes_slot_repaint(SHOES_SLOT_OS *);
void shoes_app_reset_styles(shoes_app *);
void shoes_app_style(shoes_app *, VALUE, VALUE);
VALUE shoes_app_location(VALUE);
VALUE shoes_app_quit(VALUE);

#endif
