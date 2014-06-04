#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_tick.h"
#include "s_sound.h"
#include "z_zone.h"

result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, boolean crush, int floorOrCeiling, int direction)
{

    boolean flag;
    fixed_t lastpos;
    fixed_t destheight;

    switch (floorOrCeiling)
    {

    case 0:
        switch (direction)
        {

        case -1:
            if (sector->floorheight - speed < dest)
            {

                lastpos = sector->floorheight;
                sector->floorheight = dest;
                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    sector->floorheight = lastpos;

                    P_CheckSector(sector, crush);

                }

                return pastdest;

            }

            else
            {

                lastpos = sector->floorheight;
                sector->floorheight -= speed;
                flag = P_CheckSector(sector, crush);
                
            }

            break;

        case 1:
            destheight = (dest < sector->ceilingheight) ? dest : sector->ceilingheight;

            if (sector->floorheight + speed > destheight)
            {

                lastpos = sector->floorheight;
                sector->floorheight = destheight;
                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    sector->floorheight = lastpos;

                    P_CheckSector(sector, crush);

                }

                return pastdest;

            }

            else
            {

                lastpos = sector->floorheight;
                sector->floorheight += speed;
                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    sector->floorheight = lastpos;

                    P_CheckSector(sector,crush);

                    return crushed;

                }

            }

            break;

        }

        break;

    case 1:
        switch (direction)
        {

        case -1:
            destheight = (dest > sector->floorheight) ? dest : sector->floorheight;

            if (sector->ceilingheight - speed < destheight)
            {

                lastpos = sector->ceilingheight;
                sector->ceilingheight = destheight;

                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    sector->ceilingheight = lastpos;

                    P_CheckSector(sector, crush);

                }

                return pastdest;

            }

            else
            {

                lastpos = sector->ceilingheight;
                sector->ceilingheight -= speed;
                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    if (crush == true)
                        return crushed;

                    sector->ceilingheight = lastpos;

                    P_CheckSector(sector, crush);

                    return crushed;

                }

            }

            break;

        case 1:
            if (sector->ceilingheight + speed > dest)
            {

                lastpos = sector->ceilingheight;
                sector->ceilingheight = dest;
                flag = P_CheckSector(sector, crush);

                if (flag == true)
                {

                    sector->ceilingheight = lastpos;

                    P_CheckSector(sector, crush);

                }

                return pastdest;

            }

            else
            {

                lastpos = sector->ceilingheight;
                sector->ceilingheight += speed;
                flag = P_CheckSector(sector, crush);

            }

            break;

        }

        break;

    }

    return ok;

}

void T_MoveFloor(floormove_t *floor)
{

    result_e res = T_MovePlane(floor->sector, floor->speed, floor->floordestheight, floor->crush, 0, floor->direction);

    if (!(leveltime & 7))
        S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_stnmov);

    if (res == pastdest)
    {

        if (floor->direction == 1)
        {

            switch (floor->type)
            {

            case donutRaise:
                floor->sector->special = floor->newspecial;
                floor->sector->floorpic = floor->texture;

                break;

            case genFloorChgT:
            case genFloorChg0:
                floor->sector->special = floor->newspecial;
                floor->sector->oldspecial = floor->oldspecial;

            case genFloorChg:
                floor->sector->floorpic = floor->texture;

                break;

            default:
                break;

            }

        }

        else if (floor->direction == -1)
        {

            switch (floor->type)
            {

            case lowerAndChange:
                floor->sector->special = floor->newspecial;
                floor->sector->oldspecial = floor->oldspecial;
                floor->sector->floorpic = floor->texture;

                break;

            case genFloorChgT:
            case genFloorChg0:
                floor->sector->special = floor->newspecial;
                floor->sector->oldspecial = floor->oldspecial;

            case genFloorChg:
                floor->sector->floorpic = floor->texture;

                break;

            default:
                break;

            }

        }

        floor->sector->floordata = NULL;

        P_RemoveThinker(&floor->thinker);

        if (floor->sector->stairlock == -2)
        {

            sector_t *sec = floor->sector;
            sec->stairlock = -1;

            while (sec->prevsec != -1 && sectors[sec->prevsec].stairlock != -2)
                sec = &sectors[sec->prevsec];

            if (sec->prevsec == -1)
            {

                sec = floor->sector;

                while (sec->nextsec != -1 && sectors[sec->nextsec].stairlock != -2)
                    sec = &sectors[sec->nextsec];

                if (sec->nextsec == -1)
                {

                    while (sec->prevsec != -1)
                    {

                        sec->stairlock = 0;
                        sec = &sectors[sec->prevsec];

                    }

                    sec->stairlock = 0;

                }
            
            }

        }

        S_StartSound((mobj_t *)&floor->sector->soundorg, sfx_pstop);

    }

}

