
#include "video.h"
#include "shoes/native.h"

/* libvlc_exception_t removed since vlc 1.1.0
 * static void shoes_vlc_exception(libvlc_exception_t *excp)
*/

void
shoes_video_mark(shoes_video *video)
{
  rb_gc_mark_maybe(video->path);
  rb_gc_mark_maybe(video->parent);
  rb_gc_mark_maybe(video->attr);
}

static void
shoes_video_free(shoes_video *video)
{
  if (video->mediaList != NULL)
    libvlc_media_list_release(video->mediaList);
  if (video->vlcListplayer != NULL)
    libvlc_media_list_player_release(video->vlcListplayer);
  if (video->vlcplayer != NULL)
    libvlc_media_player_release(video->vlcplayer);
  RUBY_CRITICAL(SHOE_FREE(video));
  /* we keep SHOES_VLC(video) one vlc instance for the whole shoes session */
}

VALUE
shoes_video_alloc(VALUE klass)
{
  VALUE obj;
  const char *ppsz_argv[10] = {"vlc", "-I", "dummy", "--quiet", "--no-stats",
                        "--no-overlay", "--no-video-on-top", NULL, NULL, NULL};
 #ifndef SHOES_GTK
  char pathsw[SHOES_BUFSIZE];
  int ppsz_argc = 8;
  sprintf(pathsw, "--plugin-path=%s/plugins", shoes_world->path);
  ppsz_argv[7] = pathsw;
#else
  int ppsz_argc = 7;
#endif
  ppsz_argv[ppsz_argc++] = "--ignore-config";
  ppsz_argv[ppsz_argc++] = "--no-video-title-show"; 
  
  shoes_video *video = SHOE_ALLOC(shoes_video);
  SHOE_MEMZERO(video, shoes_video, 1);

  obj = Data_Wrap_Struct(klass, shoes_video_mark, shoes_video_free, video);
  
  if (SHOES_VLC(video) == NULL)
    SHOES_VLC(video) = libvlc_new(0, NULL);  //as per vlc doc
    //SHOES_VLC(video) = libvlc_new(ppsz_argc, ppsz_argv); 
  
  video->path = Qnil;
  video->attr = Qnil;
  video->parent = Qnil;
  return obj;
}

VALUE
shoes_video_new(VALUE path, VALUE attr, VALUE parent)
{
  shoes_video *video;
  VALUE obj = shoes_video_alloc(cVideo);
  Data_Get_Struct(obj, shoes_video, video);

  video->path = path;
  video->attr = attr;
  video->parent = parent;
  return obj;
}

VALUE
shoes_video_remove(VALUE self)
{
  shoes_canvas *canvas;
  GET_STRUCT(video, self_t);
  shoes_canvas_remove_item(self_t->parent, self, 1, 0);

  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_native_surface_remove(canvas, self_t->ref);
  return self;
}

