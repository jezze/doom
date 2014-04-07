#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_mobj.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "p_inter.h"
#include "m_random.h"
#include "i_system.h"
#include "z_zone.h"
#include "z_bmalloc.h"

IMPLEMENT_BLOCK_MEMORY_ALLOC_ZONE(secnodezone, sizeof(msecnode_t), PU_LEVEL, 32, "SecNodes");

static mobj_t *tmthing;
static fixed_t tmx;
static fixed_t tmy;
static int pe_x;
static int pe_y;
static int ls_x;
static int ls_y;
boolean floatok;
boolean felldown;
fixed_t tmbbox[4];
fixed_t tmfloorz;
static fixed_t tmceilingz;
static fixed_t tmdropoffz;
line_t *ceilingline;
line_t *blockline;
static line_t *floorline;
static int tmunstuck;
line_t **spechit;
static int spechit_max;
int numspechit;
msecnode_t *sector_list = NULL;
static boolean telefrag;
static fixed_t bestslidefrac;
static line_t *bestslideline;
static mobj_t *slidemo;
static fixed_t tmxmove;
static fixed_t tmymove;
mobj_t *linetarget;
static mobj_t *shootthing;
static uint_64_t aim_flags_mask;
static fixed_t shootz;
static int la_damage;
fixed_t attackrange;
static fixed_t aimslope;
static fixed_t topslope;
static fixed_t bottomslope;
static mobj_t *bombsource, *bombspot;
static int bombdamage;
static mobj_t *usething;
static boolean crushchange, nofit;

boolean PIT_StompThing(mobj_t* thing)
{

    fixed_t blockdist;

    if (thing == tmthing)
        return true;

    if (!(thing->flags & MF_SHOOTABLE))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
        return true;

    if (!telefrag)
        return false;

    P_DamageMobj (thing, tmthing, tmthing, 10000);

    return true;

}

int P_GetFriction(const mobj_t *mo, int *frictionfactor)
{

    int friction = ORIG_FRICTION;
    int movefactor = ORIG_FRICTION_FACTOR;
    const msecnode_t *m;
    const sector_t *sec;

    if (!(mo->flags & (MF_NOCLIP|MF_NOGRAVITY)) && mo->player)
        for (m = mo->touching_sectorlist; m; m = m->m_tnext)
            if ((sec = m->m_sector)->special & FRICTION_MASK && (sec->friction < friction || friction == ORIG_FRICTION) && (mo->z <= sec->floorheight || (sec->heightsec != -1 && mo->z <= sectors[sec->heightsec].floorheight)))
                friction = sec->friction, movefactor = sec->movefactor;

    if (frictionfactor)
        *frictionfactor = movefactor;

    return friction;

}

int P_GetMoveFactor(const mobj_t *mo, int *frictionp)
{

    int movefactor, friction;
    
    if ((friction = P_GetFriction(mo, &movefactor)) < ORIG_FRICTION)
    {

        int momentum = P_AproxDistance(mo->momx, mo->momy);

        if (momentum > MORE_FRICTION_MOMENTUM << 2)
            movefactor <<= 3;
        else if (momentum > MORE_FRICTION_MOMENTUM << 1)
            movefactor <<= 2;
        else if (momentum > MORE_FRICTION_MOMENTUM)
            movefactor <<= 1;

    }

    if (frictionp)
        *frictionp = friction;

    return movefactor;

}

boolean P_TeleportMove(mobj_t* thing,fixed_t x,fixed_t y, boolean boss)
{

    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;

    subsector_t *newsubsec;
    telefrag = thing->player || boss;
    tmthing = thing;
    tmx = x;
    tmy = y;
    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;
    newsubsec = R_PointInSubsector(x,y);
    ceilingline = NULL;
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;
    validcount++;
    numspechit = 0;
    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_StompThing))
                return false;

    P_UnsetThingPosition(thing);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition (thing);

    thing->PrevX = x;
    thing->PrevY = y;
    thing->PrevZ = thing->floorz;

    return true;

}

static boolean PIT_CrossLine(line_t* ld)
{

    if (!(ld->flags & ML_TWOSIDED) || (ld->flags & (ML_BLOCKING|ML_BLOCKMONSTERS)))
        if (!(tmbbox[BOXLEFT] > ld->bbox[BOXRIGHT] || tmbbox[BOXRIGHT] < ld->bbox[BOXLEFT] || tmbbox[BOXTOP] < ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] > ld->bbox[BOXTOP]))
            if (P_PointOnLineSide(pe_x,pe_y,ld) != P_PointOnLineSide(ls_x,ls_y,ld))
                return false;

    return true;

}

