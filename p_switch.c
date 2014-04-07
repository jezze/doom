#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "p_spec.h"
#include "g_game.h"
#include "s_sound.h"
#include "i_system.h"

static int *switchlist;
static int max_numswitches;
static int numswitches;
button_t buttonlist[MAXBUTTONS];

switchlist_t alphSwitchList[] = {
    {"SW1BRCOM",	"SW2BRCOM",	1},
    {"SW1BRN1",	"SW2BRN1",	1},
    {"SW1BRN2",	"SW2BRN2",	1},
    {"SW1BRNGN",	"SW2BRNGN",	1},
    {"SW1BROWN",	"SW2BROWN",	1},
    {"SW1COMM",	"SW2COMM",	1},
    {"SW1COMP",	"SW2COMP",	1},
    {"SW1DIRT",	"SW2DIRT",	1},
    {"SW1EXIT",	"SW2EXIT",	1},
    {"SW1GRAY",	"SW2GRAY",	1},
    {"SW1GRAY1",	"SW2GRAY1",	1},
    {"SW1METAL",	"SW2METAL",	1},
    {"SW1PIPE",	"SW2PIPE",	1},
    {"SW1SLAD",	"SW2SLAD",	1},
    {"SW1STARG",	"SW2STARG",	1},
    {"SW1STON1",	"SW2STON1",	1},
    {"SW1STON2",	"SW2STON2",	1},
    {"SW1STONE",	"SW2STONE",	1},
    {"SW1STRTN",	"SW2STRTN",	1},
    {"SW1BLUE",	"SW2BLUE",	2},
    {"SW1CMT",		"SW2CMT",	2},
    {"SW1GARG",	"SW2GARG",	2},
    {"SW1STON1",	"SW2STON1",	1},
    {"SW1STON2",	"SW2STON2",	1},
    {"SW1STONE",	"SW2STONE",	1},
    {"SW1STRTN",	"SW2STRTN",	1},
    {"SW1BLUE",	"SW2BLUE",	2},
    {"SW1CMT",		"SW2CMT",	2},
    {"SW1GARG",	"SW2GARG",	2},
    {"SW1GSTON",	"SW2GSTON",	2},
    {"SW1HOT",		"SW2HOT",	2},
    {"SW1LION",	"SW2LION",	2},
    {"SW1SATYR",	"SW2SATYR",	2},
    {"SW1SKIN",	"SW2SKIN",	2},
    {"SW1VINE",	"SW2VINE",	2},
    {"SW1WOOD",	"SW2WOOD",	2},
    {"SW1PANEL",	"SW2PANEL",	3},
    {"SW1ROCK",	"SW2ROCK",	3},
    {"SW1MET2",	"SW2MET2",	3},
    {"SW1WDMET",	"SW2WDMET",	3},
    {"SW1BRIK",	"SW2BRIK",	3},
    {"SW1MOD1",	"SW2MOD1",	3},
    {"SW1ZIM",		"SW2ZIM",	3},
    {"SW1STON6",	"SW2STON6",	3},
    {"SW1TEK",		"SW2TEK",	3},
    {"SW1MARB",	"SW2MARB",	3},
    {"SW1SKULL",	"SW2SKULL",	3},
    {"\0",		"\0",		0}
};

void P_InitSwitchList(void)
{

    int i, index = 0;
    int episode = (gamemode == registered || gamemode==retail) ? 2 : gamemode == commercial ? 3 : 1;

    for (i = 0; ; i++)
    {

        if (index + 1 >= max_numswitches)
            switchlist = realloc(switchlist, sizeof *switchlist * (max_numswitches = max_numswitches ? max_numswitches * 2 : 8));

        if (alphSwitchList[i].episode <= episode)
        {

            int texture1, texture2;

            if (!alphSwitchList[i].episode)
                break;

            texture1 = R_CheckTextureNumForName(alphSwitchList[i].name1);

            if (texture1 == -1)
                I_Error("P_InitSwitchList: Unknown texture %s", alphSwitchList[i].name1);

            texture2 = R_CheckTextureNumForName(alphSwitchList[i].name2);
            
            if (texture2 == -1)
                I_Error("P_InitSwitchList: Unknown texture %s", alphSwitchList[i].name2);

            if (texture1 != -1 && texture2 != -1)
            {
            
                switchlist[index++] = texture1;
                switchlist[index++] = texture2;

            }

        }

    }

    numswitches = index / 2;
    switchlist[index] = -1;

}

