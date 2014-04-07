#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "m_random.h"
#include "s_sound.h"
#include "z_zone.h"

int EV_DoGenFloor(line_t *line)
{

  int                   secnum;
  int                   rtn;
  boolean               manual;
  sector_t*             sec;
  floormove_t*          floor;
  unsigned              value = (unsigned)line->special - GenFloorBase;
  int Crsh = (value & FloorCrush) >> FloorCrushShift;
  int ChgT = (value & FloorChange) >> FloorChangeShift;
  int Targ = (value & FloorTarget) >> FloorTargetShift;
  int Dirn = (value & FloorDirection) >> FloorDirectionShift;
  int ChgM = (value & FloorModel) >> FloorModelShift;
  int Sped = (value & FloorSpeed) >> FloorSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = 0;
  manual = false;

  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_floor;
  }

  secnum = -1;

  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_floor:

    if (P_SectorActive(floor_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }


    rtn = 1;
    floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
    memset(floor, 0, sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function = T_MoveFloor;
    floor->crush = Crsh;
    floor->direction = Dirn? 1 : -1;
    floor->sector = sec;
    floor->texture = sec->floorpic;
    floor->newspecial = sec->special;

    floor->oldspecial = sec->oldspecial;
    floor->type = genFloor;


    switch (Sped)
    {
      case SpeedSlow:
        floor->speed = FLOORSPEED;
        break;
      case SpeedNormal:
        floor->speed = FLOORSPEED*2;
        break;
      case SpeedFast:
        floor->speed = FLOORSPEED*4;
        break;
      case SpeedTurbo:
        floor->speed = FLOORSPEED*8;
        break;
      default:
        break;
    }


    switch(Targ)
    {
      case FtoHnF:
        floor->floordestheight = P_FindHighestFloorSurrounding(sec);
        break;
      case FtoLnF:
        floor->floordestheight = P_FindLowestFloorSurrounding(sec);
        break;
      case FtoNnF:
        floor->floordestheight = Dirn?
          P_FindNextHighestFloor(sec,sec->floorheight) :
          P_FindNextLowestFloor(sec,sec->floorheight);
        break;
      case FtoLnC:
        floor->floordestheight = P_FindLowestCeilingSurrounding(sec);
        break;
      case FtoC:
        floor->floordestheight = sec->ceilingheight;
        break;
      case FbyST:
        floor->floordestheight = (floor->sector->floorheight>>FRACBITS) +
          floor->direction * (P_FindShortestTextureAround(secnum)>>FRACBITS);
        if (floor->floordestheight>32000)
          floor->floordestheight=32000;
        if (floor->floordestheight<-32000)
          floor->floordestheight=-32000;
        floor->floordestheight<<=FRACBITS;
        break;
      case Fby24:
        floor->floordestheight = floor->sector->floorheight +
          floor->direction * 24*FRACUNIT;
        break;
      case Fby32:
        floor->floordestheight = floor->sector->floorheight +
          floor->direction * 32*FRACUNIT;
        break;
      default:
        break;
    }


    if (ChgT)
    {
      if (ChgM)
      {
        sector_t *sec;



        sec = (Targ==FtoLnC || Targ==FtoC)?
          P_FindModelCeilingSector(floor->floordestheight,secnum) :
          P_FindModelFloorSector(floor->floordestheight,secnum);
        if (sec)
        {
          floor->texture = sec->floorpic;
          switch(ChgT)
          {
            case FChgZero:
              floor->newspecial = 0;

              floor->oldspecial = 0;
              floor->type = genFloorChg0;
              break;
            case FChgTyp:
              floor->newspecial = sec->special;

              floor->oldspecial = sec->oldspecial;
              floor->type = genFloorChgT;
              break;
            case FChgTxt:
              floor->type = genFloorChg;
              break;
            default:
              break;
          }
        }
      }
      else
      {
        floor->texture = line->frontsector->floorpic;
        switch (ChgT)
        {
          case FChgZero:
            floor->newspecial = 0;

            floor->oldspecial = 0;
            floor->type = genFloorChg0;
            break;
          case FChgTyp:
            floor->newspecial = line->frontsector->special;

            floor->oldspecial = line->frontsector->oldspecial;
            floor->type = genFloorChgT;
            break;
          case FChgTxt:
            floor->type = genFloorChg;
          default:
            break;
        }
      }
    }
    if (manual) return rtn;
  }
  return rtn;
}