VALUE
shoes_video_hide(VALUE self)
{
  GET_STRUCT(video, self_t);
  ATTRSET(self_t->attr, hidden, Qtrue);
  //shoes_native_surface_hide(self_t->ref);
  shoes_native_control_hide(self_t->ref);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE
shoes_video_show(VALUE self)
{
  GET_STRUCT(video, self_t);
  ATTRSET(self_t->attr, hidden, Qfalse);
  //shoes_native_surface_show(self_t->ref);
  shoes_native_control_show(self_t->ref);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE
shoes_video_is_playing(VALUE self)
{
  GET_STRUCT(video, self_t);
  if (self_t->init == 1) {
    return libvlc_media_list_player_is_playing(self_t->vlcListplayer) ? Qtrue : Qfalse;
  }
  return Qfalse;
}

VALUE
shoes_video_play(VALUE self)
{
  GET_STRUCT(video, self_t);
  if (self_t->init == 1) {
    libvlc_media_list_player_play(self_t->vlcListplayer);
  }
  return self;
}

#define VIDEO_METHOD(x) \
  VALUE \
  shoes_video_##x(VALUE self) \
  { \
    GET_STRUCT(video, self_t); \
    if (self_t->init == 1) \
    { \
      shoes_libvlc_##x(self_t->vlcListplayer); \
    } \
    return self; \
  }

#define VIDEO_GET_METHOD(x, ctype, rbtype) \
  VALUE shoes_video_get_##x(VALUE self) \
  { \
    GET_STRUCT(video, self_t); \
    if (self_t->init == 1) \
    { \
      ctype len = libvlc_media_player_get_##x(self_t->vlcplayer); \
      return rbtype(len); \
    } \
    return Qnil; \
  }

#define VIDEO_SET_METHOD(x, rbconv) \
  VALUE shoes_video_set_##x(VALUE self, VALUE val) \
  { \
    GET_STRUCT(video, self_t); \
    if (self_t->init == 1) \
    { \
      libvlc_media_player_set_##x(self_t->vlcplayer, rbconv(val)); \
    } \
    return val; \
  }

VIDEO_METHOD(pause);
VIDEO_METHOD(stop);
//VIDEO_METHOD(prev);
//VIDEO_METHOD(next);

VIDEO_GET_METHOD(length, vlc_int64_t, INT2NUM);
VIDEO_GET_METHOD(time, vlc_int64_t, INT2NUM);
VIDEO_SET_METHOD(time, NUM2INT);
VIDEO_GET_METHOD(position, float, rb_float_new);
VIDEO_SET_METHOD(position, NUM2DBL);


VALUE
shoes_video_get_path(VALUE self)
{
  GET_STRUCT(video, self_t);
  return self_t->path;
}

VALUE
shoes_video_set_path(VALUE self, VALUE path)
{
  GET_STRUCT(video, self_t);
  if(shoes_video_is_playing(self)) shoes_video_stop(self);
  
  self_t->path = path;
  self_t->init = 0;
  shoes_canvas_repaint_all(self_t->parent);
  return path;
}

VALUE
shoes_video_get_volume(VALUE self)
{
  GET_STRUCT(video, self_t);
  int vol = -1;
  if (self_t->init == 1) {
    vol = libvlc_audio_get_volume(self_t->vlcplayer);
  }
  return INT2NUM(vol);
}

VALUE
shoes_video_set_volume(VALUE self , VALUE volume)
{
  GET_STRUCT(video, self_t);
  int done = -1;
  if (self_t->init == 1) {
    done = libvlc_audio_set_volume(self_t->vlcplayer, NUM2INT(volume));
  }
  return INT2NUM(done);
}


#define FINISH() \
  if (!ABSY(place)) { \
    canvas->cx += place.w; \
    canvas->cy = place.y; \
    canvas->endx = canvas->cx; \
    canvas->endy = max(canvas->endy, place.y + place.h); \
  } \
  if (ck == cStack) { \
    canvas->cx = CPX(canvas); \
    canvas->cy = canvas->endy; \
  }

static void  /* Private */
init_list_player(shoes_video *video)
{
  // using a media_list_player for obviously having media_list features but also
  // gives us the ability to read videos on youtube like sites (lua script, subitems on media)
  if (!video->vlcListplayer) {
    video->vlcListplayer = libvlc_media_list_player_new(SHOES_VLC(video));
    libvlc_media_list_player_set_media_player(video->vlcListplayer, video->vlcplayer);
    
    video->mediaList = libvlc_media_list_new(SHOES_VLC(video));
    libvlc_media_list_player_set_media_list(video->vlcListplayer, video->mediaList);
  }
  
  libvlc_media_list_lock(video->mediaList);
    if (libvlc_media_list_count(video->mediaList) != 0) 
      libvlc_media_list_remove_index(video->mediaList, 0);
    libvlc_media_list_add_media(video->mediaList, video->media);
    //printf("medialist count : %d\n", libvlc_media_list_count(video->mediaList));
  libvlc_media_list_unlock(video->mediaList);
  
}

static int  /* Private */
load_media(shoes_video *self_t)
{
  libvlc_media_t *media;
  libvlc_media_track_t **tracks;
  unsigned n_tracks, video_width, video_heigth;
  int i;
  
    /*  TODO  while waiting for vlc 3 ...
     *  libvlc_media_type_t media_type = libvlc_media_get_type(media);
     *  if (media_type == libvlc_media_type_file) */
  if(strstr(RSTRING_PTR(self_t->path), "://") != NULL) {          /* uri */
    media = libvlc_media_new_location(shoes_world->vlc, RSTRING_PTR(self_t->path));
    self_t->media = media;
    return 1;               /* don't try to fetch video dimensions TODO ?*/
  } else if (strcmp(RSTRING_PTR(self_t->path), "") == 0 ||  !RTEST(self_t->path)) {
    self_t->media = libvlc_media_new_path(shoes_world->vlc, "");  /* dummy path */
                                       /* fall back to default video dimentions */
    return 0;
  } else {                                                        /* file */
    media = libvlc_media_new_path(shoes_world->vlc, RSTRING_PTR(self_t->path));
    self_t->media = media;
  }
  
  if (media) {
    // wether user supplied width and/or height attributes
    VALUE uw = ATTR(self_t->attr, width), uh = ATTR(self_t->attr, height);
    
    if ( NIL_P(uw) && NIL_P(uh) ) {  /* no attributes at all, check video track dimension */
      libvlc_media_parse(media);
      
      n_tracks = libvlc_media_tracks_get(media, &tracks);
      for (i = 0; i < n_tracks; i++) {
        libvlc_media_track_t *track = *tracks+i;
        if (track->i_type == libvlc_track_video) {
          video_width = track->video->i_width;
          video_heigth = track->video->i_height;
        }
      }
      libvlc_media_tracks_release(tracks, n_tracks);

      ATTRSET(self_t->attr, width, UINT2NUM(video_width));
      ATTRSET(self_t->attr, height, UINT2NUM(video_heigth));
    }
    return 1;
  }

  return 0;
}

VALUE
shoes_video_draw(VALUE self, VALUE c, VALUE actual)
{
  shoes_video *self_t;
  shoes_place place;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_video, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);
  
  /*  Placements are calculated in two passes (request + allocation ?), 
   *  first pass "actual" is false, second it's true.
   *  Here we calculate the dimensions of the video track in order to decide 
   *  how we are going to fill the drawing area (self_t->ref) with the vlc pixels,
   *  we need that info on first pass to feed "shoes_place_decide" where placement 
   *  calculations are done
   */ 
  if ( !RTEST(actual) &&      /* do this only once at first pass */
        self_t->init == 0 ) { /* and only when loading a media (threading issues at redraw events !!) */
    
    int loaded = load_media(self_t);
    
    if (!loaded) {/*TODO handle error*/ /* i couldn't crash vlc whatever the file i tried to load */
      printf("no media loaded");
    }
  }
  
  shoes_place_decide(&place, c, self_t->attr, 500, 400, REL_CANVAS, TRUE);
  VALUE ck = rb_obj_class(c); // flow vs stack management in FINISH macro
  
  
  if (RTEST(actual)) {    /*second pass*/
    
    // not sure what's the use of this, aren't we sure canvas->slot->oscanvas 
    // allways have a window ?, Symmetry with osx, Windows ?
    if (HAS_DRAWABLE(canvas->slot)) {
      if (self_t->init == 0) {
        self_t->init = 1;
        
        if (!self_t->vlcplayer)
          self_t->vlcplayer = libvlc_media_player_new(SHOES_VLC(self_t));
        libvlc_media_player_set_media(self_t->vlcplayer, self_t->media);
        libvlc_media_release(self_t->media);
        
        if (!self_t->ref)
          self_t->ref = shoes_native_surface_new(canvas, self_t, &self_t->place);
        shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        
        if (libvlc_media_player_get_xwindow(self_t->vlcplayer) == 0)
          libvlc_media_player_set_xwindow(self_t->vlcplayer, DRAWABLE(self_t->ref));
        // libvlc_media_player_set_hwnd       on Windows
        // libvlc_media_player_set_nsobject   on osx

/* from old api, still valid ? */
//#ifdef SHOES_QUARTZ
//        libvlc_rectangle_t view, clip;
//        view.top = -(self_t->place.iy + self_t->place.dy);
//        view.left = -(self_t->place.ix + self_t->place.dx);
//        view.right = view.left + self_t->place.iw;
//        view.bottom = view.top + self_t->place.ih;
//        clip.top = self_t->place.iy + self_t->place.dy;
//        clip.left = self_t->place.ix + self_t->place.dx;
//        clip.bottom = clip.top + self_t->place.ih; 
//        clip.right = clip.left + self_t->place.iw; 
//        libvlc_video_set_viewport(self_t->vlcplayer, &view, &clip, NULL);
//#endif
        
        int vol = ATTR2(int, self_t->attr, volume, 80);
        libvlc_audio_set_volume(self_t->vlcplayer, vol);
        
        init_list_player(self_t);
        
        if (RTEST(ATTR(self_t->attr, autoplay))) {
            libvlc_media_list_player_play(self_t->vlcListplayer);
        }

      } else {
#ifndef SHOES_QUARTZ
        shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
#endif
      }
    }
  }

  FINISH();
  return self;
}