void T_MoveElevator(elevator_t* elevator)
{

    result_e res;

    if (elevator->direction < 0)
    {

        res = T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1, elevator->direction);

        if (res == ok || res == pastdest)
            T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0, elevator->direction);

    }

    else
    {

        res = T_MovePlane(elevator->sector, elevator->speed, elevator->floordestheight, 0, 0, elevator->direction);

        if (res == ok || res == pastdest)
            T_MovePlane(elevator->sector, elevator->speed, elevator->ceilingdestheight, 0, 1, elevator->direction);

    }

    if (!(leveltime & 7))
        S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_stnmov);

    if (res == pastdest)
    {

        elevator->sector->floordata = NULL;
        elevator->sector->ceilingdata = NULL;

        P_RemoveThinker(&elevator->thinker);
        S_StartSound((mobj_t *)&elevator->sector->soundorg, sfx_pstop);

    }

}

int EV_DoFloor(line_t * line, floor_e floortype)
{

    int secnum = -1;
    int rtn = 0;
    int i;
    sector_t *sec;
    floormove_t *floor;

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (P_SectorActive(floor_special, sec))
            continue;

        rtn = 1;
        floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

        memset(floor, 0, sizeof(*floor));
        P_AddThinker(&floor->thinker);

        sec->floordata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->type = floortype;
        floor->crush = false;

        switch (floortype)
        {

        case lowerFloor:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindHighestFloorSurrounding(sec);

            break;

        case lowerFloor24:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;

            break;

        case lowerFloor32Turbo:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;

            break;

        case lowerFloorToLowest:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec);

            break;

        case lowerFloorToNearest:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindNextLowestFloor(sec,floor->sector->floorheight);

            break;

        case turboLower:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = P_FindHighestFloorSurrounding(sec);

            if (floor->floordestheight != sec->floorheight)
                floor->floordestheight += 8 * FRACUNIT;

            break;

        case raiseFloorCrush:
            floor->crush = true;

        case raiseFloor:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestCeilingSurrounding(sec);

            if (floor->floordestheight > sec->ceilingheight)
                floor->floordestheight = sec->ceilingheight;

            floor->floordestheight -= (8 * FRACUNIT) * (floortype == raiseFloorCrush);

            break;

        case raiseFloorTurbo:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED * 4;
            floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);

            break;

        case raiseFloorToNearest:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);

            break;

        case raiseFloor24:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;

            break;

        case raiseFloor32Turbo:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED*4;
            floor->floordestheight = floor->sector->floorheight + 32 * FRACUNIT;

            break;

        case raiseFloor512:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 512 * FRACUNIT;

            break;

        case raiseFloor24AndChange:
            floor->direction = 1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = floor->sector->floorheight + 24 * FRACUNIT;
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            sec->oldspecial = line->frontsector->oldspecial;

            break;

        case raiseToTexture:
            {

                int minsize = INT_MAX;
                side_t *side;

                minsize = 32000 << FRACBITS;
                floor->direction = 1;
                floor->sector = sec;
                floor->speed = FLOORSPEED;

                for (i = 0; i < sec->linecount; i++)
                {

                    if (twoSided(secnum, i))
                    {

                        side = getSide(secnum, i, 0);

                        if (side->bottomtexture > 0)
                            if (textureheight[side->bottomtexture] < minsize)
                                minsize = textureheight[side->bottomtexture];

                        side = getSide(secnum,i,1);

                        if (side->bottomtexture > 0)
                            if (textureheight[side->bottomtexture] < minsize)
                                minsize = textureheight[side->bottomtexture];

                    }

                }

                floor->floordestheight = (floor->sector->floorheight >> FRACBITS) + (minsize >> FRACBITS);

                if (floor->floordestheight > 32000)
                    floor->floordestheight = 32000;

                floor->floordestheight <<= FRACBITS;

            }

            break;

        case lowerAndChange:
            floor->direction = -1;
            floor->sector = sec;
            floor->speed = FLOORSPEED;
            floor->floordestheight = P_FindLowestFloorSurrounding(sec);
            floor->texture = sec->floorpic;
            floor->newspecial = sec->special;
            floor->oldspecial = sec->oldspecial;
            sec = P_FindModelFloorSector(floor->floordestheight, sec - sectors);

            if (sec)
            {

                floor->texture = sec->floorpic;
                floor->newspecial = sec->special;
                floor->oldspecial = sec->oldspecial;

            }

            break;

        default:
            break;

        }

    }

    return rtn;

}