int EV_DoGenCeiling(line_t *line)
{
  int                   secnum;
  int                   rtn;
  boolean               manual;
  fixed_t               targheight;
  sector_t*             sec;
  ceiling_t*            ceiling;
  unsigned              value = (unsigned)line->special - GenCeilingBase;

  int Crsh = (value & CeilingCrush) >> CeilingCrushShift;
  int ChgT = (value & CeilingChange) >> CeilingChangeShift;
  int Targ = (value & CeilingTarget) >> CeilingTargetShift;
  int Dirn = (value & CeilingDirection) >> CeilingDirectionShift;
  int ChgM = (value & CeilingModel) >> CeilingModelShift;
  int Sped = (value & CeilingSpeed) >> CeilingSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = 0;


  manual = false;
  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_ceiling;
  }

  secnum = -1;

  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_ceiling:

    if (P_SectorActive(ceiling_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }

    rtn = 1;
    ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
    memset(ceiling, 0, sizeof(*ceiling));
    P_AddThinker (&ceiling->thinker);
    sec->ceilingdata = ceiling;
    ceiling->thinker.function = T_MoveCeiling;
    ceiling->crush = Crsh;
    ceiling->direction = Dirn? 1 : -1;
    ceiling->sector = sec;
    ceiling->texture = sec->ceilingpic;
    ceiling->newspecial = sec->special;

    ceiling->oldspecial = sec->oldspecial;
    ceiling->tag = sec->tag;
    ceiling->type = genCeiling;


    switch (Sped)
    {
      case SpeedSlow:
        ceiling->speed = CEILSPEED;
        break;
      case SpeedNormal:
        ceiling->speed = CEILSPEED*2;
        break;
      case SpeedFast:
        ceiling->speed = CEILSPEED*4;
        break;
      case SpeedTurbo:
        ceiling->speed = CEILSPEED*8;
        break;
      default:
        break;
    }


    targheight = sec->ceilingheight;
    switch(Targ)
    {
      case CtoHnC:
        targheight = P_FindHighestCeilingSurrounding(sec);
        break;
      case CtoLnC:
        targheight = P_FindLowestCeilingSurrounding(sec);
        break;
      case CtoNnC:
        targheight = Dirn?
          P_FindNextHighestCeiling(sec,sec->ceilingheight) :
          P_FindNextLowestCeiling(sec,sec->ceilingheight);
        break;
      case CtoHnF:
        targheight = P_FindHighestFloorSurrounding(sec);
        break;
      case CtoF:
        targheight = sec->floorheight;
        break;
      case CbyST:
        targheight = (ceiling->sector->ceilingheight>>FRACBITS) +
          ceiling->direction * (P_FindShortestUpperAround(secnum)>>FRACBITS);
        if (targheight>32000)
          targheight=32000;
        if (targheight<-32000)
          targheight=-32000;
        targheight<<=FRACBITS;
        break;
      case Cby24:
        targheight = ceiling->sector->ceilingheight +
          ceiling->direction * 24*FRACUNIT;
        break;
      case Cby32:
        targheight = ceiling->sector->ceilingheight +
          ceiling->direction * 32*FRACUNIT;
        break;
      default:
        break;
    }
    if (Dirn) ceiling->topheight = targheight;
    else ceiling->bottomheight = targheight;


    if (ChgT)
    {
      if (ChgM)
      {
        sector_t *sec;



        sec = (Targ==CtoHnF || Targ==CtoF)?
          P_FindModelFloorSector(targheight,secnum) :
          P_FindModelCeilingSector(targheight,secnum);
        if (sec)
        {
          ceiling->texture = sec->ceilingpic;
          switch (ChgT)
          {
            case CChgZero:
              ceiling->newspecial = 0;

              ceiling->oldspecial = 0;
              ceiling->type = genCeilingChg0;
              break;
            case CChgTyp:
              ceiling->newspecial = sec->special;

              ceiling->oldspecial = sec->oldspecial;
              ceiling->type = genCeilingChgT;
              break;
            case CChgTxt:
              ceiling->type = genCeilingChg;
              break;
            default:
              break;
          }
        }
      }
      else
      {
        ceiling->texture = line->frontsector->ceilingpic;
        switch (ChgT)
        {
          case CChgZero:
            ceiling->newspecial = 0;

            ceiling->oldspecial = 0;
            ceiling->type = genCeilingChg0;
            break;
          case CChgTyp:
            ceiling->newspecial = line->frontsector->special;

            ceiling->oldspecial = line->frontsector->oldspecial;
            ceiling->type = genCeilingChgT;
            break;
          case CChgTxt:
            ceiling->type = genCeilingChg;
            break;
          default:
            break;
        }
      }
    }
    P_AddActiveCeiling(ceiling);
    if (manual) return rtn;
  }
  return rtn;
}

