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
shoes_load(char *path)
{
  char bootup[512];
  sprintf(bootup,
    "begin;"
      "DIR = File.expand_path(File.dirname(%%q<%s>));"
      "$:.replace([DIR+'/ruby/lib/'+PLATFORM, DIR+'/ruby/lib', DIR+'/lib']);"
      "require 'shoes';"
      "'OK';"
    "rescue Object => e;"
      SHOES_META
        "define_method :load do |path|; end;"
        EXC_RUN
      "end;"
      "e.message;"
    "end",
    path);
  VALUE str = rb_eval_string(bootup);
  StringValue(str);
  INFO("Bootup: %s\n", RSTRING(str)->ptr);

  VALUE uri = rb_eval_string("$SHOES_URI = Shoes.args!");
  if (!RTEST(uri))
    return SHOES_QUIT;

  sprintf(bootup,
    "begin;"
      "Shoes.load($SHOES_URI) if $SHOES_URI.is_a?(String);"
    "rescue Object => e;"
      SHOES_META
        EXC_RUN
      "end;"
    "end;");
  rb_eval_string(bootup);

  return SHOES_OK;
}

shoes_code
shoes_start(char *uri)
{
  return shoes_app_start(shoes_world->app, uri);
}

shoes_code
shoes_final()
{
  shoes_world_free(shoes_world);
  return SHOES_OK;
}
