#ifndef __Z_ZONE__
#define __Z_ZONE__

#ifndef __GNUC__
#define __attribute__(x)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

enum {PU_FREE, PU_STATIC, PU_SOUND, PU_MUSIC, PU_LEVEL, PU_LEVSPEC, PU_CACHE, PU_MAX};

#define PU_PURGELEVEL PU_CACHE

void *(Z_Malloc)(size_t size, int tag, void **ptr);
void (Z_Free)(void *ptr);
void (Z_FreeTags)(int lowtag, int hightag);
void (Z_ChangeTag)(void *ptr, int tag);
void *(Z_Calloc)(size_t n, size_t n2, int tag, void **user);
void *(Z_Realloc)(void *p, size_t n, int tag, void **user);
char *(Z_Strdup)(const char *s, int tag, void **user);

#endif
