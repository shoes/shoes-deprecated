//
// shoes/native.h
// Common native Shoes routines.
//
#define CHANGED_COORDS() \
  (p1->ix != p2->ix || p1->iy != p2->iy || \
   p1->iw != p2->iw || p1->dx != p2->dx || \
   p1->dy != p2->dy || p1->ih - HEIGHT_PAD != p2->ih)
#define PLACE_COORDS() p2->h -= HEIGHT_PAD; p2->ih -= HEIGHT_PAD; *p1 = *p2

void shoes_native_init();
void shoes_native_cleanup();
void shoes_native_quit();
void shoes_native_slot_mark(SHOES_SLOT_OS *slot);
void shoes_native_slot_reset(SHOES_SLOT_OS *slot);
void shoes_native_slot_clear(SHOES_SLOT_OS *slot);
void shoes_native_slot_paint(SHOES_SLOT_OS *slot);
void shoes_native_slot_scroll_top(SHOES_SLOT_OS *slot);
int shoes_native_slot_gutter(SHOES_SLOT_OS *slot);
void shoes_native_remove_item(SHOES_SLOT_OS *slot);
shoes_code shoes_app_cursor(shoes_app *, ID);
void shoes_native_app_resized(shoes_app *);
void shoes_native_app_title(shoes_app *, char *);
shoes_code shoes_native_app_open(shoes_app *, char *);
void shoes_native_app_show(shoes_app *);
void shoes_native_loop();
void shoes_native_app_close(shoes_app *);
void shoes_browser_open(char *);
void shoes_slot_init(VALUE, SHOES_SLOT_OS *, int, int, int, int, int, int);
cairo_t *shoes_cairo_create(SHOES_SLOT_OS *, int, int, int);
void shoes_cairo_destroy(SHOES_SLOT_OS *);
void shoes_group_clear(SHOES_GROUP_OS *);
void shoes_native_canvas_place(shoes_canvas *, shoes_canvas *);
void shoes_native_control_hide(SHOES_CONTROL_REF);
void shoes_native_control_show(SHOES_CONTROL_REF);
void shoes_native_control_position(SHOES_CONTROL_REF, shoes_place *, 
  VALUE, shoes_canvas *, shoes_place *);
void shoes_native_control_repaint(SHOES_CONTROL_REF, shoes_place *,
  shoes_canvas *, shoes_place *);
void shoes_native_control_remove(SHOES_CONTROL_REF, shoes_canvas *);
void shoes_native_control_free(SHOES_CONTROL_REF);
SHOES_CONTROL_REF shoes_native_surface_new(shoes_canvas *, VALUE, shoes_place *);
void shoes_native_surface_position(shoes_canvas *, shoes_canvas *, shoes_place *);
void shoes_native_surface_remove(shoes_canvas *, SHOES_CONTROL_REF);
SHOES_CONTROL_REF shoes_native_button(VALUE, shoes_canvas *, shoes_place *, char *);
SHOES_CONTROL_REF shoes_native_edit_line(VALUE, shoes_canvas *, shoes_place *, VALUE, char *);
VALUE shoes_native_edit_line_get_text(SHOES_CONTROL_REF);
void shoes_native_edit_line_set_text(SHOES_CONTROL_REF, char *);
