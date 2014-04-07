#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"
#include "p_spec.h"
#include "s_sound.h"
#include "p_inter.h"
#include "g_game.h"
#include "p_enemy.h"
#include "p_tick.h"

#define SKULLSPEED                      (20 * FRACUNIT)
#define FATSPREAD                       (ANG90 / 8)

typedef enum
{

    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS

} dirtype_t;

static mobj_t *corpsehit;
static mobj_t *vileobj;
static mobj_t **braintargets;
static mobj_t *current_actor;
static fixed_t viletryx;
static fixed_t viletryy;
static int TRACEANGLE = 0xc000000;
static int current_allaround;
static fixed_t dropoff_deltax, dropoff_deltay, floorz;
static fixed_t xspeed[8] = {FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000, 0, 47000};
static fixed_t yspeed[8] = {0, 47000, FRACUNIT, 47000, 0, -47000, -FRACUNIT, -47000};
static int numbraintargets_alloc;
static int numbraintargets;

struct brain_s brain;
extern line_t **spechit;
extern int numspechit;

static void P_RecursiveSound(sector_t *sec, int soundblocks, mobj_t *soundtarget)
{

    int i;

    if (sec->validcount == validcount && sec->soundtraversed <= soundblocks + 1)
        return;

    sec->validcount = validcount;
    sec->soundtraversed = soundblocks + 1;

    P_SetTarget(&sec->soundtarget, soundtarget);

    for (i = 0; i < sec->linecount; i++)
    {

        sector_t *other;
        line_t *check = sec->lines[i];

        if (!(check->flags & ML_TWOSIDED))
            continue;

        P_LineOpening(check);

        if (openrange <= 0)
            continue;

        other = sides[check->sidenum[sides[check->sidenum[0]].sector == sec]].sector;

        if (!(check->flags & ML_SOUNDBLOCK))
            P_RecursiveSound(other, soundblocks, soundtarget);
        else if (!soundblocks)
            P_RecursiveSound(other, 1, soundtarget);

    }

}

void P_NoiseAlert(mobj_t *target, mobj_t *emitter)
{

    validcount++;

    P_RecursiveSound(emitter->subsector->sector, 0, target);

}

static boolean P_CheckMeleeRange(mobj_t *actor)
{

    mobj_t *pl = actor->target;

    return pl && !(actor->flags & pl->flags & MF_FRIEND) && (P_AproxDistance(pl->x-actor->x, pl->y-actor->y) < MELEERANGE - 20*FRACUNIT + pl->info->radius) && P_CheckSight(actor, actor->target);

}

static boolean P_HitFriend(mobj_t *actor)
{

    return actor->flags & MF_FRIEND && actor->target && (P_AimLineAttack(actor, R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y), P_AproxDistance(actor->x-actor->target->x, actor->y-actor->target->y), 0), linetarget) && linetarget != actor->target && !((linetarget->flags ^ actor->flags) & MF_FRIEND);

}

static boolean P_CheckMissileRange(mobj_t *actor)
{

    fixed_t dist;

    if (!P_CheckSight(actor, actor->target))
        return false;

    if (actor->flags & MF_JUSTHIT)
    {

        actor->flags &= ~MF_JUSTHIT;

        return !(actor->flags & MF_FRIEND) || (actor->target->health > 0 && (!(actor->target->flags & MF_FRIEND) || (actor->target->player ? monster_infighting || P_Random(pr_defect) > 128 : !(actor->target->flags & MF_JUSTHIT) && P_Random(pr_defect) > 128)));

    }

    if (actor->flags & actor->target->flags & MF_FRIEND)
        return false;

    if (actor->reactiontime)
        return false;

    dist = P_AproxDistance (actor->x-actor->target->x, actor->y-actor->target->y) - 64 * FRACUNIT;

    if (!actor->info->meleestate)
        dist -= 128 * FRACUNIT;

    dist >>= FRACBITS;

    if (actor->type == MT_VILE)
        if (dist > 14 * 64)
            return false;

    if (actor->type == MT_UNDEAD)
    {

        if (dist < 196)
            return false;

        dist >>= 1;

    }

    if (actor->type == MT_CYBORG || actor->type == MT_SPIDER || actor->type == MT_SKULL)
        dist >>= 1;

    if (dist > 200)
        dist = 200;

    if (actor->type == MT_CYBORG && dist > 160)
        dist = 160;

    if (P_Random(pr_missrange) < dist)
        return false;

    if (P_HitFriend(actor))
        return false;

    return true;

}

static boolean P_IsOnLift(const mobj_t *actor)
{

    const sector_t *sec = actor->subsector->sector;
    line_t line;
    int l;

    if (sec->floordata && ((thinker_t *) sec->floordata)->function == T_PlatRaise)
        return true;

    if ((line.tag = sec->tag))
    {

        for (l = -1; (l = P_FindLineFromLineTag(&line, l)) >= 0;)
        {

            switch (lines[l].special)
            {

            case  10: case  14: case  15: case  20: case  21: case  22:
            case  47: case  53: case  62: case  66: case  67: case  68:
            case  87: case  88: case  95: case 120: case 121: case 122:
            case 123: case 143: case 162: case 163: case 181: case 182:
            case 144: case 148: case 149: case 211: case 227: case 228:
            case 231: case 232: case 235: case 236:
                return true;

            }

        }

    }

    return false;

}

static int P_IsUnderDamage(mobj_t *actor)
{

    const struct msecnode_s *seclist;
    const ceiling_t *cl;
    int dir = 0;

    for (seclist = actor->touching_sectorlist; seclist; seclist = seclist->m_tnext)
        if ((cl = seclist->m_sector->ceilingdata) && cl->thinker.function == T_MoveCeiling)
            dir |= cl->direction;

    return dir;

}