int EV_DoGenLift(line_t *line)
{

  plat_t*         plat;
  int             secnum;
  int             rtn;
  boolean         manual;
  sector_t*       sec;
  unsigned        value = (unsigned)line->special - GenLiftBase;



  int Targ = (value & LiftTarget) >> LiftTargetShift;
  int Dely = (value & LiftDelay) >> LiftDelayShift;
  int Sped = (value & LiftSpeed) >> LiftSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  secnum = -1;
  rtn = 0;

  if (Targ==LnF2HnF)
    P_ActivateInStasis(line->tag);


  manual = false;
  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_lift;
  }


  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_lift:

    if (P_SectorActive(floor_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }


    rtn = 1;
    plat = Z_Malloc( sizeof(*plat), PU_LEVSPEC, 0);
    memset(plat, 0, sizeof(*plat));
    P_AddThinker(&plat->thinker);

    plat->sector = sec;
    plat->sector->floordata = plat;
    plat->thinker.function = T_PlatRaise;
    plat->crush = false;
    plat->tag = line->tag;

    plat->type = genLift;
    plat->high = sec->floorheight;
    plat->status = down;


    switch(Targ)
    {
      case F2LnF:
        plat->low = P_FindLowestFloorSurrounding(sec);
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        break;
      case F2NnF:
        plat->low = P_FindNextLowestFloor(sec,sec->floorheight);
        break;
      case F2LnC:
        plat->low = P_FindLowestCeilingSurrounding(sec);
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        break;
      case LnF2HnF:
        plat->type = genPerpetual;
        plat->low = P_FindLowestFloorSurrounding(sec);
        if (plat->low > sec->floorheight)
          plat->low = sec->floorheight;
        plat->high = P_FindHighestFloorSurrounding(sec);
        if (plat->high < sec->floorheight)
          plat->high = sec->floorheight;
        plat->status = P_Random(pr_genlift)&1;
        break;
      default:
        break;
    }


    switch(Sped)
    {
      case SpeedSlow:
        plat->speed = PLATSPEED * 2;
        break;
      case SpeedNormal:
        plat->speed = PLATSPEED * 4;
        break;
      case SpeedFast:
        plat->speed = PLATSPEED * 8;
        break;
      case SpeedTurbo:
        plat->speed = PLATSPEED * 16;
        break;
      default:
        break;
    }


    switch(Dely)
    {
      case 0:
        plat->wait = 1*35;
        break;
      case 1:
        plat->wait = PLATWAIT*35;
        break;
      case 2:
        plat->wait = 5*35;
        break;
      case 3:
        plat->wait = 10*35;
        break;
    }

    S_StartSound((mobj_t *)&sec->soundorg,sfx_pstart);
    P_AddActivePlat(plat);

    if (manual)
      return rtn;
  }
  return rtn;
}

