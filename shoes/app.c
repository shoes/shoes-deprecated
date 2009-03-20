//
// shoes/app.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include <glib.h>
#include "shoes/app.h"
#include "shoes/internal.h"
#include "shoes/ruby.h"
#include "shoes/canvas.h"
#include "shoes/world.h"
#include "shoes/native.h"

static void
shoes_app_mark(shoes_app *app)
{
  shoes_native_slot_mark(app->slot);
  rb_gc_mark_maybe(app->title);
  rb_gc_mark_maybe(app->location);
  rb_gc_mark_maybe(app->canvas);
  rb_gc_mark_maybe(app->nestslot);
  rb_gc_mark_maybe(app->nesting);
  rb_gc_mark_maybe(app->extras);
  rb_gc_mark_maybe(app->styles);
  rb_gc_mark_maybe(app->groups);
  rb_gc_mark_maybe(app->owner);
}

static void
shoes_app_free(shoes_app *app)
{
  SHOE_FREE(app->slot);
  cairo_destroy(app->scratch);
  RUBY_CRITICAL(free(app));
}

VALUE
shoes_app_alloc(VALUE klass)
{
  shoes_app *app = SHOE_ALLOC(shoes_app);
  SHOE_MEMZERO(app, shoes_app, 1);
  app->slot = SHOE_ALLOC(SHOES_SLOT_OS);
  SHOE_MEMZERO(app->slot, SHOES_SLOT_OS, 1);
  app->slot->owner = app;
  app->started = FALSE;
  app->owner = Qnil;
  app->location = Qnil;
  app->canvas = shoes_canvas_new(cShoes, app);
  app->nestslot = Qnil;
  app->nesting = rb_ary_new();
  app->extras = rb_ary_new();
  app->groups = Qnil;
  app->styles = Qnil;
  app->title = Qnil;
  app->width = SHOES_APP_WIDTH;
  app->height = SHOES_APP_HEIGHT;
  app->minwidth = 0;
  app->minheight = 0;
  app->fullscreen = FALSE;
  app->resizable = TRUE;
  app->cursor = s_arrow;
  app->scratch = cairo_create(cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1));
  app->self = Data_Wrap_Struct(klass, shoes_app_mark, shoes_app_free, app);
  return app->self;
}

VALUE
shoes_app_new(VALUE klass)
{
  VALUE app = shoes_app_alloc(klass);
  rb_ary_push(shoes_world->apps, app);
  return app;
}

VALUE
shoes_apps_get(VALUE self)
{
  return rb_ary_dup(shoes_world->apps);
}

static void
shoes_app_clear(shoes_app *app)
{
  shoes_ele_remove_all(app->extras);
  shoes_canvas_clear(app->canvas);
  app->nestslot = Qnil;
  app->groups = Qnil;
}

//
// When a window is finished, call this to delete it from the master
// list.  Returns 1 if all windows are gone.
//
int
shoes_app_remove(shoes_app *app)
{
  shoes_app_clear(app);
  rb_ary_delete(shoes_world->apps, app->self);
  return (RARRAY_LEN(shoes_world->apps) == 0);
}

shoes_code
shoes_app_resize(shoes_app *app, int width, int height)
{
  app->width = width;
  app->height = height;
  shoes_native_app_resized(app);
  return SHOES_OK;
}

VALUE
shoes_app_window(int argc, VALUE *argv, VALUE self, VALUE owner)
{
  rb_arg_list args;
  VALUE attr = Qnil;
  VALUE app = shoes_app_new(self == cDialog ? cDialog : cApp);
  shoes_app *app_t;
  char *url = "/";
  Data_Get_Struct(app, shoes_app, app_t);

  switch (rb_parse_args(argc, argv, "h,s|h,", &args))
  {
    case 1:
      attr = args.a[0];
    break;

    case 2:
      url = RSTRING_PTR(args.a[0]);
      attr = args.a[1];
    break;
  }

  if (rb_block_given_p()) rb_iv_set(app, "@main_app", rb_block_proc());
  app_t->owner = owner;
  app_t->title = ATTR(attr, title);
  app_t->fullscreen = RTEST(ATTR(attr, fullscreen));
  app_t->resizable = (ATTR(attr, resizable) != Qfalse);
  app_t->hidden = (ATTR(attr, hidden) == Qtrue);
  shoes_app_resize(app_t, ATTR2(int, attr, width, SHOES_APP_WIDTH), ATTR2(int, attr, height, SHOES_APP_HEIGHT));
  if (RTEST(ATTR(attr, width)))
    app_t->minwidth = app_t->width;
  if (RTEST(ATTR(attr, height)))
    app_t->minheight = app_t->height;
  shoes_canvas_init(app_t->canvas, app_t->slot, attr, app_t->width, app_t->height);
  if (shoes_world->mainloop)
    shoes_app_open(app_t, url);
  return app;
}

