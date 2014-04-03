#include "d_think.h"
#include "doomstat.h"
#include "doomtype.h"
#include "w_wad.h"
#include "z_zone.h"
#include "i_system.h"

static struct
{

    void *cache;
    unsigned int locks;

} *cachelump;

void W_InitCache(void)
{

    cachelump = calloc(sizeof *cachelump, numlumps);

    if (!cachelump)
        I_Error ("W_Init: Couldn't allocate lumpcache");

}

const void *W_CacheLumpNum(int lump)
{

    const int locks = 1;

    if (!cachelump[lump].cache)
        W_ReadLump(lump, Z_Malloc(W_LumpLength(lump), PU_CACHE, &cachelump[lump].cache));

    if (!cachelump[lump].locks && locks)
        Z_ChangeTag(cachelump[lump].cache, PU_STATIC);

    cachelump[lump].locks += locks;

    return cachelump[lump].cache;

}

const void *W_LockLumpNum(int lump)
{

    return W_CacheLumpNum(lump);

}

void W_UnlockLumpNum(int lump)
{

    const int unlocks = 1;

    cachelump[lump].locks -= unlocks;

    if (unlocks && !cachelump[lump].locks)
        Z_ChangeTag(cachelump[lump].cache, PU_CACHE);

}