static boolean P_Move(mobj_t *actor, boolean dropoff)
{

    fixed_t tryx, tryy, deltax, deltay, origx, origy;
    boolean try_ok;
    int movefactor = ORIG_FRICTION_FACTOR;
    int friction = ORIG_FRICTION;
    int speed;

    if (actor->movedir == DI_NODIR)
        return false;

    if (monster_friction)
        movefactor = P_GetMoveFactor(actor, &friction);

    speed = actor->info->speed;

    if (friction < ORIG_FRICTION && !(speed = ((ORIG_FRICTION_FACTOR - (ORIG_FRICTION_FACTOR - movefactor) / 2) * speed) / ORIG_FRICTION_FACTOR))
        speed = 1;

    tryx = (origx = actor->x) + (deltax = speed * xspeed[actor->movedir]);
    tryy = (origy = actor->y) + (deltay = speed * yspeed[actor->movedir]);
    try_ok = P_TryMove(actor, tryx, tryy, dropoff);

    if (try_ok && friction > ORIG_FRICTION)
    {

        actor->x = origx;
        actor->y = origy;
        movefactor *= FRACUNIT / ORIG_FRICTION_FACTOR / 4;
        actor->momx += FixedMul(deltax, movefactor);
        actor->momy += FixedMul(deltay, movefactor);

    }

    if (!try_ok)
    {

        int good;

        if (actor->flags & MF_FLOAT && floatok)
        {

            if (actor->z < tmfloorz)
                actor->z += FLOATSPEED;
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;

            return true;

        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;

        for (good = false; numspechit--; )
            if (P_UseSpecialLine(actor, spechit[numspechit], 0))
                good |= spechit[numspechit] == blockline ? 1 : 2;

        if (!good)
            return good;

        return ((P_Random(pr_opendoor) >= 230) ^ (good & 1));

    }

    else
    {

        actor->flags &= ~MF_INFLOAT;

    }

    if (!(actor->flags & MF_FLOAT) && !felldown)
        actor->z = actor->floorz;

    return true;

}

static boolean P_SmartMove(mobj_t *actor)
{

    mobj_t *target = actor->target;
    int dropoff = false;
    int on_lift = target && target->health > 0 && target->subsector->sector->tag==actor->subsector->sector->tag && P_IsOnLift(actor);
    int under_damage = monster_avoid_hazards && P_IsUnderDamage(actor);

    if (!P_Move(actor, dropoff))
        return false;

    if ((on_lift && P_Random(pr_stayonlift) < 230 && !P_IsOnLift(actor)) || (monster_avoid_hazards && !under_damage && (under_damage = P_IsUnderDamage(actor)) && (under_damage < 0 || P_Random(pr_avoidcrush) < 200)))
        actor->movedir = DI_NODIR;

    return true;

}

static boolean P_TryWalk(mobj_t *actor)
{

    if (!P_SmartMove(actor))
        return false;

    actor->movecount = P_Random(pr_trywalk) & 15;

    return true;

}

static void P_DoNewChaseDir(mobj_t *actor, fixed_t deltax, fixed_t deltay)
{

    dirtype_t xdir, ydir, tdir;
    dirtype_t olddir = actor->movedir;
    dirtype_t turnaround = olddir;

    if (turnaround != DI_NODIR)
        turnaround ^= 4;

    xdir = deltax > 10 * FRACUNIT ? DI_EAST : deltax < -10 * FRACUNIT ? DI_WEST : DI_NODIR;
    ydir = deltay < -10 * FRACUNIT ? DI_SOUTH : deltay > 10 * FRACUNIT ? DI_NORTH : DI_NODIR;

    if (xdir != DI_NODIR && ydir != DI_NODIR && turnaround != (actor->movedir = deltay < 0 ? deltax > 0 ? DI_SOUTHEAST : DI_SOUTHWEST : deltax > 0 ? DI_NORTHEAST : DI_NORTHWEST) && P_TryWalk(actor))
        return;

    if (P_Random(pr_newchase) > 200 || D_abs(deltay) > D_abs(deltax))
        tdir = xdir, xdir = ydir, ydir = tdir;

    if ((xdir == turnaround ? xdir = DI_NODIR : xdir) != DI_NODIR && (actor->movedir = xdir, P_TryWalk(actor)))
        return;

    if ((ydir == turnaround ? ydir = DI_NODIR : ydir) != DI_NODIR && (actor->movedir = ydir, P_TryWalk(actor)))
        return;

    if (olddir != DI_NODIR && (actor->movedir = olddir, P_TryWalk(actor)))
        return;


    if (P_Random(pr_newchasedir) & 1)
    {

        for (tdir = DI_EAST; tdir <= DI_SOUTHEAST; tdir++)
        {

            if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
                return;

        }

    }

    else
    {

        for (tdir = DI_SOUTHEAST; tdir != DI_EAST - 1; tdir--)
        {

            if (tdir != turnaround && (actor->movedir = tdir, P_TryWalk(actor)))
                return;

        }

    }

    if ((actor->movedir = turnaround) != DI_NODIR && !P_TryWalk(actor))
        actor->movedir = DI_NODIR;

}


static boolean PIT_AvoidDropoff(line_t *line)
{

    if (line->backsector && tmbbox[BOXRIGHT] > line->bbox[BOXLEFT] && tmbbox[BOXLEFT] < line->bbox[BOXRIGHT] && tmbbox[BOXTOP] > line->bbox[BOXBOTTOM] && tmbbox[BOXBOTTOM] < line->bbox[BOXTOP] && P_BoxOnLineSide(tmbbox, line) == -1)
    {

        fixed_t front = line->frontsector->floorheight;
        fixed_t back  = line->backsector->floorheight;
        angle_t angle;

        if (back == floorz && front < floorz - FRACUNIT * 24)
            angle = R_PointToAngle2(0,0,line->dx,line->dy);
        else if (front == floorz && back < floorz - FRACUNIT * 24)
            angle = R_PointToAngle2(line->dx,line->dy,0,0);
        else
            return true;

        dropoff_deltax -= finesine[angle >> ANGLETOFINESHIFT] * 32;
        dropoff_deltay += finecosine[angle >> ANGLETOFINESHIFT] * 32;

    }

    return true;

}

static fixed_t P_AvoidDropoff(mobj_t *actor)
{

    int yh = ((tmbbox[BOXTOP] = actor->y+actor->radius)-bmaporgy) >> MAPBLOCKSHIFT;
    int yl = ((tmbbox[BOXBOTTOM]= actor->y-actor->radius)-bmaporgy) >> MAPBLOCKSHIFT;
    int xh = ((tmbbox[BOXRIGHT] = actor->x+actor->radius)-bmaporgx) >> MAPBLOCKSHIFT;
    int xl = ((tmbbox[BOXLEFT] = actor->x-actor->radius)-bmaporgx) >> MAPBLOCKSHIFT;
    int bx, by;

    floorz = actor->z;
    dropoff_deltax = dropoff_deltay = 0;
    validcount++;

    for (bx = xl; bx <= xh; bx++)
    {

        for (by = yl; by <= yh; by++)
            P_BlockLinesIterator(bx, by, PIT_AvoidDropoff);

    }

    return dropoff_deltax | dropoff_deltay;

}

static void P_NewChaseDir(mobj_t *actor)
{

    mobj_t *target = actor->target;
    fixed_t deltax = target->x - actor->x;
    fixed_t deltay = target->y - actor->y;

    actor->strafecount = 0;

    if (actor->floorz - actor->dropoffz > FRACUNIT * 24 && actor->z <= actor->floorz && !(actor->flags & (MF_DROPOFF|MF_FLOAT)) && P_AvoidDropoff(actor))
    {

        P_DoNewChaseDir(actor, dropoff_deltax, dropoff_deltay);

        actor->movecount = 1;

        return;

    }

    P_DoNewChaseDir(actor, deltax, deltay);

    if (actor->strafecount)
        actor->movecount = actor->strafecount;

}

static boolean P_IsVisible(mobj_t *actor, mobj_t *mo, boolean allaround)
{

    if (!allaround)
    {

        angle_t an = R_PointToAngle2(actor->x, actor->y, mo->x, mo->y) - actor->angle;

        if (an > ANG90 && an < ANG270 && P_AproxDistance(mo->x-actor->x, mo->y-actor->y) > MELEERANGE)
            return false;

    }

    return P_CheckSight(actor, mo);

}


static boolean PIT_FindTarget(mobj_t *mo)
{

    mobj_t *actor = current_actor;

    if (!((mo->flags ^ actor->flags) & MF_FRIEND && mo->health > 0 && (mo->flags & MF_COUNTKILL || mo->type == MT_SKULL)))
        return true;

    {

        const mobj_t *targ = mo->target;

        if (targ && targ->target == mo && P_Random(pr_skiptarget) > 100 && (targ->flags ^ mo->flags) & MF_FRIEND && targ->health * 2 >= targ->info->spawnhealth)
            return true;

    }

    if (!P_IsVisible(actor, mo, current_allaround))
        return true;

    P_SetTarget(&actor->lastenemy, actor->target);
    P_SetTarget(&actor->target, mo);

    {

        thinker_t *cap = &thinkerclasscap[mo->flags & MF_FRIEND ? th_friends : th_enemies];
        (mo->thinker.cprev->cnext = mo->thinker.cnext)->cprev = mo->thinker.cprev;
        (mo->thinker.cprev = cap->cprev)->cnext = &mo->thinker;
        (mo->thinker.cnext = cap)->cprev = &mo->thinker;

    }

    return false;

}

static boolean P_LookForPlayers(mobj_t *actor, boolean allaround)
{

    player_t *player;
    int stop, stopc, c;

    if (actor->flags & MF_FRIEND)
    {

        int anyone;

        for (anyone = 0; anyone <= 1; anyone++)
            for (c = 0; c < MAXPLAYERS; c++)
                if (playeringame[c] && players[c].playerstate == PST_LIVE && (anyone || P_IsVisible(actor, players[c].mo, allaround)))
                {

                    P_SetTarget(&actor->target, players[c].mo);

                    if (actor->info->missilestate)
                    {

                        P_SetMobjState(actor, actor->info->seestate);

                        actor->flags &= ~MF_JUSTHIT;

                    }

                    return true;

                }

                return false;

    }

    stop = (actor->lastlook - 1) & (MAXPLAYERS - 1);
    c = 0;
    stopc = 2;

    for (;; actor->lastlook = (actor->lastlook + 1) & (MAXPLAYERS - 1))
    {

        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == stopc || actor->lastlook == stop)
            return false;

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;

        if (!P_IsVisible(actor, player->mo, allaround))
            continue;

        P_SetTarget(&actor->target, player->mo);

        actor->threshold = 60;

        return true;

    }

}

