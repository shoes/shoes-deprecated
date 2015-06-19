
#include "video.h"

/* libvlc_exception_t removed since vlc 1.1.0
 * 
 * static void shoes_vlc_exception(libvlc_exception_t *excp)
 * {
 *   if (libvlc_exception_raised(excp)) 
 *     shoes_error("from VLC: %s", libvlc_exception_get_message(excp)); 
 * }
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
  if (video->vlc != NULL)
    libvlc_destroy(video->vlc);
  RUBY_CRITICAL(SHOE_FREE(video));
}

VALUE
shoes_video_alloc(VALUE klass)
{
  VALUE obj;
  const char *ppsz_argv[10] = {"vlc", "-I", "dummy", "--quiet", "--no-stats",
    "--no-overlay", "--no-video-on-top", NULL, NULL, NULL};
  shoes_video *video = SHOE_ALLOC(shoes_video);
  SHOE_MEMZERO(video, shoes_video, 1);
  
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

  obj = Data_Wrap_Struct(klass, shoes_video_mark, shoes_video_free, video);
  
  if (SHOES_VLC(video) == NULL)
    SHOES_VLC(video) = libvlc_new(0, NULL);
    //SHOES_VLC(video) = libvlc_new(ppsz_argc, ppsz_argv); //as per vlc doc
  
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
shoes_video_remove(VALUE self)
{
  shoes_canvas *canvas;
  GET_STRUCT(video, self_t);
  shoes_canvas_remove_item(self_t->parent, self, 1, 0);

  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);
  shoes_native_surface_remove(canvas, self_t->ref);
  return self;
}

#define SETUP(self_type, rel, dw, dh) \
  self_type *self_t; \
  shoes_place place; \
  shoes_canvas *canvas; \
  Data_Get_Struct(self, self_type, self_t); \
  Data_Get_Struct(c, shoes_canvas, canvas); \
  if (ATTR(self_t->attr, hidden) == Qtrue) return self; \
  shoes_place_decide(&place, c, self_t->attr, dw, dh, rel, REL_COORDS(rel) == REL_CANVAS)

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

VALUE
shoes_video_draw(VALUE self, VALUE c, VALUE actual)
{
  SETUP(shoes_video, REL_CANVAS, 400, 300);
  VALUE ck = rb_obj_class(c);

  if (RTEST(actual)) {
    if (HAS_DRAWABLE(canvas->slot)) {
      if (self_t->init == 0) {
        self_t->init = 1;
        
        libvlc_media_t *media;
          // while waiting for vlc 3 ...
          // libvlc_media_type_t media_type = libvlc_media_get_type(media);
          // if (media_type == libvlc_media_type_file)
        if(strstr(RSTRING_PTR(self_t->path), "://") != NULL) {  /* a uri */
          media = libvlc_media_new_location(shoes_world->vlc, RSTRING_PTR(self_t->path));
        } else {                                                /* a file */
          media = libvlc_media_new_path(shoes_world->vlc, RSTRING_PTR(self_t->path));
        }
        //printf("media : %s\n", libvlc_media_get_mrl(media));
        
        self_t->vlc = libvlc_media_player_new_from_media(media);
        
        if (self_t->ref)
          shoes_native_surface_remove(canvas, self_t->ref);
        self_t->ref = shoes_native_surface_new(canvas, self, &self_t->place);
        shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);
        
        if (libvlc_media_player_get_xwindow(self_t->vlc) == 0)
          libvlc_media_player_set_xwindow(self_t->vlc, DRAWABLE(self_t->ref));
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
//        libvlc_video_set_viewport(self_t->vlc, &view, &clip, NULL);
//#endif
        
        int vol = ATTR2(int, self_t->attr, volume, 80);
        libvlc_audio_set_volume(self_t->vlc, vol);
        
        if (RTEST(ATTR(self_t->attr, autoplay))) {
          INFO("Starting playlist.\n");
          libvlc_media_player_play(self_t->vlc);
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

VALUE
shoes_video_is_playing(VALUE self)
{
  GET_STRUCT(video, self_t);
  if (self_t->init == 1) {
    libvlc_state_t s = libvlc_media_player_get_state(self_t->vlc);
    return s == libvlc_Playing ? Qtrue : Qfalse;
  }
  return Qfalse;
}

VALUE
shoes_video_play(VALUE self)
{
  GET_STRUCT(video, self_t);
  if (self_t->init == 1) {
    libvlc_media_player_play(self_t->vlc);
  }
  return self;
}

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
  if(shoes_video_is_playing(self)) libvlc_media_player_stop(self_t->vlc);
  
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
    vol = libvlc_audio_get_volume(self_t->vlc);
  }
  return INT2NUM(vol);
}

VALUE
shoes_video_set_volume(VALUE self , VALUE volume)
{
  GET_STRUCT(video, self_t);
  int done = -1;
  if (self_t->init == 1) {
    done = libvlc_audio_set_volume(self_t->vlc, NUM2INT(volume));
  }
  return INT2NUM(done);
}

#define VIDEO_METHOD(x) \
  VALUE \
  shoes_video_##x(VALUE self) \
  { \
    GET_STRUCT(video, self_t); \
    if (self_t->init == 1) \
    { \
      shoes_libvlc_##x(self_t->vlc); \
    } \
    return self; \
  }

#define VIDEO_GET_METHOD(x, ctype, rbtype) \
  VALUE shoes_video_get_##x(VALUE self) \
  { \
    GET_STRUCT(video, self_t); \
    if (self_t->init == 1) \
    { \
      ctype len = libvlc_media_player_get_##x(self_t->vlc); \
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
      libvlc_media_player_set_##x(self_t->vlc, rbconv(val)); \
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


