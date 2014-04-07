#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_setup.h"
#include "m_random.h"
#include "d_englsh.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "g_game.h"
#include "p_inter.h"
#include "s_sound.h"
#include "r_plane.h"
#include "i_system.h"
#include "z_zone.h"

typedef struct
{

    boolean istexture;
    int picnum;
    int basepic;
    int numpics;
    int speed;

} anim_t;

typedef struct
{
  char istexture;
  char endname[9];
  char startname[9];
  int speed;
} animdef_t;

animdef_t animdefs[] = {
    {false, "NUKAGE3", "NUKAGE1", 8},
    {false, "FWATER4", "FWATER1", 8},
    {false, "SWATER4", "SWATER1",  8},
    {false, "LAVA4", "LAVA1", 8},
    {false, "BLOOD3", "BLOOD1", 8},
    {false, "RROCK08", "RROCK05", 8},  
    {false, "SLIME04", "SLIME01", 8},
    {false, "SLIME08", "SLIME05", 8},
    {false, "SLIME12", "SLIME09", 8},
    {true, "BLODGR4", "BLODGR1", 8},
    {true, "SLADRIP3", "SLADRIP1", 8},
    {true, "BLODRIP4", "BLODRIP1", 8},
    {true, "FIREWALL", "FIREWALA", 8},
    {true, "GSTFONT3", "GSTFONT1", 8},
    {true, "FIRELAVA", "FIRELAV3", 8},
    {true, "FIREMAG3", "FIREMAG1", 8},
    {true, "FIREBLU2", "FIREBLU1", 8},
    {true, "ROCKRED3", "ROCKRED1", 8},
    {true, "BFALL4", "BFALL1", 8},
    {true, "SFALL4", "SFALL1", 8},
    {true, "WFALL4", "WFALL1", 8},
    {true, "DBRAIN4", "DBRAIN1", 8},
    {-1}
};

#define MAXANIMS 32

static anim_t *lastanim;
static anim_t *anims;
static size_t maxanims;
static void P_SpawnScrollers(void);
static void P_SpawnFriction(void);
static void P_SpawnPushers(void);

void P_InitPicAnims (void)
{

    int i;

    lastanim = anims;

  for (i=0 ; animdefs[i].istexture != -1 ; i++)
  {

    if (lastanim >= anims + maxanims)
    {
      size_t newmax = maxanims ? maxanims*2 : MAXANIMS;
      anims = realloc(anims, newmax*sizeof(*anims));
      lastanim = anims + maxanims;
      maxanims = newmax;
    }

    if (animdefs[i].istexture)
    {

      if (R_CheckTextureNumForName(animdefs[i].startname) == -1)
          continue;

      lastanim->picnum = R_TextureNumForName (animdefs[i].endname);
      lastanim->basepic = R_TextureNumForName (animdefs[i].startname);
    }
    else
    {
      if (W_CheckNumForName(animdefs[i].startname, ns_flats) == -1)
          continue;

      lastanim->picnum = R_FlatNumForName (animdefs[i].endname);
      lastanim->basepic = R_FlatNumForName (animdefs[i].startname);
    }

    lastanim->istexture = animdefs[i].istexture;
    lastanim->numpics = lastanim->picnum - lastanim->basepic + 1;

    if (lastanim->numpics < 2)
        I_Error ("P_InitPicAnims: bad cycle from %s to %s",
                  animdefs[i].startname,
                  animdefs[i].endname);

    lastanim->speed = animdefs[i].speed;
    lastanim++;
  }
}

side_t* getSide
( int           currentSector,
  int           line,
  int           side )
{
  return &sides[ (sectors[currentSector].lines[line])->sidenum[side] ];
}

sector_t* getSector
( int           currentSector,
  int           line,
  int           side )
{
  return sides[ (sectors[currentSector].lines[line])->sidenum[side] ].sector;
}

int twoSided
( int   sector,
  int   line )
{

  return (sectors[sector].lines[line])->sidenum[1] != NO_INDEX;
}

sector_t* getNextSector
( line_t*       line,
  sector_t*     sec )
{

  if (line->frontsector == sec) {
    if (line->backsector!=sec)
      return line->backsector;
    else
      return NULL;
  }
  return line->frontsector;
}

fixed_t P_FindLowestFloorSurrounding(sector_t* sec)
{
  int                 i;
  line_t*             check;
  sector_t*           other;
  fixed_t             floor = sec->floorheight;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight < floor)
      floor = other->floorheight;
  }
  return floor;
}

fixed_t P_FindHighestFloorSurrounding(sector_t *sec)
{
  int i;
  line_t* check;
  sector_t* other;
  fixed_t floor = -32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->floorheight > floor)
      floor = other->floorheight;
  }
  return floor;
}

fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight > currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight < height &&
            other->floorheight > currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
}

fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->floorheight < currentheight)
    {
      int height = other->floorheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->floorheight > height &&
            other->floorheight < currentheight)
          height = other->floorheight;
      return height;
    }
  return currentheight;
}

fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
        other->ceilingheight < currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight > height &&
            other->ceilingheight < currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}

fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight)
{
  sector_t *other;
  int i;

  for (i=0 ;i < sec->linecount ; i++)
    if ((other = getNextSector(sec->lines[i],sec)) &&
         other->ceilingheight > currentheight)
    {
      int height = other->ceilingheight;
      while (++i < sec->linecount)
        if ((other = getNextSector(sec->lines[i],sec)) &&
            other->ceilingheight < height &&
            other->ceilingheight > currentheight)
          height = other->ceilingheight;
      return height;
    }
  return currentheight;
}

fixed_t P_FindLowestCeilingSurrounding(sector_t* sec)
{
  int                 i;
  line_t*             check;
  sector_t*           other;
  fixed_t             height = 32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight < height)
      height = other->ceilingheight;
  }
  return height;
}

