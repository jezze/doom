#ifndef __D_ITEMS__
#define __D_ITEMS__

#include "doomdef.h"

typedef struct
{

    ammotype_t  ammo;
    int upstate;
    int downstate;
    int readystate;
    int atkstate;
    int flashstate;

} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS];

#endif
