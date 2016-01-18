
#include "shoes/world.h"
#include "shoes/internal.h"
#include "shoes/app.h"


#ifndef VIDEO_H
#define	VIDEO_H

#ifdef VIDEO

#define SHOES_VIDEO 1

#include <vlc/vlc.h>       
#include <vlc/libvlc.h>

#define vlc_int64_t     libvlc_time_t
// TODO
//#define shoes_libvlc_prev  libvlc_media_list_player_previous
//#define shoes_libvlc_next  libvlc_media_list_player_next 	
#define shoes_libvlc_pause libvlc_media_list_player_pause
#define shoes_libvlc_stop  libvlc_media_list_player_stop
#define SHOES_VLC(self_t) shoes_world->vlc


typedef struct {
  VALUE parent;
  VALUE attr;
  shoes_place place;
  SHOES_CONTROL_REF ref;
  //libvlc_exception_t excp; removed since vlc 1.1.0
  libvlc_media_player_t *vlcplayer;
  libvlc_media_list_player_t *vlcListplayer;
  libvlc_media_t *media;
  libvlc_media_list_t *mediaList;
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

