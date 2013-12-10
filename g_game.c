#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <fcntl.h>
#include "doomstat.h"
#include "d_client.h"
#include "f_finale.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_setup.h"
#include "p_saveg.h"
#include "p_tick.h"
#include "p_map.h"
#include "d_main.h"
#include "d_englsh.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "am_map.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "p_map.h"
#include "s_sound.h"
#include "sounds.h"
#include "r_data.h"
#include "r_sky.h"
#include "p_inter.h"
#include "g_game.h"
#include "lprintf.h"
#include "i_main.h"
#include "i_system.h"
#include "r_demo.h"
#include "r_fps.h"
#include "z_zone.h"

#define SAVEGAMESIZE  0x20000
#define SAVESTRINGSIZE  24

static size_t   savegamesize = SAVEGAMESIZE;
static const byte *demobuffer;   /* cph - only used for playback */
static int demolength;
static FILE    *demofp; /* cph - record straight to file */
static const byte *demo_p;
static short    consistancy[MAXPLAYERS][BACKUPTICS];

gameaction_t    gameaction;
gamestate_t     gamestate;
skill_t         gameskill;
boolean         respawnmonsters;
int             gameepisode;
int             gamemap;
boolean         paused;

static boolean command_loadgame = false;

boolean         usergame;
boolean         timingdemo;
boolean         fastdemo;
int             starttime;
boolean         playeringame[MAXPLAYERS];
player_t        players[MAXPLAYERS];
int             consoleplayer;
int             displayplayer;
int             gametic;
int             basetic;       /* killough 9/29/98: for demo sync */
int             totalkills, totallive, totalitems, totalsecret;
boolean         demorecording;
boolean         demoplayback;
int             demover;
boolean         singledemo;
wbstartstruct_t wminfo;
boolean         haswolflevels = false;
static byte     *savebuffer;
int             autorun = false;
int             totalleveltimes;
int        longtics;
int     key_right;
int     key_left;
int     key_up;
int     key_down;
int     key_menu_right;
int     key_menu_left;
int     key_menu_up;
int     key_menu_down;
int     key_menu_backspace;
int     key_menu_escape;
int     key_menu_enter;
int     key_strafeleft;
int     key_straferight;
int     key_fire;
int     key_use;
int     key_strafe;
int     key_speed;
int     key_escape = KEYD_ESCAPE;
int     key_savegame;
int     key_loadgame;
int     key_autorun;
int     key_reverse;
int     key_zoomin;
int     key_zoomout;
int     key_chat;
int     key_backspace;
int     key_enter;
int     key_map_right;
int     key_map_left;
int     key_map_up;
int     key_map_down;
int     key_map_zoomin;
int     key_map_zoomout;
int     key_map;
int     key_map_gobig;
int     key_map_follow;
int     key_map_mark;
int     key_map_clear;
int     key_map_grid;
int     key_map_overlay;
int     key_map_rotate;
int     key_help = KEYD_F1;
int     key_soundvolume;
int     key_hud;
int     key_quicksave;
int     key_endgame;
int     key_messages;
int     key_quickload;
int     key_quit;
int     key_gamma;
int     key_spy;
int     key_pause;
int     key_setup;
int     destination_keys[MAXPLAYERS];
int     key_weapontoggle;
int     key_weapon1;
int     key_weapon2;
int     key_weapon3;
int     key_weapon4;
int     key_weapon5;
int     key_weapon6;
int     key_weapon7;
int     key_weapon8;
int     key_weapon9;

int     key_screenshot;
int     mousebfire;
int     mousebstrafe;
int     mousebforward;

#define MAXPLMOVE   (forwardmove[1])
#define TURBOTHRESHOLD  0x32
#define SLOWTURNTICS  6
#define QUICKREVERSE (short)32768
#define NUMKEYS   512

fixed_t forwardmove[2] = {0x19, 0x32};
fixed_t sidemove[2]    = {0x18, 0x28};
fixed_t angleturn[3]   = {640, 1280, 320};

static boolean gamekeydown[NUMKEYS];
static int     turnheld;
static boolean mousearray[4];
static boolean *mousebuttons = &mousearray[1];
static int   mousex;
static int   mousey;
static int   dclicktime;
static int   dclickstate;
static int   dclicks;
static int   dclicktime2;
static int   dclickstate2;
static int   dclicks2;
static buttoncode_t special_event;
static byte  savegameslot;
char         savedescription[SAVEDESCLEN];
int defaultskill;
int    bodyqueslot, bodyquesize;
mobj_t **bodyque = 0;

static void G_DoSaveGame (boolean menu);
static const byte* G_ReadDemoHeader(const byte* demo_p, size_t size, boolean failonerror);

static inline signed char fudgef(signed char b)
{
  static int c;
  if (!b || !demo_compatibility || longtics) return b;
  if (++c & 0x1f) return b;
  b |= 1; if (b>2) b-=2;
  return b;
}

static inline signed short fudgea(signed short b)
{
  if (!b || !demo_compatibility || !longtics) return b;
  b |= 1; if (b>2) b-=2;
  return b;
}

