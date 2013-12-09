#include "doomtype.h"
#include "z_zone.h"
#include "z_bmalloc.h"
#include "lprintf.h"

typedef struct bmalpool_s {
  struct bmalpool_s *nextpool;
  size_t             blocks;
  byte               used[0];
} bmalpool_t;

inline static void* getelem(bmalpool_t *p, size_t size, size_t n)
{
  return (((byte*)p) + sizeof(bmalpool_t) + sizeof(byte)*(p->blocks) + size*n);
}

inline static int iselem(const bmalpool_t *pool, size_t size, const void* p)
{

  int dif = (const char*)p - (const char*)pool;

  dif -= sizeof(bmalpool_t);
  dif -= pool->blocks;
  if (dif<0) return -1;
  dif /= size;
  return (((size_t)dif >= pool->blocks) ? -1 : dif);
}

enum { unused_block = 0, used_block = 1};

void* Z_BMalloc(struct block_memory_alloc_s *pzone)
{
  register bmalpool_t **pool = (bmalpool_t **)&(pzone->firstpool);
  while (*pool != NULL) {
    byte *p = memchr((*pool)->used, unused_block, (*pool)->blocks);
    if (p) {
      int n = p - (*pool)->used;
      (*pool)->used[n] = used_block;
      return getelem(*pool, pzone->size, n);
    } else
      pool = &((*pool)->nextpool);
  }
  {

    bmalpool_t *newpool;



    *pool = newpool = Z_Calloc(sizeof(*newpool) + (sizeof(byte) + pzone->size)*(pzone->perpool),
             1,  pzone->tag, NULL);
    newpool->nextpool = NULL;


    newpool->used[0] = used_block;
    newpool->blocks = pzone->perpool;
    return getelem(newpool, pzone->size, 0);
  }
}

void Z_BFree(struct block_memory_alloc_s *pzone, void* p)
{
  register bmalpool_t **pool = (bmalpool_t**)&(pzone->firstpool);

  while (*pool != NULL) {
    int n = iselem(*pool, pzone->size, p);
    if (n >= 0) {
      (*pool)->used[n] = unused_block;
      if (memchr(((*pool)->used), used_block, (*pool)->blocks) == NULL) {

  bmalpool_t *oldpool = *pool;
  *pool = (*pool)->nextpool;
  Z_Free(oldpool);
      }
      return;
    } else pool = &((*pool)->nextpool);
  }
  I_Error("Z_BFree: Free not in zone %s", pzone->desc);
}
