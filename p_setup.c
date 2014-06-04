#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "g_game.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_enemy.h"
#include "s_sound.h"
#include "i_system.h"
#include "v_video.h"
#include "z_zone.h"

#define gNd2                            0x32644E67
#define gNd3                            0x33644E67
#define gNd4                            0x34644E67
#define gNd5                            0x35644E67
#define ZNOD                            0x444F4E5A
#define ZGLN                            0x4E4C475A
#define GL_VERT_OFFSET                  4
#define blkshift                        7
#define blkmask                         ((1 << blkshift) - 1)
#define blkmargin                       0

static int numvertexes;
static vertex_t *vertexes;
int numsegs;
seg_t *segs;
int numsectors;
sector_t *sectors;
int numsubsectors;
subsector_t *subsectors;
int numnodes;
node_t *nodes;
int numlines;
line_t *lines;
int numsides;
side_t *sides;
static int firstglvertex = 0;
static int nodesVersion = 0;

typedef struct linelist_t
{

    long num;
    struct linelist_t *next;

} linelist_t;

typedef struct
{

    unsigned short v1;
    unsigned short v2;
    unsigned short linedef;
    short side;
    unsigned short partner;

} glseg_t;

typedef struct
{

    fixed_t x,y;

} mapglvertex_t;

enum
{

   ML_GL_LABEL = 0,
   ML_GL_VERTS,
   ML_GL_SEGS,
   ML_GL_SSECT,
   ML_GL_NODES

};

int bmapwidth, bmapheight;
long *blockmap;
long *blockmaplump;
fixed_t bmaporgx, bmaporgy;
mobj_t **blocklinks;
static int rejectlump = -1;
const byte *rejectmatrix;
mapthing_t playerstarts[MAXPLAYERS];

static void P_AddToBox(fixed_t* box, fixed_t x, fixed_t y)
{

    if (x < box[BOXLEFT])
        box[BOXLEFT] = x;
    else if (x > box[BOXRIGHT])
        box[BOXRIGHT] = x;
    if (y < box[BOXBOTTOM])
        box[BOXBOTTOM] = y;
    else if (y > box[BOXTOP])
        box[BOXTOP] = y;

}

static boolean P_CheckForZDoomNodes(int lumpnum, int gl_lumpnum)
{

    const void *data;

    data = W_CacheLumpNum(lumpnum + ML_NODES);

    if (*(const int *)data == ZNOD)
        I_Error("P_CheckForZDoomNodes: ZDoom nodes not supported yet");

    data = W_CacheLumpNum(lumpnum + ML_SSECTORS);

    if (*(const int *)data == ZGLN)
        I_Error("P_CheckForZDoomNodes: ZDoom GL nodes not supported yet");

    return false;

}

static void P_GetNodesVersion(int lumpnum, int gl_lumpnum)
{

    const void *data;

    data = W_CacheLumpNum(gl_lumpnum + ML_GL_VERTS);

    if (gl_lumpnum > lumpnum)
    {

        if (*(const int *)data == gNd2)
        {

            data = W_CacheLumpNum(gl_lumpnum + ML_GL_SEGS);

            if (*(const int *)data == gNd3)
                nodesVersion = gNd3;
            else
                nodesVersion = gNd2;

        }
        
        if (*(const int *)data == gNd4)
            nodesVersion = gNd4;

        if (*(const int *)data == gNd5)
            nodesVersion = gNd5;

    }
    
    else
    {

        nodesVersion = 0;

        if (P_CheckForZDoomNodes(lumpnum, gl_lumpnum))
            I_Error("P_GetNodesVersion: ZDoom nodes not supported yet");

    }

}