void G_BuildTiccmd(ticcmd_t* cmd)
{
  boolean strafe;
  boolean bstrafe;
  int speed;
  int tspeed;
  int forward;
  int side;
  int newweapon;

  memset(cmd,0,sizeof*cmd);
  cmd->consistancy = consistancy[consoleplayer][maketic%BACKUPTICS];
  strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe];
  speed = (gamekeydown[key_speed] ? !autorun : autorun);
  forward = side = 0;

  if (gamekeydown[key_right] || gamekeydown[key_left])
    turnheld += ticdup;
  else
    turnheld = 0;

  if (turnheld < SLOWTURNTICS)
    tspeed = 2;
  else
    tspeed = speed;

  if (gamekeydown[key_reverse])
    {
      cmd->angleturn += QUICKREVERSE;
      gamekeydown[key_reverse] = false;
    }

  if (strafe)
    {
      if (gamekeydown[key_right])
        side += sidemove[speed];
      if (gamekeydown[key_left])
        side -= sidemove[speed];
    }
  else
    {
      if (gamekeydown[key_right])
        cmd->angleturn -= angleturn[tspeed];
      if (gamekeydown[key_left])
        cmd->angleturn += angleturn[tspeed];
    }

  if (gamekeydown[key_up])
    forward += forwardmove[speed];
  if (gamekeydown[key_down])
    forward -= forwardmove[speed];
  if (gamekeydown[key_straferight])
    side += sidemove[speed];
  if (gamekeydown[key_strafeleft])
    side -= sidemove[speed];


  cmd->chatchar = HU_dequeueChatChar();

  if (gamekeydown[key_fire] || mousebuttons[mousebfire])
    cmd->buttons |= BT_ATTACK;

  if (gamekeydown[key_use])
    {
      cmd->buttons |= BT_USE;

      dclicks = 0;
    }

  if ((!demo_compatibility && players[consoleplayer].attackdown &&
       !P_CheckAmmo(&players[consoleplayer])) || gamekeydown[key_weapontoggle])
    newweapon = P_SwitchWeapon(&players[consoleplayer]);
  else
    {
      newweapon =
        gamekeydown[key_weapon1] ? wp_fist :
        gamekeydown[key_weapon2] ? wp_pistol :
        gamekeydown[key_weapon3] ? wp_shotgun :
        gamekeydown[key_weapon4] ? wp_chaingun :
        gamekeydown[key_weapon5] ? wp_missile :
        gamekeydown[key_weapon6] && gamemode != shareware ? wp_plasma :
        gamekeydown[key_weapon7] && gamemode != shareware ? wp_bfg :
        gamekeydown[key_weapon8] ? wp_chainsaw :
        (!demo_compatibility && gamekeydown[key_weapon9] && gamemode == commercial) ? wp_supershotgun :
        wp_nochange;

      if (!demo_compatibility)
        {
          const player_t *player = &players[consoleplayer];

          if (newweapon==wp_fist && player->weaponowned[wp_chainsaw] &&
              player->readyweapon!=wp_chainsaw &&
              (player->readyweapon==wp_fist ||
               !player->powers[pw_strength] ||
               P_WeaponPreferred(wp_chainsaw, wp_fist)))
            newweapon = wp_chainsaw;

          if (newweapon == wp_shotgun && gamemode == commercial &&
              player->weaponowned[wp_supershotgun] &&
              (!player->weaponowned[wp_shotgun] ||
               player->readyweapon == wp_shotgun ||
               (player->readyweapon != wp_supershotgun &&
                P_WeaponPreferred(wp_supershotgun, wp_shotgun))))
            newweapon = wp_supershotgun;
        }

    }

  if (newweapon != wp_nochange)
    {
      cmd->buttons |= BT_CHANGE;
      cmd->buttons |= newweapon<<BT_WEAPONSHIFT;
    }


  if (mousebuttons[mousebforward])
    forward += forwardmove[speed];


  if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 )
    {
      dclickstate = mousebuttons[mousebforward];
      if (dclickstate)
        dclicks++;
      if (dclicks == 2)
        {
          cmd->buttons |= BT_USE;
          dclicks = 0;
        }
      else
        dclicktime = 0;
    }
  else
    if ((dclicktime += ticdup) > 20)
      {
        dclicks = 0;
        dclickstate = 0;
      }

  bstrafe = mousebuttons[mousebstrafe];

  if (bstrafe != dclickstate2 && dclicktime2 > 1 )
    {
      dclickstate2 = bstrafe;
      if (dclickstate2)
        dclicks2++;
      if (dclicks2 == 2)
        {
          cmd->buttons |= BT_USE;
          dclicks2 = 0;
        }
      else
        dclicktime2 = 0;
    }
  else
    if ((dclicktime2 += ticdup) > 20)
      {
        dclicks2 = 0;
        dclickstate2 = 0;
      }
  forward += mousey;
  if (strafe)
    side += mousex / 4;       /* mead  Don't want to strafe as fast as turns.*/
  else
    cmd->angleturn -= mousex; /* mead now have enough dynamic range 2-10-00 */

  mousex = mousey = 0;

  if (forward > MAXPLMOVE)
    forward = MAXPLMOVE;
  else if (forward < -MAXPLMOVE)
    forward = -MAXPLMOVE;
  if (side > MAXPLMOVE)
    side = MAXPLMOVE;
  else if (side < -MAXPLMOVE)
    side = -MAXPLMOVE;

  cmd->forwardmove += fudgef((signed char)forward);
  cmd->sidemove += side;
  cmd->angleturn = fudgea(cmd->angleturn);

  if (special_event & BT_SPECIAL) {
    cmd->buttons = special_event;
    special_event = 0;
  }
}

void G_RestartLevel(void)
{
  special_event = BT_SPECIAL | (BTS_RESTARTLEVEL & BT_SPECIALMASK);
}

#include "z_bmalloc.h"

static void G_DoLoadLevel (void)
{
  int i;

  skyflatnum = R_FlatNumForName ( SKYFLATNAME );

  if (gamemode == commercial)
    {
      skytexture = R_TextureNumForName ("SKY3");
      if (gamemap < 12)
        skytexture = R_TextureNumForName ("SKY1");
      else
        if (gamemap < 21)
          skytexture = R_TextureNumForName ("SKY2");
    }
  else
    switch (gameepisode)
      {
      case 1:
        skytexture = R_TextureNumForName ("SKY1");
        break;
      case 2:
        skytexture = R_TextureNumForName ("SKY2");
        break;
      case 3:
        skytexture = R_TextureNumForName ("SKY3");
        break;
      case 4:
        skytexture = R_TextureNumForName ("SKY4");
        break;
      }

  if (!demo_compatibility && !mbf_features)
    basetic = gametic;

  if (wipegamestate == GS_LEVEL)
    wipegamestate = -1;

  gamestate = GS_LEVEL;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      if (playeringame[i] && players[i].playerstate == PST_DEAD)
        players[i].playerstate = PST_REBORN;
      memset (players[i].frags,0,sizeof(players[i].frags));
    }

  {
    DECLARE_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);
    NULL_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);


  }

  P_SetupLevel (gameepisode, gamemap, 0, gameskill);
  if (!demoplayback)
    displayplayer = consoleplayer;
  gameaction = ga_nothing;


  memset (gamekeydown, 0, sizeof(gamekeydown));
  mousex = mousey = 0;
  special_event = 0; paused = false;
  memset (mousebuttons, 0, sizeof(mousebuttons));


  ST_Start();
  HU_Start();
}

boolean G_Responder (event_t* ev)
{

  if (gameaction == ga_nothing && (demoplayback || gamestate == GS_DEMOSCREEN))
    {

      if (ev->type == ev_keydown && ev->data1 == key_pause)
  {
    if (paused ^= 2)
      S_PauseSound();
    else
      S_ResumeSound();
    return true;
  }

      return gamestate == GS_DEMOSCREEN && !(paused & 2) && !(automapmode & am_active) && ((ev->type == ev_keydown) || (ev->type == ev_mouse && ev->data1)) ? M_StartControlPanel(), true : false;
    }

  if (gamestate == GS_FINALE && F_Responder(ev))
    return true;

  switch (ev->type)
    {
    case ev_keydown:
      if (ev->data1 == key_pause)
        {
          special_event = BT_SPECIAL | (BTS_PAUSE & BT_SPECIALMASK);
          return true;
        }
      if (ev->data1 <NUMKEYS)
        gamekeydown[ev->data1] = true;
      return true;

    case ev_keyup:
      if (ev->data1 <NUMKEYS)
        gamekeydown[ev->data1] = false;
      return false;

    case ev_mouse:
      mousebuttons[0] = ev->data1 & 1;
      mousebuttons[1] = ev->data1 & 2;
      mousebuttons[2] = ev->data1 & 4;
      mousex += (ev->data2*(mouseSensitivity_horiz))/10;  /* killough */
      mousey += (ev->data3*(mouseSensitivity_vert))/10;  /*Mead rm *4 */
      return true;

    default:
      break;
    }
  return false;
}