int EV_DoChange(line_t *line, change_e changetype)
{

    int secnum = -1;
    int rtn = 0;
    sector_t *sec;
    sector_t *secm;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {

        sec = &sectors[secnum];
        rtn = 1;

        switch (changetype)
        {

        case trigChangeOnly:
            sec->floorpic = line->frontsector->floorpic;
            sec->special = line->frontsector->special;
            sec->oldspecial = line->frontsector->oldspecial;

            break;

        case numChangeOnly:
            secm = P_FindModelFloorSector(sec->floorheight, secnum);

            if (secm)
            {

                sec->floorpic = secm->floorpic;
                sec->special = secm->special;
                sec->oldspecial = secm->oldspecial;

            }

            break;

        default:
            break;

        }

    }

    return rtn;

}

static inline int P_FindSectorFromLineTagWithLowerBound(line_t *l, int start, int min)
{

    do
    {

        start = P_FindSectorFromLineTag(l, start);

    } while (start >= 0 && start <= min);

    return start;

}

int EV_BuildStairs(line_t *line, stair_e type)
{

    int ssec = -1;
    int minssec = -1;
    int rtn = 0;

    while ((ssec = P_FindSectorFromLineTagWithLowerBound(line, ssec, minssec)) >= 0)
    {

        int secnum = ssec;
        sector_t *sec = &sectors[secnum];

        if (!P_SectorActive(floor_special, sec))
        {

            floormove_t *floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);
            int texture, height;
            fixed_t stairsize;
            fixed_t speed;
            int ok;

            rtn = 1;

            memset(floor, 0, sizeof(*floor));
            P_AddThinker(&floor->thinker);

            sec->floordata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->direction = 1;
            floor->sector = sec;
            floor->type = buildStair;

            switch (type)
            {

            default:
            case build8:
                speed = FLOORSPEED / 4;
                stairsize = 8 * FRACUNIT;
                floor->crush = false;

                break;

            case turbo16:
                speed = FLOORSPEED * 4;
                stairsize = 16 * FRACUNIT;
                floor->crush = true;

                break;

            }

            floor->speed = speed;
            height = sec->floorheight + stairsize;
            floor->floordestheight = height;
            texture = sec->floorpic;

            do
            {

                int i;

                ok = 0;

                for (i = 0; i < sec->linecount; i++)
                {

                    sector_t *tsec = (sec->lines[i])->frontsector;
                    int newsecnum;

                    if (!((sec->lines[i])->flags & ML_TWOSIDED))
                        continue;

                    newsecnum = tsec-sectors;

                    if (secnum != newsecnum)
                        continue;

                    tsec = (sec->lines[i])->backsector;

                    if (!tsec)
                        continue;

                    newsecnum = tsec - sectors;

                    if (tsec->floorpic != texture)
                        continue;

                    if (P_SectorActive(floor_special, tsec))
                        continue;

                    height += stairsize;
                    sec = tsec;
                    secnum = newsecnum;
                    floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

                    memset(floor, 0, sizeof(*floor));

                    P_AddThinker(&floor->thinker);

                    sec->floordata = floor;
                    floor->thinker.function = T_MoveFloor;
                    floor->direction = 1;
                    floor->sector = sec;
                    floor->speed = speed;
                    floor->floordestheight = height;
                    floor->type = buildStair;
                    floor->crush = type == build8 ? false : true;
                    ok = 1;

                    break;

                }

            } while (ok);

        }

    }

    return rtn;

}

