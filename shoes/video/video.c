#include "video.h"
#include "shoes/native.h"
#include "shoes/ruby.h"

// from ruby's eval.c
//
static inline VALUE
call_cfunc(HOOK func, VALUE recv, int len, int argc, VALUE *argv)
{
  if (len >= 0 && argc != len) {
    rb_raise(rb_eArgError, "wrong number of arguments (%d for %d)",
      argc, len);
  }

  switch (len) {
    case -2:
    return (*func)(recv, rb_ary_new4(argc, argv));
    case -1:
    return (*func)(argc, argv, recv);
    case 0:
    return (*func)(recv);
    case 1:
    return (*func)(recv, argv[0]);
    case 2:
    return (*func)(recv, argv[0], argv[1]);
    case 3:
    return (*func)(recv, argv[0], argv[1], argv[2]);
    case 4:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3]);
    case 5:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4]);
    case 6:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5]);
    case 7:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6]);
    case 8:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7]);
    case 9:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8]);
    case 10:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9]);
    case 11:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9], argv[10]);
    case 12:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9],
               argv[10], argv[11]);
    case 13:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
               argv[11], argv[12]);
    case 14:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
               argv[11], argv[12], argv[13]);
    case 15:
    return (*func)(recv, argv[0], argv[1], argv[2], argv[3], argv[4],
               argv[5], argv[6], argv[7], argv[8], argv[9], argv[10],
               argv[11], argv[12], argv[13], argv[14]);
    default:
    rb_raise(rb_eArgError, "too many arguments (%d)", len);
    break;
  }
  return Qnil;        /* not reached */
}

/* from canvas.c */
VALUE shoes_canvas_video(int argc, VALUE *argv, VALUE self) {
  rb_arg_list args;
  rb_parse_args(argc, argv, "|h", &args);
  VALUE video = shoes_video_new(args.a[0], self);

  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_canvas, canvas);
  cairo_t *cr;
  cr = CCR(canvas);
  if (canvas->insertion <= -1)
    rb_ary_push(canvas->contents, video);
  else {
    rb_ary_insert_at(canvas->contents, canvas->insertion, 0, video);
    canvas->insertion++;
  }
  return video;
}

void shoes_video_mark(shoes_video *video) {
  rb_gc_mark_maybe(video->parent);
  rb_gc_mark_maybe(video->attr);
}


static void shoes_video_free(shoes_video *video) {
  RUBY_CRITICAL(SHOE_FREE(video));
}

VALUE shoes_video_alloc(VALUE klass) {
  shoes_video *video = SHOE_ALLOC(shoes_video);
  SHOE_MEMZERO(video, shoes_video, 1);

  VALUE obj = Data_Wrap_Struct(klass, shoes_video_mark, shoes_video_free, video);
  video->attr = Qnil;
  video->parent = Qnil;
  video->realized = 0;
  return obj;
}

VALUE shoes_video_new(VALUE attr, VALUE parent)
{
  shoes_video *video;
  VALUE obj = shoes_video_alloc(cVideo);
  Data_Get_Struct(obj, shoes_video, video);

  if (NIL_P(attr)) attr = rb_hash_new();
  video->attr = attr;
  video->parent = shoes_find_canvas(parent);
  
  /* getting surface dimensions, first try at video widget, then parent canvas, then video track size */
  shoes_canvas *canvas;
  Data_Get_Struct(video->parent, shoes_canvas, canvas);
  if ( !RTEST(ATTR(attr, width)) )
    if ( RTEST(ATTR(canvas->attr, width)) ) ATTRSET(attr, width, ATTR(canvas->attr, width));
    else {
      ATTRSET(attr, width, RTEST(rb_hash_aref(attr, ID2SYM(rb_intern("video_width")))) ? 
                      rb_hash_aref(attr, ID2SYM(rb_intern("video_width"))) : INT2NUM(0));
    }
  if ( !RTEST(ATTR(attr, height)) )
    if ( RTEST(ATTR(canvas->attr, height)) ) ATTRSET(attr, height, ATTR(canvas->attr, height));
    else {
      if (RTEST(rb_hash_aref(attr, ID2SYM(rb_intern("video_height"))))) {
        ATTRSET(attr, height, rb_hash_aref(attr, ID2SYM(rb_intern("video_height"))));
      } else ATTRSET(attr, height, INT2NUM(0));
      /* No dimensions provided, using the video track size, make info avalaible to Shoes */
      rb_hash_aset(attr, ID2SYM(rb_intern("using_video_dim")), Qtrue);
    }
  
  video->ref = shoes_native_surface_new(attr, obj);
  return obj;
}

