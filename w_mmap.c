#include <sys/mman.h>
#include "doomstat.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_system.h"
#include "lprintf.h"

static struct {
  void *cache;
  int locks;
} *cachelump;

void ** mapped_wad;

void W_InitCache(void)
{
  int maxfd = 0;

  cachelump = calloc(numlumps, sizeof *cachelump);
  if (!cachelump)
    I_Error ("W_Init: Couldn't allocate lumpcache");

  {
    int i;
    for (i=0; i<numlumps; i++)
      if (lumpinfo[i].wadfile)
        if (lumpinfo[i].wadfile->handle > maxfd) maxfd = lumpinfo[i].wadfile->handle;
  }
  mapped_wad = calloc(maxfd+1,sizeof *mapped_wad);
  {
    int i;
    for (i=0; i<numlumps; i++) {
      cachelump[i].locks = -1;
      if (lumpinfo[i].wadfile) {
        int fd = lumpinfo[i].wadfile->handle;
        if (!mapped_wad[fd])
          if ((mapped_wad[fd] = mmap(NULL,I_Filelength(fd),PROT_READ,MAP_SHARED,fd,0)) == MAP_FAILED) 
            I_Error("W_InitCache: failed to mmap");
      }
    }
  }
}

void W_DoneCache(void)
{
  {
    int i;
    for (i=0; i<numlumps; i++)
      if (lumpinfo[i].wadfile) {
        int fd = lumpinfo[i].wadfile->handle;
        if (mapped_wad[fd]) {
          if (munmap(mapped_wad[fd],I_Filelength(fd))) 
            I_Error("W_DoneCache: failed to munmap");
          mapped_wad[fd] = NULL;
        }
      }
  }
  free(mapped_wad);
}

const void* W_CacheLumpNum(int lump)
{
  if (!lumpinfo[lump].wadfile)
    return NULL;
  return (mapped_wad[lumpinfo[lump].wadfile->handle]+lumpinfo[lump].position);
}

const void* W_LockLumpNum(int lump)
{
  size_t len = W_LumpLength(lump);
  const void *data = W_CacheLumpNum(lump);

  if (!cachelump[lump].cache) {

    Z_Malloc(len, PU_CACHE, &cachelump[lump].cache);
    memcpy(cachelump[lump].cache, data, len);
  }

  if (cachelump[lump].locks <= 0) {
    Z_ChangeTag(cachelump[lump].cache,PU_STATIC);

    cachelump[lump].locks = 1;
  } else {

    cachelump[lump].locks += 1;
  }

  return cachelump[lump].cache;
}

void W_UnlockLumpNum(int lump) {
  if (cachelump[lump].locks == -1)
    return;

  cachelump[lump].locks -= 1;
  if (cachelump[lump].locks == 0)
    Z_ChangeTag(cachelump[lump].cache, PU_CACHE);
}

