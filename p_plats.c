#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

platlist_t *activeplats;

void T_PlatRaise(plat_t* plat)
{

    result_e res;

    switch (plat->status)
    {

    case up:
        res = T_MovePlane(plat->sector,plat->speed,plat->high,plat->crush,0,1);

        if (plat->type == raiseAndChange || plat->type == raiseToNearestAndChange)
        {

            if (!(leveltime&7))
                S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_stnmov);

        }

        if (res == crushed && (!plat->crush))
        {

            plat->count = plat->wait;
            plat->status = down;

            S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstart);

        }

        else
        {

            if (res == pastdest)
            {

                if (plat->type!=toggleUpDn)
                {

                    plat->count = plat->wait;
                    plat->status = waiting;

                    S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstop);

                }

                else
                {

                    plat->oldstatus = plat->status;
                    plat->status = in_stasis;

                }

                switch (plat->type)
                {

                case blazeDWUS:
                case downWaitUpStay:
                case raiseAndChange:
                case raiseToNearestAndChange:
                case genLift:
                    P_RemoveActivePlat(plat);

                default:
                    break;

                }

            }

        }

        break;

    case down:
        res = T_MovePlane(plat->sector, plat->speed, plat->low, false, 0, -1);


        if (res == pastdest)
        {

            if (plat->type!=toggleUpDn)
            {

                plat->count = plat->wait;
                plat->status = waiting;

                S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstop);

            }

            else
            {

                plat->oldstatus = plat->status;
                plat->status = in_stasis;

            }

            switch (plat->type)
            {

            case raiseAndChange:
            case raiseToNearestAndChange:
                P_RemoveActivePlat(plat);

            default:
                break;

            }

        }

        break;

    case waiting:
        if (!--plat->count)
        {

            if (plat->sector->floorheight == plat->low)
                plat->status = up;
            else
                plat->status = down;

            S_StartSound((mobj_t *)&plat->sector->soundorg, sfx_pstart);

        }

        break;

    case in_stasis:
        break;

    }

}

int EV_DoPlat(line_t *line, plattype_e type, int amount)
{

    plat_t *plat;
    int secnum = -1;
    int rtn = 0;
    sector_t *sec;

    switch(type)
    {

    case perpetualRaise:
        P_ActivateInStasis(line->tag);

        break;

    case toggleUpDn:
        P_ActivateInStasis(line->tag);

        rtn = 1;

        break;

    default:
        break;

    }

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (P_SectorActive(floor_special,sec))
            continue;

        rtn = 1;
        plat = Z_Malloc(sizeof(*plat), PU_LEVSPEC, 0);

        memset(plat, 0, sizeof(*plat));
        P_AddThinker(&plat->thinker);

        plat->type = type;
        plat->sector = sec;
        plat->sector->floordata = plat;
        plat->thinker.function = T_PlatRaise;
        plat->crush = false;
        plat->tag = line->tag;
        plat->low = sec->floorheight;

        switch(type)
        {

        case raiseToNearestAndChange:
            plat->speed = PLATSPEED / 2;
            sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
            plat->high = P_FindNextHighestFloor(sec, sec->floorheight);
            plat->wait = 0;
            plat->status = up;
            sec->special = 0;
            sec->oldspecial = 0;

            S_StartSound((mobj_t *)&sec->soundorg, sfx_stnmov);

            break;

        case raiseAndChange:
            plat->speed = PLATSPEED / 2;
            sec->floorpic = sides[line->sidenum[0]].sector->floorpic;
            plat->high = sec->floorheight + amount*FRACUNIT;
            plat->wait = 0;
            plat->status = up;

            S_StartSound((mobj_t *)&sec->soundorg, sfx_stnmov);

            break;

        case downWaitUpStay:
            plat->speed = PLATSPEED * 4;
            plat->low = P_FindLowestFloorSurrounding(sec);

            if (plat->low > sec->floorheight)
                plat->low = sec->floorheight;

            plat->high = sec->floorheight;
            plat->wait = 35*PLATWAIT;
            plat->status = down;

            S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);

            break;

        case blazeDWUS:
            plat->speed = PLATSPEED * 8;
            plat->low = P_FindLowestFloorSurrounding(sec);

            if (plat->low > sec->floorheight)
                plat->low = sec->floorheight;

            plat->high = sec->floorheight;
            plat->wait = 35 * PLATWAIT;
            plat->status = down;

            S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);

            break;

        case perpetualRaise:
            plat->speed = PLATSPEED;
            plat->low = P_FindLowestFloorSurrounding(sec);

            if (plat->low > sec->floorheight)
                plat->low = sec->floorheight;

            plat->high = P_FindHighestFloorSurrounding(sec);

            if (plat->high < sec->floorheight)
                plat->high = sec->floorheight;

            plat->wait = 35 * PLATWAIT;
            plat->status = P_Random(pr_plats) & 1;

            S_StartSound((mobj_t *)&sec->soundorg, sfx_pstart);

            break;

        case toggleUpDn:
            plat->speed = PLATSPEED;
            plat->wait = 35 * PLATWAIT;
            plat->crush = true;
            plat->low = sec->ceilingheight;
            plat->high = sec->floorheight;
            plat->status =  down;

            break;

        default:
            break;

        }

        P_AddActivePlat(plat);

    }

    return rtn;

}

void P_ActivateInStasis(int tag)
{

    platlist_t *pl;

    for (pl = activeplats; pl; pl = pl->next)
    {

        plat_t *plat = pl->plat;

        if (plat->tag == tag && plat->status == in_stasis)
        {

            if (plat->type==toggleUpDn)
                plat->status = plat->oldstatus == up ? down : up;
            else
                plat->status = plat->oldstatus;

            plat->thinker.function = T_PlatRaise;

        }

    }

}

int EV_StopPlat(line_t* line)
{

    platlist_t *pl;

    for (pl = activeplats; pl; pl = pl->next)
    {

        plat_t *plat = pl->plat;

        if (plat->status != in_stasis && plat->tag == line->tag)
        {

            plat->oldstatus = plat->status;
            plat->status = in_stasis;
            plat->thinker.function = NULL;

        }

    }

    return 1;

}

void P_AddActivePlat(plat_t *plat)
{

    platlist_t *list = malloc(sizeof *list);

    list->plat = plat;
    plat->list = list;

    if ((list->next = activeplats))
        list->next->prev = &list->next;

    list->prev = &activeplats;
    activeplats = list;

}

void P_RemoveActivePlat(plat_t *plat)
{

    platlist_t *list = plat->list;

    plat->sector->floordata = NULL;

    P_RemoveThinker(&plat->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);

}

void P_RemoveAllActivePlats(void)
{

    while (activeplats)
    {

        platlist_t *next = activeplats->next;

        free(activeplats);

        activeplats = next;

    }

}

