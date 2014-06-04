#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_tick.h"
#include "st_stuff.h"
#include "hu_stuff.h"
#include "s_sound.h"
#include "info.h"
#include "g_game.h"
#include "p_inter.h"
#include "i_system.h"
#include "z_zone.h"

static mapthing_t itemrespawnque[ITEMQUESIZE];
static int itemrespawntime[ITEMQUESIZE];
static int iquehead;
static int iquetail;
extern fixed_t attackrange;

boolean P_SetMobjState(mobj_t *mobj, statenum_t state)
{

    state_t *st;
    static statenum_t seenstate_tab[NUMSTATES];
    statenum_t *seenstate = seenstate_tab;
    static int recursion;
    statenum_t i = state;
    boolean ret = true;
    statenum_t tempstate[NUMSTATES];

    if (recursion++)
        memset(seenstate = tempstate, 0, sizeof tempstate);

    do
    {

        if (state == S_NULL)
        {

            mobj->state = (state_t *)S_NULL;

            P_RemoveMobj(mobj);

            ret = false;

            break;

        }

        st = &states[state];
        mobj->state = st;
        mobj->tics = st->tics;
        mobj->sprite = st->sprite;
        mobj->frame = st->frame;

        if (st->action)
            st->action(mobj);

        seenstate[state] = 1 + st->nextstate;
        state = st->nextstate;

    } while (!mobj->tics && !seenstate[state]);

    if (ret && !mobj->tics)
        I_Error("P_SetMobjState: State Cycle Detected");

    if (!--recursion)
    {

        for (; (state=seenstate[i]); i = state - 1)
            seenstate[i] = 0;

    }

    return ret;

}

void P_ExplodeMissile(mobj_t *mo)
{

    mo->momx = mo->momy = mo->momz = 0;

    P_SetMobjState (mo, mobjinfo[mo->type].deathstate);

    mo->tics -= P_Random(pr_explode)&3;

    if (mo->tics < 1)
        mo->tics = 1;

    mo->flags &= ~MF_MISSILE;

    if (mo->info->deathsound)
        S_StartSound (mo, mo->info->deathsound);

}