int EV_DoGenStairs(line_t *line)
{

  int                   secnum;
  int                   osecnum;
  int                   height;
  int                   i;
  int                   newsecnum;
  int                   texture;
  int                   ok;
  int                   rtn;
  boolean               manual;

  sector_t*             sec;
  sector_t*             tsec;

  floormove_t*  floor;

  fixed_t               stairsize;
  fixed_t               speed;

  unsigned              value = (unsigned)line->special - GenStairsBase;



  int Igno = (value & StairIgnore) >> StairIgnoreShift;
  int Dirn = (value & StairDirection) >> StairDirectionShift;
  int Step = (value & StairStep) >> StairStepShift;
  int Sped = (value & StairSpeed) >> StairSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = 0;


  manual = false;
  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_stair;
  }

  secnum = -1;

  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_stair:



    if (P_SectorActive(floor_special,sec) || sec->stairlock)
    {
      if (!manual)
        continue;
      else
        return rtn;
    }


    rtn = 1;
    floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);
    memset(floor, 0, sizeof(*floor));
    P_AddThinker (&floor->thinker);
    sec->floordata = floor;
    floor->thinker.function = T_MoveFloor;
    floor->direction = Dirn? 1 : -1;
    floor->sector = sec;


    switch(Sped)
      {
      default:
      case SpeedSlow:
        floor->speed = FLOORSPEED/4;
        break;
      case SpeedNormal:
        floor->speed = FLOORSPEED/2;
        break;
      case SpeedFast:
        floor->speed = FLOORSPEED*2;
        break;
      case SpeedTurbo:
        floor->speed = FLOORSPEED*4;
        break;
      }


    switch(Step)
    {
      default:
      case 0:
        stairsize = 4*FRACUNIT;
        break;
      case 1:
        stairsize = 8*FRACUNIT;
        break;
      case 2:
        stairsize = 16*FRACUNIT;
        break;
      case 3:
        stairsize = 24*FRACUNIT;
        break;
    }

    speed = floor->speed;
    height = sec->floorheight + floor->direction * stairsize;
    floor->floordestheight = height;
    texture = sec->floorpic;
    floor->crush = false;
    floor->type = genBuildStair;

    sec->stairlock = -2;
    sec->nextsec = -1;
    sec->prevsec = -1;

    osecnum = secnum;



    do
    {
      ok = 0;
      for (i = 0;i < sec->linecount;i++)
      {
        if ( !((sec->lines[i])->backsector) )
          continue;

        tsec = (sec->lines[i])->frontsector;
        newsecnum = tsec-sectors;

        if (secnum != newsecnum)
          continue;

        tsec = (sec->lines[i])->backsector;
        newsecnum = tsec - sectors;

        if (!Igno && tsec->floorpic != texture)
          continue;

        if (P_SectorActive(floor_special,tsec) || tsec->stairlock)
          continue;

        height += floor->direction * stairsize;
        sec->nextsec = newsecnum;
        tsec->prevsec = secnum;
        tsec->nextsec = -1;
        tsec->stairlock = -2;

        sec = tsec;
        secnum = newsecnum;
        floor = Z_Malloc (sizeof(*floor), PU_LEVSPEC, 0);

        memset(floor, 0, sizeof(*floor));
        P_AddThinker (&floor->thinker);

        sec->floordata = floor;
        floor->thinker.function = T_MoveFloor;
        floor->direction = Dirn? 1 : -1;
        floor->sector = sec;
        floor->speed = speed;
        floor->floordestheight = height;
        floor->crush = false;
        floor->type = genBuildStair;

        ok = 1;
        break;
      }
    } while(ok);
      if (manual)
        return rtn;
      secnum = osecnum;
  }

  if (rtn)
    line->special ^= StairDirection;
  return rtn;
}

