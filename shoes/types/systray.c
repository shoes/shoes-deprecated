/* systray
*/

#include "shoes/types/systray.h"

// ruby
VALUE cSystray

FUNC_M("+systray", systray, -1);

// systray
void shoes_systray_mark(shoes_systray *handle) {
    // we don't have any Ruby objects to mark.
}

static void shoes_systray_free(shoes_systray *handle) {
    if (handle->path) free(handle->icon_path);
    if (handle->data) free(handle->title);
    if (handle->subid) free(handle->message);
    RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE shoes_systray_alloc(VALUE klass) {
    VALUE obj;
    shoes_systray *handle = SHOE_ALLOC(shoes_systray);
    SHOE_MEMZERO(handle, shoes_systray, 1);
    obj = Data_Wrap_Struct(klass, NULL, shoes_systray_free, handle);
    handle->handl = NULL;
    handle->subid = NULL;
    return obj;
}

VALUE shoes_systray_new(int argc, VALUE *argv, VALUE parent) {
}
