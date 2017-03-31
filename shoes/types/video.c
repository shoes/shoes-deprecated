#include "shoes/types/video.h"
#include "shoes/types/native.h"
#include "shoes/native/native.h"
#include "shoes/ruby.h"

VALUE cVideo, eVlcError;

FUNC_M("+video", video, -1);

#ifdef VIDEO_C_VLC
PLACE_COMMON(video)
CLASS_COMMON(video)
#endif

void shoes_video_init() {
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
    rb_define_method(cVideo, "tooltip", CASTHOOK(shoes_control_get_tooltip), 0);
    rb_define_method(cVideo, "tooltip=", CASTHOOK(shoes_control_set_tooltip), 1);

    rb_const_set(cTypes, rb_intern("VIDEO"),  Qtrue);

    eVlcError = rb_define_class_under(cTypes, "VideoError", rb_eStandardError);

    RUBY_M("+video_c", video, -1);
}

// ruby
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

VALUE shoes_video_new(VALUE attr, VALUE parent) {
    shoes_video *video;
    VALUE obj = shoes_video_alloc(cVideo);
    Data_Get_Struct(obj, shoes_video, video);

    if (NIL_P(attr)) attr = rb_hash_new();
    video->attr = attr;
    video->parent = shoes_find_canvas(parent);

    /* getting surface dimensions, first try at video widget, then parent canvas, then video track size */
    // TODO: this needs review to make sure it does what was intended
    shoes_canvas *canvas;
    Data_Get_Struct(video->parent, shoes_canvas, canvas);
    if ( !RTEST(ATTR(attr, width)) ) {
        if ( RTEST(ATTR(canvas->attr, width)) ) {
            ATTRSET(attr, width, ATTR(canvas->attr, width));
        } else {
            ATTRSET(attr, width, RTEST(rb_hash_aref(attr, ID2SYM(rb_intern("video_width")))) ?
                    rb_hash_aref(attr, ID2SYM(rb_intern("video_width"))) : INT2NUM(0));
        }
    }
    if ( !RTEST(ATTR(attr, height)) ) {
        if ( RTEST(ATTR(canvas->attr, height)) ) {
            ATTRSET(attr, height, ATTR(canvas->attr, height));
        } else {
            if (RTEST(rb_hash_aref(attr, ID2SYM(rb_intern("video_height"))))) {
                ATTRSET(attr, height, rb_hash_aref(attr, ID2SYM(rb_intern("video_height"))));
            } else ATTRSET(attr, height, INT2NUM(0));
            /* No dimensions provided, using the video track size, make info avalaible to Shoes */
            rb_hash_aset(attr, ID2SYM(rb_intern("using_video_dim")), Qtrue);
        }
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
    return ULONG2NUM((unsigned long)GDK_WINDOW_HWND(gtk_widget_get_window(self_t->ref)));
#else
#ifdef SHOES_QUARTZ
    return ULONG2NUM((unsigned long)self_t->ref);
#else
    return UINT2NUM(GDK_WINDOW_XID(gtk_widget_get_window(self_t->ref)));
#endif
#endif
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
    return self;
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
    return self;
}

VALUE shoes_video_remove(VALUE self) {
    GET_STRUCT(video, self_t);
    shoes_canvas *canvas;
    Data_Get_Struct(self_t->parent, shoes_canvas, canvas);

    rb_ary_delete(canvas->contents, self);
#ifdef SHOES_QUARTZ
    shoes_native_surface_remove((CGrafPtr)self_t->ref);
#else
    shoes_native_surface_remove(self_t->ref);
#endif
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
        case 2:
            return rb_obj_freeze(rb_obj_dup(self_t->attr));
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

/* internal method used in fiddle-video protocol
*  letting Shoes know when drawable is avalaible
*/
VALUE shoes_video_get_realized(VALUE self) {
    shoes_video *self_t;
    Data_Get_Struct(self, shoes_video, self_t);

    return self_t->realized ? Qtrue : Qfalse;
}

// canvas
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