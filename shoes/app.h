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

#define SHOES_APP_HEIGHT 500
#define SHOES_APP_WIDTH  700
#define SHOES_SHORTNAME  "shoes"
#define SHOES_APPNAME    "Shoes"
#define SHOES_VLCLASS    "Shoes VLC"
#define SHOES_SLOTCLASS  "Shoes Slot"

//
// abstract window struct
//
typedef struct _shoes_app {
  SHOES_APP_OS os;
  SHOES_SLOT_OS slot;
  int width, height, mousex, mousey, resizable, hidden, started;
  VALUE self;
  VALUE canvas;
  VALUE nestslot;
  VALUE nesting;
  VALUE timers;
  VALUE styles;
  ID cursor;
  VALUE title;
  VALUE location;
  VALUE owner;
} shoes_app;

//
// function signatures
//
VALUE shoes_app_alloc(VALUE);
VALUE shoes_app_new(VALUE);
VALUE shoes_apps_get(VALUE);
#ifdef SHOES_WIN32
shoes_code shoes_classex_init();
#endif
#ifdef SHOES_QUARTZ
void shoes_app_quartz_install();
#endif
shoes_code shoes_app_start(VALUE, char *);
shoes_code shoes_app_open(shoes_app *, char *);
shoes_code shoes_app_loop();
shoes_code shoes_app_visit(shoes_app *, char *);
shoes_code shoes_app_paint(shoes_app *);
shoes_code shoes_app_cursor(shoes_app *, ID);
shoes_code shoes_app_motion(shoes_app *, int, int);
shoes_code shoes_app_click(shoes_app *, int, int, int);
shoes_code shoes_app_release(shoes_app *, int, int, int);
shoes_code shoes_app_keypress(shoes_app *, VALUE);
VALUE shoes_app_close_window(shoes_app *);
shoes_code shoes_app_goto(shoes_app *, char *);
shoes_code shoes_slot_repaint(SHOES_SLOT_OS *);
void shoes_app_reset_styles(shoes_app *);
void shoes_app_style(shoes_app *, VALUE, VALUE);
VALUE shoes_app_location(VALUE);
VALUE shoes_app_is_started(VALUE);
VALUE shoes_app_quit(VALUE);

#endif