static void P_XYMovement(mobj_t *mo)
{

    player_t *player;
    fixed_t xmove, ymove;

    if (!(mo->momx | mo->momy))
    {

        if (mo->flags & MF_SKULLFLY)
        {

            mo->flags &= ~MF_SKULLFLY;
            mo->momz = 0;

            P_SetMobjState(mo, mo->info->spawnstate);

        }

        return;

    }

    player = mo->player;

    if (mo->momx > MAXMOVE)
        mo->momx = MAXMOVE;
    else if (mo->momx < -MAXMOVE)
        mo->momx = -MAXMOVE;

    if (mo->momy > MAXMOVE)
        mo->momy = MAXMOVE;
    else if (mo->momy < -MAXMOVE)
        mo->momy = -MAXMOVE;

    xmove = mo->momx;
    ymove = mo->momy;

    do
    {

        fixed_t ptryx, ptryy;

        if (xmove > MAXMOVE / 2 || ymove > MAXMOVE / 2 || (xmove < -MAXMOVE / 2 || ymove < -MAXMOVE / 2))
        {

            ptryx = mo->x + xmove / 2;
            ptryy = mo->y + ymove / 2;
            xmove >>= 1;
            ymove >>= 1;

        }

        else
        {

            ptryx = mo->x + xmove;
            ptryy = mo->y + ymove;
            xmove = ymove = 0;

        }

        if (!P_TryMove(mo, ptryx, ptryy, true))
        {

            if (!(mo->flags & MF_MISSILE) && (mo->flags & MF_BOUNCES || (!player && blockline && mo->z <= mo->floorz && P_GetFriction(mo, NULL) > ORIG_FRICTION)))
            {

                if (blockline)
                {

                    fixed_t r = ((blockline->dx >> FRACBITS) * mo->momx + (blockline->dy >> FRACBITS) * mo->momy) / ((blockline->dx >> FRACBITS) * (blockline->dx >> FRACBITS) + (blockline->dy >> FRACBITS)*(blockline->dy >> FRACBITS));
                    fixed_t x = FixedMul(r, blockline->dx);
                    fixed_t y = FixedMul(r, blockline->dy);
                    mo->momx = x * 2 - mo->momx;
                    mo->momy = y * 2 - mo->momy;

                    if (!(mo->flags & MF_NOGRAVITY))
                    {

                        mo->momx = (mo->momx + x) / 2;
                        mo->momy = (mo->momy + y) / 2;

                    }

                }

                else
                {

                    mo->momx = mo->momy = 0;

                }

            }

            else if (player)
            {

                P_SlideMove(mo);

            }

            else if (mo->flags & MF_MISSILE)
            {


                if (ceilingline && ceilingline->backsector && ceilingline->backsector->ceilingpic == skyflatnum)
                {

                    if (mo->z > ceilingline->backsector->ceilingheight)
                    {

                        P_RemoveMobj(mo);

                        return;

                    }

                }

                P_ExplodeMissile(mo);

            }

            else
            {

                mo->momx = mo->momy = 0;

            }

        }

    } while (xmove || ymove);

    if (mo->flags & (MF_MISSILE | MF_SKULLFLY) || mo->z > mo->floorz)
        return;

    if (((mo->flags & MF_BOUNCES && mo->z > mo->dropoffz) || mo->flags & MF_CORPSE || mo->intflags & MIF_FALLING) && (mo->momx > FRACUNIT / 4 || mo->momx < -FRACUNIT / 4 || mo->momy > FRACUNIT / 4 || mo->momy < -FRACUNIT / 4) && mo->floorz != mo->subsector->sector->floorheight)
        return;

    if (mo->momx > -STOPSPEED && mo->momx < STOPSPEED && mo->momy > -STOPSPEED && mo->momy < STOPSPEED && (!player || !(player->cmd.forwardmove | player->cmd.sidemove) || (player->mo != mo)))
    {

        if (player && (unsigned)(player->mo->state - states - S_PLAY_RUN1) < 4)
            P_SetMobjState(player->mo, S_PLAY);

        mo->momx = mo->momy = 0;

        if (player && player->mo == mo)
            player->momx = player->momy = 0;

    }

    else
    {

        fixed_t friction = P_GetFriction(mo, NULL);

        mo->momx = FixedMul(mo->momx, friction);
        mo->momy = FixedMul(mo->momy, friction);

        if (player && player->mo == mo)
        {

            player->momx = FixedMul(player->momx, ORIG_FRICTION);
            player->momy = FixedMul(player->momy, ORIG_FRICTION);

        }

    }

}