VALUE
shoes_app_main(int argc, VALUE *argv, VALUE self)
{
  return shoes_app_window(argc, argv, self, Qnil);
}

VALUE
shoes_app_slot(VALUE app)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return app_t->nestslot;
}

VALUE
shoes_app_get_width(VALUE app)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return INT2NUM(app_t->width);
}

VALUE
shoes_app_get_height(VALUE app)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return INT2NUM(app_t->height);
}

VALUE
shoes_app_get_title(VALUE app)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return app_t->title;
}

VALUE
shoes_app_set_title(VALUE app, VALUE title)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return app_t->title = title;
}

VALUE
shoes_app_get_fullscreen(VALUE app)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  return app_t->fullscreen ? Qtrue : Qfalse;
}

VALUE
shoes_app_set_fullscreen(VALUE app, VALUE yn)
{
  shoes_app *app_t;
  Data_Get_Struct(app, shoes_app, app_t);
  shoes_native_app_fullscreen(app_t, app_t->fullscreen = RTEST(yn));
  return yn;
}

void
shoes_app_title(shoes_app *app, VALUE title)
{
  char *msg;
  if (!NIL_P(title))
    app->title = title;
  else
    app->title = rb_str_new2(SHOES_APPNAME);
  msg = RSTRING_PTR(app->title);
  shoes_native_app_title(app, msg);
}

shoes_code
shoes_app_start(VALUE allapps, char *uri)
{
  int i;
  shoes_code code;
  shoes_app *app;

  for (i = 0; i < RARRAY_LEN(allapps); i++)
  {
    VALUE appobj2 = rb_ary_entry(allapps, i);
    Data_Get_Struct(appobj2, shoes_app, app);
    if (!app->started)
    {
      code = shoes_app_open(app, uri);
      app->started = TRUE;
      if (code != SHOES_OK)
        return code;
    }
  }

  return shoes_app_loop();
}

shoes_code
shoes_app_open(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;
  int dialog = (rb_obj_class(app->self) == cDialog);

  code = shoes_native_app_open(app, path, dialog);
  if (code != SHOES_OK)
    return code;

  shoes_app_title(app, app->title);
  if (app->slot != NULL) shoes_native_slot_reset(app->slot);
  shoes_slot_init(app->canvas, app->slot, 0, 0, app->width, app->height, TRUE, TRUE);
  code = shoes_app_goto(app, path);
  if (code != SHOES_OK)
    return code;

  if (!app->hidden)
    shoes_native_app_show(app);

  return code;
}

shoes_code
shoes_app_loop()
{
  if (shoes_world->mainloop)
    return SHOES_OK;

  shoes_world->mainloop = TRUE;
  INFO("RUNNING LOOP.\n");
  shoes_native_loop();
  return SHOES_OK;
}

typedef struct
{
  shoes_app *app;
  VALUE canvas;
  VALUE block;
  char ieval;
  VALUE args;
} shoes_exec;

#ifdef RUBY_1_9
struct METHOD {
    VALUE oclass;		/* class that holds the method */
    VALUE rklass;		/* class of the receiver */
    VALUE recv;
    ID id, oid;
    void *body; /* NODE *body; */
};
#else
struct METHOD {
    VALUE klass, rklass;
    VALUE recv;
    ID id, oid;
    int safe_level;
    NODE *body;
};
#endif