static boolean P_LookForMonsters(mobj_t *actor, boolean allaround)
{

    thinker_t *cap, *th;

    if (actor->lastenemy && actor->lastenemy->health > 0 && monsters_remember && !(actor->lastenemy->flags & actor->flags & MF_FRIEND))
    {

        P_SetTarget(&actor->target, actor->lastenemy);
        P_SetTarget(&actor->lastenemy, NULL);

        return true;

    }

    cap = &thinkerclasscap[actor->flags & MF_FRIEND ? th_enemies : th_friends];

    if (cap->cnext != cap)
    {

        int x = (actor->x - bmaporgx) >> MAPBLOCKSHIFT;
        int y = (actor->y - bmaporgy) >> MAPBLOCKSHIFT;
        int d;

        current_actor = actor;
        current_allaround = allaround;

        if (!P_BlockThingsIterator(x, y, PIT_FindTarget))
            return true;

        for (d = 1; d < 5; d++)
        {

            int i = 1 - d;

            do
            {

                if (!P_BlockThingsIterator(x+i, y-d, PIT_FindTarget) || !P_BlockThingsIterator(x+i, y+d, PIT_FindTarget))
                    return true;

            } while (++i < d);

            do
            {

                if (!P_BlockThingsIterator(x-d, y+i, PIT_FindTarget) || !P_BlockThingsIterator(x+d, y+i, PIT_FindTarget))
                    return true;

            } while (--i + d >= 0);

        }

        {

            int n = (P_Random(pr_friends) & 31) + 15;

            for (th = cap->cnext; th != cap; th = th->cnext)
                if (--n < 0)
                {

                    (cap->cnext->cprev = cap->cprev)->cnext = cap->cnext;
                    (cap->cprev = th->cprev)->cnext = cap;
                    (th->cprev = cap)->cnext = th;

                    break;

                }

            else if (!PIT_FindTarget((mobj_t *)th))
                return true;

        }

    }

    return false;

}

static boolean P_LookForTargets(mobj_t *actor, int allaround)
{

    return actor->flags & MF_FRIEND ? P_LookForMonsters(actor, allaround) || P_LookForPlayers (actor, allaround) : P_LookForPlayers (actor, allaround) || P_LookForMonsters(actor, allaround);

}

