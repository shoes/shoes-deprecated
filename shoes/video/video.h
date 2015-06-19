
#include "shoes/world.h"
#include "shoes/internal.h"


#ifndef VIDEO_H
#define	VIDEO_H

#ifdef VIDEO

#define SHOES_VIDEO 1

#include <vlc/vlc.h>       
#include <vlc/libvlc.h>

#define libvlc_destroy  libvlc_media_player_release
#define vlc_int64_t     libvlc_time_t
//#define shoes_libvlc_prev  libvlc_media_player_stop
//#define shoes_libvlc_next  libvlc_media_player_stop
#define shoes_libvlc_pause libvlc_media_player_pause
#define shoes_libvlc_stop  libvlc_media_player_stop
#define SHOES_VLC(self_t) shoes_world->vlc

SHOES_CONTROL_REF shoes_native_surface_new(shoes_canvas *, VALUE, shoes_place *);
void shoes_native_surface_position(SHOES_CONTROL_REF, shoes_place *, 
                                            VALUE, shoes_canvas *, shoes_place *);
void shoes_native_surface_hide(SHOES_CONTROL_REF);
void shoes_native_surface_show(SHOES_CONTROL_REF);
void shoes_native_surface_remove(shoes_canvas *, SHOES_CONTROL_REF);

typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  SHOES_CONTROL_REF ref;
  //libvlc_exception_t excp; removed since vlc 1.1.0
  libvlc_media_player_t *vlc;
  int init;
  VALUE path;
  SHOES_SLOT_OS *slot;
} shoes_video;

void shoes_video_mark(shoes_video *);
VALUE shoes_video_alloc(VALUE);
VALUE shoes_video_new(VALUE, VALUE, VALUE);
VALUE shoes_video_draw(VALUE, VALUE, VALUE);
VALUE shoes_video_remove(VALUE);
VALUE shoes_video_show(VALUE);
VALUE shoes_video_hide(VALUE);
VALUE shoes_video_is_playing(VALUE);
VALUE shoes_video_play(VALUE);
VALUE shoes_video_pause(VALUE);
VALUE shoes_video_stop(VALUE);
//VALUE shoes_video_prev(VALUE);
//VALUE shoes_video_next(VALUE);
VALUE shoes_video_get_length(VALUE);
VALUE shoes_video_get_time(VALUE);
VALUE shoes_video_set_time(VALUE, VALUE);
VALUE shoes_video_get_position(VALUE);
VALUE shoes_video_set_position(VALUE, VALUE);
VALUE shoes_video_get_path(VALUE);
VALUE shoes_video_set_path(VALUE, VALUE);
VALUE shoes_video_get_volume(VALUE);
VALUE shoes_video_set_volume(VALUE, VALUE);

#else
#define SHOES_VIDEO 0
#endif

#endif	/* VIDEO_H */

