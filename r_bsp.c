#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_bsp.h"
#include "v_video.h"
#include "p_map.h"
#include "i_video.h"

static const int checkcoord[12][4] = {
    {3, 0, 2, 1},
    {3, 0, 2, 0},
    {3, 1, 2, 0},
    {0},
    {2, 0, 2, 1},
    {0, 0, 0, 0},
    {3, 1, 3, 0},
    {0},
    {2, 0, 3, 1},
    {2, 1, 3, 1},
    {2, 1, 3, 0}
};

seg_t *curline;
side_t *sidedef;
line_t *linedef;
sector_t *frontsector;
sector_t *backsector;
drawseg_t *ds_p;
drawseg_t *drawsegs;
unsigned maxdrawsegs;
byte solidcol[MAX_SCREENWIDTH];

void R_ClearDrawSegs(void)
{

    ds_p = drawsegs;

}

static void R_ClipWallSegment(int first, int last, boolean solid)
{

    byte *p;

    while (first < last)
    {

        if (solidcol[first])
        {

            if (!(p = memchr(solidcol + first, 0, last - first)))
                return;

            first = p - solidcol;

        }
        
        else
        {

            int to;

            if (!(p = memchr(solidcol + first, 1, last - first)))
                to = last;
            else
                to = p - solidcol;

            R_StoreWallRange(first, to - 1);

            if (solid)
                memset(solidcol + first, 1, to - first);

            first = to;

        }

    }

}

void R_ClearClipSegs(void)
{

    memset(solidcol, 0, SCREENWIDTH);

}

static void R_RecalcLineFlags(void)
{

    linedef->r_validcount = gametic;

    if (!(linedef->flags & ML_TWOSIDED) || backsector->ceilingheight <= frontsector->floorheight || backsector->floorheight >= frontsector->ceilingheight || (backsector->ceilingheight <= backsector->floorheight && (backsector->ceilingheight >= frontsector->ceilingheight || curline->sidedef->toptexture) && (backsector->floorheight <= frontsector->floorheight || curline->sidedef->bottomtexture) && (backsector->ceilingpic != skyflatnum || frontsector->ceilingpic != skyflatnum)))
    {

        linedef->r_flags = RF_CLOSED;

    }

    else
    {

        if (backsector->ceilingheight != frontsector->ceilingheight || backsector->floorheight != frontsector->floorheight || curline->sidedef->midtexture || memcmp(&backsector->floor_xoffs, &frontsector->floor_xoffs, sizeof(frontsector->floor_xoffs) + sizeof(frontsector->floor_yoffs) + sizeof(frontsector->ceiling_xoffs) + sizeof(frontsector->ceiling_yoffs) + sizeof(frontsector->ceilingpic) + sizeof(frontsector->floorpic) + sizeof(frontsector->lightlevel) + sizeof(frontsector->floorlightsec) + sizeof(frontsector->ceilinglightsec)))
        {

            linedef->r_flags = 0;
            
            return;

        }
        else
        {

            linedef->r_flags = RF_IGNORE;

        }

    }

    if (curline->sidedef->rowoffset)
        return;

    if (linedef->flags & ML_TWOSIDED)
    {

        int c;

        if ((c = frontsector->ceilingheight - backsector->ceilingheight) > 0 && (textureheight[texturetranslation[curline->sidedef->toptexture]] > c))
            linedef->r_flags |= RF_TOP_TILE;

        if ((c = frontsector->floorheight - backsector->floorheight) > 0 && (textureheight[texturetranslation[curline->sidedef->bottomtexture]] > c))
            linedef->r_flags |= RF_BOT_TILE;

    }
    
    else
    {

        int c;

        if ((c = frontsector->ceilingheight - frontsector->floorheight) > 0 && (textureheight[texturetranslation[curline->sidedef->midtexture]] > c))
            linedef->r_flags |= RF_MID_TILE;

    }

}

