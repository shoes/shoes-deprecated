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

typedef struct _shoes_world_t {
  SHOES_WORLD_OS os;
  char path[SHOES_BUFSIZE];
  VALUE app;
} shoes_world_t;

extern shoes_world_t *shoes_world;

#define GLOBAL_APP(appvar) \
  shoes_app *appvar; \
  Data_Get_Struct(shoes_world->app, shoes_app, appvar)

//
// Shoes World
// 
shoes_world_t *shoes_world_alloc(void);
void shoes_world_free(shoes_world_t *);

//
// Shoes
// 
shoes_code shoes_init(void);
shoes_code shoes_load(char *, char *);
shoes_code shoes_start(char *, char *);
shoes_code shoes_final(void);

#endif