int EV_DoGenCrusher(line_t *line)
{

  int                   secnum;
  int                   rtn;
  boolean               manual;
  sector_t*             sec;
  ceiling_t*            ceiling;
  unsigned              value = (unsigned)line->special - GenCrusherBase;

  int Slnt = (value & CrusherSilent) >> CrusherSilentShift;
  int Sped = (value & CrusherSpeed) >> CrusherSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = P_ActivateInStasisCeiling(line);
  manual = false;

  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_crusher;
  }

  secnum = -1;

  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];

manual_crusher:

    if (P_SectorActive(ceiling_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }


    rtn = 1;
    ceiling = Z_Malloc (sizeof(*ceiling), PU_LEVSPEC, 0);
    memset(ceiling, 0, sizeof(*ceiling));
    P_AddThinker (&ceiling->thinker);
    sec->ceilingdata = ceiling;
    ceiling->thinker.function = T_MoveCeiling;
    ceiling->crush = true;
    ceiling->direction = -1;
    ceiling->sector = sec;
    ceiling->texture = sec->ceilingpic;
    ceiling->newspecial = sec->special;
    ceiling->tag = sec->tag;
    ceiling->type = Slnt? genSilentCrusher : genCrusher;
    ceiling->topheight = sec->ceilingheight;
    ceiling->bottomheight = sec->floorheight + (8*FRACUNIT);


    switch (Sped)
    {
      case SpeedSlow:
        ceiling->speed = CEILSPEED;
        break;
      case SpeedNormal:
        ceiling->speed = CEILSPEED*2;
        break;
      case SpeedFast:
        ceiling->speed = CEILSPEED*4;
        break;
      case SpeedTurbo:
        ceiling->speed = CEILSPEED*8;
        break;
      default:
        break;
    }
    ceiling->oldspeed=ceiling->speed;

    P_AddActiveCeiling(ceiling);
    if (manual) return rtn;
  }
  return rtn;
}

int EV_DoGenLockedDoor(line_t *line)
{

  int   secnum,rtn;
  sector_t* sec;
  vldoor_t* door;
  boolean manual;
  unsigned  value = (unsigned)line->special - GenLockedBase;



  int Kind = (value & LockedKind) >> LockedKindShift;
  int Sped = (value & LockedSpeed) >> LockedSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = 0;


  manual = false;
  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_locked;
  }

  secnum = -1;
  rtn = 0;


  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
manual_locked:

    if (P_SectorActive(ceiling_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }

    rtn = 1;
    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
    memset(door, 0, sizeof(*door));
    P_AddThinker (&door->thinker);
    sec->ceilingdata = door;

    door->thinker.function = T_VerticalDoor;
    door->sector = sec;
    door->topwait = VDOORWAIT;
    door->line = line;
    door->topheight = P_FindLowestCeilingSurrounding(sec);
    door->topheight -= 4*FRACUNIT;
    door->direction = 1;
    door->lighttag = (line->special&6) == 6 && line->special > GenLockedBase ? line->tag : 0;

    switch(Sped)
    {
      default:
      case SpeedSlow:
        door->type = Kind? genOpen : genRaise;
        door->speed = VDOORSPEED;
        break;
      case SpeedNormal:
        door->type = Kind? genOpen : genRaise;
        door->speed = VDOORSPEED*2;
        break;
      case SpeedFast:
        door->type = Kind? genBlazeOpen : genBlazeRaise;
        door->speed = VDOORSPEED*4;
        break;
      case SpeedTurbo:
        door->type = Kind? genBlazeOpen : genBlazeRaise;
        door->speed = VDOORSPEED*8;

        break;
    }

    S_StartSound((mobj_t *)&door->sector->soundorg, door->speed >= VDOORSPEED*4 ? sfx_bdopn : sfx_doropn);

    if (manual)
      return rtn;
  }
  return rtn;
}

