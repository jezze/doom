#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "r_main.h"
#include "d_englsh.h"
#include "z_zone.h"
#include "i_system.h"

void T_VerticalDoor(vldoor_t *door)
{

    result_e res;

    switch (door->direction)
    {

    case 0:
        if (!--door->topcountdown)
        {

            switch (door->type)
            {

            case blazeRaise:
            case genBlazeRaise:
                door->direction = -1;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);

                break;

            case normal:
            case genRaise:
                door->direction = -1;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);
                
                break;

            case close30ThenOpen:
            case genCdO:
                door->direction = 1;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);

                break;

            case genBlazeCdO:
                door->direction = 1;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdopn);

                break;

            default:
                break;

            }

        }

        break;

    case 2:
        if (!--door->topcountdown)
        {

            switch (door->type)
            {

            case raiseIn5Mins:
                door->direction = 1;
                door->type = normal;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);

                break;

            default:
                break;
            }

        }

        break;

    case -1:
        res = T_MovePlane(door->sector, door->speed, door->sector->floorheight, false, 1, door->direction);

        if (door->lighttag && door->topheight - door->sector->floorheight)
            EV_LightTurnOnPartway(door->line, FixedDiv(door->sector->ceilingheight - door->sector->floorheight, door->topheight - door->sector->floorheight));

        if (res == pastdest)
        {

            switch (door->type)
            {

            case blazeRaise:
            case blazeClose:
            case genBlazeRaise:
            case genBlazeClose:
                door->sector->ceilingdata = NULL;

                P_RemoveThinker(&door->thinker);

                break;

            case normal:
            case close:
            case genRaise:
            case genClose:
                door->sector->ceilingdata = NULL;

                P_RemoveThinker(&door->thinker);

                break;

            case close30ThenOpen:
                door->direction = 0;
                door->topcountdown = TICRATE * 30;

                break;

            case genCdO:
            case genBlazeCdO:
                door->direction = 0;
                door->topcountdown = door->topwait;

                break;

            default:
                break;

            }

        }

        else if (res == crushed)
        {

            switch (door->type)
            {

            case genClose:
            case genBlazeClose:
            case blazeClose:
            case close:
                break;

            case blazeRaise:
            case genBlazeRaise:
                door->direction = 1;

                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdopn);

                break;

            default:
                door->direction = 1;

                S_StartSound((mobj_t *)&door->sector->soundorg,sfx_doropn);

                break;

            }
        }

        break;

    case 1:
        res = T_MovePlane(door->sector, door->speed, door->topheight, false, 1, door->direction);

        if (door->lighttag && door->topheight - door->sector->floorheight)
            EV_LightTurnOnPartway(door->line, FixedDiv(door->sector->ceilingheight - door->sector->floorheight, door->topheight - door->sector->floorheight));

        if (res == pastdest)
        {

            switch (door->type)
            {

            case blazeRaise:
            case normal:
            case genRaise:
            case genBlazeRaise:
                door->direction = 0;
                door->topcountdown = door->topwait;

                break;

            case close30ThenOpen:
            case blazeOpen:
            case open:
            case genBlazeOpen:
            case genOpen:
            case genCdO:
            case genBlazeCdO:
                door->sector->ceilingdata = NULL;

                P_RemoveThinker (&door->thinker);

                break;

            default:
                break;

            }

        }

        break;

    }

}

int EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing)
{

    player_t *p = thing->player;

    if (!p)
        return 0;

    switch (line->special)
    {

    case 99:
    case 133:
        if (!p->cards[it_bluecard] && !p->cards[it_blueskull])
        {

            p->message = PD_BLUEO;

            S_StartSound(p->mo, sfx_oof);

            return 0;

        }

        break;

    case 134:
    case 135:
        if (!p->cards[it_redcard] && !p->cards[it_redskull])
        {

            p->message = PD_REDO;

            S_StartSound(p->mo, sfx_oof);

            return 0;

        }

        break;

    case 136:
    case 137:
        if (!p->cards[it_yellowcard] && !p->cards[it_yellowskull])
        {

            p->message = PD_YELLOWO;

            S_StartSound(p->mo, sfx_oof);

            return 0;

        }

        break;

    }

    return EV_DoDoor(line, type);

}