static VALUE
rb_unbound_get_class(VALUE method)
{
  struct METHOD *data;
  Data_Get_Struct(method, struct METHOD, data);
  return data->rklass;
}

static VALUE
shoes_app_run(VALUE rb_exec)
{
  shoes_exec *exec = (shoes_exec *)rb_exec;
  rb_ary_push(exec->app->nesting, exec->canvas);
  if (exec->ieval)
  {
    VALUE obj;
    obj = mfp_instance_eval(exec->app->self, exec->block);
    return obj;
  }
  else
  {
    int i;
    VALUE vargs[10];
    for (i = 0; i < RARRAY_LEN(exec->args); i++)
      vargs[i] = rb_ary_entry(exec->args, i);
    return rb_funcall2(exec->block, s_call, RARRAY_LEN(exec->args), vargs);
  }
}

static VALUE
shoes_app_exception(VALUE rb_exec, VALUE e)
{
  shoes_exec *exec = (shoes_exec *)rb_exec;
  rb_ary_clear(exec->app->nesting);
  shoes_canvas_error(exec->canvas, e);
  return Qnil;
}

shoes_code
shoes_app_visit(shoes_app *app, char *path)
{
  shoes_exec exec;
  shoes_canvas *canvas;
  VALUE meth;
  Data_Get_Struct(app->canvas, shoes_canvas, canvas);

  canvas->slot->scrolly = 0;
  shoes_native_slot_clear(canvas);
  shoes_app_clear(app);
  shoes_app_reset_styles(app);
  meth = rb_funcall(cShoes, s_run, 1, app->location = rb_str_new2(path));

  VALUE app_block = rb_iv_get(app->self, "@main_app");
  if (!NIL_P(app_block))
    rb_ary_store(meth, 0, app_block);

  exec.app = app;
  exec.block = rb_ary_entry(meth, 0);
  exec.args = rb_ary_entry(meth, 1);
  if (rb_obj_is_kind_of(exec.block, rb_cUnboundMethod)) {
    VALUE klass = rb_unbound_get_class(exec.block);
    exec.canvas = app->nestslot = shoes_slot_new(klass, ssNestSlot, app->canvas);
    exec.block = rb_funcall(exec.block, s_bind, 1, exec.canvas);
    exec.ieval = 0;
    rb_ary_push(canvas->contents, exec.canvas);
  } else {
    exec.canvas = app->nestslot = app->canvas;
    exec.ieval = 1;
  }

  rb_rescue2(CASTHOOK(shoes_app_run), (VALUE)&exec, CASTHOOK(shoes_app_exception), (VALUE)&exec, rb_cObject, 0);

  rb_ary_clear(exec.app->nesting);
  return SHOES_OK;
}

shoes_code
shoes_app_paint(shoes_app *app)
{
  shoes_canvas_paint(app->canvas);
  return SHOES_OK;
}

shoes_code
shoes_app_motion(shoes_app *app, int x, int y)
{
  app->mousex = x; app->mousey = y;
  shoes_canvas_send_motion(app->canvas, x, y, Qnil);
  return SHOES_OK;
}

shoes_code
shoes_app_click(shoes_app *app, int button, int x, int y)
{
  app->mouseb = button;
  shoes_canvas_send_click(app->canvas, button, x, y);
  return SHOES_OK;
}

shoes_code
shoes_app_release(shoes_app *app, int button, int x, int y)
{
  app->mouseb = 0;
  shoes_canvas_send_release(app->canvas, button, x, y);
  return SHOES_OK;
}

shoes_code
shoes_app_wheel(shoes_app *app, ID dir, int x, int y)
{
  shoes_canvas *canvas;
  Data_Get_Struct(app->canvas, shoes_canvas, canvas);
  if (canvas->slot->vscroll)
  {
    if (dir == s_up)
      shoes_slot_scroll_to(canvas, -16, 1);
    else if (dir == s_down)
      shoes_slot_scroll_to(canvas, 16, 1);
  }
  shoes_canvas_send_wheel(app->canvas, dir, x, y);
  return SHOES_OK;
}

