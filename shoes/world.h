//
// shoes/world.h
//
// The shoes_world struct contains global information about the environment which is shared between
// app windows.
//
#ifndef SHOES_WORLD_H
#define SHOES_WORLD_H

#include "shoes/config.h"
#include "shoes/ruby.h"
#include "shoes/code.h"

#ifdef __cplusplus
extern "C" {
#endif

SHOES_EXTERN typedef struct _shoes_world_t {
  SHOES_WORLD_OS os;
  int mainloop;
  char path[SHOES_BUFSIZE];
  VALUE apps, msgs;
  st_table *image_cache;
  guint thread_event;
  cairo_surface_t *blank_image;
  shoes_cached_image *blank_cache;
  PangoFontDescription *default_font;
#ifdef VLC_0_9
  libvlc_instance_t *vlc;
#endif
} shoes_world_t;

extern SHOES_EXTERN shoes_world_t *shoes_world;

#define GLOBAL_APP(appvar) \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) \
    Data_Get_Struct(rb_ary_entry(shoes_world->apps, 0), shoes_app, appvar)
#define ACTUAL_APP(appvar) \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) \
      Data_Get_Struct(self, shoes_app, appvar)

#define ROUND(x) ((x) >= 0 ? (int)round((x)+0.5) : (int)round((x)-0.5))

//
// Shoes World
// 
SHOES_EXTERN shoes_world_t *shoes_world_alloc(void);
SHOES_EXTERN void shoes_world_free(shoes_world_t *);
void shoes_update_fonts(VALUE);

//
// Shoes
// 
SHOES_EXTERN shoes_code shoes_init(SHOES_INIT_ARGS);
SHOES_EXTERN shoes_code shoes_load(char *);
SHOES_EXTERN shoes_code shoes_start(char *, char *);
#ifdef SHOES_WIN32
SHOES_EXTERN int shoes_win32_cmdvector(const char *, char ***);
#endif
SHOES_EXTERN void shoes_set_argv(int, char **);
SHOES_EXTERN shoes_code shoes_final(void);

#ifdef __cplusplus
}
#endif

#endif
