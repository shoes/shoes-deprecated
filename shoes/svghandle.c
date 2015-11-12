/*
 * svghandle -  experimental  in 3.3.0
*/
#include "shoes/app.h"
#include "shoes/canvas.h"
#include "shoes/ruby.h"
#include "shoes/internal.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/version.h"
#include "shoes/http.h"
#include "shoes/effects.h"
#include <math.h>

#include <rsvg.h>


// alloc some memory for a shoes_svghandle; We'll protect it from gc
// out of caution.
void
shoes_svghandle_mark(shoes_svghandle *handle)
{
  rb_gc_mark_maybe(handle->parent);
}

static void
shoes_svghandle_free(shoes_svghandle *handle)
{
  RUBY_CRITICAL(free(handle));
}

VALUE
shoes_svghandle_alloc(VALUE klass)
{
  VALUE obj;
  shoes_svghandle *handle = SHOE_ALLOC(shoes_svghandle);
  SHOE_MEMZERO(handle, shoes_svghandle, 1);
  obj = Data_Wrap_Struct(klass, shoes_svghandle_mark, shoes_svghandle_free, handle);
  handle->handle = NULL;
  handle->parent = Qnil;
  return obj;
}

// only one of path or str is non-nil
VALUE
shoes_svghandle_new(VALUE klass, VALUE path, VALUE str, VALUE parent)
{
  RsvgHandle *rhandle;
  GError *gerror = NULL;
  VALUE obj;
  if (!NIL_P(path)) {
    // load it from a file
    char *filename = RSTRING_PTR(path);
    rhandle = rsvg_handle_new_from_file (filename, &gerror);
    if (rhandle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else if (!NIL_P(str)) {
    // load it from a string
    char *data = RSTRING_PTR(str);
    int len = RSTRING_LEN(str);
    rhandle = rsvg_handle_new_from_data (data, len, &gerror);
    if (rhandle == NULL) {
      printf("Failed SVG: %s\n", gerror->message);
    }
  } else {
    // raise an exception
  }
  obj = shoes_svghandle_alloc(klass); 
  // get a ptr to the stuct inside obj
  shoes_svghandle *svghan;
  Data_Get_Struct(obj, shoes_svghandle, svghan);
  svghan->handle = rhandle;
  
  return obj;
}

VALUE
shoes_svghandle_close(VALUE self)
{
  // unref the handle and mark this object as available for ruby gc
  shoes_svghandle *svghan = NULL;
  RsvgHandle *rhan;
  Data_Get_Struct(self, shoes_svghandle, svghan);
  rhan = svghan->handle;
  g_object_unref(rhan);
  return Qnil;
}