void G_Ticker (void)
{
  int i;
  static gamestate_t prevgamestate;


  if (!demoplayback && mapcolor_plyr[consoleplayer] != mapcolor_me) {

    int net_cl = LONG(mapcolor_me);
    G_ChangedPlayerColour(consoleplayer, mapcolor_me);
  }
  P_MapStart();

  for (i=0 ; i<MAXPLAYERS ; i++)
    if (playeringame[i] && players[i].playerstate == PST_REBORN)
      G_DoReborn (i);
  P_MapEnd();


  while (gameaction != ga_nothing)
    {
      switch (gameaction)
        {
        case ga_loadlevel:

    for (i=0 ; i<MAXPLAYERS ; i++)
      players[i].playerstate = PST_REBORN;
          G_DoLoadLevel ();
          break;
        case ga_newgame:
          G_DoNewGame ();
          break;
        case ga_loadgame:
          G_DoLoadGame ();
          break;
        case ga_savegame:
          G_DoSaveGame (false);
          break;
        case ga_playdemo:
          G_DoPlayDemo ();
          break;
        case ga_completed:
          G_DoCompleted ();
          break;
        case ga_victory:
          F_StartFinale ();
          break;
        case ga_worlddone:
          G_DoWorldDone ();
          break;
        case ga_nothing:
          break;
        }
    }

  if (paused & 2 || (!demoplayback && menuactive))
    basetic++;
  else {

    int buf = (gametic/ticdup)%BACKUPTICS;

    for (i=0 ; i<MAXPLAYERS ; i++) {
      if (playeringame[i])
        {
          ticcmd_t *cmd = &players[i].cmd;

          memcpy(cmd, &netcmds[i][buf], sizeof *cmd);

          if (demoplayback)
            G_ReadDemoTiccmd (cmd);
          if (demorecording)
            G_WriteDemoTiccmd (cmd);

          if ((demoplayback) && cmd->forwardmove > TURBOTHRESHOLD &&
              !(gametic&31) && ((gametic>>5)&3) == i )
            {
        extern char *player_names[];
        /* cph - don't use sprintf, use doom_printf */
              doom_printf ("%s is turbo!", player_names[i]);
            }

        }
    }


    for (i=0; i<MAXPLAYERS; i++) {
      if (playeringame[i])
        {
          if (players[i].cmd.buttons & BT_SPECIAL)
            {
              switch (players[i].cmd.buttons & BT_SPECIALMASK)
                {
                case BTS_PAUSE:
                  paused ^= 1;
                  if (paused)
                    S_PauseSound ();
                  else
                    S_ResumeSound ();
                  break;

                case BTS_SAVEGAME:
                  if (!savedescription[0])
                    strcpy(savedescription, "NET GAME");
                  savegameslot =
                    (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
                  gameaction = ga_savegame;
                  break;


                case BTS_LOADGAME:
                  savegameslot =
                    (players[i].cmd.buttons & BTS_SAVEMASK)>>BTS_SAVESHIFT;
                  gameaction = ga_loadgame;
      command_loadgame = false;
                  break;


    case BTS_RESTARTLEVEL:
                  if (demoplayback || (compatibility_level < lxdoom_1_compatibility))
                    break;
      gameaction = ga_loadlevel;
      break;
                }
        players[i].cmd.buttons = 0;
            }
        }
    }
  }

  if (gamestate != prevgamestate) {
    switch (prevgamestate) {
    case GS_LEVEL:

      break;
    case GS_INTERMISSION:
      WI_End();
    default:
      break;
    }
    prevgamestate = gamestate;
  }

  if (paused & 2 && gamestate != GS_LEVEL)
    return;


  switch (gamestate)
    {
    case GS_LEVEL:
      P_Ticker ();
      ST_Ticker ();
      AM_Ticker ();
      HU_Ticker ();
      break;

    case GS_INTERMISSION:
      WI_Ticker ();
      break;

    case GS_FINALE:
      F_Ticker ();
      break;

    case GS_DEMOSCREEN:
      D_PageTicker ();
      break;
    }
}

static void G_PlayerFinishLevel(int player)
{
  player_t *p = &players[player];
  memset(p->powers, 0, sizeof p->powers);
  memset(p->cards, 0, sizeof p->cards);
  p->mo = NULL;
  p->extralight = 0;
  p->fixedcolormap = 0;
  p->damagecount = 0;
  p->bonuscount = 0;
}

#include "r_draw.h"

void G_ChangedPlayerColour(int pn, int cl)
{

}

void G_PlayerReborn (int player)
{
  player_t *p;
  int i;
  int frags[MAXPLAYERS];
  int killcount;
  int itemcount;
  int secretcount;

  memcpy (frags, players[player].frags, sizeof frags);
  killcount = players[player].killcount;
  itemcount = players[player].itemcount;
  secretcount = players[player].secretcount;

  p = &players[player];


  {
    int cheats = p->cheats;
    memset (p, 0, sizeof(*p));
    p->cheats = cheats;
  }

  memcpy(players[player].frags, frags, sizeof(players[player].frags));
  players[player].killcount = killcount;
  players[player].itemcount = itemcount;
  players[player].secretcount = secretcount;

  p->usedown = p->attackdown = true;
  p->playerstate = PST_LIVE;
  p->health = initial_health;
  p->readyweapon = p->pendingweapon = wp_pistol;
  p->weaponowned[wp_fist] = true;
  p->weaponowned[wp_pistol] = true;
  p->ammo[am_clip] = initial_bullets;

  for (i=0 ; i<NUMAMMO ; i++)
    p->maxammo[i] = maxammo[i];
}

void G_DoReborn (int playernum)
{
    gameaction = ga_loadlevel;
}


int pars[4][10] = {
  {0},
  {0,30,75,120,90,165,180,180,30,165},
  {0,90,90,90,120,90,360,240,30,170},
  {0,90,45,90,150,90,90,165,30,135}
};


int cpars[32] = {
  30,90,120,120,90,150,120,120,270,90,
  210,150,150,150,210,150,420,150,210,150,
  240,150,180,150,150,300,330,420,300,180,
  120,30
};

static boolean secretexit;

void G_ExitLevel (void)
{
  secretexit = false;
  gameaction = ga_completed;
}

void G_SecretExitLevel (void)
{
  if (gamemode!=commercial || haswolflevels)
    secretexit = true;
  else
    secretexit = false;
  gameaction = ga_completed;
}

void G_DoCompleted (void)
{
  int i;

  gameaction = ga_nothing;

  for (i=0; i<MAXPLAYERS; i++)
    if (playeringame[i])
      G_PlayerFinishLevel(i);

  if (automapmode & am_active)
    AM_Stop();

  if (gamemode != commercial)
    switch(gamemap)
      {

      case 9:
        for (i=0 ; i<MAXPLAYERS ; i++)
          players[i].didsecret = true;
        break;
      }

  wminfo.didsecret = players[consoleplayer].didsecret;
  wminfo.epsd = gameepisode -1;
  wminfo.last = gamemap -1;

  if (gamemode == commercial)
    {
      if (secretexit)
        switch(gamemap)
          {
          case 15:
            wminfo.next = 30; break;
          case 31:
            wminfo.next = 31; break;
          }
      else
        switch(gamemap)
          {
          case 31:
          case 32:
            wminfo.next = 15; break;
          default:
            wminfo.next = gamemap;
          }
    }
  else
    {
      if (secretexit)
        wminfo.next = 8;
      else
        if (gamemap == 9)
          {

            switch (gameepisode)
              {
              case 1:
                wminfo.next = 3;
                break;
              case 2:
                wminfo.next = 5;
                break;
              case 3:
                wminfo.next = 6;
                break;
              case 4:
                wminfo.next = 2;
                break;
              }
          }
        else
          wminfo.next = gamemap;
    }

  wminfo.maxkills = totalkills;
  wminfo.maxitems = totalitems;
  wminfo.maxsecret = totalsecret;
  wminfo.maxfrags = 0;

  if ( gamemode == commercial )
    wminfo.partime = TICRATE*cpars[gamemap-1];
  else
    wminfo.partime = TICRATE*pars[gameepisode][gamemap];

  wminfo.pnum = consoleplayer;

  for (i=0 ; i<MAXPLAYERS ; i++)
    {
      wminfo.plyr[i].in = playeringame[i];
      wminfo.plyr[i].skills = players[i].killcount;
      wminfo.plyr[i].sitems = players[i].itemcount;
      wminfo.plyr[i].ssecret = players[i].secretcount;
      wminfo.plyr[i].stime = leveltime;
      memcpy (wminfo.plyr[i].frags, players[i].frags,
              sizeof(wminfo.plyr[i].frags));
    }

  wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime%35));

  gamestate = GS_INTERMISSION;
  automapmode &= ~am_active;

  WI_Start (&wminfo);
}