fixed_t P_FindHighestCeilingSurrounding(sector_t* sec)
{
  int             i;
  line_t* check;
  sector_t*       other;
  fixed_t height = -32000*FRACUNIT;

  for (i=0 ;i < sec->linecount ; i++)
  {
    check = sec->lines[i];
    other = getNextSector(check,sec);

    if (!other)
      continue;

    if (other->ceilingheight > height)
      height = other->ceilingheight;
  }
  return height;
}

fixed_t P_FindShortestTextureAround(int secnum)
{
  int minsize = 32000<<FRACBITS;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->bottomtexture > 0)
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
      side = getSide(secnum,i,1);
      if (side->bottomtexture > 0)
        if (textureheight[side->bottomtexture] < minsize)
          minsize = textureheight[side->bottomtexture];
    }
  }
  return minsize;
}

fixed_t P_FindShortestUpperAround(int secnum)
{
  int minsize = 32000<<FRACBITS;
  side_t*     side;
  int i;
  sector_t *sec = &sectors[secnum];

  for (i = 0; i < sec->linecount; i++)
  {
    if (twoSided(secnum, i))
    {
      side = getSide(secnum,i,0);
      if (side->toptexture > 0)
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
      side = getSide(secnum,i,1);
      if (side->toptexture > 0)
        if (textureheight[side->toptexture] < minsize)
          minsize = textureheight[side->toptexture];
    }
  }
  return minsize;
}

sector_t *P_FindModelFloorSector(fixed_t floordestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;

  sec = &sectors[secnum];

  for (i = 0; i < sec->linecount; i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector-sectors == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (sec->floorheight == floordestheight)
        return sec;
    }
  }
  return NULL;
}

sector_t *P_FindModelCeilingSector(fixed_t ceildestheight,int secnum)
{
  int i;
  sector_t *sec=NULL;

  sec = &sectors[secnum];

  for (i = 0; i < sec->linecount; i++)
  {
    if ( twoSided(secnum, i) )
    {
      if (getSide(secnum,i,0)->sector-sectors == secnum)
          sec = getSector(secnum,i,1);
      else
          sec = getSector(secnum,i,0);

      if (sec->ceilingheight == ceildestheight)
        return sec;
    }
  }
  return NULL;
}

int P_FindSectorFromLineTag(const line_t *line, int start)
{
  start = start >= 0 ? sectors[start].nexttag :
    sectors[(unsigned) line->tag % (unsigned) numsectors].firsttag;
  while (start >= 0 && sectors[start].tag != line->tag)
    start = sectors[start].nexttag;
  return start;
}



int P_FindLineFromLineTag(const line_t *line, int start)
{
  start = start >= 0 ? lines[start].nexttag :
    lines[(unsigned) line->tag % (unsigned) numlines].firsttag;
  while (start >= 0 && lines[start].tag != line->tag)
    start = lines[start].nexttag;
  return start;
}


static void P_InitTagLists(void)
{
  register int i;

  for (i=numsectors; --i>=0; )
    sectors[i].firsttag = -1;
  for (i=numsectors; --i>=0; )
    {
      int j = (unsigned) sectors[i].tag % (unsigned) numsectors;
      sectors[i].nexttag = sectors[j].firsttag;
      sectors[j].firsttag = i;
    }



  for (i=numlines; --i>=0; )
    lines[i].firsttag = -1;
  for (i=numlines; --i>=0; )
    {
      int j = (unsigned) lines[i].tag % (unsigned) numlines;
      lines[i].nexttag = lines[j].firsttag;
      lines[j].firsttag = i;
    }
}


int P_FindMinSurroundingLight
( sector_t*     sector,
  int           max )
{
  int         i;
  int         min;
  line_t*     line;
  sector_t*   check;

  min = max;
  for (i=0 ; i < sector->linecount ; i++)
  {
    line = sector->lines[i];
    check = getNextSector(line,sector);

    if (!check)
      continue;

    if (check->lightlevel < min)
      min = check->lightlevel;
  }
  return min;
}