static int untouched(line_t *ld)
{

    fixed_t x, y, tmbbox[4];

    return (tmbbox[BOXRIGHT] = (x=tmthing->x)+tmthing->radius) <= ld->bbox[BOXLEFT] || (tmbbox[BOXLEFT] = x-tmthing->radius) >= ld->bbox[BOXRIGHT] || (tmbbox[BOXTOP] = (y=tmthing->y)+tmthing->radius) <= ld->bbox[BOXBOTTOM] || (tmbbox[BOXBOTTOM] = y-tmthing->radius) >= ld->bbox[BOXTOP] || P_BoxOnLineSide(tmbbox, ld) != -1;

}

static boolean PIT_CheckLine(line_t *ld)
{

    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;

    if (!ld->backsector)
    {

        blockline = ld;

        return tmunstuck && !untouched(ld) && FixedMul(tmx - tmthing->x, ld->dy) > FixedMul(tmy - tmthing->y, ld->dx);

    }

    if (!(tmthing->flags & (MF_MISSILE | MF_BOUNCES)))
    {

        if (ld->flags & ML_BLOCKING)
            return tmunstuck && !untouched(ld);

        if (!(tmthing->flags & MF_FRIEND || tmthing->player) && ld->flags & ML_BLOCKMONSTERS)
            return false;

    }

    P_LineOpening (ld);

    if (opentop < tmceilingz)
    {

        tmceilingz = opentop;
        ceilingline = ld;
        blockline = ld;

    }

    if (openbottom > tmfloorz)
    {

        tmfloorz = openbottom;
        floorline = ld;
        blockline = ld;

    }

    if (lowfloor < tmdropoffz)
        tmdropoffz = lowfloor;

    if (ld->special)
    {

        if (numspechit >= spechit_max)
        {

            spechit_max = spechit_max ? spechit_max * 2 : 8;
            spechit = realloc(spechit, sizeof *spechit * spechit_max);

        }

        spechit[numspechit++] = ld;

    }

    return true;

}

static boolean PIT_CheckThing(mobj_t *thing)
{

    fixed_t blockdist;
    int damage;

    if (!(thing->flags & (MF_SOLID|MF_SPECIAL|MF_SHOOTABLE|MF_TOUCHY)))
        return true;

    blockdist = thing->radius + tmthing->radius;

    if (D_abs(thing->x - tmx) >= blockdist || D_abs(thing->y - tmy) >= blockdist)
        return true;

    if (thing == tmthing)
        return true;

    if (thing->flags & MF_TOUCHY && tmthing->flags & MF_SOLID && thing->health > 0 && (thing->intflags & MIF_ARMED || sentient(thing)) && (thing->type != tmthing->type || thing->type == MT_PLAYER) && thing->z + thing->height >= tmthing->z && tmthing->z + tmthing->height >= thing->z && (thing->type ^ MT_PAIN) | (tmthing->type ^ MT_SKULL) && (thing->type ^ MT_SKULL) | (tmthing->type ^ MT_PAIN))
    {

        P_DamageMobj(thing, NULL, NULL, thing->health);

        return true;

    }

    if (tmthing->flags & MF_SKULLFLY)
    {

        int damage = ((P_Random(pr_skullfly) % 8) + 1) * tmthing->info->damage;

        P_DamageMobj (thing, tmthing, tmthing, damage);

        tmthing->flags &= ~MF_SKULLFLY;
        tmthing->momx = tmthing->momy = tmthing->momz = 0;

        P_SetMobjState (tmthing, tmthing->info->spawnstate);

        return false;

    }

    if (tmthing->flags & MF_MISSILE || (tmthing->flags & MF_BOUNCES && !(tmthing->flags & MF_SOLID)))
    {

        if (tmthing->z > thing->z + thing->height)
            return true;

        if (tmthing->z+tmthing->height < thing->z)
            return true;

        if (tmthing->target && (tmthing->target->type == thing->type || (tmthing->target->type == MT_KNIGHT && thing->type == MT_BRUISER) || (tmthing->target->type == MT_BRUISER && thing->type == MT_KNIGHT)))
        {

            if (thing == tmthing->target)
                return true;
            else if (thing->type != MT_PLAYER)
                return false;

        }

        if (!(tmthing->flags & MF_MISSILE))
        {

            if (!(thing->flags & MF_SOLID))
            {

                return true;

            }
            
            else
            {

                tmthing->momx = -tmthing->momx;
                tmthing->momy = -tmthing->momy;

                if (!(tmthing->flags & MF_NOGRAVITY))
                {

                    tmthing->momx >>= 2;
                    tmthing->momy >>= 2;

                }

                return false;

            }

        }

        if (!(thing->flags & MF_SHOOTABLE))
            return !(thing->flags & MF_SOLID);

        damage = ((P_Random(pr_damage)%8)+1)*tmthing->info->damage;

        P_DamageMobj(thing, tmthing, tmthing->target, damage);

        return false;

    }

    if (thing->flags & MF_SPECIAL)
    {

        uint_64_t solid = thing->flags & MF_SOLID;

        if (tmthing->flags & MF_PICKUP)
            P_TouchSpecialThing(thing, tmthing);

        return !solid;

    }

    return !(thing->flags & MF_SOLID) || ((thing->flags & MF_NOCLIP || !(tmthing->flags & MF_SOLID)));

}