static void P_ZMovement(mobj_t *mo)
{

    if (mo->flags & MF_BOUNCES && mo->momz)
    {

        mo->z += mo->momz;

        if (mo->z <= mo->floorz)
        {

            mo->z = mo->floorz;

            if (mo->momz < 0)
            {

                mo->momz = -mo->momz;

                if (!(mo->flags & MF_NOGRAVITY))
                {

                    mo->momz = mo->flags & MF_FLOAT ? mo->flags & MF_DROPOFF ? FixedMul(mo->momz, (fixed_t)(FRACUNIT * 0.85)) : FixedMul(mo->momz, (fixed_t)(FRACUNIT * 0.70)) : FixedMul(mo->momz, (fixed_t)(FRACUNIT * 0.45));

                    if (D_abs(mo->momz) <= mo->info->mass * (GRAVITY * 4 / 256))
                        mo->momz = 0;

                }

                if (mo->flags & MF_TOUCHY && mo->intflags & MIF_ARMED && mo->health > 0)
                    P_DamageMobj(mo, NULL, NULL, mo->health);
                else if (mo->flags & MF_FLOAT && sentient(mo))
                    goto floater;

                return;

            }

        }
        
        else if (mo->z >= mo->ceilingz - mo->height)
        {

            mo->z = mo->ceilingz - mo->height;

            if (mo->momz > 0)
            {

                if (mo->subsector->sector->ceilingpic != skyflatnum)
                    mo->momz = -mo->momz;
                else if (mo->flags & MF_MISSILE)
                    P_RemoveMobj(mo);
                else if (mo->flags & MF_NOGRAVITY)
                    mo->momz = -mo->momz;

                if (mo->flags & MF_FLOAT && sentient(mo))
                    goto floater;

                return;

            }

        }
        
        else
        {

            if (!(mo->flags & MF_NOGRAVITY))
                mo->momz -= mo->info->mass * (GRAVITY / 256);

            if (mo->flags & MF_FLOAT && sentient(mo))
                goto floater;

            return;

        }

        mo->momz = 0;

        if (mo->flags & MF_MISSILE)
        {

            if (ceilingline && ceilingline->backsector && ceilingline->backsector->ceilingpic == skyflatnum && mo->z > ceilingline->backsector->ceilingheight)
                P_RemoveMobj(mo);
            else
                P_ExplodeMissile(mo);

        }

        if (mo->flags & MF_FLOAT && sentient(mo))
            goto floater;

        return;

    }

    if (mo->player && mo->player->mo == mo && mo->z < mo->floorz)
    {

        mo->player->viewheight -= mo->floorz-mo->z;
        mo->player->deltaviewheight = (VIEWHEIGHT - mo->player->viewheight) >> 3;

    }

    mo->z += mo->momz;

floater:
    if ((mo->flags & MF_FLOAT) && mo->target)
    {

        if (!((mo->flags ^ MF_FLOAT) & (MF_FLOAT | MF_SKULLFLY | MF_INFLOAT)) && mo->target)
        {

            fixed_t delta = mo->target->z + (mo->height >> 1) - mo->z;

            if (P_AproxDistance(mo->x - mo->target->x, mo->y - mo->target->y) < D_abs(delta) * 3)
                mo->z += delta < 0 ? -FLOATSPEED : FLOATSPEED;

        }

    }

    if (mo->z <= mo->floorz)
    {

        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;

        if (mo->momz < 0)
        {

            if (mo->flags & MF_TOUCHY && mo->intflags & MIF_ARMED && mo->health > 0)
            {

                P_DamageMobj(mo, NULL, NULL, mo->health);

            }

            else if (mo->player && mo->player->mo == mo && mo->momz < -GRAVITY * 8)
            {

                mo->player->deltaviewheight = mo->momz >> 3;

                if (mo->health > 0)
                    S_StartSound (mo, sfx_oof);

            }

            mo->momz = 0;

        }

        mo->z = mo->floorz;

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {

            P_ExplodeMissile (mo);

            return;

        }

    }

    else if (!(mo->flags & MF_NOGRAVITY))
    {

        if (!mo->momz)
            mo->momz = -GRAVITY;

        mo->momz -= GRAVITY;

    }

    if (mo->z + mo->height > mo->ceilingz)
    {

        if (mo->flags & MF_SKULLFLY)
            mo->momz = -mo->momz;

        if (mo->momz > 0)
            mo->momz = 0;

        mo->z = mo->ceilingz - mo->height;

        if ((mo->flags & MF_MISSILE) && !(mo->flags & MF_NOCLIP))
        {

            P_ExplodeMissile(mo);

            return;

        }

    }

}

