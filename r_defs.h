#ifndef __R_DEFS__
#define __R_DEFS__

#include "doomdef.h"
#include "m_fixed.h"
#include "p_mobj.h"

#define SIL_NONE                        0
#define SIL_BOTTOM                      1
#define SIL_TOP                         2
#define SIL_BOTH                        3
#define MAXDRAWSEGS                     256

typedef struct
{

    int x, y;

} vertex_t;

typedef struct
{

    thinker_t thinker;
    int x, y, z;

} degenmobj_t;

typedef struct
{

    int iSectorID;
    boolean no_toptextures;
    boolean no_bottomtextures;
    int floorheight;
    int ceilingheight;
    int nexttag, firsttag;
    int soundtraversed;
    mobj_t *soundtarget;
    int blockbox[4];
    degenmobj_t soundorg;
    int validcount;
    mobj_t *thinglist;
    int friction, movefactor;
    void *floordata;
    void *ceilingdata;
    void *lightingdata;
    int stairlock;
    int prevsec;
    int nextsec;
    int heightsec;
    int bottommap, midmap, topmap;
    struct msecnode_s *touching_thinglist;
    int linecount;
    struct line_s **lines;
    int sky;
    int floor_xoffs, floor_yoffs;
    int ceiling_xoffs, ceiling_yoffs;
    int floorlightsec, ceilinglightsec;
    short floorpic;
    short ceilingpic;
    short lightlevel;
    short special;
    short oldspecial;
    short tag;

} sector_t;

typedef struct
{

    int textureoffset;
    int rowoffset;
    short toptexture;
    short bottomtexture;
    short midtexture;
    sector_t *sector;
    int special;

} side_t;

typedef enum
{

    ST_HORIZONTAL,
    ST_VERTICAL,
    ST_POSITIVE,
    ST_NEGATIVE

} slopetype_t;

typedef struct line_s
{

    int iLineID;
    vertex_t *v1, *v2;
    int dx, dy;
    unsigned short flags;
    short special;
    short tag;
    unsigned short sidenum[2];
    int bbox[4];
    slopetype_t slopetype;
    sector_t *frontsector;
    sector_t *backsector;
    int validcount;
    void *specialdata;
    int tranlump;
    int firsttag,nexttag;
    int r_validcount;
    enum
    {

    RF_TOP_TILE = 1,
    RF_MID_TILE = 2,
    RF_BOT_TILE = 4,
    RF_IGNORE = 8,
    RF_CLOSED = 16

    } r_flags;
    degenmobj_t soundorg;

} line_t;

typedef struct msecnode_s
{

    sector_t *m_sector;
    struct mobj_s *m_thing;
    struct msecnode_s *m_tprev;
    struct msecnode_s *m_tnext;
    struct msecnode_s *m_sprev;
    struct msecnode_s *m_snext;
    boolean visited;

} msecnode_t;

typedef struct
{

    vertex_t *v1, *v2;
    int offset;
    angle_t angle;
    side_t *sidedef;
    line_t *linedef;
    int iSegID;
    float length;
    boolean miniseg;
    sector_t *frontsector, *backsector;

} seg_t;

typedef struct subsector_s
{

    sector_t *sector;
    unsigned short numlines, firstline;

} subsector_t;

typedef struct
{

    int x, y, dx, dy;
    int bbox[2][4];
    unsigned short children[2];

} node_t;

typedef byte  lighttable_t;

typedef struct drawseg_s
{

    seg_t *curline;
    int x1, x2;
    int scale1, scale2, scalestep;
    int silhouette;
    int bsilheight;
    int tsilheight;
    int rw_offset, rw_distance, rw_centerangle; 
    int *sprtopclip, *sprbottomclip, *maskedtexturecol;

} drawseg_t;

typedef struct
{

    int width,height;
    int leftoffset,topoffset;
    int lumpnum;

} patchnum_t;

typedef struct vissprite_s
{

    mobj_t *thing;
    boolean flip;
    int x1, x2;
    int gx, gy;
    int gz, gzt;
    int startfrac;
    int scale;
    int xiscale;
    int texturemid;
    int patch;
    uint_64_t mobjflags;
    const lighttable_t *colormap;
    int heightsec;
    boolean isplayersprite;

} vissprite_t;

typedef struct
{

    boolean rotate;
    short lump[8];
    byte flip[8];

} spriteframe_t;

typedef struct
{

    int numframes;
    spriteframe_t *spriteframes;

} spritedef_t;

typedef struct visplane
{

    struct visplane *next;
    int picnum, lightlevel, minx, maxx;
    int height;
    int xoffs, yoffs;
    unsigned int pad1;
    unsigned int top[MAX_SCREENWIDTH];
    unsigned int pad2;
    unsigned int pad3;
    unsigned int bottom[MAX_SCREENWIDTH];
    unsigned int pad4;

} visplane_t;

#endif