boolean Check_Sides(mobj_t* actor, int x, int y)
{

    int bx, by, xl, xh, yl, yh;

    pe_x = actor->x;
    pe_y = actor->y;
    ls_x = x;
    ls_y = y;
    tmbbox[BOXLEFT] = pe_x < x ? pe_x : x;
    tmbbox[BOXRIGHT] = pe_x > x ? pe_x : x;
    tmbbox[BOXTOP] = pe_y > y ? pe_y : y;
    tmbbox[BOXBOTTOM] = pe_y < y ? pe_y : y;
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;
    validcount++;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx,by,PIT_CrossLine))
                return true;

    return false;

}

boolean P_CheckPosition (mobj_t* thing, fixed_t x, fixed_t y)
{

    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;
    subsector_t *newsubsec;
    tmthing = thing;
    tmx = x;
    tmy = y;
    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;
    newsubsec = R_PointInSubsector(x, y);
    floorline = blockline = ceilingline = NULL;
    tmunstuck = thing->player && thing->player->mo == thing;
    tmfloorz = tmdropoffz = newsubsec->sector->floorheight;
    tmceilingz = newsubsec->sector->ceilingheight;
    validcount++;
    numspechit = 0;

    if (tmthing->flags & MF_NOCLIP)
        return true;

    xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockThingsIterator(bx, by, PIT_CheckThing))
                return false;

    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            if (!P_BlockLinesIterator(bx, by, PIT_CheckLine))
                return false;

    return true;

}

boolean P_TryMove(mobj_t* thing, fixed_t x, fixed_t y, boolean dropoff)
{

    fixed_t oldx;
    fixed_t oldy;

    felldown = floatok = false;

    if (!P_CheckPosition (thing, x, y))
        return false;

    if (!(thing->flags & MF_NOCLIP))
    {

        if (tmceilingz - tmfloorz < thing->height || (floatok = true, !(thing->flags & MF_TELEPORT) && tmceilingz - thing->z < thing->height) || (!(thing->flags & MF_TELEPORT) && tmfloorz - thing->z > 24 * FRACUNIT))
            return tmunstuck && !(ceilingline && untouched(ceilingline)) && !(floorline && untouched(floorline));

        if (!(thing->flags & (MF_DROPOFF|MF_FLOAT)))
        {

            if (!dropoff || (dropoff == 2 && (tmfloorz-tmdropoffz > 128 * FRACUNIT || !thing->target || thing->target->z >tmdropoffz)))
            {

                if (thing->floorz  - tmfloorz > 24 * FRACUNIT || thing->dropoffz - tmdropoffz > 24 * FRACUNIT)
                    return false;

            }

            else
            {

                felldown = !(thing->flags & MF_NOGRAVITY) && thing->z - tmfloorz > 24 * FRACUNIT;

            }

        }

        if (thing->flags & MF_BOUNCES && !(thing->flags & (MF_MISSILE|MF_NOGRAVITY)) && !sentient(thing) && tmfloorz - thing->z > 16 * FRACUNIT)
            return false;

        if (thing->intflags & MIF_FALLING && tmfloorz - thing->z > FixedMul(thing->momx,thing->momx)+FixedMul(thing->momy,thing->momy))
            return false;

    }

    P_UnsetThingPosition (thing);

    oldx = thing->x;
    oldy = thing->y;
    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;
    thing->x = x;
    thing->y = y;

    P_SetThingPosition(thing);

    if (!(thing->flags & (MF_TELEPORT|MF_NOCLIP)))
    {

        while (numspechit--)
        {

            if (spechit[numspechit]->special)
            {

                int oldside;

                if ((oldside = P_PointOnLineSide(oldx, oldy, spechit[numspechit])) != P_PointOnLineSide(thing->x, thing->y, spechit[numspechit]))
                    P_CrossSpecialLine(spechit[numspechit], oldside, thing);

            }

        }

    }

    return true;

}