static void P_StartButton(line_t *line, bwhere_e w, int texture, int time)
{

    int i;

    for (i = 0; i < MAXBUTTONS; i++)
    {

        if (buttonlist[i].btimer && buttonlist[i].line == line)
            return;

    }

    for (i = 0; i < MAXBUTTONS; i++)
    {

        if (!buttonlist[i].btimer)
        {

            buttonlist[i].line = line;
            buttonlist[i].where = w;
            buttonlist[i].btexture = texture;
            buttonlist[i].btimer = time;
            buttonlist[i].soundorg = (mobj_t *)&line->soundorg;

            return;

        }

    }

    I_Error("P_StartButton: no button slots left!");

}

void P_ChangeSwitchTexture(line_t *line, int useAgain)
{

  mobj_t  *soundorg;
  int     i, sound;
  short   *texture, *ttop, *tmid, *tbot;
  bwhere_e position;

  ttop = &sides[line->sidenum[0]].toptexture;
  tmid = &sides[line->sidenum[0]].midtexture;
  tbot = &sides[line->sidenum[0]].bottomtexture;

  sound = sfx_swtchn;
  soundorg = (mobj_t *)&line->soundorg;

  if (!useAgain)
    line->special = 0;

  texture = NULL; position = 0;
  for (i = 0;i < numswitches*2;i++)
  {

    if (switchlist[i] == *ttop) {
      texture = ttop; position = top; break;
    } else if (switchlist[i] == *tmid) {
      texture = tmid; position = middle; break;
    } else if (switchlist[i] == *tbot) {
      texture = tbot; position = bottom; break;
    }
  }
  if (texture == NULL)
    return;
  *texture = switchlist[i^1];

  S_StartSound(soundorg, sound);

  if (useAgain)
    P_StartButton(line, position, switchlist[i], BUTTONTIME);
}

boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side)
{

    if (side)
        return false;

    {

    int (*linefunc)(line_t *line)=NULL;


    if ((unsigned)line->special >= GenEnd)
    {

    }
    else if ((unsigned)line->special >= GenFloorBase)
    {
      if (!thing->player)
        if ((line->special & FloorChange) || !(line->special & FloorModel))
          return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenFloor;
    }
    else if ((unsigned)line->special >= GenCeilingBase)
    {
      if (!thing->player)
        if ((line->special & CeilingChange) || !(line->special & CeilingModel))
          return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenCeiling;
    }
    else if ((unsigned)line->special >= GenDoorBase)
    {
      if (!thing->player)
      {
        if (!(line->special & DoorMonster))
          return false;
        if (line->flags & ML_SECRET)
          return false;
      }
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenDoor;
    }
    else if ((unsigned)line->special >= GenLockedBase)
    {
      if (!thing->player)
        return false;
      if (!P_CanUnlockGenDoor(line,thing->player))
        return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;

      linefunc = EV_DoGenLockedDoor;
    }
    else if ((unsigned)line->special >= GenLiftBase)
    {
      if (!thing->player)
        if (!(line->special & LiftMonster))
          return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenLift;
    }
    else if ((unsigned)line->special >= GenStairsBase)
    {
      if (!thing->player)
        if (!(line->special & StairMonster))
          return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenStairs;
    }
    else if ((unsigned)line->special >= GenCrusherBase)
    {
      if (!thing->player)
        if (!(line->special & CrusherMonster))
          return false;
      if (!line->tag && ((line->special&6)!=6))
        return false;
      linefunc = EV_DoGenCrusher;
    }

    if (linefunc)
      switch((line->special & TriggerType) >> TriggerTypeShift)
      {
        case PushOnce:
          if (!side)
            if (linefunc(line))
              line->special = 0;
          return true;
        case PushMany:
          if (!side)
            linefunc(line);
          return true;
        case SwitchOnce:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,0);
          return true;
        case SwitchMany:
          if (linefunc(line))
            P_ChangeSwitchTexture(line,1);
          return true;
        default:
          return false;
      }
  }


  if (!thing->player)
  {

    if (line->flags & ML_SECRET)
      return false;

    switch(line->special)
    {
      case 1:
      case 32:
      case 33:
      case 34:

      case 195:
      case 174:
      case 210:
      case 209:
        break;

      default:
        return false;
        break;
    }
  }

  if (!P_CheckTag(line))
    return false;


  switch (line->special)
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
      EV_VerticalDoor (line, thing);
      break;


    case 7:

      if (EV_BuildStairs(line,build8))
        P_ChangeSwitchTexture(line,0);
      break;

    case 9:

      if (EV_DoDonut(line))
        P_ChangeSwitchTexture(line,0);
      break;

    case 11:
      if (thing->player && thing->player->health <= 0)
      {
        S_StartSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,0);
      G_ExitLevel ();
      break;

    case 14:

      if (EV_DoPlat(line,raiseAndChange,32))
        P_ChangeSwitchTexture(line,0);
      break;

    case 15:

      if (EV_DoPlat(line,raiseAndChange,24))
        P_ChangeSwitchTexture(line,0);
      break;

    case 18:

      if (EV_DoFloor(line, raiseFloorToNearest))
        P_ChangeSwitchTexture(line,0);
      break;

    case 20:

      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 21:

      if (EV_DoPlat(line,downWaitUpStay,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 23:

      if (EV_DoFloor(line,lowerFloorToLowest))
        P_ChangeSwitchTexture(line,0);
      break;

    case 29:

      if (EV_DoDoor(line,normal))
        P_ChangeSwitchTexture(line,0);
      break;

    case 41:

      if (EV_DoCeiling(line,lowerToFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 71:

      if (EV_DoFloor(line,turboLower))
        P_ChangeSwitchTexture(line,0);
      break;

    case 49:

      if (EV_DoCeiling(line,crushAndRaise))
        P_ChangeSwitchTexture(line,0);
      break;

    case 50:

      if (EV_DoDoor(line,close))
        P_ChangeSwitchTexture(line,0);
      break;

    case 51:
      if (thing->player && thing->player->health <= 0)
      {
        S_StartSound(thing, sfx_noway);
        return false;
      }

      P_ChangeSwitchTexture(line,0);
      G_SecretExitLevel ();
      break;

    case 55:

      if (EV_DoFloor(line,raiseFloorCrush))
        P_ChangeSwitchTexture(line,0);
      break;

    case 101:

      if (EV_DoFloor(line,raiseFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 102:

      if (EV_DoFloor(line,lowerFloor))
        P_ChangeSwitchTexture(line,0);
      break;

    case 103:

      if (EV_DoDoor(line,open))
        P_ChangeSwitchTexture(line,0);
      break;

    case 111:

      if (EV_DoDoor (line,blazeRaise))
        P_ChangeSwitchTexture(line,0);
      break;

    case 112:

      if (EV_DoDoor (line,blazeOpen))
        P_ChangeSwitchTexture(line,0);
      break;

    case 113:

      if (EV_DoDoor (line,blazeClose))
        P_ChangeSwitchTexture(line,0);
      break;

    case 122:

      if (EV_DoPlat(line,blazeDWUS,0))
        P_ChangeSwitchTexture(line,0);
      break;

    case 127:

      if (EV_BuildStairs(line,turbo16))
        P_ChangeSwitchTexture(line,0);
      break;

    case 131:

      if (EV_DoFloor(line,raiseFloorTurbo))
        P_ChangeSwitchTexture(line,0);
      break;

    case 133:

    case 135:

    case 137:

      if (EV_DoLockedDoor (line,blazeOpen,thing))
        P_ChangeSwitchTexture(line,0);
      break;

    case 140:

      if (EV_DoFloor(line,raiseFloor512))
        P_ChangeSwitchTexture(line,0);
      break;

    default:
        switch (line->special)
        {

          case 158:


            if (EV_DoFloor(line,raiseToTexture))
              P_ChangeSwitchTexture(line,0);
            break;

          case 159:


            if (EV_DoFloor(line,lowerAndChange))
              P_ChangeSwitchTexture(line,0);
            break;

          case 160:


            if (EV_DoFloor(line,raiseFloor24AndChange))
              P_ChangeSwitchTexture(line,0);
            break;

          case 161:


            if (EV_DoFloor(line,raiseFloor24))
              P_ChangeSwitchTexture(line,0);
            break;

          case 162:


            if (EV_DoPlat(line,perpetualRaise,0))
              P_ChangeSwitchTexture(line,0);
            break;

          case 163:


            EV_StopPlat(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 164:


            if (EV_DoCeiling(line,fastCrushAndRaise))
              P_ChangeSwitchTexture(line,0);
            break;

          case 165:


            if (EV_DoCeiling(line,silentCrushAndRaise))
              P_ChangeSwitchTexture(line,0);
            break;

          case 166:


            if (EV_DoCeiling(line, raiseToHighest) ||
                EV_DoFloor(line, lowerFloorToLowest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 167:


            if (EV_DoCeiling(line, lowerAndCrush))
              P_ChangeSwitchTexture(line,0);
            break;

          case 168:


            if (EV_CeilingCrushStop(line))
              P_ChangeSwitchTexture(line,0);
            break;

          case 169:


            EV_LightTurnOn(line,0);
            P_ChangeSwitchTexture(line,0);
            break;

          case 170:


            EV_LightTurnOn(line,35);
            P_ChangeSwitchTexture(line,0);
            break;

          case 171:


            EV_LightTurnOn(line,255);
            P_ChangeSwitchTexture(line,0);
            break;

          case 172:


            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 173:


            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line,0);
            break;

          case 174:


            if (EV_Teleport(line,side,thing))
              P_ChangeSwitchTexture(line,0);
            break;

          case 175:


            if (EV_DoDoor(line,close30ThenOpen))
              P_ChangeSwitchTexture(line,0);
            break;

          case 189:


            if (EV_DoChange(line,trigChangeOnly))
              P_ChangeSwitchTexture(line,0);
            break;

          case 203:


            if (EV_DoCeiling(line,lowerToLowest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 204:


            if (EV_DoCeiling(line,lowerToMaxFloor))
              P_ChangeSwitchTexture(line,0);
            break;

          case 209:


            if (EV_SilentTeleport(line, side, thing))
              P_ChangeSwitchTexture(line,0);
            break;

          case 241:


            if (EV_DoChange(line,numChangeOnly))
              P_ChangeSwitchTexture(line,0);
            break;

          case 221:


            if (EV_DoFloor(line,lowerFloorToNearest))
              P_ChangeSwitchTexture(line,0);
            break;

          case 229:


            if (EV_DoElevator(line,elevateUp))
              P_ChangeSwitchTexture(line,0);
            break;

          case 233:


            if (EV_DoElevator(line,elevateDown))
              P_ChangeSwitchTexture(line,0);
            break;

          case 237:


            if (EV_DoElevator(line,elevateCurrent))
              P_ChangeSwitchTexture(line,0);
            break;







          case 78:


            if (EV_DoChange(line,numChangeOnly))
              P_ChangeSwitchTexture(line,1);
            break;

          case 176:


            if (EV_DoFloor(line,raiseToTexture))
              P_ChangeSwitchTexture(line,1);
            break;

          case 177:


            if (EV_DoFloor(line,lowerAndChange))
              P_ChangeSwitchTexture(line,1);
            break;

          case 178:


            if (EV_DoFloor(line,raiseFloor512))
              P_ChangeSwitchTexture(line,1);
            break;

          case 179:


            if (EV_DoFloor(line,raiseFloor24AndChange))
              P_ChangeSwitchTexture(line,1);
            break;

          case 180:


            if (EV_DoFloor(line,raiseFloor24))
              P_ChangeSwitchTexture(line,1);
            break;

          case 181:



            EV_DoPlat(line,perpetualRaise,0);
            P_ChangeSwitchTexture(line,1);
            break;

          case 182:


            EV_StopPlat(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 183:


            if (EV_DoCeiling(line,fastCrushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 184:


            if (EV_DoCeiling(line,crushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 185:


            if (EV_DoCeiling(line,silentCrushAndRaise))
              P_ChangeSwitchTexture(line,1);
            break;

          case 186:


            if (EV_DoCeiling(line, raiseToHighest) ||
                EV_DoFloor(line, lowerFloorToLowest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 187:


            if (EV_DoCeiling(line, lowerAndCrush))
              P_ChangeSwitchTexture(line,1);
            break;

          case 188:


            if (EV_CeilingCrushStop(line))
              P_ChangeSwitchTexture(line,1);
            break;

          case 190:


            if (EV_DoChange(line,trigChangeOnly))
              P_ChangeSwitchTexture(line,1);
            break;

          case 191:


            if (EV_DoDonut(line))
              P_ChangeSwitchTexture(line,1);
            break;

          case 192:


            EV_LightTurnOn(line,0);
            P_ChangeSwitchTexture(line,1);
            break;

          case 193:


            EV_StartLightStrobing(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 194:


            EV_TurnTagLightsOff(line);
            P_ChangeSwitchTexture(line,1);
            break;

          case 195:


            if (EV_Teleport(line,side,thing))
              P_ChangeSwitchTexture(line,1);
            break;

          case 196:


            if (EV_DoDoor(line,close30ThenOpen))
              P_ChangeSwitchTexture(line,1);
            break;

          case 205:


            if (EV_DoCeiling(line,lowerToLowest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 206:


            if (EV_DoCeiling(line,lowerToMaxFloor))
              P_ChangeSwitchTexture(line,1);
            break;

          case 210:


            if (EV_SilentTeleport(line, side, thing))
              P_ChangeSwitchTexture(line,1);
            break;

          case 211:


            if (EV_DoPlat(line,toggleUpDn,0))
              P_ChangeSwitchTexture(line,1);
            break;

          case 222:


            if (EV_DoFloor(line,lowerFloorToNearest))
              P_ChangeSwitchTexture(line,1);
            break;

          case 230:


            if (EV_DoElevator(line,elevateUp))
              P_ChangeSwitchTexture(line,1);
            break;

          case 234:


            if (EV_DoElevator(line,elevateDown))
              P_ChangeSwitchTexture(line,1);
            break;

          case 238:


            if (EV_DoElevator(line,elevateCurrent))
              P_ChangeSwitchTexture(line,1);
            break;

          case 258:


            if (EV_BuildStairs(line,build8))
              P_ChangeSwitchTexture(line,1);
            break;

          case 259:


            if (EV_BuildStairs(line,turbo16))
              P_ChangeSwitchTexture(line,1);
            break;



        }
      break;


    case 42:

      if (EV_DoDoor(line,close))
        P_ChangeSwitchTexture(line,1);
      break;

    case 43:

      if (EV_DoCeiling(line,lowerToFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 45:

      if (EV_DoFloor(line,lowerFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 60:

      if (EV_DoFloor(line,lowerFloorToLowest))
        P_ChangeSwitchTexture(line,1);
      break;

    case 61:

      if (EV_DoDoor(line,open))
        P_ChangeSwitchTexture(line,1);
      break;

    case 62:

      if (EV_DoPlat(line,downWaitUpStay,1))
        P_ChangeSwitchTexture(line,1);
      break;

    case 63:

      if (EV_DoDoor(line,normal))
        P_ChangeSwitchTexture(line,1);
      break;

    case 64:

      if (EV_DoFloor(line,raiseFloor))
        P_ChangeSwitchTexture(line,1);
      break;

    case 66:

      if (EV_DoPlat(line,raiseAndChange,24))
        P_ChangeSwitchTexture(line,1);
      break;

    case 67:

      if (EV_DoPlat(line,raiseAndChange,32))
        P_ChangeSwitchTexture(line,1);
      break;

    case 65:

      if (EV_DoFloor(line,raiseFloorCrush))
        P_ChangeSwitchTexture(line,1);
      break;

    case 68:

      if (EV_DoPlat(line,raiseToNearestAndChange,0))
        P_ChangeSwitchTexture(line,1);
      break;

    case 69:

      if (EV_DoFloor(line, raiseFloorToNearest))
        P_ChangeSwitchTexture(line,1);
      break;

    case 70:

      if (EV_DoFloor(line,turboLower))
        P_ChangeSwitchTexture(line,1);
      break;

    case 114:

      if (EV_DoDoor (line,blazeRaise))
        P_ChangeSwitchTexture(line,1);
      break;

    case 115:

      if (EV_DoDoor (line,blazeOpen))
        P_ChangeSwitchTexture(line,1);
      break;

    case 116:

      if (EV_DoDoor (line,blazeClose))
        P_ChangeSwitchTexture(line,1);
      break;

    case 123:

      if (EV_DoPlat(line,blazeDWUS,0))
        P_ChangeSwitchTexture(line,1);
      break;

    case 132:

      if (EV_DoFloor(line,raiseFloorTurbo))
        P_ChangeSwitchTexture(line,1);
      break;

    case 99:

    case 134:

    case 136:

      if (EV_DoLockedDoor (line,blazeOpen,thing))
        P_ChangeSwitchTexture(line,1);
      break;

    case 138:

      EV_LightTurnOn(line,255);
      P_ChangeSwitchTexture(line,1);
      break;

    case 139:

      EV_LightTurnOn(line,35);
      P_ChangeSwitchTexture(line,1);
      break;
  }
  return true;
}
