//
// shoes/native-cocoa.m
// ObjC Cocoa-specific code for Shoes.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"
#import "shoes/native/cocoa.h"

#define HEIGHT_PAD 10

@implementation ShoesView
- (id)initWithFrame: (NSRect)frame
{
  return [super initWithFrame: frame];
}
- (void)drawRect: (NSRect)rect
{
}
@end

stvoid shoes_native_init()
{
}

void shoes_native_cleanup(shoes_world_t *world)
{
}

void shoes_native_quit()
{
}

void shoes_native_slot_mark(SHOES_SLOT_OS *slot)
{
}

void shoes_native_slot_reset(SHOES_SLOT_OS *slot)
{
}

void shoes_native_slot_clear(SHOES_SLOT_OS *slot)
{
}

void shoes_native_slot_paint(SHOES_SLOT_OS *slot)
{
}

void shoes_native_slot_lengthen(SHOES_SLOT_OS *slot, int height, int endy)
{
}

void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot)
{
}

int shoes_native_slot_gutter(SHOES_SLOT_OS *slot)
{
  return 0;
}

void shoes_native_remove_item(SHOES_SLOT_OS *slot, VALUE item, char c)
{
}

void 
shoes_app_quartz_install()
{
}

shoes_code
shoes_app_cursor(shoes_app *app, ID cursor)
{
done:
  return SHOES_OK;
}

void
shoes_native_app_resized(shoes_app *app)
{
}

void
shoes_native_app_title(shoes_app *app, char *msg)
{
}

shoes_code
shoes_native_app_open(shoes_app *app, char *path, int dialog)
{
  shoes_code code = SHOES_OK;
quit:
  return code;
}

void
shoes_native_app_show(shoes_app *app)
{
}

void
shoes_native_loop()
{
}

void
shoes_native_app_close(shoes_app *app)
{
}

void
shoes_browser_open(char *url)
{
  VALUE browser = rb_str_new2("open ");
  rb_str_cat2(browser, url);
  shoes_sys(RSTRING_PTR(browser), 1);
}

void
shoes_slot_init(VALUE c, SHOES_SLOT_OS *parent, int x, int y, int width, int height, int scrolls, int toplevel)
{
}

cairo_t *
shoes_cairo_create(shoes_canvas *canvas)
{
  return NULL;
}

void shoes_cairo_destroy(shoes_canvas *canvas)
{
}

void
shoes_group_clear(SHOES_GROUP_OS *group)
{
}

void
shoes_native_canvas_place(shoes_canvas *self_t, shoes_canvas *pc)
{
}

void
shoes_native_canvas_resize(shoes_canvas *canvas)
{
}

void
shoes_native_control_hide(SHOES_CONTROL_REF ref)
{
}

void
shoes_native_control_show(SHOES_CONTROL_REF ref)
{
}

void
shoes_native_control_position(SHOES_CONTROL_REF ref, shoes_place *p1, VALUE self,
  shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
}

void
shoes_native_control_repaint(SHOES_CONTROL_REF ref, shoes_place *p1,
  shoes_canvas *canvas, shoes_place *p2)
{
  if (CHANGED_COORDS()) {
    PLACE_COORDS();
  }
}

void
shoes_native_control_focus(SHOES_CONTROL_REF ref)
{
}

void
shoes_native_control_remove(SHOES_CONTROL_REF ref, shoes_canvas *canvas)
{
}

void
shoes_native_control_free(SHOES_CONTROL_REF ref)
{
}

SHOES_CONTROL_REF
shoes_native_surface_new(shoes_canvas *canvas, VALUE self, shoes_place *place)
{
  return NULL;
}

void
shoes_native_surface_position(SHOES_CONTROL_REF ref, shoes_place *p1, 
  VALUE self, shoes_canvas *canvas, shoes_place *p2)
{
  PLACE_COORDS();
}

void
shoes_native_surface_remove(shoes_canvas *canvas, SHOES_CONTROL_REF ref)
{
}

SHOES_CONTROL_REF
shoes_native_button(VALUE self, shoes_canvas *canvas, shoes_place *place, char *msg)
{
  return NULL;
}

SHOES_CONTROL_REF
shoes_native_edit_line(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

VALUE
shoes_native_edit_line_get_text(SHOES_CONTROL_REF ref)
{
  return Qnil;
}

void
shoes_native_edit_line_set_text(SHOES_CONTROL_REF ref, char *msg)
{
}

SHOES_CONTROL_REF
shoes_native_edit_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

VALUE
shoes_native_edit_box_get_text(SHOES_CONTROL_REF ref)
{
  return Qnil;
}

void
shoes_native_edit_box_set_text(SHOES_CONTROL_REF ref, char *msg)
{
}

SHOES_CONTROL_REF
shoes_native_list_box(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

void
shoes_native_list_box_update(SHOES_CONTROL_REF ref, VALUE ary)
{
}

VALUE
shoes_native_list_box_get_active(SHOES_CONTROL_REF ref, VALUE items)
{
  return Qnil;
}

void
shoes_native_list_box_set_active(SHOES_CONTROL_REF box, VALUE ary, VALUE item)
{
}

SHOES_CONTROL_REF
shoes_native_progress(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

double
shoes_native_progress_get_fraction(SHOES_CONTROL_REF ref)
{
  return 0.0;
}

void
shoes_native_progress_set_fraction(SHOES_CONTROL_REF ref, double perc)
{
}

SHOES_CONTROL_REF
shoes_native_check(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

VALUE
shoes_native_check_get(SHOES_CONTROL_REF ref)
{
  return Qfalse;
}

void
shoes_native_check_set(SHOES_CONTROL_REF ref, int on)
{
}

SHOES_CONTROL_REF
shoes_native_radio(VALUE self, shoes_canvas *canvas, shoes_place *place, VALUE attr, char *msg)
{
  return NULL;
}

void
shoes_native_timer_remove(shoes_canvas *canvas, SHOES_TIMER_REF ref)
{
}

SHOES_TIMER_REF
shoes_native_timer_start(VALUE self, shoes_canvas *canvas, unsigned int interval)
{
  return NULL;
}

VALUE
shoes_native_clipboard_get(shoes_app *app)
{
  return Qnil;
}

void
shoes_native_clipboard_set(shoes_app *app, VALUE string)
{
}

VALUE
shoes_native_window_color(shoes_app *app)
{
  return Qnil;
}

VALUE
shoes_native_dialog_color(shoes_app *app)
{
  return Qnil;
}

VALUE
shoes_dialog_alert(VALUE self, VALUE msg)
{
  return Qnil;
}

VALUE
shoes_dialog_ask(VALUE self, VALUE quiz)
{
  return Qnil;
}

VALUE
shoes_dialog_confirm(VALUE self, VALUE quiz)
{
  return Qnil;
}

VALUE
shoes_dialog_color(VALUE self, VALUE title)
{
  return Qnil;
}

VALUE
shoes_dialog_open(VALUE self)
{
  return Qnil;
}

VALUE
shoes_dialog_save(VALUE self)
{
  return Qnil;
}