void G_WorldDone (void)
{
  gameaction = ga_worlddone;

  if (secretexit)
    players[consoleplayer].didsecret = true;

  if (gamemode == commercial)
    {
      switch (gamemap)
        {
        case 15:
        case 31:
          if (!secretexit)
            break;
        case 6:
        case 11:
        case 20:
        case 30:
          F_StartFinale ();
          break;
        }
    }
  else if (gamemap == 8)
    gameaction = ga_victory;
}

void G_DoWorldDone (void)
{
  idmusnum = -1;
  gamestate = GS_LEVEL;
  gamemap = wminfo.next+1;
  G_DoLoadLevel();
  gameaction = ga_nothing;
  AM_ClearMarks();
}

#define MIN_MAXPLAYERS 32

extern boolean setsizeneeded;

static uint_64_t G_UpdateSignature(uint_64_t s, const char *name)
{
  int i, lump = W_CheckNumForName(name);
  if (lump != -1 && (i = lump+10) < numlumps)
    do
      {
  int size = W_LumpLength(i);
  const byte *p = W_CacheLumpNum(i);
  while (size--)
    s <<= 1, s += *p++;
  W_UnlockLumpNum(i);
      }
    while (--i > lump);
  return s;
}

static uint_64_t G_Signature(void)
{
  static uint_64_t s = 0;
  static boolean computed = false;
  char name[9];
  int episode, map;

  if (!computed) {
   computed = true;
   if (gamemode == commercial)
    for (map = haswolflevels ? 32 : 30; map; map--)
      sprintf(name, "map%02d", map), s = G_UpdateSignature(s, name);
   else
    for (episode = gamemode==retail ? 4 :
     gamemode==shareware ? 1 : 3; episode; episode--)
      for (map = 9; map; map--)
  sprintf(name, "E%dM%d", episode, map), s = G_UpdateSignature(s, name);
  }
  return s;
}

void G_LoadGame(int slot, boolean command)
{
  if (!demoplayback && !command) {
    special_event = BT_SPECIAL | (BTS_LOADGAME & BT_SPECIALMASK) | ((slot << BTS_SAVESHIFT) & BTS_SAVEMASK);
  } else {

    gameaction = ga_loadgame;
    savegameslot = slot;
    demoplayback = false;
  }
  command_loadgame = command;
  R_SmoothPlaying_Reset(NULL);
}

static void G_LoadGameErr(const char *msg)
{
  Z_Free(savebuffer);
  if (command_loadgame)
    {
      D_StartTitle();
      gamestate = GS_DEMOSCREEN;
    }
}


#define VERSIONSIZE   16

const char * comp_lev_str[MAX_COMPATIBILITY_LEVEL] =
{ "doom v1.2", "doom v1.666", "doom/doom2 v1.9", "ultimate doom", "final doom",
  "dosdoom compatibility", "tasdoom compatibility", "\"boom compatibility\"", "boom v2.01", "boom v2.02", "lxdoom v1.3.2+",
  "MBF", "PrBoom 2.03beta", "PrBoom v2.1.0-2.1.1", "PrBoom v2.1.2-v2.2.6",
  "PrBoom v2.3.x", "PrBoom 2.4.0", "Current PrBoom"  };