void A_KeenDie(mobj_t* mo)
{

    thinker_t *th;
    line_t   junk;

    A_Fall(mo);

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
        if (th->function == P_MobjThinker)
        {

            mobj_t *mo2 = (mobj_t *)th;

            if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
                return;

        }

    junk.tag = 666;

    EV_DoDoor(&junk, open);

}

void A_Look(mobj_t *actor)
{

    mobj_t *targ = actor->subsector->sector->soundtarget;

    actor->threshold = 0;
    actor->pursuecount = 0;

    if (!(actor->flags & MF_FRIEND && P_LookForTargets(actor, false)) && !((targ = actor->subsector->sector->soundtarget) && targ->flags & MF_SHOOTABLE && (P_SetTarget(&actor->target, targ), !(actor->flags & MF_AMBUSH) || P_CheckSight(actor, targ))) && (actor->flags & MF_FRIEND || !P_LookForTargets(actor, false)))
        return;

    if (actor->info->seesound)
    {

        int sound;

        switch (actor->info->seesound)
        {

        case sfx_posit1:
        case sfx_posit2:
        case sfx_posit3:
            sound = sfx_posit1+P_Random(pr_see) % 3;

            break;

        case sfx_bgsit1:
        case sfx_bgsit2:
            sound = sfx_bgsit1+P_Random(pr_see) % 2;

            break;

        default:
            sound = actor->info->seesound;

            break;

        }

        if (actor->type==MT_SPIDER || actor->type == MT_CYBORG)
            S_StartSound(NULL, sound);
        else
            S_StartSound(actor, sound);

    }

    P_SetMobjState(actor, actor->info->seestate);

}

void A_Chase(mobj_t *actor)
{

    if (actor->reactiontime)
        actor->reactiontime--;

    if (actor->threshold)
    {

        if (!actor->target || actor->target->health <= 0)
            actor->threshold = 0;
        else
            actor->threshold--;

    }

    if (actor->strafecount)
        A_FaceTarget(actor);        
    else if (actor->movedir < 8)
    {

        int delta = (actor->angle &= (7 << 29)) - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90 / 2;
        else if (delta < 0)
            actor->angle += ANG90 / 2;

    }

    if (!actor->target || !(actor->target->flags & MF_SHOOTABLE))
    {

        if (!P_LookForTargets(actor, true))
            P_SetMobjState(actor, actor->info->spawnstate);

        return;

    }

    if (actor->flags & MF_JUSTATTACKED)
    {

        actor->flags &= ~MF_JUSTATTACKED;

        if (gameskill != sk_nightmare)
            P_NewChaseDir(actor);

        return;

    }

    if (actor->info->meleestate && P_CheckMeleeRange(actor))
    {

        if (actor->info->attacksound)
            S_StartSound(actor, actor->info->attacksound);

        P_SetMobjState(actor, actor->info->meleestate);

        if (!actor->info->missilestate)
            actor->flags |= MF_JUSTHIT;

        return;

    }

    if (actor->info->missilestate)
        if (!(gameskill < sk_nightmare && actor->movecount))
            if (P_CheckMissileRange(actor))
            {

                P_SetMobjState(actor, actor->info->missilestate);

                actor->flags |= MF_JUSTATTACKED;

                return;

            }

    if (!actor->threshold)
    {

        if (actor->pursuecount)
            actor->pursuecount--;
        else
        {

            actor->pursuecount = BASETHRESHOLD;

            if (!(actor->target && actor->target->health > 0 && ((((actor->target->flags ^ actor->flags) & MF_FRIEND || (!(actor->flags & MF_FRIEND) && monster_infighting)) && P_CheckSight(actor, actor->target)))) && P_LookForTargets(actor, true))
                return;

            if (!actor->info->missilestate && actor->flags & MF_FRIEND)
            {

                if (actor->flags & MF_JUSTHIT)
                    actor->flags &= ~MF_JUSTHIT;
                else if (P_LookForPlayers(actor, true))
                    return;

            }

        }

    }

    if (actor->strafecount)
        actor->strafecount--;

    if (--actor->movecount<0 || !P_SmartMove(actor))
        P_NewChaseDir(actor);

    if (actor->info->activesound && P_Random(pr_see) < 3)
        S_StartSound(actor, actor->info->activesound);

}

void A_FaceTarget(mobj_t *actor)
{

    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;
    actor->angle = R_PointToAngle2(actor->x, actor->y, actor->target->x, actor->target->y);

    if (actor->target->flags & MF_SHADOW)
    {

        int t = P_Random(pr_facetarget);

        actor->angle += (t - P_Random(pr_facetarget)) << 21;

    }

}

void A_PosAttack(mobj_t *actor)
{

    int angle, damage, slope, t;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    angle = actor->angle;
    slope = P_AimLineAttack(actor, angle, MISSILERANGE, 0);

    S_StartSound(actor, sfx_pistol);

    t = P_Random(pr_posattack);
    angle += (t - P_Random(pr_posattack)) << 20;
    damage = (P_Random(pr_posattack) % 5 + 1) * 3;

    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);

}

void A_SPosAttack(mobj_t *actor)
{

    int i, bangle, slope;

    if (!actor->target)
        return;

    S_StartSound(actor, sfx_shotgn);
    A_FaceTarget(actor);

    bangle = actor->angle;
    slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0);

    for (i = 0; i < 3; i++)
    {

        int t = P_Random(pr_sposattack);
        int angle = bangle + ((t - P_Random(pr_sposattack)) << 20);
        int damage = ((P_Random(pr_sposattack) % 5) + 1) * 3;

        P_LineAttack(actor, angle, MISSILERANGE, slope, damage);

    }

}

void A_CPosAttack(mobj_t *actor)
{

    int angle, bangle, damage, slope, t;

    if (!actor->target)
        return;

    S_StartSound(actor, sfx_shotgn);
    A_FaceTarget(actor);

    bangle = actor->angle;
    slope = P_AimLineAttack(actor, bangle, MISSILERANGE, 0);
    t = P_Random(pr_cposattack);
    angle = bangle + ((t - P_Random(pr_cposattack)) << 20);
    damage = ((P_Random(pr_cposattack) % 5) + 1) * 3;

    P_LineAttack(actor, angle, MISSILERANGE, slope, damage);

}