boolean P_CanUnlockGenDoor
( line_t* line,
  player_t* player)
{

  int skulliscard = (line->special & LockedNKeys)>>LockedNKeysShift;


  switch((line->special & LockedKey)>>LockedKeyShift)
  {
    case AnyKey:
      if
      (
        !player->cards[it_redcard] &&
        !player->cards[it_redskull] &&
        !player->cards[it_bluecard] &&
        !player->cards[it_blueskull] &&
        !player->cards[it_yellowcard] &&
        !player->cards[it_yellowskull]
      )
      {
        player->message = PD_ANY;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case RCard:
      if
      (
        !player->cards[it_redcard] &&
        (!skulliscard || !player->cards[it_redskull])
      )
      {
        player->message = skulliscard? PD_REDK : PD_REDC;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case BCard:
      if
      (
        !player->cards[it_bluecard] &&
        (!skulliscard || !player->cards[it_blueskull])
      )
      {
        player->message = skulliscard? PD_BLUEK : PD_BLUEC;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case YCard:
      if
      (
        !player->cards[it_yellowcard] &&
        (!skulliscard || !player->cards[it_yellowskull])
      )
      {
        player->message = skulliscard? PD_YELLOWK : PD_YELLOWC;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case RSkull:
      if
      (
        !player->cards[it_redskull] &&
        (!skulliscard || !player->cards[it_redcard])
      )
      {
        player->message = skulliscard? PD_REDK : PD_REDS;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case BSkull:
      if
      (
        !player->cards[it_blueskull] &&
        (!skulliscard || !player->cards[it_bluecard])
      )
      {
        player->message = skulliscard? PD_BLUEK : PD_BLUES;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case YSkull:
      if
      (
        !player->cards[it_yellowskull] &&
        (!skulliscard || !player->cards[it_yellowcard])
      )
      {
        player->message = skulliscard? PD_YELLOWK : PD_YELLOWS;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
    case AllKeys:
      if
      (
        !skulliscard &&
        (
          !player->cards[it_redcard] ||
          !player->cards[it_redskull] ||
          !player->cards[it_bluecard] ||
          !player->cards[it_blueskull] ||
          !player->cards[it_yellowcard] ||
          !player->cards[it_yellowskull]
        )
      )
      {
        player->message = PD_ALL6;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      if
      (
        skulliscard &&
        (
          (!player->cards[it_redcard] &&
            !player->cards[it_redskull]) ||
          (!player->cards[it_bluecard] &&
            !player->cards[it_blueskull]) ||
          (!player->cards[it_yellowcard] &&
            !player->cards[it_yellowskull])
        )
      )
      {
        player->message = PD_ALL3;
        S_StartSound(player->mo,sfx_oof);
        return false;
      }
      break;
  }
  return true;
}

boolean P_SectorActive(special_e t, const sector_t *sec)
{
    switch (t)
    {
      case floor_special:
        return sec->floordata != NULL;
      case ceiling_special:
        return sec->ceilingdata != NULL;
      case lighting_special:
        return sec->lightingdata != NULL;
    }
  return true;
}

int P_CheckTag(line_t *line)
{
  if (line->tag)
    return 1;

  switch(line->special)
  {
    case 1:
    case 26:
    case 27:
    case 28:
    case 31:
    case 32:
    case 33:
    case 34:
    case 117:
    case 118:

    case 139:
    case 170:
    case 79:
    case 35:
    case 138:
    case 171:
    case 81:
    case 13:
    case 192:
    case 169:
    case 80:
    case 12:
    case 194:
    case 173:
    case 157:
    case 104:
    case 193:
    case 172:
    case 156:
    case 17:

    case 195:
    case 174:
    case 97:
    case 39:
    case 126:
    case 125:
    case 210:
    case 209:
    case 208:
    case 207:

    case 11:
    case 52:
    case 197:
    case 51:
    case 124:
    case 198:

    case 48:
    case 85:
      return 1;

    default:
      break;
  }
  return 0;
}

boolean P_IsSecret(const sector_t *sec)
{
  return (sec->special==9 || (sec->special&SECRET_MASK));
}

boolean P_WasSecret(const sector_t *sec)
{
  return (sec->oldspecial==9 || (sec->oldspecial&SECRET_MASK));
}

void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing)
{
  int         ok;


  if (!thing->player)
  {

    switch(thing->type)
    {
      case MT_ROCKET:
      case MT_PLASMA:
      case MT_BFG:
      case MT_TROOPSHOT:
      case MT_HEADSHOT:
      case MT_BRUISERSHOT:
        return;
        break;

      default: break;
    }
  }


  {


    int (*linefunc)(line_t *line)=NULL;


    if ((unsigned)line->special >= GenEnd)
    {

    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player)
      {
        if (!(line->special & DoorMonster))
          return;
        if (line->flags & ML_SECRET)
          return;
      }
      if (!line->tag)
        return;
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player)
        return;
      if (((line->special&TriggerType)==WalkOnce) || ((line->special&TriggerType)==WalkMany))
      {
        if (!P_CanUnlockGenDoor(line,thing->player))
          return;
      }
      else
        return;
      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player)
        if (!(line->special & LiftMonster))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenStairs;
    }

    if (linefunc)
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case WalkOnce:
          if (linefunc(line))
            line->special = 0;
          return;
        case WalkMany:
          linefunc(line);
          return;
        default:
          return;
      }
  }

  if (!thing->player)
  {
    ok = 0;
    switch(line->special)
    {
      case 39:
      case 97:
      case 125:
      case 126:
      case 4:
      case 10:
      case 88:

      case 208:
      case 207:
      case 243:
      case 244:
      case 262:
      case 263:
      case 264:
      case 265:
      case 266:
      case 267:
      case 268:
      case 269:
        ok = 1;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))
    return;




  switch (line->special)
  {


    case 2:

      if (EV_DoDoor(line,open))
        line->special = 0;
      break;

    case 3:

      if (EV_DoDoor(line,close))
        line->special = 0;
      break;

    case 4:

      if (EV_DoDoor(line,normal))
        line->special = 0;
      break;

    case 5:

      if (EV_DoFloor(line,raiseFloor))
        line->special = 0;
      break;

    case 6:

      if (EV_DoCeiling(line,fastCrushAndRaise))
        line->special = 0;
      break;

    case 8:

      if (EV_BuildStairs(line,build8))
        line->special = 0;
      break;

    case 10:

      if (EV_DoPlat(line,downWaitUpStay,0))
        line->special = 0;
      break;

    case 12:

      if (EV_LightTurnOn(line,0))
        line->special = 0;
      break;

    case 13:

      if (EV_LightTurnOn(line,255))
        line->special = 0;
      break;

    case 16:

      if (EV_DoDoor(line,close30ThenOpen))
        line->special = 0;
      break;

    case 17:

      if (EV_StartLightStrobing(line))
        line->special = 0;
      break;

    case 19:

      if (EV_DoFloor(line,lowerFloor))
        line->special = 0;
      break;

    case 22:

      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        line->special = 0;
      break;

    case 25:

      if (EV_DoCeiling(line,crushAndRaise))
        line->special = 0;
      break;

    case 30:


      if (EV_DoFloor(line,raiseToTexture))
        line->special = 0;
      break;

    case 35:

      if (EV_LightTurnOn(line,35))
        line->special = 0;
      break;

    case 36:

      if (EV_DoFloor(line,turboLower))
        line->special = 0;
      break;

    case 37:

      if (EV_DoFloor(line,lowerAndChange))
        line->special = 0;
      break;

    case 38:

      if (EV_DoFloor(line, lowerFloorToLowest))
        line->special = 0;
      break;

    case 39:

      if (EV_Teleport(line, side, thing))
        line->special = 0;
      break;

    case 40:

      if (EV_DoCeiling(line, raiseToHighest))
          line->special = 0;
      break;

    case 44:

      if (EV_DoCeiling(line, lowerAndCrush))
        line->special = 0;
      break;

    case 52:


      if (!(thing->player && thing->player->health <= 0))
        G_ExitLevel ();
      break;

    case 53:

      if (EV_DoPlat(line,perpetualRaise,0))
        line->special = 0;
      break;

    case 54:

      if (EV_StopPlat(line))
        line->special = 0;
      break;

    case 56:

      if (EV_DoFloor(line,raiseFloorCrush))
        line->special = 0;
      break;

    case 57:

      if (EV_CeilingCrushStop(line))
        line->special = 0;
      break;

    case 58:

      if (EV_DoFloor(line,raiseFloor24))
        line->special = 0;
      break;

    case 59:

      if (EV_DoFloor(line,raiseFloor24AndChange))
        line->special = 0;
      break;

    case 100:

      if (EV_BuildStairs(line,turbo16))
        line->special = 0;
      break;

    case 104:

      if (EV_TurnTagLightsOff(line))
        line->special = 0;
      break;

    case 108:

      if (EV_DoDoor(line,blazeRaise))
        line->special = 0;
      break;

    case 109:

      if (EV_DoDoor (line,blazeOpen))
        line->special = 0;
      break;

    case 110:

      if (EV_DoDoor (line,blazeClose))
        line->special = 0;
      break;

    case 119:

      if (EV_DoFloor(line,raiseFloorToNearest))
        line->special = 0;
      break;

    case 121:

      if (EV_DoPlat(line,blazeDWUS,0))
        line->special = 0;
      break;

    case 124:



      if (!(thing->player && thing->player->health <= 0))
            G_SecretExitLevel ();
      break;

    case 125:

      if (!thing->player &&
          (EV_Teleport(line, side, thing)))
        line->special = 0;
      break;

    case 130:

      if (EV_DoFloor(line,raiseFloorTurbo))
        line->special = 0;
      break;

    case 141:

      if (EV_DoCeiling(line,silentCrushAndRaise))
        line->special = 0;
      break;



    case 72:

      EV_DoCeiling( line, lowerAndCrush );
      break;

    case 73:

      EV_DoCeiling(line,crushAndRaise);
      break;

    case 74:

      EV_CeilingCrushStop(line);
      break;

    case 75:

      EV_DoDoor(line,close);
      break;

    case 76:

      EV_DoDoor(line,close30ThenOpen);
      break;

    case 77:

      EV_DoCeiling(line,fastCrushAndRaise);
      break;

    case 79:

      EV_LightTurnOn(line,35);
      break;

    case 80:

      EV_LightTurnOn(line,0);
      break;

    case 81:

      EV_LightTurnOn(line,255);
      break;

    case 82:

      EV_DoFloor( line, lowerFloorToLowest );
      break;

    case 83:

      EV_DoFloor(line,lowerFloor);
      break;

    case 84:

      EV_DoFloor(line,lowerAndChange);
      break;

    case 86:

      EV_DoDoor(line,open);
      break;

    case 87:

      EV_DoPlat(line,perpetualRaise,0);
      break;

    case 88:

      EV_DoPlat(line,downWaitUpStay,0);
      break;

    case 89:

      EV_StopPlat(line);
      break;

    case 90:

      EV_DoDoor(line,normal);
      break;

    case 91:

      EV_DoFloor(line,raiseFloor);
      break;

    case 92:

      EV_DoFloor(line,raiseFloor24);
      break;

    case 93:

      EV_DoFloor(line,raiseFloor24AndChange);
      break;

    case 94:

      EV_DoFloor(line,raiseFloorCrush);
      break;

    case 95:


      EV_DoPlat(line,raiseToNearestAndChange,0);
      break;

    case 96:


      EV_DoFloor(line,raiseToTexture);
      break;

    case 97:

      EV_Teleport( line, side, thing );
      break;

    case 98:

      EV_DoFloor(line,turboLower);
      break;

    case 105:

      EV_DoDoor (line,blazeRaise);
      break;

    case 106:

      EV_DoDoor (line,blazeOpen);
      break;

    case 107:

      EV_DoDoor (line,blazeClose);
      break;

    case 120:

      EV_DoPlat(line,blazeDWUS,0);
      break;

    case 126:

      if (!thing->player)
        EV_Teleport( line, side, thing );
      break;

    case 128:

      EV_DoFloor(line,raiseFloorToNearest);
      break;

    case 129:

      EV_DoFloor(line,raiseFloorTurbo);
      break;

    default:
        switch (line->special)
        {


          case 142:


            if (EV_DoFloor(line,raiseFloor512))
              line->special = 0;
            break;

          case 143:


            if (EV_DoPlat(line,raiseAndChange,24))
              line->special = 0;
            break;

          case 144:


            if (EV_DoPlat(line,raiseAndChange,32))
              line->special = 0;
            break;

          case 145:


            if (EV_DoCeiling( line, lowerToFloor ))
              line->special = 0;
            break;

          case 146:


            if (EV_DoDonut(line))
              line->special = 0;
            break;

          case 199:


            if (EV_DoCeiling(line,lowerToLowest))
              line->special = 0;
            break;

          case 200:


            if (EV_DoCeiling(line,lowerToMaxFloor))
              line->special = 0;
            break;

          case 207:

            if (EV_SilentTeleport(line, side, thing))
              line->special = 0;
            break;


          case 153:


            if (EV_DoChange(line,trigChangeOnly))
              line->special = 0;
            break;

          case 239:


            if (EV_DoChange(line,numChangeOnly))
              line->special = 0;
            break;

          case 219:


            if (EV_DoFloor(line,lowerFloorToNearest))
              line->special = 0;
            break;

          case 227:


            if (EV_DoElevator(line,elevateUp))
              line->special = 0;
            break;

          case 231:


            if (EV_DoElevator(line,elevateDown))
              line->special = 0;
            break;

          case 235:


            if (EV_DoElevator(line,elevateCurrent))
              line->special = 0;
            break;

          case 243:

            if (EV_SilentLineTeleport(line, side, thing, false))
              line->special = 0;
            break;

          case 262:
            if (EV_SilentLineTeleport(line, side, thing, true))
              line->special = 0;
            break;

          case 264:
            if (!thing->player &&
                EV_SilentLineTeleport(line, side, thing, true))
              line->special = 0;
            break;

          case 266:
            if (!thing->player &&
                EV_SilentLineTeleport(line, side, thing, false))
              line->special = 0;
            break;

          case 268:
            if (!thing->player && EV_SilentTeleport(line, side, thing))
              line->special = 0;
            break;

          case 147:


            EV_DoFloor(line,raiseFloor512);
            break;

          case 148:


            EV_DoPlat(line,raiseAndChange,24);
            break;

          case 149:


            EV_DoPlat(line,raiseAndChange,32);
            break;

          case 150:


            EV_DoCeiling(line,silentCrushAndRaise);
            break;

          case 151:



            EV_DoCeiling( line, raiseToHighest );
            EV_DoFloor( line, lowerFloorToLowest );
            break;

          case 152:


            EV_DoCeiling( line, lowerToFloor );
            break;


          case 256:


            EV_BuildStairs(line,build8);
            break;


          case 257:


            EV_BuildStairs(line,turbo16);
            break;

          case 155:


            EV_DoDonut(line);
            break;

          case 156:


            EV_StartLightStrobing(line);
            break;

          case 157:


            EV_TurnTagLightsOff(line);
            break;

          case 201:


            EV_DoCeiling(line,lowerToLowest);
            break;

          case 202:


            EV_DoCeiling(line,lowerToMaxFloor);
            break;

          case 208:

            EV_SilentTeleport(line, side, thing);
            break;

          case 212:


            EV_DoPlat(line,toggleUpDn,0);
            break;


          case 154:


            EV_DoChange(line,trigChangeOnly);
            break;

          case 240:


            EV_DoChange(line,numChangeOnly);
            break;

          case 220:


            EV_DoFloor(line,lowerFloorToNearest);
            break;

          case 228:


            EV_DoElevator(line,elevateUp);
            break;

          case 232:


            EV_DoElevator(line,elevateDown);
            break;

          case 236:


            EV_DoElevator(line,elevateCurrent);
            break;

          case 244:

            EV_SilentLineTeleport(line, side, thing, false);
            break;

          case 263:
            EV_SilentLineTeleport(line, side, thing, true);
            break;

          case 265:
            if (!thing->player)
              EV_SilentLineTeleport(line, side, thing, true);
            break;

          case 267:
            if (!thing->player)
              EV_SilentLineTeleport(line, side, thing, false);
            break;

          case 269:
            if (!thing->player)
              EV_SilentTeleport(line, side, thing);
            break;


        }
      break;
  }
}

void P_ShootSpecialLine
( mobj_t*       thing,
  line_t*       line )
{

  {


    int (*linefunc)(line_t *line)=NULL;


    if ((unsigned)line->special >= GenEnd)
    {

    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return;
      if (!line->tag)
        return;

      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player)
      {
        if (!(line->special & DoorMonster))
          return;
        if (line->flags & ML_SECRET)
          return;
      }
      if (!line->tag)
        return;
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player)
        return;
      if (((line->special&TriggerType)==GunOnce) || ((line->special&TriggerType)==GunMany))
      {
        if (!P_CanUnlockGenDoor(line,thing->player))
          return;
      }
      else
        return;
      if (!line->tag)
        return;

      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player)
        if (!(line->special & LiftMonster))
          return;
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenStairs;
    }
    else if ((unsigned)line->special >= GenCrusherBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return;
      if (!line->tag)
        return;
      linefunc = EV_DoGenCrusher;
    }

    if (linefunc)
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case GunOnce:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,0);
          return;
        case GunMany:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,1);
          return;
        default:
          return;
      }
  }


  if (!thing->player)
  {
    int ok = 0;
    switch(line->special)
    {
      case 46:

        ok = 1;
        break;
    }
    if (!ok)
      return;
  }

  if (!P_CheckTag(line))
    return;

  switch(line->special)
  {
    case 24:

      if (EV_DoFloor(line,raiseFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 46:

      EV_DoDoor(line,open);
      P_ChangeSwitchTexture(line,1);
      break;

    case 47:

      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        P_ChangeSwitchTexture(line,0);
      break;




    default:
        switch (line->special)
        {
          case 197:


            if(thing->player && thing->player->health<=0)
              break;
            P_ChangeSwitchTexture(line,0);
            G_ExitLevel();
            break;

          case 198:


            if(thing->player && thing->player->health<=0)
              break;
            P_ChangeSwitchTexture(line,0);
            G_SecretExitLevel();
            break;

        }
      break;
  }
}

void P_PlayerInSpecialSector (player_t* player)
{
  sector_t*   sector;

  sector = player->mo->subsector->sector;



  if (player->mo->z != sector->floorheight)
    return;



  if (sector->special<32)
  {
    switch (sector->special)
      {
      case 5:

        if (!player->powers[pw_ironfeet])
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 10);
        break;

      case 7:

        if (!player->powers[pw_ironfeet])
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 5);
        break;

      case 16:

      case 4:

        if (!player->powers[pw_ironfeet]
            || (P_Random(pr_slimehurt)<5) )
        {
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 20);
        }
        break;

      case 9:

        player->secretcount++;
        sector->special = 0;
        break;

      case 11:

        if (!(leveltime&0x1f))
          P_DamageMobj (player->mo, NULL, NULL, 20);

        if (player->health <= 10)
          G_ExitLevel();
        break;

      default:

        break;
      };
  }
  else
  {
    switch ((sector->special&DAMAGE_MASK)>>DAMAGE_SHIFT)
    {
      case 0:
        break;
      case 1:
        if (!player->powers[pw_ironfeet])
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 5);
        break;
      case 2:
        if (!player->powers[pw_ironfeet])
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 10);
        break;
      case 3:
        if (!player->powers[pw_ironfeet]
            || (P_Random(pr_slimehurt)<5))
        {
          if (!(leveltime&0x1f))
            P_DamageMobj (player->mo, NULL, NULL, 20);
        }
        break;
    }
    if (sector->special&SECRET_MASK)
    {
      player->secretcount++;
      sector->special &= ~SECRET_MASK;
      if (sector->special<32)
        sector->special=0;
    }

  }
}