shoes_code
shoes_app_keypress(shoes_app *app, VALUE key)
{
  if (key == symAltSlash)
    rb_eval_string("Shoes.show_log");
  else if (key == symAltQuest)
    rb_eval_string("Shoes.show_manual");
  else if (key == symAltDot)
    rb_eval_string("Shoes.show_selector");
  else
    shoes_canvas_send_keypress(app->canvas, key);
  return SHOES_OK;
}

VALUE
shoes_sys(char *cmd, int detach)
{
  if (detach)
    return rb_funcall(rb_mKernel, rb_intern("system"), 1, rb_str_new2(cmd));
  else
    return rb_funcall(rb_mKernel, '`', 1, rb_str_new2(cmd));
}

shoes_code
shoes_app_goto(shoes_app *app, char *path)
{
  shoes_code code = SHOES_OK;
  const char http_scheme[] = "http://";
  if (strlen(path) > strlen(http_scheme) && strncmp(http_scheme, path, strlen(http_scheme)) == 0) {
    shoes_browser_open(path);
  } else {
    code = shoes_app_visit(app, path);
    if (code == SHOES_OK)
    {
      shoes_app_motion(app, app->mousex, app->mousey);
      shoes_slot_repaint(app->slot);
    }
  }
  return code;
}

shoes_code
shoes_slot_repaint(SHOES_SLOT_OS *slot)
{
  shoes_native_slot_paint(slot);
  return SHOES_OK;
}

static void
shoes_style_set(VALUE styles, VALUE klass, VALUE k, VALUE v)
{
  VALUE hsh = rb_hash_aref(styles, klass);
  if (NIL_P(hsh))
    rb_hash_aset(styles, klass, hsh = rb_hash_new());
  rb_hash_aset(hsh, k, v);
}

#define STYLE(klass, k, v) \
  shoes_style_set(app->styles, klass, \
    ID2SYM(rb_intern("" # k)), rb_str_new2("" # v))

void
shoes_app_reset_styles(shoes_app *app)
{
  app->styles = rb_hash_new();
  STYLE(cBanner,      size, 48);
  STYLE(cTitle,       size, 34);
  STYLE(cSubtitle,    size, 26);
  STYLE(cTagline,     size, 18);
  STYLE(cCaption,     size, 14);
  STYLE(cPara,        size, 12);
  STYLE(cInscription, size, 10);

  STYLE(cCode,        family, monospace);
  STYLE(cDel,         strikethrough, single);
  STYLE(cEm,          emphasis, italic);
  STYLE(cIns,         underline, single);
  STYLE(cLink,        underline, single);
  STYLE(cLink,        stroke, #06E);
  STYLE(cLinkHover,   underline, single);
  STYLE(cLinkHover,   stroke, #039);
  STYLE(cStrong,      weight, bold);
  STYLE(cSup,         rise,   10);
  STYLE(cSup,         size,   x-small);
  STYLE(cSub,         rise,   -10);
  STYLE(cSub,         size,   x-small);
}

void
shoes_app_style(shoes_app *app, VALUE klass, VALUE hsh)
{
  long i;
  VALUE keys = rb_funcall(hsh, s_keys, 0);
  for ( i = 0; i < RARRAY_LEN(keys); i++ )
  {
    VALUE key = rb_ary_entry(keys, i);
    VALUE val = rb_hash_aref(hsh, key);
    if (!SYMBOL_P(key)) key = rb_str_intern(key);
    shoes_style_set(app->styles, klass, key, val);
  }
}

VALUE
shoes_app_close_window(shoes_app *app)
{
  shoes_native_app_close(app);
  return Qnil;
}

VALUE
shoes_app_location(VALUE self)
{
  shoes_app *app;
  Data_Get_Struct(self, shoes_app, app);
  return app->location;
}

VALUE
shoes_app_is_started(VALUE self)
{
  shoes_app *app;
  Data_Get_Struct(self, shoes_app, app);
  return app->started ? Qtrue : Qfalse;
}

VALUE
shoes_app_contents(VALUE self)
{
  shoes_app *app;
  Data_Get_Struct(self, shoes_app, app);
  return shoes_canvas_contents(app->canvas);
}

VALUE
shoes_app_quit(VALUE self)
{
  shoes_native_quit();
  return self;
}