/*
 * Get the drawable area so vlc can draw on it
 * in ruby side via Fiddle
 */
VALUE shoes_video_get_drawable(VALUE self) {
  shoes_video *self_t;
  Data_Get_Struct(self, shoes_video, self_t);
#ifdef SHOES_GTK_WIN32
  return ULONG2NUM(GDK_WINDOW_HWND(gtk_widget_get_window(self_t->ref)));
#else
#ifdef SHOES_QUARTZ
  return ULONG2NUM(self_t->ref);
#else
  return UINT2NUM(GDK_WINDOW_XID(gtk_widget_get_window(self_t->ref)));
#endif
#endif
}

/* internal method used in fiddle-video protocol 
*  letting Shoes know when drawable is avalaible 
*/ 
VALUE
shoes_video_get_realized(VALUE self) {
  shoes_video *self_t;
  Data_Get_Struct(self, shoes_video, self_t);
  
  return self_t->realized ? Qtrue : Qfalse;
}

/* from ruby.c */
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

VALUE shoes_video_draw(VALUE self, VALUE c, VALUE actual) {
  shoes_video *self_t;
  shoes_place place;
  shoes_canvas *canvas;
  Data_Get_Struct(self, shoes_video, self_t);
  Data_Get_Struct(c, shoes_canvas, canvas);

  shoes_place_decide(&place, c, self_t->attr, canvas->place.iw, canvas->place.ih, REL_CANVAS, TRUE);
  VALUE ck = rb_obj_class(c); // flow vs stack management in FINISH macro

  if (RTEST(actual)) {
    if (self_t->init == 0) {
      self_t->init = 1;

      if (!self_t->ref)
        self_t->ref = shoes_native_surface_new(self_t->attr, self);
      shoes_native_control_position(self_t->ref, &self_t->place, self, canvas, &place);

    } else {
      shoes_native_control_repaint(self_t->ref, &self_t->place, canvas, &place);
    }
  }
  FINISH()
}


VALUE shoes_video_get_parent(VALUE self) {
  GET_STRUCT(video, self_t);
  return self_t->parent;
}

VALUE shoes_video_get_left(VALUE self) {
  GET_STRUCT(video, self_t);
  return INT2NUM(self_t->place.ix + self_t->place.dx);
}

VALUE shoes_video_get_top(VALUE self) {
  GET_STRUCT(video, self_t);
  return INT2NUM(self_t->place.iy + self_t->place.dy);
}

VALUE shoes_video_get_height(VALUE self) {
  GET_STRUCT(video, self_t);
  return INT2NUM(self_t->place.h);
}

VALUE shoes_video_get_width(VALUE self) {
  GET_STRUCT(video, self_t);
  return INT2NUM(self_t->place.w);
}