static void P_LoadVertexes(int lump)
{

    const mapvertex_t *data;
    int i;

    numvertexes = W_LumpLength(lump) / sizeof (mapvertex_t);
    vertexes = Z_Malloc(numvertexes * sizeof (vertex_t), PU_LEVEL, 0);
    data = (const mapvertex_t *)W_CacheLumpNum(lump);

    for (i = 0; i < numvertexes; i++)
    {

        vertexes[i].x = data[i].x << FRACBITS;
        vertexes[i].y = data[i].y << FRACBITS;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadVertexes2(int lump, int gllump)
{

    const byte *gldata;
    int i;
    const mapvertex_t *ml;

    firstglvertex = W_LumpLength(lump) / sizeof (mapvertex_t);
    numvertexes = W_LumpLength(lump) / sizeof (mapvertex_t);

    if (gllump >= 0)
    {

        gldata = W_CacheLumpNum(gllump);

        if (nodesVersion == gNd2)
        {

            const mapglvertex_t *mgl;

            numvertexes += (W_LumpLength(gllump) - GL_VERT_OFFSET) / sizeof (mapglvertex_t);
            vertexes = Z_Malloc(numvertexes * sizeof (vertex_t), PU_LEVEL, 0);
            mgl = (const mapglvertex_t *)(gldata + GL_VERT_OFFSET);

            for (i = firstglvertex; i < numvertexes; i++)
            {

                vertexes[i].x = mgl->x;
                vertexes[i].y = mgl->y;
                mgl++;

            }

        }

        else
        {

            numvertexes += W_LumpLength(gllump) / sizeof (mapvertex_t);
            vertexes = Z_Malloc(numvertexes * sizeof (vertex_t), PU_LEVEL, 0);
            ml = (const mapvertex_t *)gldata;

            for (i = firstglvertex; i < numvertexes; i++)
            {

                vertexes[i].x = ml->x << FRACBITS;
                vertexes[i].y = ml->y << FRACBITS;
                ml++;

            }

        }

        W_UnlockLumpNum(gllump);

    }

    ml = (const mapvertex_t*)W_CacheLumpNum(lump);

    for (i = 0; i < firstglvertex; i++)
    {

        vertexes[i].x = ml->x << FRACBITS;
        vertexes[i].y = ml->y << FRACBITS;
        ml++;

    }

    W_UnlockLumpNum(lump);

}

static int checkGLVertex(int num)
{

    if (num & 0x8000)
        num = (num & 0x7FFF) + firstglvertex;

    return num;

}

static float GetDistance(int dx, int dy)
{

    float fx = (float)(dx) / FRACUNIT;
    float fy = (float)(dy) / FRACUNIT;

    return (float)sqrt(fx * fx + fy * fy);

}

static int GetOffset(vertex_t *v1, vertex_t *v2)
{

    float a, b;
    int r;

    a = (float)(v1->x - v2->x) / (float)FRACUNIT;
    b = (float)(v1->y - v2->y) / (float)FRACUNIT;
    r = (int)(sqrt(a*a+b*b) * (float)FRACUNIT);

    return r;

}

static void P_LoadSegs(int lump)
{

    int i;
    const mapseg_t *data;

    numsegs = W_LumpLength(lump) / sizeof (mapseg_t);
    segs = Z_Calloc(numsegs, sizeof (seg_t), PU_LEVEL, 0);
    data = (const mapseg_t *)W_CacheLumpNum(lump);

    if ((!data) || (!numsegs))
        I_Error("P_LoadSegs: no segs in level");

    for (i = 0; i < numsegs; i++)
    {

        seg_t *li = segs + i;
        const mapseg_t *ml = data + i;
        unsigned short v1, v2;
        int side, linedef;
        line_t *ldef;

        li->iSegID = i;
        v1 = (unsigned short)ml->v1;
        v2 = (unsigned short)ml->v2;
        li->v1 = &vertexes[v1];
        li->v2 = &vertexes[v2];
        li->miniseg = false;
        li->length  = GetDistance(li->v2->x - li->v1->x, li->v2->y - li->v1->y);
        li->angle = (ml->angle)<<16;
        li->offset =(ml->offset)<<16;
        linedef = (unsigned short)ml->linedef;
        ldef = &lines[linedef];
        li->linedef = ldef;
        side = ml->side;
        li->sidedef = &sides[ldef->sidenum[side]];

        if (ldef->sidenum[side] != NO_INDEX)
            li->frontsector = sides[ldef->sidenum[side]].sector;
        else
        {

            li->frontsector = 0;

            I_Error("P_LoadSegs: front of seg %i has no sidedef", i);

        }

        if (ldef->flags & ML_TWOSIDED && ldef->sidenum[side ^ 1] != NO_INDEX)
            li->backsector = sides[ldef->sidenum[side ^ 1]].sector;
        else
            li->backsector = 0;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadGLSegs(int lump)
{

    int i;
    const glseg_t *ml;
    line_t *ldef;

    numsegs = W_LumpLength(lump) / sizeof (glseg_t);
    segs = Z_Malloc(numsegs * sizeof (seg_t), PU_LEVEL, 0);

    memset(segs, 0, numsegs * sizeof (seg_t));

    ml = (const glseg_t*)W_CacheLumpNum(lump);

    if ((!ml) || (!numsegs))
        I_Error("P_LoadGLSegs: no glsegs in level");

    for (i = 0; i < numsegs; i++)
    {

        segs[i].v1 = &vertexes[checkGLVertex(ml->v1)];
        segs[i].v2 = &vertexes[checkGLVertex(ml->v2)];
        segs[i].iSegID = i;

        if (ml->linedef != (unsigned short) - 1)
        {

            ldef = &lines[ml->linedef];
            segs[i].linedef = ldef;
            segs[i].miniseg = false;
            segs[i].angle = R_PointToAngle2(segs[i].v1->x, segs[i].v1->y, segs[i].v2->x, segs[i].v2->y);
            segs[i].sidedef = &sides[ldef->sidenum[ml->side]];
            segs[i].length  = GetDistance(segs[i].v2->x - segs[i].v1->x, segs[i].v2->y - segs[i].v1->y);
            segs[i].frontsector = sides[ldef->sidenum[ml->side]].sector;

            if (ldef->flags & ML_TWOSIDED)
                segs[i].backsector = sides[ldef->sidenum[ml->side ^ 1]].sector;
            else
                segs[i].backsector = 0;

            if (ml->side)
                segs[i].offset = GetOffset(segs[i].v1, ldef->v2);
            else
                segs[i].offset = GetOffset(segs[i].v1, ldef->v1);

        }

        else
        {

            segs[i].miniseg = true;
            segs[i].angle = 0;
            segs[i].offset = 0;
            segs[i].length = 0;
            segs[i].linedef = NULL;
            segs[i].sidedef = NULL;
            segs[i].frontsector = NULL;
            segs[i].backsector = NULL;

        }

        ml++;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadSubsectors(int lump)
{

    const mapsubsector_t *data;
    int i;

    numsubsectors = W_LumpLength(lump) / sizeof (mapsubsector_t);
    subsectors = Z_Calloc(numsubsectors, sizeof (subsector_t), PU_LEVEL, 0);
    data = (const mapsubsector_t *)W_CacheLumpNum(lump);

    if ((!data) || (!numsubsectors))
        I_Error("P_LoadSubsectors: no subsectors in level");

    for (i = 0; i < numsubsectors; i++)
    {

        subsectors[i].numlines  = (unsigned short)data[i].numsegs;
        subsectors[i].firstline = (unsigned short)data[i].firstseg;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadSectors(int lump)
{

    const byte *data;
    int  i;

    numsectors = W_LumpLength(lump) / sizeof (mapsector_t);
    sectors = Z_Calloc(numsectors,sizeof (sector_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump);

    for (i = 0; i < numsectors; i++)
    {

        sector_t *ss = sectors + i;
        const mapsector_t *ms = (const mapsector_t *)data + i;

        ss->iSectorID = i;
        ss->floorheight = ms->floorheight << FRACBITS;
        ss->ceilingheight = ms->ceilingheight << FRACBITS;
        ss->floorpic = R_FlatNumForName(ms->floorpic);
        ss->ceilingpic = R_FlatNumForName(ms->ceilingpic);
        ss->lightlevel = ms->lightlevel;
        ss->special = ms->special;
        ss->oldspecial = ms->special;
        ss->tag = ms->tag;
        ss->thinglist = NULL;
        ss->touching_thinglist = NULL;
        ss->nextsec = -1;
        ss->prevsec = -1;
        ss->floor_xoffs = 0;
        ss->floor_yoffs = 0;
        ss->ceiling_xoffs = 0;
        ss->ceiling_yoffs = 0;
        ss->heightsec = -1;
        ss->floorlightsec = -1;
        ss->ceilinglightsec = -1;
        ss->bottommap = ss->midmap = ss->topmap = 0;
        ss->sky = 0;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadNodes(int lump)
{

    const byte *data;
    int  i;

    numnodes = W_LumpLength(lump) / sizeof (mapnode_t);
    nodes = Z_Malloc(numnodes * sizeof (node_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump);

    if ((!data) || (!numnodes))
    {

        if (numsubsectors != 1)
            I_Error("P_LoadNodes: no nodes in level");

    }

    for (i = 0; i < numnodes; i++)
    {

        node_t *no = nodes + i;
        const mapnode_t *mn = (const mapnode_t *)data + i;
        int j;

        no->x = mn->x << FRACBITS;
        no->y = mn->y << FRACBITS;
        no->dx = mn->dx << FRACBITS;
        no->dy = mn->dy << FRACBITS;

        for (j = 0; j < 2; j++)
        {

            int k;

            no->children[j] = mn->children[j];

            for (k = 0; k < 4; k++)
                no->bbox[j][k] = mn->bbox[j][k] << FRACBITS;

        }

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadThings(int lump)
{

    int i, numthings = W_LumpLength (lump) / sizeof (mapthing_t);
    const mapthing_t *data = W_CacheLumpNum(lump);

    if ((!data) || (!numthings))
        I_Error("P_LoadThings: no things in level");

    for (i = 0; i < numthings; i++)
    {

        mapthing_t mt = data[i];

        mt.x = mt.x;
        mt.y = mt.y;
        mt.angle = mt.angle;
        mt.type = mt.type;
        mt.options = mt.options;

        if (!P_IsDoomnumAllowed(mt.type))
            continue;

        P_SpawnMapThing(&mt);

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadLineDefs(int lump)
{

    const byte *data;
    int i;

    numlines = W_LumpLength(lump) / sizeof (maplinedef_t);
    lines = Z_Calloc(numlines, sizeof (line_t), PU_LEVEL, 0);
    data = W_CacheLumpNum(lump);

    for (i = 0; i < numlines; i++)
    {

        const maplinedef_t *mld = (const maplinedef_t *)data + i;
        line_t *ld = lines + i;
        vertex_t *v1, *v2;

        ld->flags = (unsigned short)mld->flags;
        ld->special = mld->special;
        ld->tag = mld->tag;
        v1 = ld->v1 = &vertexes[(unsigned short)mld->v1];
        v2 = ld->v2 = &vertexes[(unsigned short)mld->v2];
        ld->dx = v2->x - v1->x;
        ld->dy = v2->y - v1->y;
        ld->tranlump = -1;
        ld->slopetype = !ld->dx ? ST_VERTICAL : !ld->dy ? ST_HORIZONTAL : FixedDiv(ld->dy, ld->dx) > 0 ? ST_POSITIVE : ST_NEGATIVE;

        if (v1->x < v2->x)
        {

            ld->bbox[BOXLEFT] = v1->x;
            ld->bbox[BOXRIGHT] = v2->x;

        }

        else
        {

            ld->bbox[BOXLEFT] = v2->x;
            ld->bbox[BOXRIGHT] = v1->x;

        }

        if (v1->y < v2->y)
        {

            ld->bbox[BOXBOTTOM] = v1->y;
            ld->bbox[BOXTOP] = v2->y;

        }

        else
        {

            ld->bbox[BOXBOTTOM] = v2->y;
            ld->bbox[BOXTOP] = v1->y;

        }

        ld->soundorg.x = ld->bbox[BOXLEFT] / 2 + ld->bbox[BOXRIGHT] / 2;
        ld->soundorg.y = ld->bbox[BOXTOP] / 2 + ld->bbox[BOXBOTTOM] / 2;
        ld->iLineID = i;
        ld->sidenum[0] = mld->sidenum[0];
        ld->sidenum[1] = mld->sidenum[1];

        {

            int j;

            for (j = 0; j < 2; j++)
            {

                if (ld->sidenum[j] != NO_INDEX && ld->sidenum[j] >= numsides)
                {

                    ld->sidenum[j] = NO_INDEX;

                    I_Error("P_LoadLineDefs: linedef %d has out-of-range sidedef number", numlines - i - 1);

                }

            }
        
            if (ld->sidenum[0] == NO_INDEX)
            {

                ld->sidenum[0] = 0;

                I_Error("P_LoadLineDefs: linedef %d missing first sidedef", numlines - i - 1);

            }
        
            if ((ld->sidenum[1] == NO_INDEX) && (ld->flags & ML_TWOSIDED))
            {

                ld->flags &= ~ML_TWOSIDED;

                I_Error("P_LoadLineDefs: linedef %d has two-sided flag set, but no second sidedef", numlines - i - 1);

            }

        }

        if (ld->sidenum[0] != NO_INDEX && ld->special)
            sides[*ld->sidenum].special = ld->special;

    }

    W_UnlockLumpNum(lump);

}

static void P_LoadLineDefs2(int lump)
{

    int i = numlines;
    register line_t *ld = lines;

    for (; i--; ld++)
    {

        ld->frontsector = sides[ld->sidenum[0]].sector;
        ld->backsector = ld->sidenum[1] != NO_INDEX ? sides[ld->sidenum[1]].sector : 0;

        switch (ld->special)
        {

            int lump, j;

        case 260:
            lump = sides[*ld->sidenum].special;

            if (!ld->tag)
                ld->tranlump = lump;
            else
                for (j = 0; j < numlines; j++)
                    if (lines[j].tag == ld->tag)
                        lines[j].tranlump = lump;

            break;

        }

    }

}

static void P_LoadSideDefs(int lump)
{

    numsides = W_LumpLength(lump) / sizeof (mapsidedef_t);
    sides = Z_Calloc(numsides, sizeof (side_t), PU_LEVEL, 0);

}

static void P_LoadSideDefs2(int lump)
{

    const byte *data = W_CacheLumpNum(lump);
    int i;

    for (i = 0; i < numsides; i++)
    {

        register const mapsidedef_t *msd = (const mapsidedef_t *)data + i;
        register side_t *sd = sides + i;
        register sector_t *sec;

        sd->textureoffset = msd->textureoffset << FRACBITS;
        sd->rowoffset = msd->rowoffset << FRACBITS;

        {

            unsigned short sector_num = msd->sector;

            if (sector_num >= numsectors)
            {

                I_Error("P_LoadSideDefs2: sidedef %i has out-of-range sector num %u", i, sector_num);
                sector_num = 0;

            }

            sd->sector = sec = &sectors[sector_num];

        }

        switch (sd->special)
        {

        case 242:
            sd->bottomtexture = (sec->bottommap = R_ColormapNumForName(msd->bottomtexture)) < 0 ? sec->bottommap = 0, R_TextureNumForName(msd->bottomtexture): 0;
            sd->midtexture = (sec->midmap = R_ColormapNumForName(msd->midtexture)) < 0 ? sec->midmap = 0, R_TextureNumForName(msd->midtexture) : 0;
            sd->toptexture = (sec->topmap = R_ColormapNumForName(msd->toptexture)) < 0 ? sec->topmap = 0, R_TextureNumForName(msd->toptexture) : 0;

            break;

        case 260:
            sd->midtexture = strncasecmp("TRANMAP", msd->midtexture, 8) ? (sd->special = W_CheckNumForName(msd->midtexture, ns_global)) < 0 || W_LumpLength(sd->special) != 65536 ? sd->special = 0, R_TextureNumForName(msd->midtexture) : (sd->special++, 0) : (sd->special = 0);
            sd->toptexture = R_TextureNumForName(msd->toptexture);
            sd->bottomtexture = R_TextureNumForName(msd->bottomtexture);

            break;

        default:
            sd->midtexture = R_SafeTextureNumForName(msd->midtexture, i);
            sd->toptexture = R_SafeTextureNumForName(msd->toptexture, i);
            sd->bottomtexture = R_SafeTextureNumForName(msd->bottomtexture, i);

            break;

        }

    }

    W_UnlockLumpNum(lump);

}

static void AddBlockLine(linelist_t **lists, int *count, int *done, int blockno, long lineno)
{

    linelist_t *l;

    if (done[blockno])
        return;

    l = malloc(sizeof(linelist_t));
    l->num = lineno;
    l->next = lists[blockno];
    lists[blockno] = l;
    count[blockno]++;
    done[blockno] = 1;

}

static void P_CreateBlockMap(void)
{

    int xorg,yorg;
    int nrows,ncols;
    linelist_t **blocklists = NULL;
    int *blockcount = NULL;
    int *blockdone = NULL;
    int NBlocks;
    long linetotal = 0;
    int i, j;
    int map_minx = INT_MAX;
    int map_miny = INT_MAX;
    int map_maxx = INT_MIN;
    int map_maxy = INT_MIN;

    for (i = 0; i < numvertexes; i++)
    {

        fixed_t t;

        if ((t=vertexes[i].x) < map_minx)
            map_minx = t;
        else if (t > map_maxx)
            map_maxx = t;

        if ((t=vertexes[i].y) < map_miny)
            map_miny = t;
        else if (t > map_maxy)
            map_maxy = t;

    }

    map_minx >>= FRACBITS;
    map_maxx >>= FRACBITS;
    map_miny >>= FRACBITS;
    map_maxy >>= FRACBITS;
    xorg = map_minx - blkmargin;
    yorg = map_miny - blkmargin;
    ncols = (map_maxx + blkmargin - xorg + 1 + blkmask) >> blkshift;
    nrows = (map_maxy + blkmargin - yorg + 1 + blkmask) >> blkshift;
    NBlocks = ncols * nrows;
    blocklists = calloc(NBlocks,sizeof (linelist_t *));
    blockcount = calloc(NBlocks,sizeof (int));
    blockdone = malloc(NBlocks * sizeof (int));

    for (i = 0; i < NBlocks; i++)
    {

        blocklists[i] = malloc(sizeof(linelist_t));
        blocklists[i]->num = -1;
        blocklists[i]->next = NULL;
        blockcount[i]++;

    }

    for (i = 0; i < numlines; i++)
    {

        int x1 = lines[i].v1->x >> FRACBITS;
        int y1 = lines[i].v1->y >> FRACBITS;
        int x2 = lines[i].v2->x >> FRACBITS;
        int y2 = lines[i].v2->y >> FRACBITS;
        int dx = x2 - x1;
        int dy = y2 - y1;
        int vert = !dx;
        int horiz = !dy;
        int spos = (dx^dy) > 0;
        int sneg = (dx^dy) < 0;
        int bx,by;
        int minx = x1 > x2 ? x2 : x1;
        int maxx = x1 > x2 ? x1 : x2;
        int miny = y1 > y2 ? y2 : y1;
        int maxy = y1 > y2 ? y1 : y2;

        memset(blockdone, 0, NBlocks * sizeof (int));

        bx = (x1 - xorg) >> blkshift;
        by = (y1 - yorg) >> blkshift;

        AddBlockLine(blocklists, blockcount, blockdone, by * ncols + bx, i);

        bx = (x2 - xorg) >> blkshift;
        by = (y2 - yorg) >> blkshift;
        AddBlockLine(blocklists, blockcount, blockdone, by * ncols + bx, i);

        if (!vert)
        {

            for (j = 0; j < ncols; j++)
            {

                int x = xorg + (j << blkshift);
                int y = (dy * (x - x1)) / dx + y1;
                int yb = (y - yorg) >> blkshift;
                int yp = (y - yorg) & blkmask;

                if (yb < 0 || yb > nrows - 1)
                    continue;

                if (x < minx || x > maxx)
                    continue;

                AddBlockLine(blocklists, blockcount, blockdone, ncols * yb + j, i);

                if (yp == 0)
                {

                    if (sneg)
                    {

                        if (yb > 0 && miny < y)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * (yb - 1) + j, i);

                        if (j > 0 && minx < x)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * yb + j - 1, i);
                    }

                    else if (spos)
                    {

                        if (yb > 0 && j > 0 && minx < x)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * (yb - 1) + j - 1, i);

                    }

                    else if (horiz)
                    {

                        if (j > 0 && minx < x)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * yb + j - 1, i);

                    }

                }

                else if (j > 0 && minx < x)
                {

                    AddBlockLine(blocklists, blockcount, blockdone, ncols * yb + j - 1, i);

                }

            }

        }

        if (!horiz)
        {

            for (j = 0; j < nrows; j++)
            {

                int y = yorg + (j << blkshift);
                int x = (dx * (y - y1)) / dy + x1;
                int xb = (x - xorg) >> blkshift;
                int xp = (x - xorg) & blkmask;

                if (xb < 0 || xb > ncols - 1)
                    continue;

                if (y < miny || y > maxy)
                    continue;

                AddBlockLine(blocklists, blockcount, blockdone, ncols * j + xb, i);

                if (xp == 0)
                {

                    if (sneg)
                    {

                        if (j > 0 && miny < y)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * (j - 1) + xb, i);

                        if (xb > 0 && minx < x)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * j + xb - 1, i);

                    }

                    else if (vert)
                    {

                        if (j > 0 && miny < y)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * (j - 1) + xb, i);

                    }

                    else if (spos)
                    {

                        if (xb > 0 && j > 0 && miny < y)
                            AddBlockLine(blocklists, blockcount, blockdone, ncols * (j - 1) + xb - 1, i);

                    }

                }

                else if (j > 0 && miny < y)
                {

                    AddBlockLine(blocklists, blockcount, blockdone, ncols * (j - 1) + xb, i);

                }

            }

        }

    }

    memset(blockdone, 0, NBlocks * sizeof (int));

    for (i = 0, linetotal = 0; i < NBlocks; i++)
    {

        AddBlockLine(blocklists, blockcount, blockdone, i, 0);

        linetotal += blockcount[i];

    }

    blockmaplump = Z_Malloc(sizeof (*blockmaplump) * (4 + NBlocks + linetotal), PU_LEVEL, 0);
    blockmaplump[0] = bmaporgx = xorg << FRACBITS;
    blockmaplump[1] = bmaporgy = yorg << FRACBITS;
    blockmaplump[2] = bmapwidth  = ncols;
    blockmaplump[3] = bmapheight = nrows;

    for (i = 0; i < NBlocks; i++)
    {

        linelist_t *bl = blocklists[i];
        long offs = blockmaplump[4 + i] = (i ? blockmaplump[4 + i - 1] : 4 + NBlocks) + (i ? blockcount[i - 1] : 0);

        while (bl)
        {

            linelist_t *tmp = bl->next;
            blockmaplump[offs++] = bl->num;

            free(bl);

            bl = tmp;

        }

    }

    free(blocklists);
    free(blockcount);
    free(blockdone);

}

static void P_LoadBlockMap(int lump)
{

    long count;

    if (W_LumpLength(lump) < 8 || (count = W_LumpLength(lump) / 2) >= 0x10000)
    {

        P_CreateBlockMap();

    }

    else
    {

        long i;
        const short *wadblockmaplump = W_CacheLumpNum(lump);

        blockmaplump = Z_Malloc(sizeof(*blockmaplump) * count, PU_LEVEL, 0);
        blockmaplump[0] = wadblockmaplump[0];
        blockmaplump[1] = wadblockmaplump[1];
        blockmaplump[2] = (long)(wadblockmaplump[2]) & 0xffff;
        blockmaplump[3] = (long)(wadblockmaplump[3]) & 0xffff;

        for (i = 4; i < count; i++)
        {

            short t = wadblockmaplump[i];

            blockmaplump[i] = t == -1 ? -1l : (long)t & 0xffff;

        }

        W_UnlockLumpNum(lump);

        bmaporgx = blockmaplump[0] << FRACBITS;
        bmaporgy = blockmaplump[1] << FRACBITS;
        bmapwidth = blockmaplump[2];
        bmapheight = blockmaplump[3];

    }

    blocklinks = Z_Calloc(bmapwidth * bmapheight, sizeof (*blocklinks), PU_LEVEL, 0);
    blockmap = blockmaplump + 4;

}

static void P_LoadReject(int lumpnum, int totallines)
{

    unsigned int length, required;
    byte *newreject;

    if (rejectlump != -1)
        W_UnlockLumpNum(rejectlump);

    rejectlump = lumpnum + ML_REJECT;
    rejectmatrix = W_CacheLumpNum(rejectlump);
    required = (numsectors * numsectors + 7) / 8;
    length = W_LumpLength(rejectlump);

    if (length >= required)
        return;

    newreject = Z_Malloc(required, PU_LEVEL, NULL);

    rejectmatrix = (const byte *)memmove(newreject, rejectmatrix, length);

    memset(newreject + length, 0, required - length);
    W_UnlockLumpNum(rejectlump);

    rejectlump = -1;

    I_Error("P_LoadReject: REJECT too short (%u<%u) - padded", length, required);

}

static void P_AddLineToSector(line_t *li, sector_t *sector)
{

    sector->lines[sector->linecount++] = li;

    P_AddToBox(sector->blockbox, li->v1->x, li->v1->y);
    P_AddToBox(sector->blockbox, li->v2->x, li->v2->y);

}

static int P_GroupLines(void)
{

    register line_t *li;
    register sector_t *sector;
    int i, j, total = numlines;

    for (i = 0; i < numsubsectors; i++)
    {

        seg_t *seg = &segs[subsectors[i].firstline];

        subsectors[i].sector = NULL;

        for (j = 0; j < subsectors[i].numlines; j++)
        {

            if (seg->sidedef)
            {

                subsectors[i].sector = seg->sidedef->sector;

                break;

            }

            seg++;

        }

        if (subsectors[i].sector == NULL)
            I_Error("P_GroupLines: Subsector a part of no sector!");

    }

    for (i = 0, li = lines; i < numlines; i++, li++)
    {

        li->frontsector->linecount++;

        if (li->backsector && li->backsector != li->frontsector)
        {

            li->backsector->linecount++;
            total++;

        }

    }

    {

        line_t **linebuffer = Z_Malloc(total * sizeof (line_t *), PU_LEVEL, 0);

        for (i = 0, sector = sectors; i < numsectors; i++, sector++)
        {

            sector->lines = linebuffer;
            linebuffer += sector->linecount;
            sector->linecount = 0;
            sector->blockbox[BOXTOP] = sector->blockbox[BOXRIGHT] = INT_MIN;
            sector->blockbox[BOXBOTTOM] = sector->blockbox[BOXLEFT] = INT_MAX;

        }

    }

    for (i = 0, li = lines; i < numlines; i++, li++)
    {

        P_AddLineToSector(li, li->frontsector);

        if (li->backsector && li->backsector != li->frontsector)
            P_AddLineToSector(li, li->backsector);

    }

    for (i = 0, sector = sectors; i < numsectors; i++, sector++)
    {

        fixed_t *bbox = (void *)sector->blockbox;
        int block;

        sector->soundorg.x = bbox[BOXRIGHT] / 2 + bbox[BOXLEFT] / 2;
        sector->soundorg.y = bbox[BOXTOP] / 2 + bbox[BOXBOTTOM] / 2;
        block = (bbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapheight ? bmapheight - 1 : block;
        sector->blockbox[BOXTOP] = block;
        block = (bbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXBOTTOM] = block;
        block = (bbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block >= bmapwidth ? bmapwidth - 1 : block;
        sector->blockbox[BOXRIGHT] = block;
        block = (bbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
        block = block < 0 ? 0 : block;
        sector->blockbox[BOXLEFT] = block;

    }

    return total;

}

static void P_RemoveSlimeTrails(void)
{

    byte *hit = calloc(1, numvertexes);
    int i;

    for (i = 0; i < numsegs; i++)
    {

        const line_t *l;

        if (segs[i].miniseg == true)
            goto error;

        l = segs[i].linedef;

        if (l->dx && l->dy)
        {

            vertex_t *v = segs[i].v1;

            do
            {

                if (!hit[v - vertexes])
                {

                    hit[v - vertexes] = 1;

                    if (v != l->v1 && v != l->v2)
                    {

                        int_64_t dx2 = (l->dx >> FRACBITS) * (l->dx >> FRACBITS);
                        int_64_t dy2 = (l->dy >> FRACBITS) * (l->dy >> FRACBITS);
                        int_64_t dxy = (l->dx >> FRACBITS) * (l->dy >> FRACBITS);
                        int_64_t s = dx2 + dy2;
                        int x0 = v->x, y0 = v->y, x1 = l->v1->x, y1 = l->v1->y;
                        v->x = (int)((dx2 * x0 + dy2 * x1 + dxy * (y0 - y1)) / s);
                        v->y = (int)((dy2 * y0 + dx2 * y1 + dxy * (x0 - x1)) / s);

                    }

                }

            }  while ((v != segs[i].v2) && (v = segs[i].v2));

        }

    }

error:
    free(hit);

}

void P_SetupLevel(int episode, int map, int playermask, skill_t skill)
{

    int i;
    char lumpname[9];
    int lumpnum;
    char gl_lumpname[9];
    int gl_lumpnum;

    totallive = totalkills = totalitems = totalsecret = wminfo.maxfrags = 0;
    wminfo.partime = 180;

    for (i = 0; i < MAXPLAYERS; i++)
        players[i].killcount = players[i].secretcount = players[i].itemcount = 0;

    players[consoleplayer].viewz = 1;

    S_Start();
    Z_FreeTags(PU_LEVEL, PU_CACHE - 1);

    if (rejectlump != -1)
    {

        W_UnlockLumpNum(rejectlump);

        rejectlump = -1;

    }

    P_InitThinkers();

    if (gamemode == commercial)
    {

        sprintf(lumpname, "map%02d", map);
        sprintf(gl_lumpname, "gl_map%02d", map);

    }

    else
    {

        sprintf(lumpname, "E%dM%d", episode, map);
        sprintf(gl_lumpname, "GL_E%iM%i", episode, map);

    }

    lumpnum = W_GetNumForName(lumpname);
    gl_lumpnum = W_CheckNumForName(gl_lumpname, ns_global);
    leveltime = 0; totallive = 0;

    if ((i = lumpnum + ML_BLOCKMAP + 1) < numlumps && !strncasecmp(lumpinfo[i].name, "BEHAVIOR", 8))
        I_Error("P_SetupLevel: %s: Hexen format not supported", lumpname);

    P_GetNodesVersion(lumpnum,gl_lumpnum);

    if (nodesVersion > 0)
        P_LoadVertexes2(lumpnum + ML_VERTEXES, gl_lumpnum + ML_GL_VERTS);
    else
        P_LoadVertexes(lumpnum + ML_VERTEXES);

    P_LoadSectors(lumpnum + ML_SECTORS);
    P_LoadSideDefs(lumpnum + ML_SIDEDEFS);
    P_LoadLineDefs(lumpnum + ML_LINEDEFS);
    P_LoadSideDefs2(lumpnum + ML_SIDEDEFS);
    P_LoadLineDefs2(lumpnum + ML_LINEDEFS);
    P_LoadBlockMap(lumpnum + ML_BLOCKMAP);

    if (nodesVersion > 0)
    {

        P_LoadSubsectors(gl_lumpnum + ML_GL_SSECT);
        P_LoadNodes(gl_lumpnum + ML_GL_NODES);
        P_LoadGLSegs(gl_lumpnum + ML_GL_SEGS);

    }

    else
    {

        P_LoadSubsectors(lumpnum + ML_SSECTORS);
        P_LoadNodes(lumpnum + ML_NODES);
        P_LoadSegs(lumpnum + ML_SEGS);

    }

    P_LoadReject(lumpnum, P_GroupLines());
    P_RemoveSlimeTrails();

    memset(playerstarts, 0, sizeof (playerstarts));

    for (i = 0; i < MAXPLAYERS; i++)
        players[i].mo = NULL;

    P_MapStart();
    P_LoadThings(lumpnum + ML_THINGS);

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && !players[i].mo)
            I_Error("P_SetupLevel: missing player %d start", i + 1);

    if (gamemode == commercial)
        P_SpawnBrainTargets();

    P_SpawnSpecials();
    P_MapEnd();

}


void P_Init (void)
{

    P_InitSwitchList();
    P_InitPicAnims();
    R_InitSprites(sprnames);

}