static boolean  levelTimer;
static int      levelTimeCount;
boolean         levelFragLimit;
int             levelFragLimitCount;

void P_UpdateSpecials (void)
{
  anim_t*     anim;
  int         pic;
  int         i;


  if (levelTimer == true)
  {
    levelTimeCount--;
    if (!levelTimeCount)
      G_ExitLevel();
  }

  if (levelFragLimit == true)
  {
    int k,m,fragcount,exitflag=false;
    for (k=0;k<MAXPLAYERS;k++)
    {
      if (!playeringame[k]) continue;
      fragcount = 0;
      for (m=0;m<MAXPLAYERS;m++)
      {
        if (!playeringame[m]) continue;
          fragcount += (m!=k)?  players[k].frags[m] : -players[k].frags[m];
      }
      if (fragcount >= levelFragLimitCount) exitflag = true;
      if (exitflag == true) break;
    }
    if (exitflag == true)
      G_ExitLevel();
  }


  for (anim = anims ; anim < lastanim ; anim++)
  {
    for (i=anim->basepic ; i<anim->basepic+anim->numpics ; i++)
    {
      pic = anim->basepic + ( (leveltime/anim->speed + i)%anim->numpics );
      if (anim->istexture)
        texturetranslation[i] = pic;
      else
        flattranslation[i] = pic;
    }
  }


  for (i = 0; i < MAXBUTTONS; i++)
    if (buttonlist[i].btimer)
    {
      buttonlist[i].btimer--;
      if (!buttonlist[i].btimer)
      {
        switch(buttonlist[i].where)
        {
          case top:
            sides[buttonlist[i].line->sidenum[0]].toptexture =
              buttonlist[i].btexture;
            break;

          case middle:
            sides[buttonlist[i].line->sidenum[0]].midtexture =
              buttonlist[i].btexture;
            break;

          case bottom:
            sides[buttonlist[i].line->sidenum[0]].bottomtexture =
              buttonlist[i].btexture;
            break;
        }
        {
          mobj_t *so = (mobj_t *)buttonlist[i].soundorg;
          S_StartSound(so, sfx_swtchn);
        }
        memset(&buttonlist[i],0,sizeof(button_t));
      }
    }
}