static byte map_old_comp_levels[] =
{ 0, 1, 2, 4, 5, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

static const struct {
  int comp_level;
  const char* ver_printf;
  int version;
} version_headers[] = {
  { prboom_3_compatibility, "PrBoom %d", 210},
  { prboom_5_compatibility, "PrBoom %d", 211},
  { prboom_6_compatibility, "PrBoom %d", 212}
};

static const size_t num_version_headers = sizeof(version_headers) / sizeof(version_headers[0]);

void G_DoLoadGame(void)
{
  int  length, i;

  char name[PATH_MAX+1];
  int savegame_compatibility = -1;

  G_SaveGameName(name,sizeof(name),savegameslot, demoplayback);

  gameaction = ga_nothing;

  length = M_ReadFile(name, &savebuffer);
  if (length<=0)
    I_Error("Couldn't read file %s: %s", name, "(Unknown Error)");
  save_p = savebuffer + SAVESTRINGSIZE;


  for (i=0; (size_t)i<num_version_headers; i++) {
    char vcheck[VERSIONSIZE];

    sprintf (vcheck, version_headers[i].ver_printf, version_headers[i].version);

    if (!strncmp(save_p, vcheck, VERSIONSIZE)) {
      savegame_compatibility = version_headers[i].comp_level;
      i = num_version_headers;
    }
  }
  if (savegame_compatibility == -1) {
      G_LoadGameErr("Unrecognised savegame version!\nAre you sure? (y/n) ");
      return;
  }

  save_p += VERSIONSIZE;

  {
    uint_64_t checksum = 0;

    checksum = G_Signature();

    if (memcmp(&checksum, save_p, sizeof checksum)) {
        char *msg = malloc(strlen(save_p + sizeof checksum) + 128);
        strcpy(msg,"Incompatible Savegame!!!\n");
        if (save_p[sizeof checksum])
          strcat(strcat(msg,"Wads expected:\n\n"), save_p + sizeof checksum);
        strcat(msg, "\nAre you sure?");
        G_LoadGameErr(msg);
        free(msg);
        return;
    }
    save_p += sizeof checksum;
   }

  save_p += strlen(save_p)+1;

  compatibility_level = (savegame_compatibility >= prboom_4_compatibility) ? *save_p : savegame_compatibility;
  if (savegame_compatibility < prboom_6_compatibility)
    compatibility_level = map_old_comp_levels[compatibility_level];
  save_p++;

  gameskill = *save_p++;
  gameepisode = *save_p++;
  gamemap = *save_p++;

  for (i=0 ; i<MAXPLAYERS ; i++)
    playeringame[i] = *save_p++;
  save_p += MIN_MAXPLAYERS-MAXPLAYERS;

  idmusnum = *save_p++;
  if (idmusnum==255) idmusnum=-1;

  save_p = (char*)G_ReadOptions(save_p);

  G_InitNew (gameskill, gameepisode, gamemap);

  memcpy(&leveltime, save_p, sizeof leveltime);
  save_p += sizeof leveltime;

  if (compatibility_level >= prboom_2_compatibility) {
    memcpy(&totalleveltimes, save_p, sizeof totalleveltimes);
    save_p += sizeof totalleveltimes;
  }
  else totalleveltimes = 0;

  basetic = gametic - *save_p++;

  P_MapStart();
  P_UnArchivePlayers ();
  P_UnArchiveWorld ();
  P_UnArchiveThinkers ();
  P_UnArchiveSpecials ();
  P_UnArchiveRNG ();
  P_UnArchiveMap ();
  P_MapEnd();
  R_SmoothPlaying_Reset(NULL);

  if (*save_p != 0xe6)
    I_Error ("G_DoLoadGame: Bad savegame");

  Z_Free (savebuffer);

  if (setsizeneeded)
    R_ExecuteSetViewSize ();

  R_FillBackScreen ();

  if (!command_loadgame)
    singledemo = false;  /* Clear singledemo flag if loading from menu */
  else
    if (singledemo) {
      gameaction = ga_loadgame; /* Mark that we're loading a game before demo */
      G_DoPlayDemo();           /* This will detect it and won't reinit level */
    } else /* Command line + record means it's a recordfrom */
      if (demorecording)
        G_BeginRecording();
}

void G_SaveGame(int slot, char *description)
{
  strcpy(savedescription, description);
  if (demoplayback) {
    savegameslot = slot;
    G_DoSaveGame(true);
  }

  special_event = BT_SPECIAL | (BTS_SAVEGAME & BT_SPECIALMASK) |
    ((slot << BTS_SAVESHIFT) & BTS_SAVEMASK);
}

void (CheckSaveGame)(size_t size, const char* file, int line)
{
  size_t pos = save_p - savebuffer;

  size += 1024;
  if (pos+size > savegamesize)
    save_p = (savebuffer = realloc(savebuffer,
           savegamesize += (size+1023) & ~1023)) + pos;
}

void G_SaveGameName(char *name, size_t size, int slot, boolean demoplayback)
{
  const char* sgn = demoplayback ? "demosav" : "savegame";
  snprintf (name, size, "%s/%s%d.dsg", basesavegame, sgn, slot);
}

static void G_DoSaveGame (boolean menu)
{
  char name[PATH_MAX+1];
  char name2[VERSIONSIZE];
  char *description;
  int  length, i;

  gameaction = ga_nothing;


  G_SaveGameName(name,sizeof(name),savegameslot, demoplayback && !menu);

  description = savedescription;

  save_p = savebuffer = malloc(savegamesize);

  CheckSaveGame(SAVESTRINGSIZE+VERSIONSIZE+sizeof(uint_64_t));
  memcpy (save_p, description, SAVESTRINGSIZE);
  save_p += SAVESTRINGSIZE;
  memset (name2,0,sizeof(name2));


  for (i=0; (size_t)i<num_version_headers; i++)
    if (version_headers[i].comp_level == best_compatibility) {

      sprintf (name2,version_headers[i].ver_printf,version_headers[i].version);
      memcpy (save_p, name2, VERSIONSIZE);
      i = num_version_headers+1;
    }

  save_p += VERSIONSIZE;

  { /* killough 3/16/98, 12/98: store lump name checksum */
    uint_64_t checksum = G_Signature();
    memcpy(save_p, &checksum, sizeof checksum);
    save_p += sizeof checksum;
  }


  {

    size_t i;
    for (i = 0; i<numwadfiles; i++)
      {
        const char *const w = wadfiles[i].name;
        CheckSaveGame(strlen(w)+2);
        strcpy(save_p, w);
        save_p += strlen(save_p);
        *save_p++ = '\n';
      }
    *save_p++ = 0;
  }

  CheckSaveGame(GAME_OPTION_SIZE+MIN_MAXPLAYERS+14);

  *save_p++ = compatibility_level;
  *save_p++ = gameskill;
  *save_p++ = gameepisode;
  *save_p++ = gamemap;

  for (i=0 ; i<MAXPLAYERS ; i++)
    *save_p++ = playeringame[i];

  for (;i<MIN_MAXPLAYERS;i++)
    *save_p++ = 0;

  *save_p++ = idmusnum;

  save_p = G_WriteOptions(save_p);

  /* cph - FIXME - endianness? */
  /* killough 11/98: save entire word */
  memcpy(save_p, &leveltime, sizeof leveltime);
  save_p += sizeof leveltime;

  /* cph - total episode time */
  if (compatibility_level >= prboom_2_compatibility) {
    memcpy(save_p, &totalleveltimes, sizeof totalleveltimes);
    save_p += sizeof totalleveltimes;
  }
  else totalleveltimes = 0;

  *save_p++ = (gametic-basetic) & 255;

  P_ArchivePlayers();
  P_ThinkerToIndex();
  P_ArchiveWorld();
  P_ArchiveThinkers();
  P_IndexToThinker();
  P_ArchiveSpecials();
  P_ArchiveRNG();
  P_ArchiveMap();

  *save_p++ = 0xe6;

  length = save_p - savebuffer;

  doom_printf( "%s", M_WriteFile(name, savebuffer, length)
         ? GGSAVED /* Ty - externalised */
         : "Game save failed!");

  free(savebuffer);
  savebuffer = save_p = NULL;

  savedescription[0] = 0;
}

static skill_t d_skill;
static int     d_episode;
static int     d_map;

void G_DeferedInitNew(skill_t skill, int episode, int map)
{
  d_skill = skill;
  d_episode = episode;
  d_map = map;
  gameaction = ga_newgame;
}

void G_Compatibility(void)
{
  static const struct {
    complevel_t fix;
    complevel_t opt;
  } levels[] = {

    { mbf_compatibility, mbf_compatibility },

    { mbf_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },



    { boom_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { mbf_compatibility, mbf_compatibility },

    { boom_compatibility_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { mbf_compatibility, mbf_compatibility },

    { boom_202_compatibility, mbf_compatibility },

    { mbf_compatibility, mbf_compatibility },

    { lxdoom_1_compatibility, mbf_compatibility },

    { boom_compatibility_compatibility, mbf_compatibility },

    { mbf_compatibility, mbf_compatibility },

    { boom_compatibility, mbf_compatibility },

    { lxdoom_1_compatibility, prboom_2_compatibility },

    { prboom_2_compatibility, prboom_2_compatibility },

    { boom_compatibility_compatibility, prboom_3_compatibility },

    { ultdoom_compatibility, prboom_4_compatibility },

    { prboom_4_compatibility, prboom_4_compatibility },

    { doom_1666_compatibility, prboom_4_compatibility },
  };
  int i;

  if (sizeof(levels)/sizeof(*levels) != COMP_NUM)
    I_Error("G_Compatibility: consistency error");

  for (i = 0; i < sizeof(levels)/sizeof(*levels); i++)
    if (compatibility_level < levels[i].opt)
      comp[i] = (compatibility_level < levels[i].fix);

  if (!mbf_features) {
    monster_infighting = 1;
    monster_backing = 0;
    monster_avoid_hazards = 0;
    monster_friction = 0;
    help_friends = 0;
  }
}

void G_ReloadDefaults(void)
{

  weapon_recoil = default_weapon_recoil;
  player_bobbing = default_player_bobbing;
  variable_friction = default_variable_friction;
  allow_pushers     = default_allow_pushers;
  monsters_remember = default_monsters_remember;
  monster_infighting = default_monster_infighting;
  distfriend = default_distfriend;
  monster_backing = default_monster_backing;
  monster_avoid_hazards = default_monster_avoid_hazards;
  monster_friction = default_monster_friction;
  help_friends = default_help_friends;
  respawnparm = clrespawnparm;
  fastparm = clfastparm;
  nomonsters = clnomonsters;

  if (startskill==sk_none)
    startskill = (skill_t)(defaultskill-1);

  demoplayback = false;
  singledemo = false;

  memset(playeringame+1, 0, sizeof(*playeringame)*(MAXPLAYERS-1));

  consoleplayer = 0;

  compatibility_level = default_compatibility_level;

  if (compatibility_level == -1)
    compatibility_level = best_compatibility;

  if (mbf_features)
    memcpy(comp, default_comp, sizeof comp);
  G_Compatibility();


  demo_insurance = default_demo_insurance == 1;

  rngseed += I_GetRandomTimeSeed() + gametic;
}

void G_DoNewGame (void)
{
  G_ReloadDefaults();
  G_InitNew (d_skill, d_episode, d_map);
  gameaction = ga_nothing;


  ST_Start();
}




void G_SetFastParms(int fast_pending)
{
  static int fast = 0;
  int i;
  if (fast != fast_pending) {     /* only change if necessary */
    if ((fast = fast_pending))
      {
        for (i=S_SARG_RUN1; i<=S_SARG_PAIN2; i++)
          if (states[i].tics != 1 || demo_compatibility)
            states[i].tics >>= 1;
        mobjinfo[MT_BRUISERSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 20*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 20*FRACUNIT;
      }
    else
      {
        for (i=S_SARG_RUN1; i<=S_SARG_PAIN2; i++)
          states[i].tics <<= 1;
        mobjinfo[MT_BRUISERSHOT].speed = 15*FRACUNIT;
        mobjinfo[MT_HEADSHOT].speed = 10*FRACUNIT;
        mobjinfo[MT_TROOPSHOT].speed = 10*FRACUNIT;
      }
  }
}

void G_InitNew(skill_t skill, int episode, int map)
{
  int i;

  if (paused)
    {
      paused = false;
      S_ResumeSound();
    }

  if (skill > sk_nightmare)
    skill = sk_nightmare;

  if (episode < 1)
    episode = 1;

  if (gamemode == retail)
    {
      if (episode > 4)
        episode = 4;
    }
  else
    if (gamemode == shareware)
      {
        if (episode > 1)
          episode = 1;
      }
    else
      if (episode > 3)
        episode = 3;

  if (map < 1)
    map = 1;
  if (map > 9 && gamemode != commercial)
    map = 9;

  G_SetFastParms(fastparm || skill == sk_nightmare);

  M_ClearRandom();

  respawnmonsters = skill == sk_nightmare || respawnparm;


  for (i=0 ; i<MAXPLAYERS ; i++)
    players[i].playerstate = PST_REBORN;

  usergame = true;
  paused = false;
  automapmode &= ~am_active;
  gameepisode = episode;
  gamemap = map;
  gameskill = skill;

  totalleveltimes = 0;


  AM_ClearMarks();

  G_DoLoadLevel ();
}

#define DEMOMARKER    0x80

void G_ReadDemoTiccmd (ticcmd_t* cmd)
{
  unsigned char at;

  if (*demo_p == DEMOMARKER)
    G_CheckDemoStatus();
  else if (demoplayback && demo_p + (longtics?5:4) > demobuffer + demolength)
  {
    lprintf(LO_WARN, "G_ReadDemoTiccmd: missing DEMOMARKER\n");
    G_CheckDemoStatus();
  }
  else
    {
      cmd->forwardmove = ((signed char)*demo_p++);
      cmd->sidemove = ((signed char)*demo_p++);
      if (!longtics) {
        cmd->angleturn = ((unsigned char)(at = *demo_p++))<<8;
      } else {
    unsigned int lowbyte = (unsigned char)*demo_p++;
        cmd->angleturn = (((signed int)(*demo_p++))<<8) + lowbyte;
      }
      cmd->buttons = (unsigned char)*demo_p++;

      if (compatibility_level == tasdoom_compatibility)
      {
        signed char k = cmd->forwardmove;
        cmd->forwardmove = cmd->sidemove;
        cmd->sidemove = (signed char)at;
        cmd->angleturn = ((unsigned char)cmd->buttons)<<8;
        cmd->buttons = (byte)k;
      }
    }
}

void G_WriteDemoTiccmd (ticcmd_t* cmd)
{
  char buf[5];
  char *p = buf;

  *p++ = cmd->forwardmove;
  *p++ = cmd->sidemove;
  if (!longtics) {
    *p++ = (cmd->angleturn+128)>>8;
  } else {
    signed short a = cmd->angleturn;
    *p++ = a & 0xff;
    *p++ = (a >> 8) & 0xff;
  }
  *p++ = cmd->buttons;
  if (fwrite(buf, p-buf, 1, demofp) != 1)
    I_Error("G_WriteDemoTiccmd: error writing demo");

  demo_p = buf;
  G_ReadDemoTiccmd (cmd);
}





void G_RecordDemo (const char* name)
{
  char     demoname[PATH_MAX];
  usergame = false;
  AddDefaultExtension(strcpy(demoname, name), ".lmp");
  demorecording = true;
  if (access(demoname, F_OK)) {
    demofp = fopen(demoname, "wb");
  } else {
    demofp = fopen(demoname, "r+");
    if (demofp) {
      int slot = -1;
      int rc;
      int bytes_per_tic;
      const byte* pos;

      { /* Read the demo header for options etc */
        byte buf[200];
        size_t len = fread(buf, 1, sizeof(buf), demofp);
        pos = G_ReadDemoHeader(buf, len, false);
        if (pos)
        {
          fseek(demofp, pos - buf, SEEK_SET);
        }
      }
      bytes_per_tic = longtics ? 5 : 4;
    if (pos)
      do {
        byte buf[5];
      
        rc = fread(buf, 1, bytes_per_tic, demofp);
        if (buf[0] == DEMOMARKER) break;
        if (buf[bytes_per_tic-1] & BT_SPECIAL)
          if ((buf[bytes_per_tic-1] & BT_SPECIALMASK) == BTS_SAVEGAME)
            slot = (buf[bytes_per_tic-1] & BTS_SAVEMASK)>>BTS_SAVESHIFT;
      } while (rc == bytes_per_tic);

      if (slot == -1) I_Error("G_RecordDemo: No save in demo, can't continue");

      fseek(demofp, -rc, SEEK_CUR);
      G_LoadGame(slot, false);
      autostart = false;
    }
  }
  if (!demofp) I_Error("G_RecordDemo: failed to open %s", name);
}

extern int forceOldBsp;

byte *G_WriteOptions(byte *demo_p)
{
  byte *target = demo_p + GAME_OPTION_SIZE;

  *demo_p++ = monsters_remember;

  *demo_p++ = variable_friction;

  *demo_p++ = weapon_recoil;

  *demo_p++ = allow_pushers;

  *demo_p++ = 0;

  *demo_p++ = player_bobbing;


  *demo_p++ = respawnparm;
  *demo_p++ = fastparm;
  *demo_p++ = nomonsters;
  *demo_p++ = demo_insurance;
  *demo_p++ = (byte)((rngseed >> 24) & 0xff);
  *demo_p++ = (byte)((rngseed >> 16) & 0xff);
  *demo_p++ = (byte)((rngseed >>  8) & 0xff);
  *demo_p++ = (byte)( rngseed        & 0xff);
  *demo_p++ = monster_infighting;
  *demo_p++ = 0;
  *demo_p++ = 0;
  *demo_p++ = 0;
  *demo_p++ = (distfriend >> 8) & 0xff;
  *demo_p++ =  distfriend       & 0xff;
  *demo_p++ = monster_backing;
  *demo_p++ = monster_avoid_hazards;
  *demo_p++ = monster_friction;
  *demo_p++ = help_friends;
  *demo_p++ = 0;
  *demo_p++ = 0;

  {
    int i;
    for (i=0; i < COMP_TOTAL; i++)
      *demo_p++ = comp[i] != 0;
  }

  *demo_p++ = (compatibility_level >= prboom_2_compatibility) && forceOldBsp;

  while (demo_p < target)
    *demo_p++ = 0;

  if (demo_p != target)
    I_Error("G_WriteOptions: GAME_OPTION_SIZE is too small");

  return target;
}

const byte *G_ReadOptions(const byte *demo_p)
{
  const byte *target = demo_p + GAME_OPTION_SIZE;

  monsters_remember = *demo_p++;
  variable_friction = *demo_p;
  demo_p++;
  weapon_recoil = *demo_p;
  demo_p++;
  allow_pushers = *demo_p;
  demo_p++;
  demo_p++;
  player_bobbing = *demo_p;
  demo_p++;
  respawnparm = *demo_p++;
  fastparm = *demo_p++;
  nomonsters = *demo_p++;
  demo_insurance = *demo_p++;
  rngseed  = *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;
  rngseed <<= 8;
  rngseed += *demo_p++ & 0xff;


  if (mbf_features)
    {
      monster_infighting = *demo_p++;

      demo_p++;

      demo_p += 2;

      distfriend = *demo_p++ << 8;
      distfriend+= *demo_p++;

      monster_backing = *demo_p++;

      monster_avoid_hazards = *demo_p++;

      monster_friction = *demo_p++;

      help_friends = *demo_p++;

      demo_p++;
      demo_p++;

      {
  int i;
  for (i=0; i < COMP_TOTAL; i++)
    comp[i] = *demo_p++;
      }

      forceOldBsp = *demo_p++;
    }
  else
    {
    }

  G_Compatibility();
  return target;
}

void G_BeginRecording (void)
{
  int i;
  byte *demostart, *demo_p;
  demostart = demo_p = malloc(1000);
  longtics = 0;

  if (mbf_features) {
    {
      unsigned char v;
      switch (compatibility_level)
      {
        case mbf_compatibility: v = 203; break;
        case prboom_2_compatibility: v = 210; break;
        case prboom_3_compatibility: v = 211; break;
        case prboom_4_compatibility: v = 212; break;
        case prboom_5_compatibility: v = 213; break;
        case prboom_6_compatibility:
        default:
                     v = 214; 
                     longtics = 1;
                     break;
      }
      *demo_p++ = v;
    }

    *demo_p++ = 0x1d;
    *demo_p++ = 'M';
    *demo_p++ = 'B';
    *demo_p++ = 'F';
    *demo_p++ = 0xe6;
    *demo_p++ = '\0';
    *demo_p++ = 0;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = false;
    *demo_p++ = consoleplayer;
    demo_p = G_WriteOptions(demo_p);

    for (i=0 ; i<MAXPLAYERS ; i++)
      *demo_p++ = playeringame[i];





    for (; i<MIN_MAXPLAYERS; i++)
      *demo_p++ = 0;

  } else if (compatibility_level > boom_compatibility_compatibility) {
    byte v, c; /* Nominally, version and compatibility bits */
    switch (compatibility_level) {
    case boom_compatibility_compatibility: v = 202, c = 1; break;
    case boom_201_compatibility: v = 201; c = 0; break;
    case boom_202_compatibility: v = 202, c = 0; break;
    default: I_Error("G_BeginRecording: Boom compatibility level unrecognised?");
    }
    *demo_p++ = v;
    *demo_p++ = 0x1d;
    *demo_p++ = 'B';
    *demo_p++ = 'o';
    *demo_p++ = 'o';
    *demo_p++ = 'm';
    *demo_p++ = 0xe6;
    *demo_p++ = c;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = false;
    *demo_p++ = consoleplayer;
    demo_p = G_WriteOptions(demo_p);

    for (i=0 ; i<MAXPLAYERS ; i++)
      *demo_p++ = playeringame[i];





    for (; i<MIN_MAXPLAYERS; i++)
      *demo_p++ = 0;
  } else {
    longtics = 0;
    *demo_p++ = longtics ? 111 : 109;
    *demo_p++ = gameskill;
    *demo_p++ = gameepisode;
    *demo_p++ = gamemap;
    *demo_p++ = false;
    *demo_p++ = respawnparm;
    *demo_p++ = fastparm;
    *demo_p++ = nomonsters;
    *demo_p++ = consoleplayer;
    for (i=0; i<4; i++)
      *demo_p++ = playeringame[i];
  }

  if (fwrite(demostart, 1, demo_p-demostart, demofp) != (size_t)(demo_p-demostart))
    I_Error("G_BeginRecording: Error writing demo header");
  free(demostart);
}

static const char *defdemoname;

void G_DeferedPlayDemo (const char* name)
{
  defdemoname = name;
  gameaction = ga_playdemo;
}

static int demolumpnum = -1;

static int G_GetOriginalDoomCompatLevel(int ver)
{
  if (gamemode == retail) return ultdoom_compatibility;
  if (gamemission >= pack_tnt) return finaldoom_compatibility;
  return doom2_19_compatibility;
}


static boolean CheckForOverrun(const byte *start_p, const byte *current_p, size_t maxsize, size_t size, boolean failonerror)
{
  size_t pos = current_p - start_p;
  if (pos + size > maxsize)
  {
    if (failonerror)
      I_Error("G_ReadDemoHeader: wrong demo header\n");
    else
      return true;
  }
  return false;
}

static const byte* G_ReadDemoHeader(const byte *demo_p, size_t size, boolean failonerror)
{
  skill_t skill;
  int i, episode, map;
  const byte *header_p = demo_p;

  basetic = gametic;

  if (CheckForOverrun(header_p, demo_p, size, 1, failonerror))
    return NULL;

  demover = *demo_p++;
  longtics = 0;

  if (!((demover >=   0  && demover <=   4) ||
        (demover >= 104  && demover <= 111) ||
        (demover >= 200  && demover <= 214)))
  {
    I_Error("G_ReadDemoHeader: Unknown demo format %d.", demover);
  }

  if (demover < 200)
    {
      if (demover >= 111) longtics = 1;

      variable_friction = 0;
      weapon_recoil = 0;
      allow_pushers = 0;
      monster_infighting = 1;
      monster_backing = 0;
      monster_avoid_hazards = 0;
      monster_friction = 0;
      help_friends = 0;

      if ((skill=demover) >= 100)
        {

          if (CheckForOverrun(header_p, demo_p, size, 8, failonerror))
            return NULL;

          compatibility_level = G_GetOriginalDoomCompatLevel(demover);
          skill = *demo_p++;
          episode = *demo_p++;
          map = *demo_p++;
          *demo_p++;
          respawnparm = *demo_p++;
          fastparm = *demo_p++;
          nomonsters = *demo_p++;
          consoleplayer = *demo_p++;
        }
      else
        {

          if (CheckForOverrun(header_p, demo_p, size, 2, failonerror))
            return NULL;

          compatibility_level = doom_12_compatibility;
          episode = *demo_p++;
          map = *demo_p++;
          respawnparm = fastparm =
            nomonsters = consoleplayer = 0;
        }
      G_Compatibility();
    }
  else
    {
      demo_p += 6;
      switch (demover) {
      case 200:
      case 201:

        if (CheckForOverrun(header_p, demo_p, size, 1, failonerror))
          return NULL;

        if (!*demo_p++)
      compatibility_level = boom_201_compatibility;
        else
      compatibility_level = boom_compatibility_compatibility;
      break;
      case 202:

        if (CheckForOverrun(header_p, demo_p, size, 1, failonerror))
          return NULL;

        if (!*demo_p++)
      compatibility_level = boom_202_compatibility;
        else
      compatibility_level = boom_compatibility_compatibility;
      break;
      case 203:
    switch (*(header_p + 2)) {
    case 'B':
      compatibility_level = lxdoom_1_compatibility;
      break;
    case 'M':
      compatibility_level = mbf_compatibility;
      demo_p++;
      break;
    }
    break;
      case 210:
    compatibility_level = prboom_2_compatibility;
    demo_p++;
    break;
      case 211:
    compatibility_level = prboom_3_compatibility;
    demo_p++;
    break;
      case 212:
    compatibility_level = prboom_4_compatibility;
    demo_p++;
    break;
      case 213:
    compatibility_level = prboom_5_compatibility;
    demo_p++;
    break;
      case 214:
    compatibility_level = prboom_6_compatibility;
        longtics = 1;
    demo_p++;
    break;
      }

      if (CheckForOverrun(header_p, demo_p, size, 5, failonerror))
        return NULL;

      skill = *demo_p++;
      episode = *demo_p++;
      map = *demo_p++;
      *demo_p++;
      consoleplayer = *demo_p++;

      if (CheckForOverrun(header_p, demo_p, size, GAME_OPTION_SIZE, failonerror))
        return NULL;

      demo_p = G_ReadOptions(demo_p);

      if (demover == 200)
        demo_p += 256-GAME_OPTION_SIZE;
    }

  if (sizeof(comp_lev_str)/sizeof(comp_lev_str[0]) != MAX_COMPATIBILITY_LEVEL)
    I_Error("G_ReadDemoHeader: compatibility level strings incomplete");
  lprintf(LO_INFO, "G_DoPlayDemo: playing demo with %s compatibility\n",
    comp_lev_str[compatibility_level]);

  if (demo_compatibility)
    {

      if (CheckForOverrun(header_p, demo_p, size, 4, failonerror))
        return NULL;

      for (i=0; i<4; i++)
        playeringame[i] = *demo_p++;
      for (;i < MAXPLAYERS; i++)
        playeringame[i] = 0;
    }
  else
    {

      if (CheckForOverrun(header_p, demo_p, size, MAXPLAYERS, failonerror))
        return NULL;

      for (i=0 ; i < MAXPLAYERS; i++)
        playeringame[i] = *demo_p++;
      demo_p += MIN_MAXPLAYERS - MAXPLAYERS;
    }

  if (gameaction != ga_loadgame) {
    G_InitNew(skill, episode, map);
  }

  for (i=0; i<MAXPLAYERS;i++)
    players[i].cheats = 0;

  return demo_p;
}

void G_DoPlayDemo(void)
{
  char basename[9];

  ExtractFileBase(defdemoname,basename);
  basename[8] = 0;

  demolumpnum = W_GetNumForName(basename);
  demobuffer = W_CacheLumpNum(demolumpnum);
  demolength = W_LumpLength(demolumpnum);

  demo_p = G_ReadDemoHeader(demobuffer, demolength, true);

  gameaction = ga_nothing;
  usergame = false;

  demoplayback = true;
  R_SmoothPlaying_Reset(NULL);

  starttime = I_GetTime_RealTime ();
}

boolean G_CheckDemoStatus (void)
{
  if (demorecording)
    {
      demorecording = false;
      fputc(DEMOMARKER, demofp);
      I_Error("G_CheckDemoStatus: Demo recorded");
      return false;
    }

  if (timingdemo)
    {
      int endtime = I_GetTime_RealTime ();

      unsigned realtics = endtime-starttime;
      I_Error ("Timed %u gametics in %u realtics = %-.1f frames per second",
               (unsigned) gametic,realtics,
               (unsigned) gametic * (double) TICRATE / realtics);
    }

  if (demoplayback)
    {
      if (singledemo)
        exit(0);

      if (demolumpnum != -1) {

  W_UnlockLumpNum(demolumpnum);
  demolumpnum = -1;
      }
      G_ReloadDefaults();
      D_AdvanceDemo ();
      return true;
    }
  return false;
}

#define MAX_MESSAGE_SIZE 1024

void doom_printf(const char *s, ...)
{
  static char msg[MAX_MESSAGE_SIZE];
  va_list v;
  va_start(v,s);
  vsnprintf(msg,sizeof(msg),s,v);
  va_end(v);
  players[consoleplayer].message = msg;
}
