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

#ifdef VIDEO
#ifdef VLC_0_8
#include <vlc/libvlc.h>
#define SHOES_VLC(self_t) self_t->vlc
#define shoes_libvlc_clear libvlc_playlist_clear
#define shoes_libvlc_prev  libvlc_playlist_prev
#define shoes_libvlc_next  libvlc_playlist_next
#define shoes_libvlc_pause libvlc_playlist_pause
#define shoes_libvlc_stop  libvlc_playlist_stop
#else
#include <vlc/vlc.h>
#include <vlc/libvlc.h>
#define libvlc_destroy  libvlc_media_player_release
#define vlc_int64_t     libvlc_time_t
#define shoes_libvlc_clear libvlc_media_player_pause
#define shoes_libvlc_prev  libvlc_media_player_stop
#define shoes_libvlc_next  libvlc_media_player_stop
#define shoes_libvlc_pause libvlc_media_player_pause
#define shoes_libvlc_stop  libvlc_media_player_stop
#define SHOES_VLC(self_t) shoes_world->vlc
#define VLC_0_9
#endif
#endif

#include "canvas.h"
#include "code.h"
#include "config.h"

#define SHOES_APP_HEIGHT 500
#define SHOES_APP_WIDTH  600
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
    minwidth, minheight;
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
VALUE shoes_app_slot(VALUE);
shoes_code shoes_app_start(VALUE, char *);
shoes_code shoes_app_open(shoes_app *, char *);
shoes_code shoes_app_loop();
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

#endif
