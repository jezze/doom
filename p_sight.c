#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_maputl.h"
#include "p_setup.h"

typedef struct
{
    fixed_t sightzstart, t2x, t2y;
    divline_t strace;
    fixed_t topslope, bottomslope;
    fixed_t bbox[4];
    fixed_t maxz,minz;

} los_t;

static los_t los;

inline static int P_DivlineSide(fixed_t x, fixed_t y, const divline_t *node)
{

    fixed_t left, right;

    return !node->dx ? x == node->x ? 2 : x <= node->x ? node->dy > 0 : node->dy < 0 : !node->dy ? y == node->y ? 2 : y <= node->y ? node->dx < 0 : node->dx > 0 : (right = ((y - node->y) >> FRACBITS) * (node->dx >> FRACBITS)) < (left  = ((x - node->x) >> FRACBITS) * (node->dy >> FRACBITS)) ? 0 : right == left ? 2 : 1;

}

static boolean P_CrossSubsector(int num)
{

    seg_t *seg = segs + subsectors[num].firstline;
    int count;
    fixed_t opentop = 0, openbottom = 0;
    const sector_t *front = NULL, *back = NULL;

    for (count = subsectors[num].numlines; --count >= 0; seg++)
    {

        line_t *line = seg->linedef;
        divline_t divl;

        if (!line)
            continue;

        if (line->validcount == validcount)
            continue;

        line->validcount = validcount;

        if (line->bbox[BOXLEFT] > los.bbox[BOXRIGHT] || line->bbox[BOXRIGHT] < los.bbox[BOXLEFT] || line->bbox[BOXBOTTOM] > los.bbox[BOXTOP] || line->bbox[BOXTOP] < los.bbox[BOXBOTTOM])
            continue;

        if (line->flags & ML_TWOSIDED)
        {

            if ((front = seg->frontsector)->floorheight == (back = seg->backsector)->floorheight && front->ceilingheight == back->ceilingheight)
                continue;

            opentop = front->ceilingheight < back->ceilingheight ? front->ceilingheight : back->ceilingheight;
            openbottom = front->floorheight > back->floorheight ? front->floorheight : back->floorheight;

            if ((opentop >= los.maxz) && (openbottom <= los.minz))
                continue;

        }

        {

            const vertex_t *v1,*v2;

            v1 = line->v1;
            v2 = line->v2;

            if (P_DivlineSide(v1->x, v1->y, &los.strace) == P_DivlineSide(v2->x, v2->y, &los.strace))
                continue;

            divl.dx = v2->x - (divl.x = v1->x);
            divl.dy = v2->y - (divl.y = v1->y);

            if (P_DivlineSide(los.strace.x, los.strace.y, &divl) == P_DivlineSide(los.t2x, los.t2y, &divl))
                continue;

        }

        if (!(line->flags & ML_TWOSIDED) || (openbottom >= opentop) || (opentop < los.minz) || (openbottom > los.maxz))
            return false;

        {

            fixed_t frac = P_InterceptVector2(&los.strace, &divl);

            if (front->floorheight != back->floorheight)
            {

                fixed_t slope = FixedDiv(openbottom - los.sightzstart, frac);

                if (slope > los.bottomslope)
                    los.bottomslope = slope;

            }

            if (front->ceilingheight != back->ceilingheight)
            {

                fixed_t slope = FixedDiv(opentop - los.sightzstart, frac);

                if (slope < los.topslope)
                    los.topslope = slope;

            }

            if (los.topslope <= los.bottomslope)
                return false;

        }

    }

    return true;

}

static boolean P_CrossBSPNode(int bspnum)
{

    while (!(bspnum & NF_SUBSECTOR))
    {

        register const node_t *bsp = nodes + bspnum;
        int side, side2;

        side = P_DivlineSide(los.strace.x, los.strace.y, (const divline_t *)bsp) & 1;
        side2 = P_DivlineSide(los.t2x, los.t2y, (const divline_t *)bsp);

        if (side == side2)
            bspnum = bsp->children[side];
        else if (!P_CrossBSPNode(bsp->children[side]))
            return 0;
        else
            bspnum = bsp->children[side ^ 1];

    }

    return P_CrossSubsector(bspnum == -1 ? 0 : bspnum & ~NF_SUBSECTOR);

}

boolean P_CheckSight(mobj_t *t1, mobj_t *t2)
{

    const sector_t *s1 = t1->subsector->sector;
    const sector_t *s2 = t2->subsector->sector;
    int pnum = (s1-sectors)*numsectors + (s2-sectors);

    if (rejectmatrix[pnum >> 3] & (1 << (pnum & 7)))
        return false;

    if ((s1->heightsec != -1 && ((t1->z + t1->height <= sectors[s1->heightsec].floorheight && t2->z >= sectors[s1->heightsec].floorheight) || (t1->z >= sectors[s1->heightsec].ceilingheight && t2->z + t1->height <= sectors[s1->heightsec].ceilingheight))) || (s2->heightsec != -1 && ((t2->z + t2->height <= sectors[s2->heightsec].floorheight && t1->z >= sectors[s2->heightsec].floorheight) || (t2->z >= sectors[s2->heightsec].ceilingheight && t1->z + t2->height <= sectors[s2->heightsec].ceilingheight))))
        return false;

    if (t1->subsector == t2->subsector)
        return true;

    validcount++;
    los.topslope = (los.bottomslope = t2->z - (los.sightzstart = t1->z + t1->height - (t1->height >> 2))) + t2->height;
    los.strace.dx = (los.t2x = t2->x) - (los.strace.x = t1->x);
    los.strace.dy = (los.t2y = t2->y) - (los.strace.y = t1->y);

    if (t1->x > t2->x)
        los.bbox[BOXRIGHT] = t1->x, los.bbox[BOXLEFT] = t2->x;
    else
        los.bbox[BOXRIGHT] = t2->x, los.bbox[BOXLEFT] = t1->x;

    if (t1->y > t2->y)
        los.bbox[BOXTOP] = t1->y, los.bbox[BOXBOTTOM] = t2->y;
    else
        los.bbox[BOXTOP] = t2->y, los.bbox[BOXBOTTOM] = t1->y;

    los.maxz = INT_MAX;
    los.minz = INT_MIN;

    return P_CrossBSPNode(numnodes - 1);

}