VALUE shoes_video_show(VALUE self) {
  GET_STRUCT(video, self_t);
  ATTRSET(self_t->attr, hidden, Qfalse);
  shoes_native_control_show(self_t->ref);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE shoes_video_hide(VALUE self) {
  GET_STRUCT(video, self_t);
  ATTRSET(self_t->attr, hidden, Qtrue);
  shoes_native_control_hide(self_t->ref);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE shoes_video_toggle(VALUE self) {
  GET_STRUCT(video, self_t);
  ATTR(self_t->attr, hidden) == Qtrue ?
    shoes_video_show(self) : shoes_video_hide(self);
}

VALUE shoes_video_remove(VALUE self) {
  GET_STRUCT(video, self_t);
  shoes_canvas *canvas;
  Data_Get_Struct(self_t->parent, shoes_canvas, canvas);

  rb_ary_delete(canvas->contents, self);
  shoes_native_surface_remove(self_t->ref);
  self_t->ref = NULL;
  shoes_canvas_repaint_all(self_t->parent);

  self_t = NULL;
  self = Qnil;
  return Qtrue;
}

VALUE shoes_video_style(int argc, VALUE *argv, VALUE self) {
  rb_arg_list args;
  GET_STRUCT(video, self_t);
  switch (rb_parse_args(argc, argv, "h,", &args)) {
    case 1:
      if (NIL_P(self_t->attr)) self_t->attr = rb_hash_new();
      rb_funcall(self_t->attr, s_update, 1, args.a[0]);
      shoes_canvas_repaint_all(self_t->parent);
    break;
    case 2: return rb_obj_freeze(rb_obj_dup(self_t->attr));
  }
  return self;
}

VALUE shoes_video_displace(VALUE self, VALUE x, VALUE y) {
  GET_STRUCT(video, self_t);
  ATTRSET(self_t->attr, displace_left, x);
  ATTRSET(self_t->attr, displace_top, y);
  shoes_canvas_repaint_all(self_t->parent);
  return self;
}

VALUE shoes_video_move(VALUE self, VALUE x, VALUE y) {
    GET_STRUCT(video, self_t);
    ATTRSET(self_t->attr, left, x);
    ATTRSET(self_t->attr, top, y);
    shoes_canvas_repaint_all(self_t->parent);
    return self;
  }

/* from  CANVAS_DEFS(FUNC_M) in ruby.c */
VALUE shoes_canvas_c_video(int argc, VALUE *argv, VALUE self) {
  VALUE canvas, obj;
  GET_STRUCT(canvas, self_t);
  char *n = "+video";
  if (rb_ary_entry(self_t->app->nesting, 0) == self ||
       ((rb_obj_is_kind_of(self, cWidget) || self == self_t->app->nestslot) &&
        RARRAY_LEN(self_t->app->nesting) > 0))
    canvas = rb_ary_entry(self_t->app->nesting, RARRAY_LEN(self_t->app->nesting) - 1);
  else
    canvas = self;
  if (!rb_obj_is_kind_of(canvas, cCanvas))
    return ts_funcall2(canvas, rb_intern(n + 1), argc, argv);
  obj = call_cfunc(CASTHOOK(shoes_canvas_video), canvas, -1, argc, argv);
  if (n[0] == '+' && RARRAY_LEN(self_t->app->nesting) == 0) shoes_canvas_repaint_all(self);
  return obj;
}

VALUE shoes_app_c_video(int argc, VALUE *argv, VALUE self) {
  VALUE canvas;
  char *n = "+video";
  GET_STRUCT(app, app);
  if (RARRAY_LEN(app->nesting) > 0)
    canvas = rb_ary_entry(app->nesting, RARRAY_LEN(app->nesting) - 1);
  else
    canvas = app->canvas;
  if (!rb_obj_is_kind_of(canvas, cCanvas))
    return ts_funcall2(canvas, rb_intern(n + 1), argc, argv);
  return shoes_canvas_c_video(argc, argv, canvas);
}

// called inside shoes_ruby_init, ruby.c
void shoes_ruby_video_init() {

  /* video_c so we can use method 'video' on ruby side */
  rb_define_method(cCanvas, "+video_c" + 1, CASTHOOK(shoes_canvas_c_video), -1); /* from CANVAS_DEFS(RUBY_M) in ruby.c */
  rb_define_method(cApp, "+video_c" + 1, CASTHOOK(shoes_app_c_video), -1);       /**/

  cVideo = rb_define_class_under(cTypes, "Video", rb_cObject);
  rb_define_alloc_func(cVideo, shoes_video_alloc);
  rb_define_method(cVideo, "draw", CASTHOOK(shoes_video_draw), 2);
  rb_define_method(cVideo, "parent", CASTHOOK(shoes_video_get_parent), 0);
  rb_define_method(cVideo, "drawable", CASTHOOK(shoes_video_get_drawable), 0);
  rb_define_method(cVideo, "drawable_ready?", CASTHOOK(shoes_video_get_realized), 0);
  rb_define_method(cVideo, "remove", CASTHOOK(shoes_video_remove), 0);
  rb_define_method(cVideo, "show", CASTHOOK(shoes_video_show), 0);
  rb_define_method(cVideo, "hide", CASTHOOK(shoes_video_hide), 0);
  rb_define_method(cVideo, "toggle", CASTHOOK(shoes_video_toggle), 0);
  rb_define_method(cVideo, "style", CASTHOOK(shoes_video_style), -1);
  rb_define_method(cVideo, "displace", CASTHOOK(shoes_video_displace), 2);
  rb_define_method(cVideo, "move", CASTHOOK(shoes_video_move), 2);
  rb_define_method(cVideo, "width", CASTHOOK(shoes_video_get_width), 0);
  rb_define_method(cVideo, "height", CASTHOOK(shoes_video_get_height), 0);
  rb_define_method(cVideo, "left", CASTHOOK(shoes_video_get_left), 0);
  rb_define_method(cVideo, "top", CASTHOOK(shoes_video_get_top), 0);

}
