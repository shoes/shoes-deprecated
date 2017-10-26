//
// shoes/native.h
// Common native Shoes routines.
//
#ifndef SHOES_NATIVE_H
#define SHOES_NATIVE_H

#define CHANGED_COORDS() \
  (p1->ix != p2->ix || p1->iy != p2->iy || \
   p1->iw != p2->iw || p1->dx != p2->dx || \
   p1->dy != p2->dy || p1->ih - HEIGHT_PAD != p2->ih)
#define CHANGED_COORDS_NO_PAD() \
  (p1->ix != p2->ix || p1->iy != p2->iy || \
   p1->iw != p2->iw || p1->dx != p2->dx || \
   p1->dy != p2->dy || p1->ih != p2->ih)
#define PLACE_COORDS() p2->h -= HEIGHT_PAD; p2->ih -= HEIGHT_PAD; *p1 = *p2
#define PLACE_COORDS_NO_PAD() *p1 = *p2

#ifndef SHOES_GTK
#define SHOES_FORCE_RADIO 1
#endif

#define SHOES_THREAD_DOWNLOAD 41
#define SHOES_IMAGE_DOWNLOAD  42
#define SHOES_MAX_MESSAGE     100

VALUE shoes_font_list(void);
VALUE shoes_load_font(const char *);
void shoes_native_init(void);
void shoes_native_cleanup(shoes_world_t *world);
void shoes_native_quit(void);
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
void shoes_native_app_fullscreen(shoes_app *, char);
double shoes_native_app_get_opacity(shoes_app *app);
void shoes_native_app_set_opacity(shoes_app *app, double opacity);
void shoes_native_app_set_decoration(shoes_app *app, gboolean decorated);
int shoes_native_app_get_decoration(shoes_app *app);
void shoes_native_app_resize_window(shoes_app *);
VALUE shoes_native_get_resizable(shoes_app *app);
void shoes_native_set_resizable(shoes_app *app, int resizable);
shoes_code shoes_native_app_open(shoes_app *, char *, int);
void shoes_native_app_show(shoes_app *);
void shoes_native_loop(void);
void shoes_native_app_close(shoes_app *);
void shoes_native_app_set_icon(shoes_app *, char *);
void shoes_native_app_set_wtitle(shoes_app *, char*);
int shoes_native_console();  // Yes it's different
int shoes_native_terminal();
void shoes_native_app_console();
void shoes_browser_open(char *);
void shoes_slot_init(VALUE, SHOES_SLOT_OS *, int, int, int, int, int, int);
cairo_t *shoes_cairo_create(shoes_canvas *);
void shoes_slot_destroy(shoes_canvas *, shoes_canvas *);
void shoes_cairo_destroy(shoes_canvas *);
void shoes_group_clear(SHOES_GROUP_OS *);
void shoes_native_canvas_place(shoes_canvas *, shoes_canvas *);
void shoes_native_canvas_resize(shoes_canvas *);

// TODO: Control belongs to types/native
void shoes_native_control_hide(SHOES_CONTROL_REF);
void shoes_native_control_show(SHOES_CONTROL_REF);
void shoes_native_control_position(SHOES_CONTROL_REF, shoes_place *,
                                   VALUE, shoes_canvas *, shoes_place *);
void shoes_native_control_position_no_pad(SHOES_CONTROL_REF, shoes_place *,
        VALUE, shoes_canvas *, shoes_place *);
void shoes_native_control_repaint(SHOES_CONTROL_REF, shoes_place *,
                                  shoes_canvas *, shoes_place *);
void shoes_native_control_repaint_no_pad(SHOES_CONTROL_REF, shoes_place *,
        shoes_canvas *, shoes_place *);
void shoes_native_control_focus(SHOES_CONTROL_REF);
void shoes_native_control_state(SHOES_CONTROL_REF, SHOES_BOOL, SHOES_BOOL);
void shoes_native_control_remove(SHOES_CONTROL_REF, shoes_canvas *);
void shoes_native_control_free(SHOES_CONTROL_REF);
void shoes_native_control_set_tooltip(SHOES_CONTROL_REF ref, VALUE tooltip);
VALUE shoes_native_control_get_tooltip(SHOES_CONTROL_REF ref);

// 3.3.x might add native_text_edit_box
SHOES_CONTROL_REF shoes_native_text_edit_box(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_text_edit_box_get_text(SHOES_CONTROL_REF);
void shoes_native_text_edit_box_set_text(SHOES_CONTROL_REF, char *);
VALUE shoes_native_text_edit_box_append(SHOES_CONTROL_REF, char *);
void shoes_native_text_edit_box_scroll_to_end(SHOES_CONTROL_REF);
SHOES_CONTROL_REF shoes_native_check(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_check_get(SHOES_CONTROL_REF);
void shoes_native_check_set(SHOES_CONTROL_REF, int);
SHOES_CONTROL_REF shoes_native_radio(VALUE, shoes_canvas *, shoes_place *, VALUE, VALUE);
void shoes_native_canvas_oneshot(int, VALUE);
VALUE shoes_native_clipboard_get(shoes_app *);
void shoes_native_clipboard_set(shoes_app *, VALUE);
void shoes_native_systray(char *, char *, char *);   //temporary sig and location
VALUE shoes_native_to_s(VALUE);
char *shoes_native_to_utf8(VALUE, int *);
VALUE shoes_native_window_color(shoes_app *);
VALUE shoes_native_dialog_color(shoes_app *);
VALUE shoes_dialog_alert(int argc, VALUE *argv, VALUE self);
VALUE shoes_dialog_ask(int argc, VALUE *argv, VALUE self);
VALUE shoes_dialog_confirm(int argc, VALUE *argv, VALUE self);
VALUE shoes_dialog_color(VALUE, VALUE);
VALUE shoes_dialog_open(int,VALUE*,VALUE);
VALUE shoes_dialog_save(int,VALUE*,VALUE);
VALUE shoes_dialog_open_folder(int,VALUE*,VALUE);
VALUE shoes_dialog_save_folder(int,VALUE*,VALUE);

// new in 3.3.4
void shoes_native_app_get_window_position(shoes_app *); 
void shoes_native_app_window_move(shoes_app *, int, int);

#endif
