///
// shoes/world.c
// Abstract windowing for GTK, Quartz (OSX) and Win32.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#include "shoes/world.h"
#include "shoes/native.h"
#include "shoes/internal.h"

#ifdef SHOES_SIGNAL
#include <signal.h>
#ifdef SHOES_GTK_WIN32
#define SHOES_WIN32  // For this file.
#endif

void
shoes_sigint()
{
  shoes_native_quit();
}
#endif

#ifdef __cplusplus
extern "C" {
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
  world->image_cache = st_init_strtable();
  world->blank_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
  world->blank_cache = SHOE_ALLOC(shoes_cached_image);
  world->blank_cache->surface = world->blank_image;
  world->blank_cache->pattern = NULL;
  world->blank_cache->width = 1;
  world->blank_cache->height = 1;
  world->blank_cache->mtime = 0;
  world->default_font = pango_font_description_new();
  pango_font_description_set_family(world->default_font, "Arial");
  pango_font_description_set_absolute_size(world->default_font, 14. * PANGO_SCALE * (96./72.));
  rb_gc_register_address(&world->apps);
  rb_gc_register_address(&world->msgs);
  return world;
}

int
shoes_world_free_image_cache(char *key, shoes_cache_entry *cached, char *arg)
{
  if (cached->type != SHOES_CACHE_ALIAS && cached->image != NULL)
  {
    if (cached->image->pattern != NULL)
      cairo_pattern_destroy(cached->image->pattern);
    if (cached->image->surface != shoes_world->blank_image)
      cairo_surface_destroy(cached->image->surface);
    free(cached->image);
  }
  free(cached);
  free(key);
  return ST_CONTINUE;
}

void
shoes_world_free(shoes_world_t *world)
{
#ifdef VLC_0_9
  if (world->vlc != NULL) libvlc_release(world->vlc);
#endif
  shoes_native_cleanup(world);
  st_foreach(world->image_cache, CASTFOREACH(shoes_world_free_image_cache), 0);
  st_free_table(world->image_cache);
  SHOE_FREE(world->blank_cache);
  cairo_surface_destroy(world->blank_image);
  pango_font_description_free(world->default_font);
  rb_gc_unregister_address(&world->apps);
  rb_gc_unregister_address(&world->msgs);
  if (world != NULL)
    SHOE_FREE(world);
}

#ifndef RUBY_1_8
int
shoes_ruby_embed()
{
  VALUE v;
  char *argv[] = {"ruby", "-e", "1"};

  // TODO delete: char**  sysinit_argv = NULL;
  RUBY_INIT_STACK;
#ifdef SHOES_WIN32
  //ruby_sysinit(0, 0);
#endif
  //printf("starting ruby_init\n");
  ruby_init();
  //printf("back from ruby_init\n");
  v = (VALUE)ruby_options(3, argv);
  //printf("back from ruby_options : %d\n", !FIXNUM_P(v));
  return !FIXNUM_P(v);
}
#else
#define shoes_ruby_embed ruby_init
#endif

shoes_code
shoes_init(SHOES_INIT_ARGS)
{
  //printf("starting shoes_init\n");
#ifdef SHOES_SIGNAL
  signal(SIGINT,  shoes_sigint);
#ifndef SHOES_GTK_WIN32
  signal(SIGQUIT, shoes_sigint);
#endif
#endif
  shoes_ruby_embed();
  //printf("back from shoes_ruby_embed\n");
  shoes_ruby_init();
  //printf("back from shoes_ruby_init\n");
  shoes_world = shoes_world_alloc();
#ifdef SHOES_WIN32
  shoes_world->os.instance = inst;
  shoes_world->os.style = style;
#endif
  shoes_native_init();
  //printf("back from shoes_native_init\n");

  rb_const_set(cShoes, rb_intern("FONTS"), shoes_font_list());
  return SHOES_OK;
}

void
shoes_update_fonts(VALUE ary)
{
#if PANGO_VERSION_MAJOR > 1 || PANGO_VERSION_MINOR >= 22
  pango_cairo_font_map_set_default(NULL);
#endif
  rb_funcall(rb_const_get(cShoes, rb_intern("FONTS")), rb_intern("replace"), 1, ary);
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
  char bootup[SHOES_BUFSIZE];

  if (path)
  {
    sprintf(bootup, "Shoes.visit(%%q<%s>);", path);

    VALUE v = rb_rescue2(CASTHOOK(shoes_load_begin), (VALUE)bootup, CASTHOOK(shoes_load_exception), Qnil, rb_cObject, 0);
    if (rb_obj_is_kind_of(v, rb_eException))
    {
      shoes_canvas_error(Qnil, v);
      rb_eval_string("Shoes.show_log");
    }
  }

  return SHOES_OK;
}

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
#ifdef OLD_SHOES
      "$:.replace([DIR+'/ruby/lib/'+(ENV['SHOES_RUBY_ARCH'] || RUBY_PLATFORM), DIR+'/ruby/lib', DIR+'/lib', '.']);"
#else
      "$:.unshift(DIR+'/lib');"
      "$:.unshift('.');"
#endif
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
  strcpy(shoes_world->path, RSTRING_PTR(str));

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
  rb_funcall(cShoes, rb_intern("clean"), 0);
  shoes_world_free(shoes_world);
  return SHOES_OK;
}

#ifdef __cplusplus
}
#endif
