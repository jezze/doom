#include "doomtype.h"
#include "info.h"
#include "d_items.h"

weaponinfo_t    weaponinfo[NUMWEAPONS] =
{
  {

    am_noammo,
    S_PUNCHUP,
    S_PUNCHDOWN,
    S_PUNCH,
    S_PUNCH1,
    S_NULL
  },
  {

    am_clip,
    S_PISTOLUP,
    S_PISTOLDOWN,
    S_PISTOL,
    S_PISTOL1,
    S_PISTOLFLASH
  },
  {

    am_shell,
    S_SGUNUP,
    S_SGUNDOWN,
    S_SGUN,
    S_SGUN1,
    S_SGUNFLASH1
  },
  {

    am_clip,
    S_CHAINUP,
    S_CHAINDOWN,
    S_CHAIN,
    S_CHAIN1,
    S_CHAINFLASH1
  },
  {

    am_misl,
    S_MISSILEUP,
    S_MISSILEDOWN,
    S_MISSILE,
    S_MISSILE1,
    S_MISSILEFLASH1
  },
  {

    am_cell,
    S_PLASMAUP,
    S_PLASMADOWN,
    S_PLASMA,
    S_PLASMA1,
    S_PLASMAFLASH1
  },
  {

    am_cell,
    S_BFGUP,
    S_BFGDOWN,
    S_BFG,
    S_BFG1,
    S_BFGFLASH1
  },
  {

    am_noammo,
    S_SAWUP,
    S_SAWDOWN,
    S_SAW,
    S_SAW1,
    S_NULL
  },
  {

    am_shell,
    S_DSGUNUP,
    S_DSGUNDOWN,
    S_DSGUN,
    S_DSGUN1,
    S_DSGUNFLASH1
  },
};