void P_SpawnSpecials (void)
{
  sector_t*   sector;
  int         i;

  levelTimer = false;
  levelFragLimit = false;
  sector = sectors;

  for (i=0 ; i<numsectors ; i++, sector++)
  {
    if (!sector->special)
      continue;

    if (sector->special&SECRET_MASK)
      totalsecret++;

    switch (sector->special&31)
    {
      case 1:

        P_SpawnLightFlash (sector);
        break;

      case 2:

        P_SpawnStrobeFlash(sector,FASTDARK,0);
        break;

      case 3:

        P_SpawnStrobeFlash(sector,SLOWDARK,0);
        break;

      case 4:

        P_SpawnStrobeFlash(sector,FASTDARK,0);
        sector->special |= 3<<DAMAGE_SHIFT;
        break;

      case 8:

        P_SpawnGlowingLight(sector);
        break;
      case 9:

        if (sector->special<32)
          totalsecret++;
        break;

      case 10:

        P_SpawnDoorCloseIn30 (sector);
        break;

      case 12:

        P_SpawnStrobeFlash (sector, SLOWDARK, 1);
        break;

      case 13:

        P_SpawnStrobeFlash (sector, FASTDARK, 1);
        break;

      case 14:

        P_SpawnDoorRaiseIn5Mins (sector, i);
        break;

      case 17:

        P_SpawnFireFlicker(sector);
        break;
    }
  }

  P_RemoveAllActiveCeilings();
  P_RemoveAllActivePlats();

  for (i = 0;i < MAXBUTTONS;i++)
    memset(&buttonlist[i],0,sizeof(button_t));

  P_InitTagLists();
  P_SpawnScrollers();
  P_SpawnFriction();
  P_SpawnPushers();

  for (i=0; i<numlines; i++)
    switch (lines[i].special)
    {
      int s, sec;

      case 242:
        sec = sides[*lines[i].sidenum].sector-sectors;
        for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
          sectors[s].heightsec = sec;
        break;

      case 213:
        sec = sides[*lines[i].sidenum].sector-sectors;
        for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
          sectors[s].floorlightsec = sec;
        break;

      case 261:
        sec = sides[*lines[i].sidenum].sector-sectors;
        for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
          sectors[s].ceilinglightsec = sec;
        break;

      case 271:
      case 272:
        for (s = -1; (s = P_FindSectorFromLineTag(lines+i,s)) >= 0;)
          sectors[s].sky = i | PL_SKYFLAT;
        break;
   }
}

