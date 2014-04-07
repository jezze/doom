#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

static ceilinglist_t *activeceilings;

void T_MoveCeiling(ceiling_t* ceiling)
{

    result_e res;

    switch (ceiling->direction)
    {

    case 0:

      break;

    case 1:
        res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->topheight, false, 1, ceiling->direction);

        if (!(leveltime & 7))
        {

            switch (ceiling->type)
            {

            case silentCrushAndRaise:
            case genSilentCrusher:
                break;

            default:
                S_StartSound((mobj_t *)&ceiling->sector->soundorg,sfx_stnmov);

                break;

            }

        }

        if (res == pastdest)
        {

            switch (ceiling->type)
            {

            case raiseToHighest:
            case genCeiling:
                P_RemoveActiveCeiling(ceiling);

                break;

            case genCeilingChgT:
            case genCeilingChg0:
                ceiling->sector->special = ceiling->newspecial;
                ceiling->sector->oldspecial = ceiling->oldspecial;

            case genCeilingChg:
                ceiling->sector->ceilingpic = ceiling->texture;

                P_RemoveActiveCeiling(ceiling);

                break;

            case silentCrushAndRaise:
                S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_pstop);

            case genSilentCrusher:
            case genCrusher:
            case fastCrushAndRaise:
            case crushAndRaise:
                ceiling->direction = -1;

                break;

            default:
                break;

            }

        }

        break;

    case -1:
        res = T_MovePlane(ceiling->sector, ceiling->speed, ceiling->bottomheight, ceiling->crush, 1, ceiling->direction);

        if (!(leveltime & 7))
        {

            switch (ceiling->type)
            {

                case silentCrushAndRaise:
                case genSilentCrusher:
                    break;

                default:
                    S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_stnmov);

            }

        }

        if (res == pastdest)
        {

            switch (ceiling->type)
            {

            case genSilentCrusher:
            case genCrusher:
                if (ceiling->oldspeed < CEILSPEED * 3)
                    ceiling->speed = ceiling->oldspeed;

                ceiling->direction = 1;

                break;

            case silentCrushAndRaise:
                S_StartSound((mobj_t *)&ceiling->sector->soundorg, sfx_pstop);

            case crushAndRaise:
                ceiling->speed = CEILSPEED;

            case fastCrushAndRaise:
                ceiling->direction = 1;

                break;

            case genCeilingChgT:
            case genCeilingChg0:
                ceiling->sector->special = ceiling->newspecial;
                ceiling->sector->oldspecial = ceiling->oldspecial;

            case genCeilingChg:
                ceiling->sector->ceilingpic = ceiling->texture;

                P_RemoveActiveCeiling(ceiling);

                break;

            case lowerAndCrush:
            case lowerToFloor:
            case lowerToLowest:
            case lowerToMaxFloor:
            case genCeiling:
                P_RemoveActiveCeiling(ceiling);

                break;

            default:
                break;

            }

        }

        else
        {

            if (res == crushed)
            {

                switch (ceiling->type)
                {

                case genCrusher:
                case genSilentCrusher:
                    if (ceiling->oldspeed < CEILSPEED * 3)
                        ceiling->speed = CEILSPEED / 8;

                    break;

                case silentCrushAndRaise:
                case crushAndRaise:
                case lowerAndCrush:
                    ceiling->speed = CEILSPEED / 8;

                    break;

                default:
                    break;

                }

            }

        }

        break;

    }

}

int EV_DoCeiling(line_t *line, ceiling_e type)
{

    int secnum = -1;
    int rtn = 0;
    sector_t *sec;
    ceiling_t *ceiling;

    switch (type)
    {

    case fastCrushAndRaise:
    case silentCrushAndRaise:
    case crushAndRaise:
        rtn = P_ActivateInStasisCeiling(line);

    default:
        break;
    }

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (P_SectorActive(ceiling_special, sec))
            continue;

        rtn = 1;
        ceiling = Z_Malloc(sizeof(*ceiling), PU_LEVSPEC, 0);

        memset(ceiling, 0, sizeof(*ceiling));
        P_AddThinker (&ceiling->thinker);

        sec->ceilingdata = ceiling;
        ceiling->thinker.function = T_MoveCeiling;
        ceiling->sector = sec;
        ceiling->crush = false;

        switch(type)
        {

        case fastCrushAndRaise:
            ceiling->crush = true;
            ceiling->topheight = sec->ceilingheight;
            ceiling->bottomheight = sec->floorheight + (8 * FRACUNIT);
            ceiling->direction = -1;
            ceiling->speed = CEILSPEED * 2;

            break;

        case silentCrushAndRaise:
        case crushAndRaise:
            ceiling->crush = true;
            ceiling->topheight = sec->ceilingheight;

        case lowerAndCrush:
        case lowerToFloor:
            ceiling->bottomheight = sec->floorheight;

            if (type != lowerToFloor)
                ceiling->bottomheight += 8 * FRACUNIT;

            ceiling->direction = -1;
            ceiling->speed = CEILSPEED;

            break;

        case raiseToHighest:
            ceiling->topheight = P_FindHighestCeilingSurrounding(sec);
            ceiling->direction = 1;
            ceiling->speed = CEILSPEED;

            break;

        case lowerToLowest:
            ceiling->bottomheight = P_FindLowestCeilingSurrounding(sec);
            ceiling->direction = -1;
            ceiling->speed = CEILSPEED;

            break;

        case lowerToMaxFloor:
            ceiling->bottomheight = P_FindHighestFloorSurrounding(sec);
            ceiling->direction = -1;
            ceiling->speed = CEILSPEED;

            break;

        default:
            break;

        }

        ceiling->tag = sec->tag;
        ceiling->type = type;

        P_AddActiveCeiling(ceiling);

    }

    return rtn;

}

int P_ActivateInStasisCeiling(line_t *line)
{

    ceilinglist_t *cl;
    int rtn = 0;

    for (cl = activeceilings; cl; cl = cl->next)
    {

        ceiling_t *ceiling = cl->ceiling;

        if (ceiling->tag == line->tag && ceiling->direction == 0)
        {

            ceiling->direction = ceiling->olddirection;
            ceiling->thinker.function = T_MoveCeiling;
            rtn = 1;
        }

    }

    return rtn;

}

int EV_CeilingCrushStop(line_t* line)
{

    int rtn = 0;
    ceilinglist_t *cl;

    for (cl = activeceilings; cl; cl = cl->next)
    {

        ceiling_t *ceiling = cl->ceiling;

        if (ceiling->direction != 0 && ceiling->tag == line->tag)
        {

            ceiling->olddirection = ceiling->direction;
            ceiling->direction = 0;
            ceiling->thinker.function = NULL;
            rtn = 1;

        }

    }

    return rtn;

}

void P_AddActiveCeiling(ceiling_t *ceiling)
{

    ceilinglist_t *list = malloc(sizeof *list);

    list->ceiling = ceiling;
    ceiling->list = list;

    if ((list->next = activeceilings))
        list->next->prev = &list->next;

    list->prev = &activeceilings;
    activeceilings = list;

}

void P_RemoveActiveCeiling(ceiling_t *ceiling)
{

    ceilinglist_t *list = ceiling->list;
    ceiling->sector->ceilingdata = NULL;

    P_RemoveThinker(&ceiling->thinker);

    if ((*list->prev = list->next))
        list->next->prev = list->prev;

    free(list);

}

void P_RemoveAllActiveCeilings(void)
{

    while (activeceilings)
    {

        ceilinglist_t *next = activeceilings->next;

        free(activeceilings);

        activeceilings = next;

    }

}

