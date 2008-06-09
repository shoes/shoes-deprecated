//
// shoes/native.h
// Common native Shoes routines.
//
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
