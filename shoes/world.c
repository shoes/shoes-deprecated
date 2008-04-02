//
// shoes/world.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/internal.h"
#ifndef SHOES_WIN32
#include <signal.h>

void
shoes_sigint()
{
#ifdef SHOES_GTK
  gtk_main_quit();
#endif
#ifdef SHOES_QUARTZ
  QuitApplicationEventLoop();
#endif
}
#endif

shoes_world_t *shoes_world = NULL;

shoes_world_t *
shoes_world_alloc()
{
  shoes_world_t *world = SHOE_ALLOC(shoes_world_t);
  SHOE_MEMZERO(world, shoes_world_t, 1);
  world->apps = rb_ary_new();
  world->msgs = rb_ary_new();
  world->mainloop = FALSE;
  rb_gc_register_address(&world->apps);
  rb_gc_register_address(&world->msgs);
  return world;
}

void
shoes_world_free(shoes_world_t *world)
{
#ifdef SHOES_QUARTZ
  CFRelease(world->os.clip);
  TECDisposeConverter(world->os.converter);
#endif
  rb_gc_unregister_address(&world->apps);
  rb_gc_unregister_address(&world->msgs);
  if (world != NULL)
    SHOE_FREE(world);
}

shoes_code
shoes_init(SHOES_INIT_ARGS)
{
#ifdef SHOES_GTK
  gtk_init(NULL, NULL);
#endif
#ifdef SHOES_WIN32
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&InitCtrlEx);
#else
  signal(SIGINT,  shoes_sigint);
  signal(SIGQUIT, shoes_sigint);
#endif
  ruby_init();
  shoes_ruby_init();
  shoes_world = shoes_world_alloc();
#ifdef SHOES_QUARTZ
  shoes_app_quartz_install();
  shoes_slot_quartz_register();
  if (PasteboardCreate(kPasteboardClipboard, &shoes_world->os.clip) != noErr) {
    INFO("Apple Pasteboard create failed.\n");
  }
#endif
#ifdef SHOES_WIN32
  shoes_world->os.instance = inst;
  shoes_world->os.style = style;
  shoes_classex_init();
#endif
  return SHOES_OK;
}

static VALUE
shoes_load_begin(VALUE v)
{
  char *bootup = (char *)v;
  return rb_eval_string(bootup);
}

static VALUE
shoes_load_exception(VALUE v, VALUE exc)
{
  return exc;
}

shoes_code
shoes_load(char *path)
{
  shoes_code code = SHOES_QUIT;
  char bootup[SHOES_BUFSIZE];

  if (path)
  {
    sprintf(bootup, "Shoes.load(%%q<%s>);", path);

    VALUE v = rb_rescue2(CASTHOOK(shoes_load_begin), (VALUE)bootup, CASTHOOK(shoes_load_exception), Qnil, rb_cObject, 0);
    if (rb_obj_is_kind_of(v, rb_eException))
    {
      QUIT_ALERT(v);
    }
  }

  return SHOES_OK;
}

#ifdef SHOES_WIN32
int
shoes_win32_cmdvector(const char *cmdline, char ***argv)
{
  return rb_w32_cmdvector(cmdline, argv);
}
#endif

void
shoes_set_argv(int argc, char **argv)
{
  ruby_set_argv(argc, argv);
}

static VALUE
shoes_start_begin(VALUE v)
{
  return rb_eval_string("$SHOES_URI = Shoes.args!");
}

static VALUE
shoes_start_exception(VALUE v, VALUE exc)
{
  return exc;
}

shoes_code
shoes_start(char *path, char *uri)
{
  shoes_code code = SHOES_OK;
  char bootup[SHOES_BUFSIZE];
  int len = shoes_snprintf(bootup,
    SHOES_BUFSIZE,
    "begin;"
      "DIR = File.expand_path(File.dirname(%%q<%s>));"
      "$:.replace([DIR+'/ruby/lib/'+PLATFORM, DIR+'/ruby/lib', DIR+'/lib']);"
      "require 'shoes';"
      "DIR;"
    "rescue Object => e;"
      "puts(e.message);"
    "end",
    path);

  if (len < 0 || len >= SHOES_BUFSIZE)
  {
    QUIT("Path to script is too long.");
  }

  VALUE str = rb_eval_string(bootup);
  if (NIL_P(str))
    return SHOES_QUIT;

  StringValue(str);
  strcpy(shoes_world->path, RSTRING(str)->ptr);

  char *load_uri_str = NULL;
  VALUE load_uri = rb_rescue2(CASTHOOK(shoes_start_begin), Qnil, CASTHOOK(shoes_start_exception), Qnil, rb_cObject, 0);
  if (!RTEST(load_uri))
    return SHOES_QUIT;
  if (rb_obj_is_kind_of(load_uri, rb_eException))
  {
    QUIT_ALERT(load_uri);
  }

  if (rb_obj_is_kind_of(load_uri, rb_cString))
    load_uri_str = RSTRING_PTR(load_uri);

  code = shoes_load(load_uri_str);
  if (code != SHOES_OK)
    goto quit;

  code = shoes_app_start(shoes_world->apps, uri);
quit:
  return code;
}

shoes_code
shoes_final()
{
  shoes_world_free(shoes_world);
  return SHOES_OK;
}
