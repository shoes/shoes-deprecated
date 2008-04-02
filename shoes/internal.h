//
// shoes/internal.h
// Debugging and allocation functions within Shoes.
//

#ifndef SHOES_INTERNAL_H
#define SHOES_INTERNAL_H

#define SHOE_REALLOC_N(V, T, N)     (V)=(T *)realloc((char*)(V), sizeof(T)*(N))
#define SHOE_ALLOC_N(T, N)          (T *)malloc(sizeof(T) * N)
#define SHOE_ALLOC(T)               (T *)malloc(sizeof(T))
#define SHOE_FREE(T)                free((void*)T)

#define SHOE_MEMZERO(p,type,n)      memset((p), 0, sizeof(type)*(n))
#define SHOE_MEMCPY(p1,p2,type,n)   memcpy((p1), (p2), sizeof(type)*(n))
#define SHOE_MEMMOVE(p1,p2,type,n)  memmove((p1), (p2), sizeof(type)*(n))
#define SHOE_MEMCMP(p1,p2,type,n)   memcmp((p1), (p2), sizeof(type)*(n))

#ifndef min
#define min(a, b) ((a) <= (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) >= (b) ? (a) : (b))
#endif

#ifdef SHOES_WIN32

void odprintf(const char *format, ...);
int shoes_snprintf(char* str, size_t size, const char* format, ...);
#define DEBUGP odprintf 

#ifdef DEBUG
#define INFO DEBUGP
#else
#define INFO(msg)
#endif
#define WARN DEBUGP
#define QUIT(msg) \
  if (code == SHOES_OK) code = SHOES_FAIL; \
  DEBUGP(msg); \
  goto quit

#else

#define shoes_snprintf snprintf

#define DEBUGP printf
#ifdef DEBUG

#define INFO(f, s...) DEBUGP(f, ## s)
#else
#define INFO(f, s...)
#endif
#define WARN(f, s...) DEBUGP(f, ## s)
#define QUIT(f, s...) \
  if (code == SHOES_OK) code = SHOES_FAIL; \
  DEBUGP(f, ## s); \
  goto quit
#endif

#endif