void A_CPosRefire(mobj_t *actor)
{

    A_FaceTarget(actor);

    if (P_HitFriend(actor))
        goto stop;

    if (P_Random(pr_cposrefire) < 40)
    {

        if (actor->target && actor->flags & actor->target->flags & MF_FRIEND)
            goto stop;
        else
            return;

    }

    if (!actor->target || actor->target->health <= 0 || !P_CheckSight(actor, actor->target))
    {

    stop:
        P_SetMobjState(actor, actor->info->seestate);

    }

}

void A_SpidRefire(mobj_t* actor)
{

    A_FaceTarget(actor);

    if (P_HitFriend(actor))
        goto stop;

    if (P_Random(pr_spidrefire) < 10)
        return;

    if (!actor->target || actor->target->health <= 0 || actor->flags & actor->target->flags & MF_FRIEND || !P_CheckSight(actor, actor->target))
    {

    stop:
        P_SetMobjState(actor, actor->info->seestate);

    }

}

void A_BspiAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, MT_ARACHPLAZ);

}

void A_TroopAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {

        int damage;

        S_StartSound(actor, sfx_claw);

        damage = (P_Random(pr_troopattack) % 8 + 1) * 3;

        P_DamageMobj(actor->target, actor, actor, damage);

        return;

    }

    P_SpawnMissile(actor, actor->target, MT_TROOPSHOT);

}

void A_SargAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {

        int damage = ((P_Random(pr_sargattack) % 10) + 1) * 4;

        P_DamageMobj(actor->target, actor, actor, damage);

    }

}

void A_HeadAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {

        int damage = (P_Random(pr_headattack) % 6 + 1) * 10;

        P_DamageMobj(actor->target, actor, actor, damage);

        return;

    }

    P_SpawnMissile(actor, actor->target, MT_HEADSHOT);

}

void A_CyberAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    P_SpawnMissile(actor, actor->target, MT_ROCKET);

}

void A_BruisAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    if (P_CheckMeleeRange(actor))
    {

        int damage;
        S_StartSound(actor, sfx_claw);

        damage = (P_Random(pr_bruisattack) % 8 + 1) * 10;

        P_DamageMobj(actor->target, actor, actor, damage);

        return;

    }

    P_SpawnMissile(actor, actor->target, MT_BRUISERSHOT);

}

void A_SkelMissile(mobj_t *actor)
{

    mobj_t *mo;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    actor->z += 16 * FRACUNIT;
    mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
    actor->z -= 16 * FRACUNIT;
    mo->x += mo->momx;
    mo->y += mo->momy;

    P_SetTarget(&mo->tracer, actor->target);

}