void T_Scroll(scroll_t *s)
{
  fixed_t dx = s->dx, dy = s->dy;

  if (s->control != -1)
    {
      fixed_t height = sectors[s->control].floorheight +
        sectors[s->control].ceilingheight;
      fixed_t delta = height - s->last_height;
      s->last_height = height;
      dx = FixedMul(dx, delta);
      dy = FixedMul(dy, delta);
    }


  if (s->accel)
    {
      s->vdx = dx += s->vdx;
      s->vdy = dy += s->vdy;
    }

  if (!(dx | dy))
    return;

  switch (s->type)
    {
      side_t *side;
      sector_t *sec;
      fixed_t height, waterheight;
      msecnode_t *node;
      mobj_t *thing;

    case sc_side:
        side = sides + s->affectee;
        side->textureoffset += dx;
        side->rowoffset += dy;
        break;

    case sc_floor:
        sec = sectors + s->affectee;
        sec->floor_xoffs += dx;
        sec->floor_yoffs += dy;
        break;

    case sc_ceiling:
        sec = sectors + s->affectee;
        sec->ceiling_xoffs += dx;
        sec->ceiling_yoffs += dy;
        break;

    case sc_carry:

      sec = sectors + s->affectee;
      height = sec->floorheight;
      waterheight = sec->heightsec != -1 &&
        sectors[sec->heightsec].floorheight > height ?
        sectors[sec->heightsec].floorheight : INT_MIN;

      for (node = sec->touching_thinglist; node; node = node->m_snext)
        if (!((thing = node->m_thing)->flags & MF_NOCLIP) &&
            (!(thing->flags & MF_NOGRAVITY || thing->z > height) ||
             thing->z < waterheight))
          {


            thing->momx += dx;
            thing->momy += dy;
          }
      break;

    case sc_carry_ceiling:
      break;
    }
}