sector_t *R_FakeFlat(sector_t *sec, sector_t *tempsec, int *floorlightlevel, int *ceilinglightlevel, boolean back)
{

    if (floorlightlevel)
        *floorlightlevel = sec->floorlightsec == -1 ? sec->lightlevel : sectors[sec->floorlightsec].lightlevel;

    if (ceilinglightlevel)
        *ceilinglightlevel = sec->ceilinglightsec == -1 ? sec->lightlevel : sectors[sec->ceilinglightsec].lightlevel;

    if (sec->heightsec != -1)
    {

        const sector_t *s = &sectors[sec->heightsec];
        int heightsec = viewplayer->mo->subsector->sector->heightsec;
        int underwater = heightsec!=-1 && viewz<=sectors[heightsec].floorheight;

        *tempsec = *sec;
        tempsec->floorheight = s->floorheight;
        tempsec->ceilingheight = s->ceilingheight;

        if (underwater && (tempsec->floorheight = sec->floorheight, tempsec->ceilingheight = s->floorheight - 1, !back))
        {

            tempsec->floorpic = s->floorpic;
            tempsec->floor_xoffs = s->floor_xoffs;
            tempsec->floor_yoffs = s->floor_yoffs;

            if (underwater)
            {

                if (s->ceilingpic == skyflatnum)
                {

                    tempsec->floorheight = tempsec->ceilingheight + 1;
                    tempsec->ceilingpic = tempsec->floorpic;
                    tempsec->ceiling_xoffs = tempsec->floor_xoffs;
                    tempsec->ceiling_yoffs = tempsec->floor_yoffs;
                    
                }
                
                else
                {

                    tempsec->ceilingpic = s->ceilingpic;
                    tempsec->ceiling_xoffs = s->ceiling_xoffs;
                    tempsec->ceiling_yoffs = s->ceiling_yoffs;

                }

            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel : sectors[s->floorlightsec].lightlevel;

            if (ceilinglightlevel)
                *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel : sectors[s->ceilinglightsec].lightlevel;

        }

        else if (heightsec != -1 && viewz >= sectors[heightsec].ceilingheight && sec->ceilingheight > s->ceilingheight)
        {

            tempsec->ceilingheight = s->ceilingheight;
            tempsec->floorheight = s->ceilingheight + 1;
            tempsec->floorpic = tempsec->ceilingpic = s->ceilingpic;
            tempsec->floor_xoffs = tempsec->ceiling_xoffs = s->ceiling_xoffs;
            tempsec->floor_yoffs = tempsec->ceiling_yoffs = s->ceiling_yoffs;

            if (s->floorpic != skyflatnum)
            {

                tempsec->ceilingheight = sec->ceilingheight;
                tempsec->floorpic = s->floorpic;
                tempsec->floor_xoffs = s->floor_xoffs;
                tempsec->floor_yoffs = s->floor_yoffs;

            }

            tempsec->lightlevel = s->lightlevel;

            if (floorlightlevel)
                *floorlightlevel = s->floorlightsec == -1 ? s->lightlevel : sectors[s->floorlightsec].lightlevel;

            if (ceilinglightlevel)
                *ceilinglightlevel = s->ceilinglightsec == -1 ? s->lightlevel : sectors[s->ceilinglightsec].lightlevel;

        }

        sec = tempsec;

    }

    return sec;

}

static void R_AddLine (seg_t *line)
{

    int x1;
    int x2;
    angle_t angle1;
    angle_t angle2;
    angle_t span;
    angle_t tspan;
    static sector_t tempsec;

    curline = line;
    angle1 = R_PointToAngle(line->v1->x, line->v1->y);
    angle2 = R_PointToAngle(line->v2->x, line->v2->y);
    span = angle1 - angle2;

    if (span >= ANG180)
        return;

    rw_angle1 = angle1;
    angle1 -= viewangle;
    angle2 -= viewangle;
    tspan = angle1 + clipangle;

    if (tspan > 2 * clipangle)
    {

        tspan -= 2 * clipangle;

        if (tspan >= span)
            return;

        angle1 = clipangle;

    }

    tspan = clipangle - angle2;

    if (tspan > 2 * clipangle)
    {

        tspan -= 2 * clipangle;

        if (tspan >= span)
            return;

        angle2 = 0-clipangle;

    }

    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;
    x1 = viewangletox[angle1];
    x2 = viewangletox[angle2];

    if (x1 >= x2)
        return;

    backsector = line->backsector;

    if (backsector)
        backsector = R_FakeFlat(backsector, &tempsec, NULL, NULL, true);

    if ((linedef = curline->linedef)->r_validcount != gametic)
        R_RecalcLineFlags();

    if (linedef->r_flags & RF_IGNORE)
        return;
    else
        R_ClipWallSegment (x1, x2, linedef->r_flags & RF_CLOSED);

}

static boolean R_CheckBBox(const fixed_t *bspcoord)
{

    angle_t angle1, angle2;
    int boxpos;
    const int *check;

    boxpos = (viewx <= bspcoord[BOXLEFT] ? 0 : viewx < bspcoord[BOXRIGHT ] ? 1 : 2) + (viewy >= bspcoord[BOXTOP ] ? 0 : viewy > bspcoord[BOXBOTTOM] ? 4 : 8);

    if (boxpos == 5)
        return true;

    check = checkcoord[boxpos];
    angle1 = R_PointToAngle (bspcoord[check[0]], bspcoord[check[1]]) - viewangle;
    angle2 = R_PointToAngle (bspcoord[check[2]], bspcoord[check[3]]) - viewangle;

    if ((signed)angle1 < (signed)angle2)
    {

        if ((angle1 >= ANG180) && (angle1 < ANG270))
            angle1 = INT_MAX;
        else
            angle2 = INT_MIN;

    }

    if ((signed)angle2 >= (signed)clipangle)
        return false;

    if ((signed)angle1 <= -(signed)clipangle)
        return false;

    if ((signed)angle1 >= (signed)clipangle)
        angle1 = clipangle;

    if ((signed)angle2 <= -(signed)clipangle)
        angle2 = 0 - clipangle;

    angle1 = (angle1 + ANG90) >> ANGLETOFINESHIFT;
    angle2 = (angle2 + ANG90) >> ANGLETOFINESHIFT;

    {

        int sx1 = viewangletox[angle1];
        int sx2 = viewangletox[angle2];

        if (sx1 == sx2)
            return false;

        if (!memchr(solidcol + sx1, 0, sx2 - sx1))
            return false;

    }

    return true;

}

static void R_Subsector(int num)
{

    subsector_t *sub = &subsectors[num];
    int count = sub->numlines;
    seg_t *line = &segs[sub->firstline];
    sector_t tempsec;
    int floorlightlevel;
    int ceilinglightlevel;

    frontsector = sub->sector;
    frontsector = R_FakeFlat(frontsector, &tempsec, &floorlightlevel, &ceilinglightlevel, false);
    floorplane = frontsector->floorheight < viewz || (frontsector->heightsec != -1 && sectors[frontsector->heightsec].ceilingpic == skyflatnum) ? R_FindPlane(frontsector->floorheight, frontsector->floorpic == skyflatnum && frontsector->sky & PL_SKYFLAT ? frontsector->sky : frontsector->floorpic, floorlightlevel, frontsector->floor_xoffs, frontsector->floor_yoffs) : NULL;
    ceilingplane = frontsector->ceilingheight > viewz || frontsector->ceilingpic == skyflatnum || (frontsector->heightsec != -1 && sectors[frontsector->heightsec].floorpic == skyflatnum) ? R_FindPlane(frontsector->ceilingheight, frontsector->ceilingpic == skyflatnum && frontsector->sky & PL_SKYFLAT ? frontsector->sky : frontsector->ceilingpic, ceilinglightlevel, frontsector->ceiling_xoffs, frontsector->ceiling_yoffs) : NULL;

    R_AddSprites(sub, (floorlightlevel + ceilinglightlevel) / 2);

    while (count--)
    {

        if (line->miniseg == false)
            R_AddLine(line);

        line++;
        curline = NULL;

    }

}

void R_RenderBSPNode(int bspnum)
{

    while (!(bspnum & NF_SUBSECTOR))
    {

        const node_t *bsp = &nodes[bspnum];
        int side = R_PointOnSide(viewx, viewy, bsp);

        R_RenderBSPNode(bsp->children[side]);

        if (!R_CheckBBox(bsp->bbox[side ^ 1]))
            return;

        bspnum = bsp->children[side ^ 1];

    }

    R_Subsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);

}

