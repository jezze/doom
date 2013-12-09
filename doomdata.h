#ifndef __DOOMDATA__
#define __DOOMDATA__

#include "doomtype.h"

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

#endif
