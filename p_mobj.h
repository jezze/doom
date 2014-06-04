#ifndef __P_MOBJ__
#define __P_MOBJ__

#include "tables.h"
#include "m_fixed.h"
#include "info.h"

#define NF_SUBSECTOR                    0x8000
#define NO_INDEX                        ((unsigned short)-1)
#define ML_BLOCKING                     1
#define ML_BLOCKMONSTERS                2
#define ML_TWOSIDED                     4
#define ML_DONTPEGTOP                   8
#define ML_DONTPEGBOTTOM                16
#define ML_SECRET                       32
#define ML_SOUNDBLOCK                   64
#define ML_DONTDRAW                     128
#define ML_MAPPED                       256
#define ML_PASSUSE                      512
#define MF_SPECIAL      (uint_64_t)(0x0000000000000001)
#define MF_SOLID        (uint_64_t)(0x0000000000000002)
#define MF_SHOOTABLE    (uint_64_t)(0x0000000000000004)
#define MF_NOSECTOR     (uint_64_t)(0x0000000000000008)
#define MF_NOBLOCKMAP   (uint_64_t)(0x0000000000000010)
#define MF_AMBUSH       (uint_64_t)(0x0000000000000020)
#define MF_JUSTHIT      (uint_64_t)(0x0000000000000040)
#define MF_JUSTATTACKED (uint_64_t)(0x0000000000000080)
#define MF_SPAWNCEILING (uint_64_t)(0x0000000000000100)
#define MF_NOGRAVITY    (uint_64_t)(0x0000000000000200)
#define MF_DROPOFF      (uint_64_t)(0x0000000000000400)
#define MF_PICKUP       (uint_64_t)(0x0000000000000800)
#define MF_NOCLIP       (uint_64_t)(0x0000000000001000)
#define MF_SLIDE        (uint_64_t)(0x0000000000002000)
#define MF_FLOAT        (uint_64_t)(0x0000000000004000)
#define MF_TELEPORT     (uint_64_t)(0x0000000000008000)
#define MF_MISSILE      (uint_64_t)(0x0000000000010000)
#define MF_DROPPED      (uint_64_t)(0x0000000000020000)
#define MF_SHADOW       (uint_64_t)(0x0000000000040000)
#define MF_NOBLOOD      (uint_64_t)(0x0000000000080000)
#define MF_CORPSE       (uint_64_t)(0x0000000000100000)
#define MF_INFLOAT      (uint_64_t)(0x0000000000200000)
#define MF_COUNTKILL    (uint_64_t)(0x0000000000400000)
#define MF_COUNTITEM    (uint_64_t)(0x0000000000800000)
#define MF_SKULLFLY     (uint_64_t)(0x0000000001000000)
#define MF_NOTDMATCH    (uint_64_t)(0x0000000002000000)
#define MF_TRANSLATION  (uint_64_t)(0x000000000c000000)
#define MF_TRANSLATION1 (uint_64_t)(0x0000000004000000)
#define MF_TRANSLATION2 (uint_64_t)(0x0000000008000000)
#define MF_TRANSSHIFT 26
#define MF_UNUSED2      (uint_64_t)(0x0000000010000000)
#define MF_UNUSED3      (uint_64_t)(0x0000000020000000)
#define MF_TOUCHY          LONGLONG(0x0000000100000000)
#define MF_BOUNCES         LONGLONG(0x0000000200000000)
#define MF_FRIEND          LONGLONG(0x0000000400000000)

enum
{

    ML_LABEL,
    ML_THINGS,
    ML_LINEDEFS,
    ML_SIDEDEFS,
    ML_VERTEXES,
    ML_SEGS,
    ML_SSECTORS,
    ML_NODES,
    ML_SECTORS,
    ML_REJECT,
    ML_BLOCKMAP

};

typedef struct
{

    short x,y;

} __attribute__((packed)) mapvertex_t;