static void Add_Scroller(int type, fixed_t dx, fixed_t dy, int control, int affectee, int accel)
{
  scroll_t *s = Z_Malloc(sizeof *s, PU_LEVSPEC, 0);
  s->thinker.function = T_Scroll;
  s->type = type;
  s->dx = dx;
  s->dy = dy;
  s->accel = accel;
  s->vdx = s->vdy = 0;
  if ((s->control = control) != -1)
    s->last_height =
      sectors[control].floorheight + sectors[control].ceilingheight;
  s->affectee = affectee;
  P_AddThinker(&s->thinker);
}

static void Add_WallScroller(fixed_t dx, fixed_t dy, const line_t *l, int control, int accel)
{

    fixed_t x = D_abs(l->dx), y = D_abs(l->dy), d;

    if (y > x)
        d = x, x = y, y = d;

    d = FixedDiv(x, finesine[(tantoangle[FixedDiv(y,x) >> DBITS] + ANG90) >> ANGLETOFINESHIFT]);
    x = (fixed_t)(((int_64_t)dy * -(int_64_t)l->dy - (int_64_t)dx * (int_64_t)l->dx) / (int_64_t)d);
    y = (fixed_t)(((int_64_t)dy * (int_64_t)l->dx - (int_64_t)dx * (int_64_t)l->dy) / (int_64_t)d);

    Add_Scroller(sc_side, x, y, control, *l->sidenum, accel);

}

#define SCROLL_SHIFT 5
#define CARRYFACTOR ((fixed_t)(FRACUNIT*.09375))

static void P_SpawnScrollers(void)
{
  int i;
  line_t *l = lines;

  for (i=0;i<numlines;i++,l++)
    {
      fixed_t dx = l->dx >> SCROLL_SHIFT;
      fixed_t dy = l->dy >> SCROLL_SHIFT;
      int control = -1, accel = 0;
      int special = l->special;

      if (special >= 245 && special <= 249)
        {
          special += 250-245;
          control = sides[*l->sidenum].sector - sectors;
        }
      else
        if (special >= 214 && special <= 218)
          {
            accel = 1;
            special += 250-214;
            control = sides[*l->sidenum].sector - sectors;
          }

      switch (special)
        {
          register int s;

        case 250:
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_ceiling, -dx, dy, control, s, accel);
          break;

        case 251:
        case 253:
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_floor, -dx, dy, control, s, accel);
          if (special != 253)
            break;

        case 252:
          dx = FixedMul(dx,CARRYFACTOR);
          dy = FixedMul(dy,CARRYFACTOR);
          for (s=-1; (s = P_FindSectorFromLineTag(l,s)) >= 0;)
            Add_Scroller(sc_carry, dx, dy, control, s, accel);
          break;



        case 254:
          for (s=-1; (s = P_FindLineFromLineTag(l,s)) >= 0;)
            if (s != i)
              Add_WallScroller(dx, dy, lines+s, control, accel);
          break;

        case 255:
          s = lines[i].sidenum[0];
          Add_Scroller(sc_side, -sides[s].textureoffset,
                       sides[s].rowoffset, -1, s, accel);
          break;

        case 48:
          Add_Scroller(sc_side,  FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
          break;

        case 85:
          Add_Scroller(sc_side, -FRACUNIT, 0, -1, lines[i].sidenum[0], accel);
          break;
        }
    }
}

void T_Friction(friction_t *f)
    {
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;

    sec = sectors + f->affectee;

    if (!(sec->special & FRICTION_MASK))
        return;

    node = sec->touching_thinglist;
    while (node)
        {
        thing = node->m_thing;
        if (thing->player &&
            !(thing->flags & (MF_NOGRAVITY | MF_NOCLIP)) &&
            thing->z <= sec->floorheight)
            {
            if ((thing->friction == ORIG_FRICTION) ||
              (f->friction < thing->friction))
                {
                thing->friction   = f->friction;
                thing->movefactor = f->movefactor;
                }
            }
        node = node->m_snext;
        }
    }

static void P_SpawnFriction(void)
{
  int i;
  line_t *l = lines;


  for (i = 0; i < numsectors; i++)
    {
      sectors[i].friction = ORIG_FRICTION;
      sectors[i].movefactor = ORIG_FRICTION_FACTOR;
    }

  for (i = 0 ; i < numlines ; i++,l++)
    if (l->special == 223)
      {
        int length = P_AproxDistance(l->dx,l->dy)>>FRACBITS;
        int friction = (0x1EB8*length)/0x80 + 0xD000;
        int movefactor, s;

        if (friction > ORIG_FRICTION)
          movefactor = ((0x10092 - friction)*(0x70))/0x158;
        else
          movefactor = ((friction - 0xDB34)*(0xA))/0x80;

            if (friction > FRACUNIT)
              friction = FRACUNIT;
            if (friction < 0)
              friction = 0;
            if (movefactor < 32)
              movefactor = 32;

        for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
          {

            sectors[s].friction = friction;
            sectors[s].movefactor = movefactor;
          }
      }
}

#define PUSH_FACTOR 7