int EV_DoGenDoor(line_t *line)
{

  int   secnum,rtn;
  sector_t* sec;
  boolean   manual;
  vldoor_t* door;
  unsigned  value = (unsigned)line->special - GenDoorBase;



  int Dely = (value & DoorDelay) >> DoorDelayShift;
  int Kind = (value & DoorKind) >> DoorKindShift;
  int Sped = (value & DoorSpeed) >> DoorSpeedShift;
  int Trig = (value & TriggerType) >> TriggerTypeShift;

  rtn = 0;


  manual = false;
  if (Trig==PushOnce || Trig==PushMany)
  {
    if (!(sec = line->backsector))
      return rtn;
    secnum = sec-sectors;
    manual = true;
    goto manual_door;
  }


  secnum = -1;
  rtn = 0;


  while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
  {
    sec = &sectors[secnum];
manual_door:

    if (P_SectorActive(ceiling_special,sec))
    {
      if (!manual)
        continue;
      else
        return rtn;
    }


    rtn = 1;
    door = Z_Malloc (sizeof(*door), PU_LEVSPEC, 0);
    memset(door, 0, sizeof(*door));
    P_AddThinker (&door->thinker);
    sec->ceilingdata = door;

    door->thinker.function = T_VerticalDoor;
    door->sector = sec;

    switch(Dely)
    {
      default:
      case 0:
        door->topwait = 35;
        break;
      case 1:
        door->topwait = VDOORWAIT;
        break;
      case 2:
        door->topwait = 2*VDOORWAIT;
        break;
      case 3:
        door->topwait = 7*VDOORWAIT;
        break;
    }

    switch(Sped)
    {
      default:
      case SpeedSlow:
        door->speed = VDOORSPEED;
        break;
      case SpeedNormal:
        door->speed = VDOORSPEED*2;
        break;
      case SpeedFast:
        door->speed = VDOORSPEED*4;
        break;
      case SpeedTurbo:
        door->speed = VDOORSPEED*8;
        break;
    }
    door->line = line;
    door->lighttag = (line->special&6) == 6 && line->special > GenLockedBase ? line->tag : 0;

    switch(Kind)
    {
      case OdCDoor:
        door->direction = 1;
        door->topheight = P_FindLowestCeilingSurrounding(sec);
        door->topheight -= 4*FRACUNIT;
        if (door->topheight != sec->ceilingheight)
          S_StartSound((mobj_t *)&door->sector->soundorg,Sped>=SpeedFast || sfx_doropn);
        door->type = Sped>=SpeedFast? genBlazeRaise : genRaise;
        break;
      case ODoor:
        door->direction = 1;
        door->topheight = P_FindLowestCeilingSurrounding(sec);
        door->topheight -= 4*FRACUNIT;
        if (door->topheight != sec->ceilingheight)
          S_StartSound((mobj_t *)&door->sector->soundorg,Sped>=SpeedFast || sfx_doropn);
        door->type = Sped>=SpeedFast? genBlazeOpen : genOpen;
        break;
      case CdODoor:
        door->topheight = sec->ceilingheight;
        door->direction = -1;
        S_StartSound((mobj_t *)&door->sector->soundorg,Sped>=SpeedFast && sfx_bdcls);
        door->type = Sped>=SpeedFast? genBlazeCdO : genCdO;
        break;
      case CDoor:
        door->topheight = P_FindLowestCeilingSurrounding(sec);
        door->topheight -= 4*FRACUNIT;
        door->direction = -1;
        S_StartSound((mobj_t *)&door->sector->soundorg,Sped>=SpeedFast && sfx_bdcls);
        door->type = Sped>=SpeedFast? genBlazeClose : genClose;
        break;
      default:
        break;
    }
    if (manual)
      return rtn;
  }
  return rtn;
}
