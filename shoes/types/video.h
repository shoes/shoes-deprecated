#include "shoes/ruby.h"
#include "shoes/world.h"
#include "shoes/internal.h"
#include "shoes/app.h"

#ifndef SHOES_VIDEO_TYPE_H
#define SHOES_VIDEO_TYPE_H

#define SHOES_VIDEO 1

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

typedef struct {
    VALUE parent;
    VALUE attr;
    shoes_place place;
    SHOES_CONTROL_REF ref;
    int realized;
    SHOES_SLOT_OS *slot;
    int init;
} shoes_video;

// native forward declarations
extern SHOES_CONTROL_REF shoes_native_surface_new(VALUE, VALUE);
extern unsigned long shoes_native_surface_get_window_handle(SHOES_CONTROL_REF);
extern void shoes_native_surface_position(SHOES_SURFACE_REF, shoes_place *,
                                   VALUE, shoes_canvas *, shoes_place *);
extern void shoes_native_surface_hide(SHOES_SURFACE_REF);
extern void shoes_native_surface_show(SHOES_SURFACE_REF);
extern void shoes_native_surface_remove(SHOES_SURFACE_REF);

/* each widget should have its own init function */
void shoes_video_init();

// ruby
VALUE shoes_video_alloc(VALUE);
VALUE shoes_video_new(VALUE, VALUE);
VALUE shoes_video_draw(VALUE, VALUE, VALUE);
VALUE shoes_video_get_parent(VALUE);
VALUE shoes_video_get_drawable(VALUE);
VALUE shoes_video_get_realized(VALUE);
VALUE shoes_video_remove(VALUE);
VALUE shoes_video_show(VALUE);
VALUE shoes_video_hide(VALUE);
VALUE shoes_video_toggle(VALUE self);
VALUE shoes_video_style(int, VALUE*, VALUE);
VALUE shoes_video_displace(VALUE, VALUE, VALUE);
VALUE shoes_video_move(VALUE, VALUE, VALUE);
VALUE shoes_video_get_width(VALUE);
VALUE shoes_video_get_height(VALUE);
VALUE shoes_video_get_left(VALUE);
VALUE shoes_video_get_top(VALUE);

// Canvas
VALUE shoes_canvas_video(int, VALUE*, VALUE);
VALUE shoes_canvas_c_video(int, VALUE*, VALUE);
VALUE shoes_app_c_video(int, VALUE*, VALUE);

#endif