static void Add_Pusher(int type, int x_mag, int y_mag, mobj_t* source, int affectee)
    {
    pusher_t *p = Z_Malloc(sizeof *p, PU_LEVSPEC, 0);

    p->thinker.function = T_Pusher;
    p->source = source;
    p->type = type;
    p->x_mag = x_mag>>FRACBITS;
    p->y_mag = y_mag>>FRACBITS;
    p->magnitude = P_AproxDistance(p->x_mag,p->y_mag);
    if (source)
        {
        p->radius = (p->magnitude)<<(FRACBITS+1);
        p->x = p->source->x;
        p->y = p->source->y;
        }
    p->affectee = affectee;
    P_AddThinker(&p->thinker);
    }

pusher_t* tmpusher;

static boolean PIT_PushThing(mobj_t* thing)
{
  if ((sentient(thing) || thing->flags & MF_SHOOTABLE) && !(thing->flags & MF_NOCLIP))
    {
      angle_t pushangle;
      fixed_t speed;
      fixed_t sx = tmpusher->x;
      fixed_t sy = tmpusher->y;

      speed = (tmpusher->magnitude -
               ((P_AproxDistance(thing->x - sx,thing->y - sy)
                 >>FRACBITS)>>1))<<(FRACBITS-PUSH_FACTOR-1);


      if (speed > 0)
        {
          int x = (thing->x-sx) >> FRACBITS;
          int y = (thing->y-sy) >> FRACBITS;
          speed = (int)(((uint_64_t) tmpusher->magnitude << 23) / (x*x+y*y+1));
        }




      if (speed > 0 && P_CheckSight(thing,tmpusher->source))
        {
          pushangle = R_PointToAngle2(thing->x,thing->y,sx,sy);
          if (tmpusher->source->type == MT_PUSH)
            pushangle += ANG180;
          pushangle >>= ANGLETOFINESHIFT;
          thing->momx += FixedMul(speed,finecosine[pushangle]);
          thing->momy += FixedMul(speed,finesine[pushangle]);
        }
    }
  return true;
}

void T_Pusher(pusher_t *p)
    {
    sector_t *sec;
    mobj_t   *thing;
    msecnode_t* node;
    int xspeed,yspeed;
    int xl,xh,yl,yh,bx,by;
    int radius;
    int ht = 0;

    sec = sectors + p->affectee;

    if (!(sec->special & PUSH_MASK))
        return;

    if (p->type == p_push)
        {

        tmpusher = p;
        radius = p->radius;
        tmbbox[BOXTOP]    = p->y + radius;
        tmbbox[BOXBOTTOM] = p->y - radius;
        tmbbox[BOXRIGHT]  = p->x + radius;
        tmbbox[BOXLEFT]   = p->x - radius;

        xl = (tmbbox[BOXLEFT] - bmaporgx - MAXRADIUS)>>MAPBLOCKSHIFT;
        xh = (tmbbox[BOXRIGHT] - bmaporgx + MAXRADIUS)>>MAPBLOCKSHIFT;
        yl = (tmbbox[BOXBOTTOM] - bmaporgy - MAXRADIUS)>>MAPBLOCKSHIFT;
        yh = (tmbbox[BOXTOP] - bmaporgy + MAXRADIUS)>>MAPBLOCKSHIFT;
        for (bx=xl ; bx<=xh ; bx++)
            for (by=yl ; by<=yh ; by++)
                P_BlockThingsIterator(bx,by,PIT_PushThing);
        return;
        }



    if (sec->heightsec != -1)
        ht = sectors[sec->heightsec].floorheight;
    node = sec->touching_thinglist;
    for ( ; node ; node = node->m_snext)
        {
        thing = node->m_thing;
        if (!thing->player || (thing->flags & (MF_NOGRAVITY | MF_NOCLIP)))
            continue;
        if (p->type == p_wind)
            {
            if (sec->heightsec == -1)
                if (thing->z > thing->floorz)
                    {
                    xspeed = p->x_mag;
                    yspeed = p->y_mag;
                    }
                else
                    {
                    xspeed = (p->x_mag)>>1;
                    yspeed = (p->y_mag)>>1;
                    }
            else
                {
                if (thing->z > ht)
                    {
                    xspeed = p->x_mag;
                    yspeed = p->y_mag;
                    }
                else if (thing->player->viewz < ht)
                    xspeed = yspeed = 0;
                else
                    {
                    xspeed = (p->x_mag)>>1;
                    yspeed = (p->y_mag)>>1;
                    }
                }
            }
        else
            {
            if (sec->heightsec == -1)
                if (thing->z > sec->floorheight)
                    xspeed = yspeed = 0;
                else
                    {
                    xspeed = p->x_mag;
                    yspeed = p->y_mag;
                    }
            else
                if (thing->z > ht)
                    xspeed = yspeed = 0;
                else
                    {
                    xspeed = p->x_mag;
                    yspeed = p->y_mag;
                    }
            }
        thing->momx += xspeed<<(FRACBITS-PUSH_FACTOR);
        thing->momy += yspeed<<(FRACBITS-PUSH_FACTOR);
        }
    }

mobj_t* P_GetPushThing(int s)
    {
    mobj_t* thing;
    sector_t* sec;

    sec = sectors + s;
    thing = sec->thinglist;
    while (thing)
        {
        switch(thing->type)
            {
          case MT_PUSH:
          case MT_PULL:
            return thing;
          default:
            break;
            }
        thing = thing->snext;
        }
    return NULL;
    }

static void P_SpawnPushers(void)
    {
    int i;
    line_t *l = lines;
    register int s;
    mobj_t* thing;

    for (i = 0 ; i < numlines ; i++,l++)
        switch(l->special)
            {
          case 224:
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_wind,l->dx,l->dy,NULL,s);
            break;
          case 225:
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                Add_Pusher(p_current,l->dx,l->dy,NULL,s);
            break;
          case 226:
            for (s = -1; (s = P_FindSectorFromLineTag(l,s)) >= 0 ; )
                {
                thing = P_GetPushThing(s);
                if (thing)
                    Add_Pusher(p_push,l->dx,l->dy,thing,s);
                }
            break;
            }
    }

