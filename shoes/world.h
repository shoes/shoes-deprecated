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

SHOES_EXTERN typedef struct _shoes_world_t {
  SHOES_WORLD_OS os;
  int mainloop;
  char path[SHOES_BUFSIZE];
  VALUE apps, msgs;
} shoes_world_t;

SHOES_EXTERN_VAR shoes_world_t *shoes_world;

#define GLOBAL_APP(appvar) \
  shoes_app *appvar = NULL; \
  if (RARRAY_LEN(shoes_world->apps) > 0) \
    Data_Get_Struct(rb_ary_entry(shoes_world->apps, 0), shoes_app, appvar)

//
// Shoes World
// 
shoes_world_t *shoes_world_alloc(void);
void shoes_world_free(shoes_world_t *);

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

#endif
