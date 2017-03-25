#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native/native.h"

#ifndef SHOES_IMAGE_TYPE_H
#define SHOES_IMAGE_TYPE_H

/* extern variables necessary to communicate with other parts of Shoes */
extern VALUE cShoes, cApp, cTypes, cCanvas, cWidget;
extern shoes_app _shoes_app;

// native forward declarations
// extern SHOES_CONTROL_REF shoes_native_spinner(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg);
// extern void shoes_native_spinner_start(SHOES_CONTROL_REF ref);
// extern void shoes_native_spinner_stop(SHOES_CONTROL_REF ref);
// extern gboolean shoes_native_spinner_started(SHOES_CONTROL_REF ref);

VALUE cImage;

/* each widget should have its own init function */
void shoes_image_init();

// ruby
void shoes_image_mark(shoes_image *image);
void shoes_image_free(shoes_image *image);
VALUE shoes_image_new(VALUE klass, VALUE path, VALUE attr, VALUE parent, shoes_transform *st);
VALUE shoes_image_alloc(VALUE klass);
VALUE shoes_image_get_full_width(VALUE self);
VALUE shoes_image_get_full_height(VALUE self);
void shoes_image_ensure_dup(shoes_image *image);
unsigned char *shoes_image_surface_get_pixel(shoes_cached_image *cached, int x, int y);
VALUE shoes_image_get_pixel(VALUE self, VALUE _x, VALUE _y);
VALUE shoes_image_set_pixel(VALUE self, VALUE _x, VALUE _y, VALUE col);
VALUE shoes_image_get_path(VALUE self);
VALUE shoes_image_set_path(VALUE self, VALUE path);
VALUE shoes_image_draw(VALUE self, VALUE c, VALUE actual);
void shoes_image_image(VALUE parent, VALUE path, VALUE attr);
VALUE shoes_image_size(VALUE self);
VALUE shoes_image_motion(VALUE self, int x, int y, char *touch);
VALUE shoes_image_send_click(VALUE self, int button, int x, int y);
void shoes_image_send_release(VALUE self, int button, int x, int y);

// canvas
VALUE shoes_canvas_image(int, VALUE *, VALUE);
VALUE shoes_canvas_nostroke(VALUE);
VALUE shoes_canvas_stroke(int, VALUE *, VALUE);
VALUE shoes_canvas_strokewidth(VALUE, VALUE);
VALUE shoes_canvas_dash(VALUE, VALUE);
VALUE shoes_canvas_cap(VALUE, VALUE);
VALUE shoes_canvas_nofill(VALUE);
VALUE shoes_canvas_fill(int, VALUE *, VALUE);
VALUE shoes_canvas_move_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_line_to(VALUE, VALUE, VALUE);
VALUE shoes_canvas_curve_to(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_arc_to(VALUE, VALUE, VALUE, VALUE, VALUE, VALUE, VALUE);
VALUE shoes_canvas_transform(VALUE, VALUE);
VALUE shoes_canvas_translate(VALUE, VALUE, VALUE);
VALUE shoes_canvas_rotate(VALUE, VALUE);
VALUE shoes_canvas_scale(int, VALUE *, VALUE);
VALUE shoes_canvas_skew(int, VALUE *, VALUE);

// canvas (shape related)
VALUE shoes_canvas_rect(int, VALUE *, VALUE);
VALUE shoes_canvas_arc(int, VALUE *, VALUE);
VALUE shoes_canvas_oval(int, VALUE *, VALUE);
VALUE shoes_canvas_line(int, VALUE *, VALUE);
VALUE shoes_canvas_arrow(int, VALUE *, VALUE);
VALUE shoes_canvas_star(int, VALUE *, VALUE);

// ruby (effect related)
VALUE shoes_canvas_blur(int, VALUE *, VALUE);
VALUE shoes_canvas_glow(int, VALUE *, VALUE);
VALUE shoes_canvas_shadow(int, VALUE *, VALUE);


#define SHOES_IMAGE_PLACE(type, imw, imh, surf) \
  SETUP_DRAWING(shoes_##type, (REL_CANVAS | REL_SCALE), imw, imh); \
  VALUE ck = rb_obj_class(c); \
  if (RTEST(actual)) \
    shoes_image_draw_surface(CCR(canvas), self_t, &place, surf, imw, imh); \
  FINISH(); \
  return self;

#endif
