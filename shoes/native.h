//
// shoes/native.h
// Common native Shoes routines.
//
#define CHANGED_COORDS() \
  (p1->ix != p2->ix || p1->iy != p2->iy || \
   p1->iw != p2->iw || p1->dx != p2->dx || \
   p1->dy != p2->dy || p1->ih - HEIGHT_PAD != p2->ih)
#define PLACE_COORDS() p2->h -= HEIGHT_PAD; p2->ih -= HEIGHT_PAD; *p1 = *p2
#define PLACE_COORDS_NO_PAD() p2->h -= HEIGHT_PAD; p2->ih -= HEIGHT_PAD; *p1 = *p2

#ifndef SHOES_GTK
#define SHOES_FORCE_RADIO 1
#endif

#define SHOES_THREAD_DOWNLOAD 41
#define SHOES_IMAGE_DOWNLOAD  42
#define SHOES_MAX_MESSAGE     100

VALUE shoes_font_list();
VALUE shoes_load_font(const char *);
void shoes_native_init();
void shoes_native_cleanup(shoes_world_t *world);
void shoes_native_quit();
int shoes_throw_message(unsigned int, VALUE, void *);
void shoes_native_slot_mark(SHOES_SLOT_OS *);
void shoes_native_slot_reset(SHOES_SLOT_OS *);
void shoes_native_slot_clear(shoes_canvas *);
void shoes_native_slot_paint(SHOES_SLOT_OS *);
void shoes_native_slot_lengthen(SHOES_SLOT_OS *, int, int);
void shoes_native_slot_scroll_top(SHOES_SLOT_OS *);
int shoes_native_slot_gutter(SHOES_SLOT_OS *);
void shoes_native_remove_item(SHOES_SLOT_OS *, VALUE, char);
shoes_code shoes_app_cursor(shoes_app *, ID);
void shoes_native_app_resized(shoes_app *);
void shoes_native_app_title(shoes_app *, char *);
shoes_code shoes_native_app_open(shoes_app *, char *, int);
void shoes_native_app_show(shoes_app *);
void shoes_native_loop();
void shoes_native_app_close(shoes_app *);
void shoes_browser_open(char *);
void shoes_slot_init(VALUE, SHOES_SLOT_OS *, int, int, int, int, int, int);
cairo_t *shoes_cairo_create(shoes_canvas *);
void shoes_slot_destroy(shoes_canvas *, shoes_canvas *);
void shoes_cairo_destroy(shoes_canvas *);
void shoes_group_clear(SHOES_GROUP_OS *);
void shoes_native_canvas_place(shoes_canvas *, shoes_canvas *);
void shoes_native_canvas_resize(shoes_canvas *);
void shoes_native_control_hide(SHOES_CONTROL_REF);
void shoes_native_control_show(SHOES_CONTROL_REF);
void shoes_native_control_position(SHOES_CONTROL_REF, shoes_place *, 
  VALUE, shoes_canvas *, shoes_place *);
void shoes_native_control_repaint(SHOES_CONTROL_REF, shoes_place *,
  shoes_canvas *, shoes_place *);
void shoes_native_control_focus(SHOES_CONTROL_REF);
void shoes_native_control_remove(SHOES_CONTROL_REF, shoes_canvas *);
void shoes_native_control_free(SHOES_CONTROL_REF);
SHOES_SURFACE_REF shoes_native_surface_new(shoes_canvas *, VALUE, shoes_place *);
void shoes_native_surface_position(SHOES_SURFACE_REF, shoes_place *, 
  VALUE, shoes_canvas *, shoes_place *);
void shoes_native_surface_hide(SHOES_SURFACE_REF);
void shoes_native_surface_show(SHOES_SURFACE_REF);
void shoes_native_surface_remove(shoes_canvas *, SHOES_SURFACE_REF);
SHOES_CONTROL_REF shoes_native_button(VALUE, shoes_canvas *, shoes_place *, char *);
SHOES_CONTROL_REF shoes_native_edit_line(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF);
void shoes_native_edit_line_set_text(SHOES_CONTROL_REF, char *);
SHOES_CONTROL_REF shoes_native_edit_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_edit_box_get_text(SHOES_CONTROL_REF);
void shoes_native_edit_box_set_text(SHOES_CONTROL_REF, char *);
SHOES_CONTROL_REF shoes_native_list_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
void shoes_native_list_box_update(SHOES_CONTROL_REF, VALUE);
VALUE shoes_native_list_box_get_active(SHOES_CONTROL_REF, VALUE);
SHOES_CONTROL_REF shoes_native_progress(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
double shoes_native_progress_get_fraction(SHOES_CONTROL_REF);
void shoes_native_progress_set_fraction(SHOES_CONTROL_REF, double);
SHOES_CONTROL_REF shoes_native_check(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_check_get(SHOES_CONTROL_REF);
void shoes_native_check_set(SHOES_CONTROL_REF, int);
void shoes_native_list_box_set_active(SHOES_CONTROL_REF, VALUE, VALUE);
SHOES_CONTROL_REF shoes_native_radio(VALUE, shoes_canvas *, shoes_place *, VALUE, VALUE);
void shoes_native_timer_remove(shoes_canvas *, SHOES_TIMER_REF);
SHOES_TIMER_REF shoes_native_timer_start(VALUE, shoes_canvas *, unsigned int);
VALUE shoes_native_clipboard_get(shoes_app *);
void shoes_native_clipboard_set(shoes_app *, VALUE);
VALUE shoes_native_to_s(VALUE);
VALUE shoes_native_window_color(shoes_app *);
VALUE shoes_native_dialog_color(shoes_app *);
VALUE shoes_dialog_alert(VALUE, VALUE);
VALUE shoes_dialog_ask(int argc, VALUE *argv, VALUE self);
VALUE shoes_dialog_confirm(VALUE, VALUE);
VALUE shoes_dialog_color(VALUE, VALUE);
VALUE shoes_dialog_open(VALUE);
VALUE shoes_dialog_save(VALUE);
VALUE shoes_dialog_open_folder(VALUE);
VALUE shoes_dialog_save_folder(VALUE);
