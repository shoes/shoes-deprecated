//
// shoes/world.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/internal.h"

shoes_world_t *shoes_world = NULL;

shoes_world_t *
shoes_world_alloc()
{
  shoes_world_t *world = SHOE_ALLOC(shoes_world_t);
  SHOE_MEMZERO(world, shoes_world_t, 1);
  world->app = shoes_app_new();
  rb_gc_register_address(&world->app);
  return world;
}

void
shoes_world_free(shoes_world_t *world)
{
#ifdef SHOES_QUARTZ
  CFRelease(world->os.clip);
  TECDisposeConverter(world->os.converter);
#endif
  rb_gc_unregister_address(&world->app);
  if (world != NULL)
    SHOE_FREE(world);
}

shoes_code
shoes_init()
{
#ifdef SHOES_WIN32
  INITCOMMONCONTROLSEX InitCtrlEx;
  InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCtrlEx.dwICC = ICC_PROGRESS_CLASS;
  InitCommonControlsEx(&InitCtrlEx);
#endif
  ruby_init();
  shoes_ruby_init();
#ifdef SHOES_QUARTZ
  shoes_slot_quartz_register();
#endif
  shoes_world = shoes_world_alloc();
  return SHOES_OK;
}

shoes_code
shoes_load(char *path, char *uri)
{
  char bootup[SHOES_BUFSIZE];
  sprintf(bootup,
    "begin;"
      "Shoes.load(%%q<%s>);"
    "rescue Object => e;"
      SHOES_META
        EXC_RUN
      "end;"
    "end;", path);
  rb_eval_string(bootup);

  return shoes_app_start(shoes_world->app, uri);
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
      SHOES_META
        "define_method :load do |path|; end;"
        EXC_RUN
      "end;"
      "e.message;"
    "end",
    path);

  if (len < 0 || len >= SHOES_BUFSIZE)
  {
    QUIT("Path to script is too long.", 0);
  }

  VALUE str = rb_eval_string(bootup);
  StringValue(str);
  strcpy(shoes_world->path, RSTRING(str)->ptr);

  VALUE load_uri = rb_eval_string("$SHOES_URI = Shoes.args!");
  if (!RTEST(load_uri))
    return SHOES_QUIT;

  code = shoes_load(RSTRING_PTR(load_uri), uri);

quit:
  return code;
}

shoes_code
shoes_final()
{
  shoes_world_free(shoes_world);
  return SHOES_OK;
}