static boolean PIT_ApplyTorque(line_t *ld)
{

    if (ld->backsector && tmbbox[BOXRIGHT] > ld->bbox[BOXLEFT] && tmbbox[BOXLEFT] < ld->bbox[BOXRIGHT] && tmbbox[BOXTOP] > ld->bbox[BOXBOTTOM] && tmbbox[BOXBOTTOM] < ld->bbox[BOXTOP] && P_BoxOnLineSide(tmbbox, ld) == -1)
    {

        mobj_t *mo = tmthing;
        fixed_t dist = (ld->dx >> FRACBITS) * (mo->y >> FRACBITS) - (ld->dy >> FRACBITS) * (mo->x >> FRACBITS) - (ld->dx >> FRACBITS) * (ld->v1->y >> FRACBITS) + (ld->dy >> FRACBITS) * (ld->v1->x >> FRACBITS);

        if (dist < 0 ? ld->frontsector->floorheight < mo->z && ld->backsector->floorheight >= mo->z : ld->backsector->floorheight < mo->z && ld->frontsector->floorheight >= mo->z)
        {

            fixed_t x = D_abs(ld->dx), y = D_abs(ld->dy);

            if (y > x)
            {

                fixed_t t = x;
                x = y;
                y = t;

            }

            y = finesine[(tantoangle[FixedDiv(y,x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT];
            dist = FixedDiv(FixedMul(dist, (mo->gear < OVERDRIVE) ? y << -(mo->gear - OVERDRIVE) : y >> (mo->gear - OVERDRIVE)), x);
            x = FixedMul(ld->dy, dist);
            y = FixedMul(ld->dx, dist);
            dist = FixedMul(x, x) + FixedMul(y, y);

            while (dist > FRACUNIT * 4 && mo->gear < MAXGEAR)
                ++mo->gear, x >>= 1, y >>= 1, dist >>= 1;

            mo->momx -= x;
            mo->momy += y;

        }

    }

    return true;

}

void P_ApplyTorque(mobj_t *mo)
{

    int xl = ((tmbbox[BOXLEFT] = mo->x - mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int xh = ((tmbbox[BOXRIGHT] = mo->x + mo->radius) - bmaporgx) >> MAPBLOCKSHIFT;
    int yl = ((tmbbox[BOXBOTTOM] = mo->y - mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int yh = ((tmbbox[BOXTOP] = mo->y + mo->radius) - bmaporgy) >> MAPBLOCKSHIFT;
    int bx, by, flags = mo->intflags;

    tmthing = mo;
    validcount++;

    for (bx = xl; bx <= xh; bx++)
        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_ApplyTorque);

    if (mo->momx | mo->momy)
        mo->intflags |= MIF_FALLING;
    else
        mo->intflags &= ~MIF_FALLING;

    if (!((mo->intflags | flags) & MIF_FALLING))
        mo->gear = 0;
    else if (mo->gear < MAXGEAR)
        mo->gear++;

}

boolean P_ThingHeightClip(mobj_t *thing)
{

    boolean onfloor = (thing->z == thing->floorz);

    P_CheckPosition(thing, thing->x, thing->y);

    thing->floorz = tmfloorz;
    thing->ceilingz = tmceilingz;
    thing->dropoffz = tmdropoffz;

    if (onfloor)
    {

        thing->z = thing->floorz;

        if (thing->intflags & MIF_FALLING && thing->gear >= MAXGEAR)
            thing->gear = 0;

    }

    else
    {

        if (thing->z + thing->height > thing->ceilingz)
            thing->z = thing->ceilingz - thing->height;

    }

    return thing->ceilingz - thing->floorz >= thing->height;

}

void P_HitSlideLine (line_t* ld)
{

    int side;
    angle_t lineangle;
    angle_t moveangle;
    angle_t deltaangle;
    fixed_t movelen;
    fixed_t newlen;
    boolean icyfloor = P_AproxDistance(tmxmove, tmymove) > 4 * FRACUNIT && slidemo->z <= slidemo->floorz && P_GetFriction(slidemo, NULL) > ORIG_FRICTION;

    if (ld->slopetype == ST_HORIZONTAL)
    {

        if (icyfloor && (D_abs(tmymove) > D_abs(tmxmove)))
        {

            tmxmove /= 2;
            tmymove = -tmymove / 2;

            S_StartSound(slidemo, sfx_oof);

        }

        else
            tmymove = 0;

        return;

    }

    if (ld->slopetype == ST_VERTICAL)
    {

        if (icyfloor && (D_abs(tmxmove) > D_abs(tmymove)))
        {

            tmxmove = -tmxmove / 2;
            tmymove /= 2;

            S_StartSound(slidemo, sfx_oof);

        }

        else
        {

            tmxmove = 0;

        }

        return;

    }

    side = P_PointOnLineSide(slidemo->x, slidemo->y, ld);
    lineangle = R_PointToAngle2(0, 0, ld->dx, ld->dy);

    if (side == 1)
        lineangle += ANG180;

    moveangle = R_PointToAngle2(0,0, tmxmove, tmymove);
    moveangle += 10;
    deltaangle = moveangle - lineangle;
    movelen = P_AproxDistance(tmxmove, tmymove);

    if (icyfloor && (deltaangle > ANG45) && (deltaangle < ANG90 + ANG45))
    {

        moveangle = lineangle - deltaangle;
        movelen /= 2;

        S_StartSound(slidemo, sfx_oof);

        moveangle >>= ANGLETOFINESHIFT;
        tmxmove = FixedMul (movelen, finecosine[moveangle]);
        tmymove = FixedMul (movelen, finesine[moveangle]);

    }

    else
    {

        if (deltaangle > ANG180)
            deltaangle += ANG180;

        lineangle >>= ANGLETOFINESHIFT;
        deltaangle >>= ANGLETOFINESHIFT;
        newlen = FixedMul(movelen, finecosine[deltaangle]);
        tmxmove = FixedMul(newlen, finecosine[lineangle]);
        tmymove = FixedMul(newlen, finesine[lineangle]);

    }

}

boolean PTR_SlideTraverse(intercept_t* in)
{

    line_t* li;

    if (!in->isaline)
        I_Error("PTR_SlideTraverse: not a line?");

    li = in->d.line;

    if (!(li->flags & ML_TWOSIDED))
    {

        if (P_PointOnLineSide (slidemo->x, slidemo->y, li))
            return true;

        goto isblocking;

    }

    P_LineOpening(li);

    if (openrange < slidemo->height)
        goto isblocking;

    if (opentop - slidemo->z < slidemo->height)
        goto isblocking;

    if (openbottom - slidemo->z > 24 * FRACUNIT)
        goto isblocking;

    return true;

isblocking:

    if (in->frac < bestslidefrac)
    {

        bestslidefrac = in->frac;
        bestslideline = li;

    }

    return false;

}

void P_SlideMove(mobj_t *mo)
{

    int hitcount = 3;

    slidemo = mo;

    do
    {

        fixed_t leadx, leady, trailx, traily;

        if (!--hitcount)
            goto stairstep;

        if (mo->momx > 0)
            leadx = mo->x + mo->radius, trailx = mo->x - mo->radius;
        else
            leadx = mo->x - mo->radius, trailx = mo->x + mo->radius;

        if (mo->momy > 0)
            leady = mo->y + mo->radius, traily = mo->y - mo->radius;
        else
            leady = mo->y - mo->radius, traily = mo->y + mo->radius;

        bestslidefrac = FRACUNIT+1;

        P_PathTraverse(leadx, leady, leadx+mo->momx, leady+mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
        P_PathTraverse(trailx, leady, trailx+mo->momx, leady+mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);
        P_PathTraverse(leadx, traily, leadx+mo->momx, traily+mo->momy,
        PT_ADDLINES, PTR_SlideTraverse);

        if (bestslidefrac == FRACUNIT+1)
        {

        stairstep:

            if (!P_TryMove(mo, mo->x, mo->y + mo->momy, true))
                P_TryMove(mo, mo->x + mo->momx, mo->y, true);

            break;

        }

        if ((bestslidefrac -= 0x800) > 0)
        {

            fixed_t newx = FixedMul(mo->momx, bestslidefrac);
            fixed_t newy = FixedMul(mo->momy, bestslidefrac);

            if (!P_TryMove(mo, mo->x + newx, mo->y + newy, true))
                goto stairstep;

        }

        bestslidefrac = FRACUNIT - (bestslidefrac + 0x800);

        if (bestslidefrac > FRACUNIT)
            bestslidefrac = FRACUNIT;

        if (bestslidefrac <= 0)
            break;

        tmxmove = FixedMul(mo->momx, bestslidefrac);
        tmymove = FixedMul(mo->momy, bestslidefrac);

        P_HitSlideLine(bestslideline);

        mo->momx = tmxmove;
        mo->momy = tmymove;

        if (mo->player && mo->player->mo == mo)
        {

            if (D_abs(mo->player->momx) > D_abs(tmxmove))
                mo->player->momx = tmxmove;

            if (D_abs(mo->player->momy) > D_abs(tmymove))
                mo->player->momy = tmymove;
                
        }

    }

    while (!P_TryMove(mo, mo->x + tmxmove, mo->y + tmymove, true));

}

boolean PTR_AimTraverse (intercept_t* in)
{

    line_t *li;
    mobj_t *th;
    fixed_t slope;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;
    fixed_t dist;

    if (in->isaline)
    {

        li = in->d.line;

        if (!(li->flags & ML_TWOSIDED))
            return false;

        P_LineOpening(li);

        if (openbottom >= opentop)
            return false;

        dist = FixedMul(attackrange, in->frac);

        if (li->frontsector->floorheight != li->backsector->floorheight)
        {

            slope = FixedDiv(openbottom - shootz, dist);

            if (slope > bottomslope)
                bottomslope = slope;

        }

        if (li->frontsector->ceilingheight != li->backsector->ceilingheight)
        {

            slope = FixedDiv(opentop - shootz, dist);

            if (slope < topslope)
                topslope = slope;

        }

        if (topslope <= bottomslope)
            return false;

        return true;

    }

    th = in->d.thing;

    if (th == shootthing)
        return true;

    if (!(th->flags & MF_SHOOTABLE))
        return true;

    if (th->flags & shootthing->flags & aim_flags_mask && !th->player)
        return true;

    dist = FixedMul(attackrange, in->frac);
    thingtopslope = FixedDiv (th->z + th->height - shootz, dist);

    if (thingtopslope < bottomslope)
        return true;

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > topslope)
        return true;

    if (thingtopslope > topslope)
        thingtopslope = topslope;

    if (thingbottomslope < bottomslope)
        thingbottomslope = bottomslope;

    aimslope = (thingtopslope + thingbottomslope) / 2;
    linetarget = th;

    return false;

}

boolean PTR_ShootTraverse (intercept_t *in)
{

    fixed_t x;
    fixed_t y;
    fixed_t z;
    fixed_t frac;
    mobj_t* th;
    fixed_t slope;
    fixed_t dist;
    fixed_t thingtopslope;
    fixed_t thingbottomslope;

    if (in->isaline)
    {

        line_t *li = in->d.line;

        if (li->special)
            P_ShootSpecialLine(shootthing, li);

        if (li->flags & ML_TWOSIDED)
        {

            P_LineOpening(li);

            dist = FixedMul(attackrange, in->frac);

            if ((li->frontsector->floorheight == li->backsector->floorheight || (slope = FixedDiv(openbottom - shootz, dist)) <= aimslope) && (li->frontsector->ceilingheight == li->backsector->ceilingheight || (slope = FixedDiv(opentop - shootz , dist)) >= aimslope))
                return true;

        }

        frac = in->frac - FixedDiv (4*FRACUNIT,attackrange);
        x = trace.x + FixedMul (trace.dx, frac);
        y = trace.y + FixedMul (trace.dy, frac);
        z = shootz + FixedMul (aimslope, FixedMul(frac, attackrange));

        if (li->frontsector->ceilingpic == skyflatnum)
        {

            if (z > li->frontsector->ceilingheight)
                return false;

            if (li->backsector && li->backsector->ceilingpic == skyflatnum)
                if (li->backsector->ceilingheight < z)
                    return false;

        }

        P_SpawnPuff(x,y,z);

        return false;

    }

    th = in->d.thing;

    if (th == shootthing)
        return true;

    if (!(th->flags&MF_SHOOTABLE))
        return true;

    dist = FixedMul (attackrange, in->frac);
    thingtopslope = FixedDiv (th->z+th->height - shootz , dist);

    if (thingtopslope < aimslope)
        return true;

    thingbottomslope = FixedDiv(th->z - shootz, dist);

    if (thingbottomslope > aimslope)
        return true;

    frac = in->frac - FixedDiv(10 * FRACUNIT, attackrange);

    x = trace.x + FixedMul(trace.dx, frac);
    y = trace.y + FixedMul(trace.dy, frac);
    z = shootz + FixedMul(aimslope, FixedMul(frac, attackrange));

    if (in->d.thing->flags & MF_NOBLOOD)
        P_SpawnPuff(x,y,z);
    else
        P_SpawnBlood(x,y,z, la_damage);

    if (la_damage)
        P_DamageMobj(th, shootthing, shootthing, la_damage);

    return false;

}

fixed_t P_AimLineAttack(mobj_t *t1, angle_t angle, fixed_t distance, uint_64_t mask)
{

    fixed_t x2;
    fixed_t y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    topslope = 100 * FRACUNIT / 160;
    bottomslope = -100 * FRACUNIT / 160;
    attackrange = distance;
    linetarget = NULL;
    aim_flags_mask = mask;

    P_PathTraverse(t1->x,t1->y,x2,y2,PT_ADDLINES|PT_ADDTHINGS,PTR_AimTraverse);

    if (linetarget)
        return aimslope;

    return 0;

}

void P_LineAttack(mobj_t* t1, angle_t angle, fixed_t distance, fixed_t slope, int damage)
{

    fixed_t x2;
    fixed_t y2;

    angle >>= ANGLETOFINESHIFT;
    shootthing = t1;
    la_damage = damage;
    x2 = t1->x + (distance >> FRACBITS) * finecosine[angle];
    y2 = t1->y + (distance >> FRACBITS) * finesine[angle];
    shootz = t1->z + (t1->height >> 1) + 8 * FRACUNIT;
    attackrange = distance;
    aimslope = slope;

    P_PathTraverse(t1->x, t1->y, x2, y2, PT_ADDLINES | PT_ADDTHINGS, PTR_ShootTraverse);

}

boolean PTR_UseTraverse(intercept_t *in)
{

    int side;

    if (!in->d.line->special)
    {

        P_LineOpening(in->d.line);

        if (openrange <= 0)
        {

            S_StartSound(usething, sfx_noway);

            return false;

        }

        return true;

    }

    side = 0;

    if (P_PointOnLineSide(usething->x, usething->y, in->d.line) == 1)
        side = 1;

    P_UseSpecialLine(usething, in->d.line, side);

    return ((in->d.line->flags & ML_PASSUSE)) ? true : false;

}

boolean PTR_NoWayTraverse(intercept_t *in)
{

    line_t *ld = in->d.line;

    return ld->special || !(ld->flags & ML_BLOCKING || (P_LineOpening(ld), openrange <= 0 || openbottom > usething->z + 24 * FRACUNIT || opentop < usething->z + usething->height));

}

void P_UseLines(player_t *player)
{

    int angle;
    fixed_t x1;
    fixed_t y1;
    fixed_t x2;
    fixed_t y2;

    usething = player->mo;

    angle = player->mo->angle >> ANGLETOFINESHIFT;

    x1 = player->mo->x;
    y1 = player->mo->y;
    x2 = x1 + (USERANGE >> FRACBITS) * finecosine[angle];
    y2 = y1 + (USERANGE >> FRACBITS) * finesine[angle];

    if (P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_UseTraverse))
        if (!P_PathTraverse ( x1, y1, x2, y2, PT_ADDLINES, PTR_NoWayTraverse))
            S_StartSound(usething, sfx_noway);

}

boolean PIT_RadiusAttack(mobj_t *thing)
{

    fixed_t dx;
    fixed_t dy;
    fixed_t dist;

    if (!(thing->flags & (MF_SHOOTABLE | MF_BOUNCES)))
        return true;

    if (bombspot->flags & MF_BOUNCES ? thing->type == MT_CYBORG && bombsource->type == MT_CYBORG : thing->type == MT_CYBORG || thing->type == MT_SPIDER)
        return true;

    dx = D_abs(thing->x - bombspot->x);
    dy = D_abs(thing->y - bombspot->y);
    dist = dx>dy ? dx : dy;
    dist = (dist - thing->radius) >> FRACBITS;

    if (dist < 0)
        dist = 0;

    if (dist >= bombdamage)
        return true;

    if (P_CheckSight(thing, bombspot))
        P_DamageMobj(thing, bombspot, bombsource, bombdamage - dist);

    return true;

}

void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage)
{

    fixed_t dist = (damage + MAXRADIUS) << FRACBITS;
    int yh = (spot->y + dist - bmaporgy) >> MAPBLOCKSHIFT;
    int yl = (spot->y - dist - bmaporgy) >> MAPBLOCKSHIFT;
    int xh = (spot->x + dist - bmaporgx) >> MAPBLOCKSHIFT;
    int xl = (spot->x - dist - bmaporgx) >> MAPBLOCKSHIFT;
    int x;
    int y;

    bombspot = spot;
    bombsource = source;
    bombdamage = damage;

    for (y = yl; y <= yh; y++)
        for (x = xl; x <= xh; x++)
            P_BlockThingsIterator(x, y, PIT_RadiusAttack);

}

boolean PIT_ChangeSector(mobj_t *thing)
{

    mobj_t *mo;

    if (P_ThingHeightClip(thing))
        return true;

    if (thing->health <= 0)
    {

        P_SetMobjState(thing, S_GIBS);

        thing->flags &= ~MF_SOLID;
        thing->height = 0;
        thing->radius = 0;
        
        return true;

    }

    if (thing->flags & MF_DROPPED)
    {

        P_RemoveMobj(thing);

        return true;

    }

    if (thing->flags & MF_TOUCHY && (thing->intflags & MIF_ARMED || sentient(thing)))
    {

        P_DamageMobj(thing, NULL, NULL, thing->health);

        return true;

    }

    if (!(thing->flags & MF_SHOOTABLE))
    {

        return true;

    }

    nofit = true;

    if (crushchange && !(leveltime & 3))
    {

        int t;

        P_DamageMobj(thing, NULL, NULL, 10);

        mo = P_SpawnMobj(thing->x, thing->y, thing->z + thing->height / 2, MT_BLOOD);
        t = P_Random(pr_crush);
        mo->momx = (t - P_Random (pr_crush)) << 12;
        t = P_Random(pr_crush);
        mo->momy = (t - P_Random (pr_crush)) << 12;

    }

    return true;

}

boolean P_ChangeSector(sector_t *sector, boolean crunch)
{

    int x;
    int y;

    nofit = false;
    crushchange = crunch;

    for (x = sector->blockbox[BOXLEFT]; x <= sector->blockbox[BOXRIGHT]; x++)
        for (y = sector->blockbox[BOXBOTTOM]; y <= sector->blockbox[BOXTOP]; y++)
            P_BlockThingsIterator(x, y, PIT_ChangeSector);

    return nofit;

}

boolean P_CheckSector(sector_t* sector, boolean crunch)
{

    msecnode_t *n;

    nofit = false;
    crushchange = crunch;

    for (n = sector->touching_thinglist; n; n = n->m_snext)
        n->visited = false;

    do
        for (n = sector->touching_thinglist; n; n = n->m_snext)
            if (!n->visited)
            {

                n->visited  = true;

                if (!(n->m_thing->flags & MF_NOBLOCKMAP))
                    PIT_ChangeSector(n->m_thing);

                break;

            }

    while (n);

    return nofit;

}

inline static msecnode_t* P_GetSecnode(void)
{

    return (msecnode_t*)Z_BMalloc(&secnodezone);

}

inline static void P_PutSecnode(msecnode_t* node)
{

    Z_BFree(&secnodezone, node);

}

msecnode_t *P_AddSecnode(sector_t *s, mobj_t *thing, msecnode_t *nextnode)
{

    msecnode_t* node = nextnode;

    while (node)
    {

        if (node->m_sector == s)
        {

            node->m_thing = thing;

            return nextnode;

        }

        node = node->m_tnext;

    }

    node = P_GetSecnode();
    node->visited = 0;
    node->m_sector = s;
    node->m_thing = thing;
    node->m_tprev = NULL;
    node->m_tnext = nextnode;

    if (nextnode)
        nextnode->m_tprev = node;

    node->m_sprev = NULL;
    node->m_snext = s->touching_thinglist;

    if (s->touching_thinglist)
        node->m_snext->m_sprev = node;

    s->touching_thinglist = node;

    return node;

}

msecnode_t *P_DelSecnode(msecnode_t *node)
{

    msecnode_t *tp;
    msecnode_t *tn;
    msecnode_t *sp;
    msecnode_t *sn;

    if (node)
    {

        tp = node->m_tprev;
        tn = node->m_tnext;

        if (tp)
            tp->m_tnext = tn;

        if (tn)
            tn->m_tprev = tp;

        sp = node->m_sprev;
        sn = node->m_snext;

        if (sp)
            sp->m_snext = sn;
        else
            node->m_sector->touching_thinglist = sn;

        if (sn)
            sn->m_sprev = sp;

        P_PutSecnode(node);

        return tn;

    }

    return NULL;

}

void P_DelSeclist(msecnode_t *node)
{

    while (node)
        node = P_DelSecnode(node);

}

boolean PIT_GetSectors(line_t *ld)
{

    if (tmbbox[BOXRIGHT] <= ld->bbox[BOXLEFT] || tmbbox[BOXLEFT] >= ld->bbox[BOXRIGHT] || tmbbox[BOXTOP] <= ld->bbox[BOXBOTTOM] || tmbbox[BOXBOTTOM] >= ld->bbox[BOXTOP])
        return true;

    if (P_BoxOnLineSide(tmbbox, ld) != -1)
        return true;

    sector_list = P_AddSecnode(ld->frontsector, tmthing, sector_list);

    if (ld->backsector && ld->backsector != ld->frontsector)
        sector_list = P_AddSecnode(ld->backsector, tmthing, sector_list);

    return true;

}

void P_CreateSecNodeList(mobj_t *thing, fixed_t x, fixed_t y)
{

    int xl;
    int xh;
    int yl;
    int yh;
    int bx;
    int by;

    msecnode_t* node;
    mobj_t* saved_tmthing = tmthing;
    node = sector_list;

    while (node)
    {

        node->m_thing = NULL;
        node = node->m_tnext;

    }

    tmthing = thing;
    tmx = x;
    tmy = y;
    tmbbox[BOXTOP] = y + tmthing->radius;
    tmbbox[BOXBOTTOM] = y - tmthing->radius;
    tmbbox[BOXRIGHT] = x + tmthing->radius;
    tmbbox[BOXLEFT] = x - tmthing->radius;
    validcount++;
    xl = (tmbbox[BOXLEFT] - bmaporgx) >> MAPBLOCKSHIFT;
    xh = (tmbbox[BOXRIGHT] - bmaporgx) >> MAPBLOCKSHIFT;
    yl = (tmbbox[BOXBOTTOM] - bmaporgy) >> MAPBLOCKSHIFT;
    yh = (tmbbox[BOXTOP] - bmaporgy) >> MAPBLOCKSHIFT;

    for (bx = xl; bx <= xh; bx++)
    {

        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_GetSectors);

    }

    sector_list = P_AddSecnode(thing->subsector->sector,thing,sector_list);
    node = sector_list;

    while (node)
    {

        if (node->m_thing == NULL)
        {

            if (node == sector_list)
                sector_list = node->m_tnext;

            node = P_DelSecnode(node);

        }

        else
        {

            node = node->m_tnext;

        }

    }

    tmthing = saved_tmthing;

}

void P_MapStart(void)
{

    if (tmthing)
        I_Error("P_MapStart: tmthing set!");

}

void P_MapEnd(void)
{

    tmthing = NULL;

}