void A_Tracer(mobj_t *actor)
{

    angle_t exact;
    fixed_t dist;
    fixed_t slope;
    mobj_t *dest;
    mobj_t *th;

    if ((gametic-basetic) & 3)
        return;

    P_SpawnPuff(actor->x, actor->y, actor->z);

    th = P_SpawnMobj (actor->x-actor->momx, actor->y-actor->momy, actor->z, MT_SMOKE);
    th->momz = FRACUNIT;
    th->tics -= P_Random(pr_tracer) & 3;

    if (th->tics < 1)
        th->tics = 1;

    dest = actor->tracer;

    if (!dest || dest->health <= 0)
        return;

    exact = R_PointToAngle2(actor->x, actor->y, dest->x, dest->y);

    if (exact != actor->angle)
    {

        if (exact - actor->angle > 0x80000000)
        {

            actor->angle -= TRACEANGLE;

            if (exact - actor->angle < 0x80000000)
                actor->angle = exact;

        }

        else
        {

            actor->angle += TRACEANGLE;

            if (exact - actor->angle > 0x80000000)
                actor->angle = exact;

        }

    }

    exact = actor->angle>>ANGLETOFINESHIFT;
    actor->momx = FixedMul(actor->info->speed, finecosine[exact]);
    actor->momy = FixedMul(actor->info->speed, finesine[exact]);
    dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist = dist / actor->info->speed;

    if (dist < 1)
        dist = 1;

    slope = (dest->z + 40 * FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
        actor->momz -= FRACUNIT / 8;
    else
        actor->momz += FRACUNIT / 8;

}

void A_SkelWhoosh(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    S_StartSound(actor, sfx_skeswg);

}

void A_SkelFist(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (P_CheckMeleeRange(actor))
    {

        int damage = ((P_Random(pr_skelfist) % 10) + 1) * 6;

        S_StartSound(actor, sfx_skepch);
        P_DamageMobj(actor->target, actor, actor, damage);

    }

}

static boolean PIT_VileCheck(mobj_t *thing)
{

    int maxdist;
    boolean check;

    if (!(thing->flags & MF_CORPSE))
        return true;

    if (thing->tics != -1)
        return true;

    if (thing->info->raisestate == S_NULL)
        return true;

    maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;

    if (D_abs(thing->x-viletryx) > maxdist || D_abs(thing->y-viletryy) > maxdist)
        return true;

    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;

    {

        int height = corpsehit->height;
        int radius = corpsehit->radius;

        corpsehit->height = corpsehit->info->height;
        corpsehit->radius = corpsehit->info->radius;
        corpsehit->flags |= MF_SOLID;
        check = P_CheckPosition(corpsehit,corpsehit->x,corpsehit->y);
        corpsehit->height = height;
        corpsehit->radius = radius;
        corpsehit->flags &= ~MF_SOLID;

    }

    if (!check)
        return true;

    return false;

}

void A_VileChase(mobj_t* actor)
{

    int xl, xh;
    int yl, yh;
    int bx, by;

    if (actor->movedir != DI_NODIR)
    {

        viletryx = actor->x + actor->info->speed*xspeed[actor->movedir];
        viletryy = actor->y + actor->info->speed*yspeed[actor->movedir];
        xl = (viletryx - bmaporgx - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        xh = (viletryx - bmaporgx + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yl = (viletryy - bmaporgy - MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        yh = (viletryy - bmaporgy + MAXRADIUS * 2) >> MAPBLOCKSHIFT;
        vileobj = actor;

        for (bx = xl; bx <= xh; bx++)
        {

            for (by = yl; by <= yh; by++)
            {

                if (!P_BlockThingsIterator(bx, by, PIT_VileCheck))
                {

                    mobjinfo_t *info;
                    mobj_t* temp = actor->target;

                    actor->target = corpsehit;

                    A_FaceTarget(actor);

                    actor->target = temp;

                    P_SetMobjState(actor, S_VILE_HEAL1);
                    S_StartSound(corpsehit, sfx_slop);

                    info = corpsehit->info;

                    P_SetMobjState(corpsehit,info->raisestate);

                    corpsehit->height = info->height;
                    corpsehit->radius = info->radius;
                    corpsehit->flags = (info->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);

                    if (!((corpsehit->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
                        totallive++;
          
                    corpsehit->health = info->spawnhealth;

                    P_SetTarget(&corpsehit->target, NULL);
                    P_SetTarget(&corpsehit->lastenemy, NULL);

                    corpsehit->flags &= ~MF_JUSTHIT;

                    P_UpdateThinker(&corpsehit->thinker);

                    return;

                }

            }

        }

    }

    A_Chase(actor);

}

void A_VileStart(mobj_t *actor)
{

    S_StartSound(actor, sfx_vilatk);

}

void A_StartFire(mobj_t *actor)
{

    S_StartSound(actor, sfx_flamst);
    A_Fire(actor);

}

void A_FireCrackle(mobj_t* actor)
{

    S_StartSound(actor, sfx_flame);
    A_Fire(actor);

}

void A_Fire(mobj_t *actor)
{

    unsigned an;
    mobj_t *dest = actor->tracer;

    if (!dest)
        return;

    if (!P_CheckSight(actor->target, dest))
        return;

    an = dest->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition(actor);

    actor->x = dest->x + FixedMul(24 * FRACUNIT, finecosine[an]);
    actor->y = dest->y + FixedMul(24 * FRACUNIT, finesine[an]);
    actor->z = dest->z;

    P_SetThingPosition(actor);

}

void A_VileTarget(mobj_t *actor)
{

    mobj_t *fog;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    fog = P_SpawnMobj(actor->target->x, actor->target->y, actor->target->z, MT_FIRE);

    P_SetTarget(&actor->tracer, fog);
    P_SetTarget(&fog->target, actor);
    P_SetTarget(&fog->tracer, actor->target);
    A_Fire(fog);

}

void A_VileAttack(mobj_t *actor)
{

    mobj_t *fire;
    int an;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    if (!P_CheckSight(actor, actor->target))
        return;

    S_StartSound(actor, sfx_barexp);
    P_DamageMobj(actor->target, actor, actor, 20);

    actor->target->momz = 1000 * FRACUNIT / actor->target->info->mass;
    an = actor->angle >> ANGLETOFINESHIFT;
    fire = actor->tracer;

    if (!fire)
        return;

    fire->x = actor->target->x - FixedMul (24 * FRACUNIT, finecosine[an]);
    fire->y = actor->target->y - FixedMul (24 * FRACUNIT, finesine[an]);

    P_RadiusAttack(fire, actor, 70);

}

void A_FatRaise(mobj_t *actor)
{

    A_FaceTarget(actor);
    S_StartSound(actor, sfx_manatk);

}

void A_FatAttack1(mobj_t *actor)
{

    mobj_t *mo;
    int an;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    actor->angle += FATSPREAD;

    P_SpawnMissile(actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

}

void A_FatAttack2(mobj_t *actor)
{

    mobj_t *mo;
    int an;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    actor->angle -= FATSPREAD;

    P_SpawnMissile(actor, actor->target, MT_FATSHOT);

    mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
    mo->angle -= FATSPREAD * 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

}

void A_FatAttack3(mobj_t *actor)
{

    mobj_t *mo;
    int an;

    if (!actor->target)
        return;

    A_FaceTarget(actor);

    mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
    mo->angle -= FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);
    mo = P_SpawnMissile(actor, actor->target, MT_FATSHOT);
    mo->angle += FATSPREAD / 2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul(mo->info->speed, finecosine[an]);
    mo->momy = FixedMul(mo->info->speed, finesine[an]);

}

void A_SkullAttack(mobj_t *actor)
{

    mobj_t *dest;
    angle_t an;
    int dist;

    if (!actor->target)
        return;

    dest = actor->target;
    actor->flags |= MF_SKULLFLY;

    S_StartSound(actor, actor->info->attacksound);
    A_FaceTarget(actor);

    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul(SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul(SKULLSPEED, finesine[an]);
    dist = P_AproxDistance(dest->x - actor->x, dest->y - actor->y);
    dist = dist / SKULLSPEED;

    if (dist < 1)
        dist = 1;

    actor->momz = (dest->z + (dest->height >> 1) - actor->z) / dist;

}

static void A_PainShootSkull(mobj_t *actor, angle_t angle)
{

    mobj_t *newmobj;
    angle_t an = angle >> ANGLETOFINESHIFT;
    int prestep = 4 * FRACUNIT + 3 * (actor->info->radius + mobjinfo[MT_SKULL].radius) / 2;
    fixed_t x = actor->x + FixedMul(prestep, finecosine[an]);
    fixed_t y = actor->y + FixedMul(prestep, finesine[an]);
    fixed_t z = actor->z + 8 * FRACUNIT;

    if (Check_Sides(actor,x,y))
        return;

    newmobj = P_SpawnMobj(x, y, z, MT_SKULL);

    if ((newmobj->z > (newmobj->subsector->sector->ceilingheight - newmobj->height)) || (newmobj->z < newmobj->subsector->sector->floorheight))
    {

        P_DamageMobj(newmobj, actor, actor, 10000);

        return;

    }

    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (actor->flags & MF_FRIEND);

    P_UpdateThinker(&newmobj->thinker);

    if (!P_TryMove(newmobj, newmobj->x, newmobj->y, false))
    {

        P_DamageMobj(newmobj, actor, actor, 10000);

        return;

    }

    P_SetTarget(&newmobj->target, actor->target);
    A_SkullAttack(newmobj);

}

void A_PainAttack(mobj_t *actor)
{

    if (!actor->target)
        return;

    A_FaceTarget(actor);
    A_PainShootSkull(actor, actor->angle);

}

void A_PainDie(mobj_t *actor)
{

    A_Fall(actor);
    A_PainShootSkull(actor, actor->angle + ANG90);
    A_PainShootSkull(actor, actor->angle + ANG180);
    A_PainShootSkull(actor, actor->angle + ANG270);

}

void A_Scream(mobj_t *actor)
{

    int sound;

    switch (actor->info->deathsound)
    {

    case 0:
        return;

    case sfx_podth1:
    case sfx_podth2:
    case sfx_podth3:
        sound = sfx_podth1 + P_Random(pr_scream) % 3;

        break;

    case sfx_bgdth1:
    case sfx_bgdth2:
        sound = sfx_bgdth1 + P_Random(pr_scream) % 2;

        break;

    default:
        sound = actor->info->deathsound;

        break;

    }

    if (actor->type == MT_SPIDER || actor->type == MT_CYBORG)
        S_StartSound(NULL, sound);
    else
        S_StartSound(actor, sound);

}

void A_XScream(mobj_t *actor)
{

    S_StartSound(actor, sfx_slop);

}

void A_Pain(mobj_t *actor)
{

    if (actor->info->painsound)
        S_StartSound(actor, actor->info->painsound);

}

void A_Fall(mobj_t *actor)
{

    actor->flags &= ~MF_SOLID;

}

void A_Explode(mobj_t *thingy)
{

    P_RadiusAttack(thingy, thingy->target, 128);

}

void A_BossDeath(mobj_t *mo)
{

    thinker_t *th;
    line_t junk;
    int i;

    if (gamemode == commercial)
    {

        if (gamemap != 7)
            return;

        if ((mo->type != MT_FATSO) && (mo->type != MT_BABY))
            return;

    }

    else
    {

        switch (gameepisode)
        {

        case 1:
            if (gamemap != 8)
                return;

            if (mo->type != MT_BRUISER)
                return;

            break;

        case 2:
            if (gamemap != 8)
                return;

            if (mo->type != MT_CYBORG)
                return;

            break;

        case 3:
            if (gamemap != 8)
                return;

            if (mo->type != MT_SPIDER)
                return;

            break;

        case 4:
            switch (gamemap)
            {

            case 6:
                if (mo->type != MT_CYBORG)
                    return;

                break;

            case 8:
                if (mo->type != MT_SPIDER)
                    return;

                break;

            default:
                return;

            }

            break;

        default:
            if (gamemap != 8)
                return;

            break;

        }

    }

    for (i = 0; i < MAXPLAYERS; i++)
        if (playeringame[i] && players[i].health > 0)
            break;

    if (i == MAXPLAYERS)
        return;

    for (th = thinkercap.next; th != &thinkercap; th = th->next)
        if (th->function == P_MobjThinker)
        {

            mobj_t *mo2 = (mobj_t *)th;

            if (mo2 != mo && mo2->type == mo->type && mo2->health > 0)
                return;

        }

    if (gamemode == commercial)
    {

        if (gamemap == 7)
        {

            if (mo->type == MT_FATSO)
            {

                junk.tag = 666;

                EV_DoFloor(&junk, lowerFloorToLowest);

                return;

            }

            if (mo->type == MT_BABY)
            {

                junk.tag = 667;

                EV_DoFloor(&junk,raiseToTexture);

                return;

            }

        }

    }

    else
    {

        switch (gameepisode)
        {

        case 1:
            junk.tag = 666;

            EV_DoFloor(&junk, lowerFloorToLowest);

            return;

        case 4:
            switch (gamemap)
            {

            case 6:
                junk.tag = 666;

                EV_DoDoor(&junk, blazeOpen);

                return;

            case 8:
                junk.tag = 666;

                EV_DoFloor(&junk, lowerFloorToLowest);

                return;

            }

        }

    }

    G_ExitLevel();

}

void A_Hoof(mobj_t *mo)
{

    S_StartSound(mo, sfx_hoof);
    A_Chase(mo);

}

void A_Metal(mobj_t *mo)
{

    S_StartSound(mo, sfx_metal);
    A_Chase(mo);

}

void A_BabyMetal(mobj_t *mo)
{

    S_StartSound(mo, sfx_bspwlk);
    A_Chase(mo);

}

void A_OpenShotgun2(player_t *player, pspdef_t *psp)
{

    S_StartSound(player->mo, sfx_dbopn);

}

void A_LoadShotgun2(player_t *player, pspdef_t *psp)
{

    S_StartSound(player->mo, sfx_dbload);

}

void A_CloseShotgun2(player_t *player, pspdef_t *psp)
{

    S_StartSound(player->mo, sfx_dbcls);
    A_ReFire(player, psp);

}

void P_SpawnBrainTargets(void)
{

    thinker_t *thinker;

    numbraintargets = 0;
    brain.targeton = 0;
    brain.easy = 0;

    for (thinker = thinkercap.next; thinker != &thinkercap; thinker = thinker->next)
    {

        if (thinker->function == P_MobjThinker)
        {

            mobj_t *m = (mobj_t *)thinker;

            if (m->type == MT_BOSSTARGET)
            {

                if (numbraintargets >= numbraintargets_alloc)
                    braintargets = realloc(braintargets, (numbraintargets_alloc = numbraintargets_alloc ? numbraintargets_alloc * 2 : 32) * sizeof *braintargets);

                braintargets[numbraintargets++] = m;

            }

        }

    }

}

void A_BrainAwake(mobj_t *mo)
{

    S_StartSound(NULL, sfx_bossit);

}

void A_BrainPain(mobj_t *mo)
{

    S_StartSound(NULL, sfx_bospn);

}

void A_BrainScream(mobj_t *mo)
{

    int x;

    for (x = mo->x - 196 * FRACUNIT; x < mo->x + 320 * FRACUNIT; x += FRACUNIT * 8)
    {

        int y = mo->y - 320 * FRACUNIT;
        int z = 128 + P_Random(pr_brainscream) * 2 * FRACUNIT;
        mobj_t *th = P_SpawnMobj(x, y, z, MT_ROCKET);

        th->momz = P_Random(pr_brainscream) * 512;

        P_SetMobjState(th, S_BRAINEXPLODE1);

        th->tics -= P_Random(pr_brainscream) & 7;

        if (th->tics < 1)
            th->tics = 1;

    }

    S_StartSound(NULL, sfx_bosdth);

}

void A_BrainExplode(mobj_t *mo)
{

    int t = P_Random(pr_brainexp);
    int x = mo->x + (t - P_Random(pr_brainexp)) * 2048;
    int y = mo->y;
    int z = 128 + P_Random(pr_brainexp) * 2 * FRACUNIT;

    mobj_t *th = P_SpawnMobj(x, y, z, MT_ROCKET);
    th->momz = P_Random(pr_brainexp) * 512;

    P_SetMobjState(th, S_BRAINEXPLODE1);

    th->tics -= P_Random(pr_brainexp) & 7;

    if (th->tics < 1)
        th->tics = 1;

}

void A_BrainDie(mobj_t *mo)
{

    G_ExitLevel();

}

void A_BrainSpit(mobj_t *mo)
{

    mobj_t *targ, *newmobj;

    if (!numbraintargets)
        return;

    brain.easy ^= 1;

    if (gameskill <= sk_easy && !brain.easy)
        return;

    targ = braintargets[brain.targeton++];
    brain.targeton %= numbraintargets;
    newmobj = P_SpawnMissile(mo, targ, MT_SPAWNSHOT);

    P_SetTarget(&newmobj->target, targ);

    newmobj->reactiontime = (short)(((targ->y-mo->y) / newmobj->momy) / newmobj->state->tics);
    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

    P_UpdateThinker(&newmobj->thinker);
    S_StartSound(NULL, sfx_bospit);

}

void A_SpawnSound(mobj_t *mo)
{

    S_StartSound(mo, sfx_boscub);
    A_SpawnFly(mo);

}

void A_SpawnFly(mobj_t *mo)
{

    mobj_t *newmobj;
    mobj_t *fog;
    mobj_t *targ;
    int r;
    mobjtype_t type;

    if (--mo->reactiontime)
        return;

    targ = mo->target;
    fog = P_SpawnMobj(targ->x, targ->y, targ->z, MT_SPAWNFIRE);

    S_StartSound(fog, sfx_telept);

    r = P_Random(pr_spawnfly);

    if (r < 50)
        type = MT_TROOP;
    else if (r < 90)
        type = MT_SERGEANT;
    else if (r < 120)
        type = MT_SHADOWS;
    else if (r < 130)
        type = MT_PAIN;
    else if (r < 160)
        type = MT_HEAD;
    else if (r < 162)
        type = MT_VILE;
    else if (r < 172)
        type = MT_UNDEAD;
    else if (r < 192)
        type = MT_BABY;
    else if (r < 222)
        type = MT_FATSO;
    else if (r < 246)
        type = MT_KNIGHT;
    else
        type = MT_BRUISER;

    newmobj = P_SpawnMobj(targ->x, targ->y, targ->z, type);
    newmobj->flags = (newmobj->flags & ~MF_FRIEND) | (mo->flags & MF_FRIEND);

    P_UpdateThinker(&newmobj->thinker);

    if (P_LookForTargets(newmobj, true))
        P_SetMobjState(newmobj, newmobj->info->seestate);

    P_TeleportMove(newmobj, newmobj->x, newmobj->y, true);
    P_RemoveMobj(mo);

}

void A_PlayerScream(mobj_t *mo)
{

    int sound = sfx_pldeth;

    if (gamemode != shareware && mo->health < -50)
        sound = sfx_pdiehi;

    S_StartSound(mo, sound);

}

void A_Die(mobj_t *actor)
{

    P_DamageMobj(actor, NULL, NULL, actor->health);

}

void A_Detonate(mobj_t *mo)
{

    P_RadiusAttack(mo, mo->target, mo->info->damage);

}

void A_Mushroom(mobj_t *actor)
{

    int i, j, n = actor->info->damage;

    A_Explode(actor);

    for (i = -n; i <= n; i += 8)
    {

        for (j = -n; j <= n; j += 8)
        {

            mobj_t target = *actor, *mo;

            target.x += i << FRACBITS;
            target.y += j << FRACBITS;
            target.z += P_AproxDistance(i, j) << (FRACBITS + 2);
            mo = P_SpawnMissile(actor, &target, MT_FATSHOT);
            mo->momx >>= 1;
            mo->momy >>= 1;
            mo->momz >>= 1;
            mo->flags &= ~MF_NOGRAVITY;

        }

    }

}

void A_Spawn(mobj_t *mo)
{

    if (mo->state->misc1)
        P_SpawnMobj(mo->x, mo->y, (mo->state->misc2 << FRACBITS) + mo->z, mo->state->misc1 - 1);

}

void A_Turn(mobj_t *mo)
{

    mo->angle += (unsigned int)(((uint_64_t) mo->state->misc1 << 32) / 360);

}

void A_Face(mobj_t *mo)
{

    mo->angle = (unsigned int)(((uint_64_t) mo->state->misc1 << 32) / 360);

}

void A_Scratch(mobj_t *mo)
{

    mo->target && (A_FaceTarget(mo), P_CheckMeleeRange(mo)) ? mo->state->misc2 ? S_StartSound(mo, mo->state->misc2) : (void) 0, P_DamageMobj(mo->target, mo, mo, mo->state->misc1) : (void) 0;

}

void A_PlaySound(mobj_t *mo)
{

    S_StartSound(mo->state->misc2 ? NULL : mo, mo->state->misc1);

}

void A_RandomJump(mobj_t *mo)
{

    if (P_Random(pr_randomjump) < mo->state->misc2)
        P_SetMobjState(mo, mo->state->misc1);

}

void A_LineEffect(mobj_t *mo)
{

    player_t player;
    player_t *oldplayer;
    static line_t junk;

    junk = *lines;
    oldplayer = mo->player;
    mo->player = &player;
    player.health = 100;
    junk.special = (short)mo->state->misc1;

    if (!junk.special)
        return;

    junk.tag = (short)mo->state->misc2;

    if (!P_UseSpecialLine(mo, &junk, 0))
        P_CrossSpecialLine(&junk, 0, mo);

    mo->state->misc1 = junk.special;
    mo->player = oldplayer;

}

