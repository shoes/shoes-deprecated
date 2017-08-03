/* systray
*/
#include "shoes/types/native.h"
#include "shoes/types/systray.h"

// ruby
VALUE cSystray;
    
FUNC_M("+systray", systray, -1);

shoes_systray_init() {
    cSystray = rb_define_class_under(cTypes, "Systray", cNative); 
    rb_define_alloc_func(cSystray, shoes_systray_alloc);
    //rb_define_method(cSystray, "width", CASTHOOK(shoes_svghandle_get_width), 0);
    //rb_define_method(cSystray, "height", CASTHOOK(shoes_svghandle_get_height), 0);
    RUBY_M("+systray", systray, -1);
}

// canvas 
VALUE shoes_canvas_systray(int argc, VALUE *argv, VALUE self) {
    VALUE han;
    han = shoes_systray_new(argc, argv, self);
    return han;
}

void shoes_systray_mark(shoes_systray *handle) {
    // we don't have any Ruby objects to mark.
}

static void shoes_systray_free(shoes_systray *handle) {
    if (handle->icon_path) free(handle->icon_path);
    if (handle->title) free(handle->title);
    if (handle->message) free(handle->message);
    RUBY_CRITICAL(SHOE_FREE(handle));
}

VALUE shoes_systray_alloc(VALUE klass) {
    VALUE obj;
    shoes_systray *handle = SHOE_ALLOC(shoes_systray);
    SHOE_MEMZERO(handle, shoes_systray, 1);
    obj = Data_Wrap_Struct(klass, NULL, shoes_systray_free, handle);
    handle->icon_path = NULL;
    handle->title = NULL;
    handle->message = NULL;
    return obj;
}

VALUE shoes_systray_new(int argc, VALUE *argv, VALUE parent) {
    // Get Ruby args. 
    VALUE rbtitle, rbmessage, rbpath;
    rbtitle = shoes_hash_get(argv[0], rb_intern("title"));
    rbmessage = shoes_hash_get(argv[0], rb_intern("message"));
    rbpath = shoes_hash_get(argv[0], rb_intern("icon"));
    char *title = NULL, *message = NULL, *path = NULL;
    
    // Alloc the object and init
    VALUE obj = shoes_systray_alloc(cSystray);
    shoes_systray *self_t;
    Data_Get_Struct(obj, shoes_systray, self_t);
    if ((!NIL_P(rbtitle)) && (RSTRING_LEN(rbtitle) > 0)) {
      title = strdup(RSTRING_PTR(rbtitle));
    }
    if ((!NIL_P(rbmessage)) && (RSTRING_LEN(rbmessage) > 0)) {
      message = strdup(RSTRING_PTR(rbmessage));
    }
    if ((!NIL_P(rbpath)) && (RSTRING_LEN(rbpath) > 0)) {
      path = strdup(RSTRING_PTR(rbpath));
    }
    // call the native widget
    shoes_native_systray(title, message, path); // temporary
}