static void P_NightmareRespawn(mobj_t *mobj)
{

    fixed_t x;
    fixed_t y;
    fixed_t z;
    subsector_t *ss;
    mobj_t *mo;
    mapthing_t *mthing;

    x = mobj->spawnpoint.x << FRACBITS;
    y = mobj->spawnpoint.y << FRACBITS;

    if (!x && !y)
    {

        x = mobj->x;
        y = mobj->y;

    }

    if (!P_CheckPosition(mobj, x, y))
        return;

    mo = P_SpawnMobj(mobj->x, mobj->y, mobj->subsector->sector->floorheight, MT_TFOG);

    S_StartSound(mo, sfx_telept);

    ss = R_PointInSubsector(x,y);
    mo = P_SpawnMobj(x, y, ss->sector->floorheight, MT_TFOG);

    S_StartSound (mo, sfx_telept);

    mthing = &mobj->spawnpoint;

    if (mobj->info->flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    mo = P_SpawnMobj(x,y,z, mobj->type);
    mo->spawnpoint = mobj->spawnpoint;
    mo->angle = ANG45 * (mthing->angle / 45);

    if (mthing->options & MTF_AMBUSH)
        mo->flags |= MF_AMBUSH;

    mo->flags = (mo->flags & ~MF_FRIEND) | (mobj->flags & MF_FRIEND);
    mo->reactiontime = 18;

    P_RemoveMobj(mobj);

}

void P_MobjThinker(mobj_t *mobj)
{

    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;

    if (mobj->momx | mobj->momy || mobj->flags & MF_SKULLFLY)
    {

        P_XYMovement(mobj);

        if (mobj->thinker.function != P_MobjThinker)
            return;

    }

    if (mobj->z != mobj->floorz || mobj->momz)
    {

        P_ZMovement(mobj);

        if (mobj->thinker.function != P_MobjThinker)
            return;

    }

    else if (!(mobj->momx | mobj->momy) && !sentient(mobj))
    {

        mobj->intflags |= MIF_ARMED;

        if (mobj->z > mobj->dropoffz && !(mobj->flags & MF_NOGRAVITY))
            P_ApplyTorque(mobj);
        else
            mobj->intflags &= ~MIF_FALLING, mobj->gear = 0;

    }

    if (mobj->tics != -1)
    {

        mobj->tics--;

        if (!mobj->tics)
        {

            if (!P_SetMobjState (mobj, mobj->state->nextstate))
                return;

        }

    }

    else
    {

        if (!(mobj->flags & MF_COUNTKILL))
            return;

        if (!respawnmonsters)
            return;

        mobj->movecount++;

        if (mobj->movecount < 12 * 35)
            return;

        if (leveltime & 31)
            return;

        if (P_Random(pr_respawn) > 4)
            return;

        P_NightmareRespawn(mobj);

    }

}

mobj_t *P_SpawnMobj(fixed_t x, fixed_t y, fixed_t z, mobjtype_t type)
{

    mobj_t *mobj;
    state_t *st;
    mobjinfo_t *info;

    mobj = Z_Malloc(sizeof (*mobj), PU_LEVEL, NULL);

    memset(mobj, 0, sizeof (*mobj));

    info = &mobjinfo[type];
    mobj->type = type;
    mobj->info = info;
    mobj->x = x;
    mobj->y = y;
    mobj->radius = info->radius;
    mobj->height = info->height;
    mobj->flags  = info->flags;

    if (type == MT_PLAYER)
        mobj->flags |= MF_FRIEND;

    mobj->health = info->spawnhealth;

    if (gameskill != sk_nightmare)
        mobj->reactiontime = info->reactiontime;

    mobj->lastlook = P_Random (pr_lastlook) % MAXPLAYERS;
    st = &states[info->spawnstate];
    mobj->state = st;
    mobj->tics = st->tics;
    mobj->sprite = st->sprite;
    mobj->frame = st->frame;
    mobj->touching_sectorlist = NULL;

    P_SetThingPosition(mobj);

    mobj->dropoffz = mobj->floorz = mobj->subsector->sector->floorheight;
    mobj->ceilingz = mobj->subsector->sector->ceilingheight;

    mobj->z = z == ONFLOORZ ? mobj->floorz : z == ONCEILINGZ ? mobj->ceilingz - mobj->height : z;
    mobj->PrevX = mobj->x;
    mobj->PrevY = mobj->y;
    mobj->PrevZ = mobj->z;
    mobj->thinker.function = P_MobjThinker;
    mobj->friction = ORIG_FRICTION;
    mobj->target = mobj->tracer = mobj->lastenemy = NULL;

    P_AddThinker(&mobj->thinker);

    if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
        totallive++;

    return mobj;

}

void P_RemoveMobj(mobj_t *mobj)
{

    if ((mobj->flags & MF_SPECIAL) && !(mobj->flags & MF_DROPPED) && (mobj->type != MT_INV) && (mobj->type != MT_INS))
    {

        itemrespawnque[iquehead] = mobj->spawnpoint;
        itemrespawntime[iquehead] = leveltime;
        iquehead = (iquehead + 1) & (ITEMQUESIZE - 1);

        if (iquehead == iquetail)
            iquetail = (iquetail + 1) & (ITEMQUESIZE - 1);

    }

    P_UnsetThingPosition(mobj);

    if (sector_list)
    {

        P_DelSeclist(sector_list);

        sector_list = NULL;

    }

    S_StopSound(mobj);
    P_SetTarget(&mobj->target, NULL);
    P_SetTarget(&mobj->tracer, NULL);
    P_SetTarget(&mobj->lastenemy, NULL);
    P_RemoveThinker(&mobj->thinker);

}

static int P_FindDoomedNum(unsigned type)
{

    static struct {int first, next;} *hash;
    register int i;

    if (!hash)
    {

        hash = Z_Malloc(sizeof *hash * NUMMOBJTYPES, PU_CACHE, (void **)&hash);

        for (i = 0; i < NUMMOBJTYPES; i++)
            hash[i].first = NUMMOBJTYPES;

        for (i = 0; i < NUMMOBJTYPES; i++)
        {

            if (mobjinfo[i].doomednum != -1)
            {

                unsigned h = (unsigned)mobjinfo[i].doomednum % NUMMOBJTYPES;
                hash[i].next = hash[h].first;
                hash[h].first = i;
            }

        }

    }

    i = hash[type % NUMMOBJTYPES].first;

     while ((i < NUMMOBJTYPES) && ((unsigned)mobjinfo[i].doomednum != type))
        i = hash[i].next;

    return i;

}

void P_SpawnPlayer(int n, const mapthing_t *mthing)
{

    player_t *p;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    mobj_t *mobj;

    if (!playeringame[n])
        return;

    p = &players[n];

    if (p->playerstate == PST_REBORN)
        G_PlayerReborn(mthing->type - 1);

    if (!mthing->options)
        I_Error("P_SpawnPlayer: attempt to spawn player at unavailable start point");
  
    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;
    z = ONFLOORZ;
    mobj = P_SpawnMobj(x, y, z, MT_PLAYER);
    mobj->angle = ANG45 * (mthing->angle / 45);
    mobj->player = p;
    mobj->health = p->health;
    p->mo = mobj;
    p->playerstate = PST_LIVE;
    p->refire = 0;
    p->message = NULL;
    p->damagecount = 0;
    p->bonuscount = 0;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->viewheight = VIEWHEIGHT;
    p->momx = p->momy = 0;

    P_SetupPsprites(p);

    if (mthing->type - 1 == consoleplayer)
    {

        ST_Start();
        HU_Start();

    }

}

boolean P_IsDoomnumAllowed(int doomnum)
{

    if (gamemode != commercial)
    {

        switch (doomnum)
        {

        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 71:
        case 84:
        case 88:
        case 89:
            return false;

      }

    }

    return true;

}

void P_SpawnMapThing(const mapthing_t *mthing)
{

    int i;
    mobj_t *mobj;
    fixed_t x;
    fixed_t y;
    fixed_t z;
    int options = mthing->options;

    switch (mthing->type)
    {

    case 0:
    case DEN_PLAYER5:
    case DEN_PLAYER6:
    case DEN_PLAYER7:
    case DEN_PLAYER8:
        return;

    }

    if ((options & MTF_RESERVED))
    {

        I_Error("P_SpawnMapThing: Correcting bad flags (%u) (thing type %d)\n", options, mthing->type);

        options &= MTF_EASY | MTF_NORMAL | MTF_HARD | MTF_AMBUSH | MTF_NOTSINGLE;

    }

    if (mthing->type == 11)
        return;

    if (mthing->type <= 4 && mthing->type > 0)
    {

        playerstarts[mthing->type - 1] = *mthing;
        playerstarts[mthing->type - 1].options = 1;

        P_SpawnPlayer(mthing->type - 1, &playerstarts[mthing->type - 1]);

        return;

    }

    if (options & MTF_NOTSINGLE)
        return;

    if (gameskill == sk_baby || gameskill == sk_easy ? !(options & MTF_EASY) : gameskill == sk_hard || gameskill == sk_nightmare ? !(options & MTF_HARD) : !(options & MTF_NORMAL))
        return;

    i = P_FindDoomedNum(mthing->type);

    if (i == NUMMOBJTYPES)
    {

        I_Error("P_SpawnMapThing: Unknown Thing type %i at (%i, %i)", mthing->type, mthing->x, mthing->y);

        return;

    }

    x = mthing->x << FRACBITS;
    y = mthing->y << FRACBITS;

    if (mobjinfo[i].flags & MF_SPAWNCEILING)
        z = ONCEILINGZ;
    else
        z = ONFLOORZ;

    mobj = P_SpawnMobj(x, y, z, i);
    mobj->spawnpoint = *mthing;

    if (mobj->tics > 0)
        mobj->tics = 1 + (P_Random(pr_spawnthing) % mobj->tics);

    if (!((mobj->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
        totalkills++;

    if (mobj->flags & MF_COUNTITEM)
        totalitems++;

    mobj->angle = ANG45 * (mthing->angle / 45);

    if (options & MTF_AMBUSH)
        mobj->flags |= MF_AMBUSH;

}

void P_SpawnPuff(fixed_t x, fixed_t y, fixed_t z)
{

    mobj_t* th;
    int t = P_Random(pr_spawnpuff);

    z += (t - P_Random(pr_spawnpuff)) << 10;
    th = P_SpawnMobj(x, y, z, MT_PUFF);
    th->momz = FRACUNIT;
    th->tics -= P_Random(pr_spawnpuff) & 3;

    if (th->tics < 1)
        th->tics = 1;

    if (attackrange == MELEERANGE)
        P_SetMobjState(th, S_PUFF3);

}

void P_SpawnBlood(fixed_t x, fixed_t y, fixed_t z, int damage)
{

    mobj_t *th;
    int t = P_Random(pr_spawnblood);

    z += (t - P_Random(pr_spawnblood)) << 10;
    th = P_SpawnMobj(x, y, z, MT_BLOOD);
    th->momz = FRACUNIT * 2;
    th->tics -= P_Random(pr_spawnblood) & 3;

    if (th->tics < 1)
        th->tics = 1;

    if (damage <= 12 && damage >= 9)
        P_SetMobjState(th, S_BLOOD2);
    else if (damage < 9)
        P_SetMobjState(th, S_BLOOD3);

}

void P_CheckMissileSpawn (mobj_t* th)
{

    th->tics -= P_Random(pr_missile) & 3;

    if (th->tics < 1)
        th->tics = 1;

    th->x += (th->momx >> 1);
    th->y += (th->momy >> 1);
    th->z += (th->momz >> 1);

    if (!(th->flags & MF_MISSILE))
        return;

    if (!P_TryMove(th, th->x, th->y, false))
        P_ExplodeMissile(th);

}

mobj_t *P_SpawnMissile(mobj_t *source, mobj_t *dest, mobjtype_t type)
{

    mobj_t* th;
    angle_t an;
    int dist;

    th = P_SpawnMobj(source->x,source->y,source->z + 4 * 8 * FRACUNIT, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    P_SetTarget(&th->target, source);
    an = R_PointToAngle2(source->x, source->y, dest->x, dest->y);

    if (dest->flags & MF_SHADOW)
    {

        int t = P_Random(pr_shadow);
        an += (t - P_Random(pr_shadow)) << 20;

    }

    th->angle = an;
    an >>= ANGLETOFINESHIFT;
    th->momx = FixedMul(th->info->speed, finecosine[an]);
    th->momy = FixedMul(th->info->speed, finesine[an]);
    dist = P_AproxDistance(dest->x - source->x, dest->y - source->y);
    dist = dist / th->info->speed;

    if (dist < 1)
        dist = 1;

    th->momz = (dest->z - source->z) / dist;

    P_CheckMissileSpawn(th);

    return th;

}

void P_SpawnPlayerMissile(mobj_t *source, mobjtype_t type)
{

    mobj_t *th;
    fixed_t x, y, z, slope = 0;
    angle_t an = source->angle;
    uint_64_t mask = MF_FRIEND;

    do
    {

        slope = P_AimLineAttack(source, an, 16 * 64 * FRACUNIT, mask);

        if (!linetarget)
            slope = P_AimLineAttack(source, an += 1 << 26, 16 * 64 * FRACUNIT, mask);

        if (!linetarget)
            slope = P_AimLineAttack(source, an -= 2 << 26, 16 * 64 * FRACUNIT, mask);

        if (!linetarget)
            an = source->angle, slope = 0;

    } while (mask && (mask = 0, !linetarget));

    x = source->x;
    y = source->y;
    z = source->z + 4 * 8 * FRACUNIT;
    th = P_SpawnMobj(x,y,z, type);

    if (th->info->seesound)
        S_StartSound(th, th->info->seesound);

    P_SetTarget(&th->target, source);

    th->angle = an;
    th->momx = FixedMul(th->info->speed, finecosine[an >> ANGLETOFINESHIFT]);
    th->momy = FixedMul(th->info->speed, finesine[an >> ANGLETOFINESHIFT]);
    th->momz = FixedMul(th->info->speed, slope);

    P_CheckMissileSpawn(th);

}