typedef struct
{

    short textureoffset;
    short rowoffset;
    char toptexture[8];
    char bottomtexture[8];
    char midtexture[8];
    short sector;

} __attribute__((packed)) mapsidedef_t;

typedef struct
{

    unsigned short v1;
    unsigned short v2;
    unsigned short flags;
    short special;
    short tag;
    unsigned short sidenum[2];

} __attribute__((packed)) maplinedef_t;

typedef struct
{

    short floorheight;
    short ceilingheight;
    char floorpic[8];
    char ceilingpic[8];
    short lightlevel;
    short special;
    short tag;

} __attribute__((packed)) mapsector_t;

typedef struct
{

    unsigned short numsegs;
    unsigned short firstseg;

} __attribute__((packed)) mapsubsector_t;

typedef struct
{

    unsigned short v1;
    unsigned short v2;
    short angle;
    unsigned short linedef;
    short side;
    short offset;

} __attribute__((packed)) mapseg_t;

typedef struct
{

  short x;
  short y;
  short dx;
  short dy;
  short bbox[2][4];
  unsigned short children[2];

} __attribute__((packed)) mapnode_t;

typedef struct
{

    short x;
    short y;
    short angle;
    short type;
    short options;

} __attribute__((packed)) mapthing_t;

enum
{

    MIF_FALLING = 1,
    MIF_ARMED = 2

};

typedef struct mobj_s
{

    thinker_t           thinker;
    fixed_t             x;
    fixed_t             y;
    fixed_t             z;
    struct mobj_s*      snext;
    struct mobj_s**     sprev;
    angle_t             angle;
    spritenum_t         sprite;
    int                 frame;
    struct mobj_s*      bnext;
    struct mobj_s**     bprev;
    struct subsector_s* subsector;
    fixed_t             floorz;
    fixed_t             ceilingz;
    fixed_t             dropoffz;
    fixed_t             radius;
    fixed_t             height;
    fixed_t             momx;
    fixed_t             momy;
    fixed_t             momz;
    int                 validcount;
    mobjtype_t          type;
    mobjinfo_t*         info;
    int                 tics;
    state_t*            state;
    uint_64_t           flags;
    int                 intflags;
    int                 health;
    short               movedir;
    short               movecount;
    short               strafecount;
    struct mobj_s*      target;
    short               reactiontime;
    short               threshold;
    short               pursuecount;
    short               gear;
    struct player_s*    player;
    short               lastlook;
    mapthing_t          spawnpoint;
    struct mobj_s*      tracer;
    struct mobj_s*      lastenemy;
    int friction;
    int movefactor;
    struct msecnode_s* touching_sectorlist;
    fixed_t             PrevX;
    fixed_t             PrevY;
    fixed_t             PrevZ;
    fixed_t             pad;

} mobj_t;

#define VIEWHEIGHT      (41*FRACUNIT)
#define GRAVITY         FRACUNIT
#define MAXMOVE         (30*FRACUNIT)
#define ONFLOORZ        INT_MIN
#define ONCEILINGZ      INT_MAX
#define ITEMQUESIZE     128
#define FLOATSPEED      (FRACUNIT*4)
#define STOPSPEED       (FRACUNIT/16)
#define OVERDRIVE 6
#define MAXGEAR (OVERDRIVE+16)
#define sentient(mobj) ((mobj)->health > 0 && (mobj)->info->seestate)

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type);
void P_RemoveMobj(mobj_t *th);
boolean P_SetMobjState(mobj_t *mobj, statenum_t state);
void P_MobjThinker(mobj_t *mobj);
void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z);
void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage);
mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type);
void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type);
boolean P_IsDoomnumAllowed(int doomnum);
void P_SpawnMapThing (const mapthing_t*  mthing);
void P_SpawnPlayer(int n, const mapthing_t *mthing);
void P_CheckMissileSpawn(mobj_t*);
void P_ExplodeMissile(mobj_t*);

#endif
