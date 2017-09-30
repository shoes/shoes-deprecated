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

#define SHOES_APP_WIDTH  600
#define SHOES_APP_HEIGHT 500
#define SHOES_SHORTNAME  "shoes"
#define SHOES_APPNAME    "Shoes"
#define SHOES_VLCLASS    "Shoes VLC"
#define SHOES_SLOTCLASS  "Shoes Slot"
#define SHOES_HIDDENCLS  "Shoes Hidden"

//
// abstract window struct
//
typedef struct _shoes_app {
    SHOES_APP_OS os;
    SHOES_SLOT_OS *slot;
    cairo_t *scratch;
    int width, height, mouseb, mousex, mousey,
        resizable, hidden, started, fullscreen,
        minwidth, minheight, decorated;
    double opacity;
    VALUE self;
    VALUE canvas;
    VALUE keypresses;
    VALUE nestslot;
    VALUE nesting;
    VALUE extras;
    VALUE styles;
    VALUE groups;
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
int shoes_app_remove(shoes_app *);
VALUE shoes_app_get_title(VALUE);
VALUE shoes_app_set_title(VALUE, VALUE);
VALUE shoes_app_get_fullscreen(VALUE);
VALUE shoes_app_set_fullscreen(VALUE, VALUE);
VALUE shoes_app_set_opacity(VALUE app, VALUE opacity);
VALUE shoes_app_get_opacity(VALUE app);
VALUE shoes_app_set_decoration(VALUE app, VALUE decorated);
VALUE shoes_app_get_decoration(VALUE app);
VALUE shoes_app_slot(VALUE);
VALUE shoes_app_set_icon(VALUE, VALUE); // New 3.2.19
VALUE shoes_app_set_wtitle(VALUE, VALUE); // New in 3.2.19
VALUE shoes_app_console(VALUE); // New in 3.2.23 ?
VALUE shoes_app_terminal(int, VALUE*, VALUE); //new in 3.3.2
shoes_code shoes_app_start(VALUE, char *);
shoes_code shoes_app_open(shoes_app *, char *);
shoes_code shoes_app_loop(void);
shoes_code shoes_app_visit(shoes_app *, char *);
shoes_code shoes_app_paint(shoes_app *);
shoes_code shoes_app_motion(shoes_app *, int, int);
shoes_code shoes_app_click(shoes_app *, int, int, int);
shoes_code shoes_app_release(shoes_app *, int, int, int);
shoes_code shoes_app_wheel(shoes_app *, ID, int, int);
shoes_code shoes_app_keydown(shoes_app *, VALUE);
shoes_code shoes_app_keypress(shoes_app *, VALUE);
shoes_code shoes_app_keyup(shoes_app *, VALUE);
VALUE shoes_app_close_window(shoes_app *);
VALUE shoes_sys(char *, int);
shoes_code shoes_app_goto(shoes_app *, char *);
shoes_code shoes_slot_repaint(SHOES_SLOT_OS *);
void shoes_app_reset_styles(shoes_app *);
void shoes_app_style(shoes_app *, VALUE, VALUE);
VALUE shoes_app_location(VALUE);
VALUE shoes_app_is_started(VALUE);
VALUE shoes_app_quit(VALUE);
VALUE shoes_app_resize_window(VALUE, VALUE, VALUE);

VALUE shoes_app_resize_window(VALUE, VALUE, VALUE);
VALUE shoes_app_get_resizable(VALUE);
VALUE shoes_app_set_resizable(VALUE, VALUE);

// global var for console up and running
extern int shoes_global_terminal;
#ifdef SHOES_QUARTZ
extern int osx_cshoes_launch;
#endif

#endif