int EV_DoDoor(line_t *line, vldoor_e type)
{

    int secnum = -1;
    int rtn = 0;
    sector_t *sec;
    vldoor_t *door;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (P_SectorActive(ceiling_special, sec))
            continue;

        rtn = 1;
        door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);
        memset(door, 0, sizeof(*door));
        P_AddThinker(&door->thinker);

        sec->ceilingdata = door;
        door->thinker.function = T_VerticalDoor;
        door->sector = sec;
        door->type = type;
        door->topwait = VDOORWAIT;
        door->speed = VDOORSPEED;
        door->line = line;
        door->lighttag = 0;

        switch (type)
        {

        case blazeClose:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->direction = -1;
            door->speed = VDOORSPEED * 4;

            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdcls);

            break;

        case close:
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->direction = -1;
            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);

            break;

        case close30ThenOpen:
            door->topheight = sec->ceilingheight;
            door->direction = -1;

            S_StartSound((mobj_t *)&door->sector->soundorg, sfx_dorcls);

            break;

        case blazeRaise:
        case blazeOpen:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;
            door->speed = VDOORSPEED * 4;

            if (door->topheight != sec->ceilingheight)
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_bdopn);

            break;

        case normal:
        case open:
            door->direction = 1;
            door->topheight = P_FindLowestCeilingSurrounding(sec);
            door->topheight -= 4 * FRACUNIT;

            if (door->topheight != sec->ceilingheight)
                S_StartSound((mobj_t *)&door->sector->soundorg, sfx_doropn);

            break;

        default:
            break;

        }

    }

    return rtn;

}

int EV_VerticalDoor(line_t *line, mobj_t *thing)
{

    player_t *player = thing->player;
    sector_t *sec;
    vldoor_t *door;

    switch (line->special)
    {

    case 26:
    case 32:
        if (!player)
            return 0;

        if (!player->cards[it_bluecard] && !player->cards[it_blueskull])
        {

            player->message = PD_BLUEK;

            S_StartSound(player->mo, sfx_oof);

            return 0;

        }

        break;

    case 27:
    case 34:
        if (!player)
            return 0;

        if (!player->cards[it_yellowcard] && !player->cards[it_yellowskull])
        {

            player->message = PD_YELLOWK;

            S_StartSound(player->mo, sfx_oof);

            return 0;

        }

        break;

    case 28:
    case 33:
        if (!player)
            return 0;

        if (!player->cards[it_redcard] && !player->cards[it_redskull])
        {

            player->message = PD_REDK;

            S_StartSound(player->mo, sfx_oof);

            return 0;

        }

        break;

    default:
        break;

    }

    if (line->sidenum[1] == NO_INDEX)
    {

        S_StartSound(player->mo, sfx_oof);

        return 0;

    }

    sec = sides[line->sidenum[1]].sector;
    door = sec->ceilingdata;

    if (door && ((line->special == 1) || (line->special == 117) || (line->special == 26) || (line->special == 27) || (line->special == 28)))
    {

        if (door->thinker.function == T_VerticalDoor)
        {

            int outval = 0;

            if (door->thinker.function == T_VerticalDoor && door->direction == -1)
            {

                outval = 1;

            }
            
            else if (player)
            {

                outval = -1;

            }

            if (outval)
            {

                if (door->thinker.function == T_VerticalDoor)
                {

                    door->direction = outval;

                }
                
                else if (door->thinker.function == T_PlatRaise)
                {

                    plat_t *p = (plat_t*)door;
                    p->wait = outval;

                }
                
                else
                {

                    I_Error("EV_VerticalDoor: Unknown thinker. Function in thinker corruption emulation");

                }

                return 1;

            }

        }
        
        return 0;

    }

    switch (line->special)
    {

    case 117:
    case 118:
        S_StartSound((mobj_t *)&sec->soundorg, sfx_bdopn);

        break;

    default:
        S_StartSound((mobj_t *)&sec->soundorg, sfx_doropn);

        break;

    }

    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);

    memset(door, 0, sizeof(*door));
    P_AddThinker (&door->thinker);

    sec->ceilingdata = door;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 1;
    door->speed = VDOORSPEED;
    door->topwait = VDOORWAIT;
    door->line = line;
    door->lighttag = line->tag;

    switch (line->special)
    {

    case 1:
    case 26:
    case 27:
    case 28:
        door->type = normal;

        break;

    case 31:
    case 32:
    case 33:
    case 34:
        door->type = open;
        line->special = 0;

        break;

    case 117:
        door->type = blazeRaise;
        door->speed = VDOORSPEED * 4;

        break;

    case 118:
        door->type = blazeOpen;
        line->special = 0;
        door->speed = VDOORSPEED * 4;

        break;

    default:
        door->lighttag = 0;

        break;

    }

    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4 * FRACUNIT;

    return 1;

}

void P_SpawnDoorCloseIn30(sector_t* sec)
{

    vldoor_t *door = Z_Malloc(sizeof(*door), PU_LEVSPEC, 0);

    memset(door, 0, sizeof(*door));
    P_AddThinker(&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 0;
    door->type = normal;
    door->speed = VDOORSPEED;
    door->topcountdown = 30 * 35;
    door->line = NULL;
    door->lighttag = 0;

}

void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum)
{

    vldoor_t *door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);

    memset(door, 0, sizeof(*door));
    P_AddThinker (&door->thinker);

    sec->ceilingdata = door;
    sec->special = 0;
    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->direction = 2;
    door->type = raiseIn5Mins;
    door->speed = VDOORSPEED;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->topwait = VDOORWAIT;
    door->topcountdown = 5 * 60 * 35;
    door->line = NULL;
    door->lighttag = 0;

}