int EV_DoDonut(line_t *line)
{

    sector_t *s1;
    sector_t *s2;
    sector_t *s3;
    int secnum = -1;
    int rtn = 0;
    int i;
    floormove_t *floor;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {

        s1 = &sectors[secnum];

        if (P_SectorActive(floor_special, s1))
            continue;

        s2 = getNextSector(s1->lines[0], s1);

        if (!s2)
            continue;

        if (P_SectorActive(floor_special, s2))
            continue;

        for (i = 0;i < s2->linecount;i++)
        {

            if (!s2->lines[i]->backsector || s2->lines[i]->backsector == s1)
                continue;

            rtn = 1;
            s3 = s2->lines[i]->backsector;
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

            memset(floor, 0, sizeof(*floor));
            P_AddThinker(&floor->thinker);

            s2->floordata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = donutRaise;
            floor->crush = false;
            floor->direction = 1;
            floor->sector = s2;
            floor->speed = FLOORSPEED / 2;
            floor->texture = s3->floorpic;
            floor->newspecial = 0;
            floor->floordestheight = s3->floorheight;
            floor = Z_Malloc(sizeof(*floor), PU_LEVSPEC, 0);

            memset(floor, 0, sizeof(*floor));
            P_AddThinker(&floor->thinker);

            s1->floordata = floor;
            floor->thinker.function = T_MoveFloor;
            floor->type = lowerFloor;
            floor->crush = false;
            floor->direction = -1;
            floor->sector = s1;
            floor->speed = FLOORSPEED / 2;
            floor->floordestheight = s3->floorheight;

            break;

        }

    }

    return rtn;

}

int EV_DoElevator(line_t *line, elevator_e elevtype)
{

    int secnum = -1;
    int rtn = 0;
    sector_t *sec;
    elevator_t *elevator;

    while ((secnum = P_FindSectorFromLineTag(line, secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (sec->floordata || sec->ceilingdata)
            continue;

        rtn = 1;
        elevator = Z_Malloc(sizeof(*elevator), PU_LEVSPEC, 0);

        memset(elevator, 0, sizeof(*elevator));
        P_AddThinker(&elevator->thinker);

        sec->floordata = elevator;
        sec->ceilingdata = elevator;
        elevator->thinker.function = T_MoveElevator;
        elevator->type = elevtype;

        switch (elevtype)
        {

        case elevateDown:
            elevator->direction = -1;
            elevator->sector = sec;
            elevator->speed = ELEVATORSPEED;
            elevator->floordestheight = P_FindNextLowestFloor(sec, sec->floorheight);
            elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight - sec->floorheight;

            break;

        case elevateUp:
            elevator->direction = 1;
            elevator->sector = sec;
            elevator->speed = ELEVATORSPEED;
            elevator->floordestheight = P_FindNextHighestFloor(sec, sec->floorheight);
            elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight - sec->floorheight;

            break;

        case elevateCurrent:
            elevator->sector = sec;
            elevator->speed = ELEVATORSPEED;
            elevator->floordestheight = line->frontsector->floorheight;
            elevator->ceilingdestheight = elevator->floordestheight + sec->ceilingheight - sec->floorheight;
            elevator->direction = elevator->floordestheight > sec->floorheight ? 1 : -1;

            break;

        default:
            break;

        }

    }

    return rtn;

}

