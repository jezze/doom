#include "doomstat.h"
#include "g_game.h"
#include "r_data.h"
#include "p_inter.h"
#include "p_tick.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_englsh.h"
#include "r_main.h"
#include "p_map.h"
#include "p_tick.h"

#define plyr (players+consoleplayer)

static void cheat_mus();
static void cheat_choppers();
static void cheat_god();
static void cheat_fa();
static void cheat_k();
static void cheat_kfa();
static void cheat_noclip();
static void cheat_pw();
static void cheat_behold();
static void cheat_clev();
static void cheat_mypos();
static void cheat_rate();
static void cheat_comp();
static void cheat_friction();
static void cheat_pushers();
static void cheat_tnttran();
static void cheat_massacre();
static void cheat_ddt();
static void cheat_hom();
static void cheat_fast();
static void cheat_tntkey();
static void cheat_tntkeyx();
static void cheat_tntkeyxx();
static void cheat_tntweap();
static void cheat_tntweapx();
static void cheat_tntammo();
static void cheat_tntammox();
static void cheat_smart();
static void cheat_pitch();
static void cheat_megaarmour();
static void cheat_health();

struct cheat_s cheat[] = {
  {"idmus", cheat_mus,      -2},
  {"idchoppers", cheat_choppers },
  {"iddqd", cheat_god      },
  {"idkfa", cheat_kfa },
  {"idfa", cheat_fa  },
  {"idspispopd", cheat_noclip },
  {"idclip", cheat_noclip },
  {"idbeholdh", cheat_health },
  {"idbeholdm", cheat_megaarmour },
  {"idbeholdv", cheat_pw,  pw_invulnerability },
  {"idbeholds", cheat_pw,  pw_strength        },
  {"idbeholdi", cheat_pw,  pw_invisibility    },
  {"idbeholdr", cheat_pw,  pw_ironfeet        },
  {"idbeholda", cheat_pw,  pw_allmap          },
  {"idbeholdl", cheat_pw,  pw_infrared        },
  {"idbehold",  cheat_behold   },
  {"idclev", cheat_clev,    -2},
  {"idmypos", cheat_mypos    },
  {"idrate", cheat_rate     },
  {"tntcomp", cheat_comp     },
  {"tntem", cheat_massacre },
  {"iddt", cheat_ddt      },
  {"tnthom", cheat_hom      },
  {"tntkey", cheat_tntkey   },
  {"tntkeyr", cheat_tntkeyx  },
  {"tntkeyy",cheat_tntkeyx  },
  {"tntkeyb", cheat_tntkeyx  },
  {"tntkeyrc", cheat_tntkeyxx, it_redcard    },
  {"tntkeyyc", cheat_tntkeyxx, it_yellowcard },
  {"tntkeybc", cheat_tntkeyxx, it_bluecard   },
  {"tntkeyrs", cheat_tntkeyxx, it_redskull   },
  {"tntkeyys", cheat_tntkeyxx, it_yellowskull},
  {"tntkeybs", cheat_tntkeyxx, it_blueskull  },
  {"tntka", cheat_k    },
  {"tntweap", cheat_tntweap  },
  {"tntweap", cheat_tntweapx, -1},
  {"tntammo", cheat_tntammo  },
  {"tntammo", cheat_tntammox, -1},
  {"tnttran", cheat_tnttran  },
  {"tntsmart", cheat_smart},
  {"tntpitch", cheat_pitch},
  {"tntran", cheat_tnttran    },
  {"tntamo", cheat_tntammo    },
  {"tntamo", cheat_tntammox, -1},
  {"tntfast", cheat_fast       },
  {"tntice", cheat_friction   },
  {"tntpush", cheat_pushers    },
  {NULL}
};



static void cheat_mus(buf)
char buf[3];
{
  int musnum;




  if (!isdigit(buf[0]) || !isdigit(buf[1]))
    return;

  plyr->message = STSTR_MUS;

  if (gamemode == commercial)
    {
      musnum = mus_runnin + (buf[0]-'0')*10 + buf[1]-'0' - 1;


      if (musnum < mus_runnin ||  ((buf[0]-'0')*10 + buf[1]-'0') > 35)
        plyr->message = STSTR_NOMUS;
      else
        {
          S_ChangeMusic(musnum, 1);
          idmusnum = musnum;
        }
    }
  else
    {
      musnum = mus_e1m1 + (buf[0]-'1')*9 + (buf[1]-'1');


      if (buf[0] < '1' || buf[1] < '1' || ((buf[0]-'1')*9 + buf[1]-'1') > 31)
        plyr->message = STSTR_NOMUS;
      else
        {
          S_ChangeMusic(musnum, 1);
          idmusnum = musnum;
        }
    }
}


static void cheat_choppers()
{
  plyr->weaponowned[wp_chainsaw] = true;
  plyr->powers[pw_invulnerability] = true;
  plyr->message = STSTR_CHOPPERS;
}

static void cheat_god()
{
  plyr->cheats ^= CF_GODMODE;
  if (plyr->cheats & CF_GODMODE)
    {
      if (plyr->mo)
        plyr->mo->health = god_health;

      plyr->health = god_health;
      plyr->message = STSTR_DQDON;
    }
  else
    plyr->message = STSTR_DQDOFF;
}


static void cheat_health()
{
  if (!(plyr->cheats & CF_GODMODE)) {
    if (plyr->mo)
      plyr->mo->health = mega_health;
    plyr->health = mega_health;
    plyr->message = STSTR_BEHOLDX;
  }
}

static void cheat_megaarmour()
{
  plyr->armorpoints = idfa_armor;
  plyr->armortype = idfa_armor_class;
  plyr->message = STSTR_BEHOLDX;
}

static void cheat_fa()
{
  int i;

  if (!plyr->backpack)
    {
      for (i=0 ; i<NUMAMMO ; i++)
        plyr->maxammo[i] *= 2;
      plyr->backpack = true;
    }

  plyr->armorpoints = idfa_armor;
  plyr->armortype = idfa_armor_class;


  for (i=0;i<NUMWEAPONS;i++)
    if (!(((i == wp_plasma || i == wp_bfg) && gamemode == shareware) ||
          (i == wp_supershotgun && gamemode != commercial)))
      plyr->weaponowned[i] = true;

  for (i=0;i<NUMAMMO;i++)
    if (i!=am_cell || gamemode!=shareware)
      plyr->ammo[i] = plyr->maxammo[i];

  plyr->message = STSTR_FAADDED;
}

static void cheat_k()
{
  int i;
  for (i=0;i<NUMCARDS;i++)
    if (!plyr->cards[i])
      {
        plyr->cards[i] = true;
        plyr->message = "Keys Added";
      }
}

static void cheat_kfa()
{
  cheat_k();
  cheat_fa();
  plyr->message = STSTR_KFAADDED;
}

static void cheat_noclip()
{



  plyr->message = (plyr->cheats ^= CF_NOCLIP) & CF_NOCLIP ?
    STSTR_NCON : STSTR_NCOFF;
}


static void cheat_pw(int pw)
{
  if (plyr->powers[pw])
    plyr->powers[pw] = pw!=pw_strength && pw!=pw_allmap;
  else
    {
      P_GivePower(plyr, pw);
      if (pw != pw_strength)
        plyr->powers[pw] = -1;
    }
  plyr->message = STSTR_BEHOLDX;
}


static void cheat_behold()
{
  plyr->message = STSTR_BEHOLD;
}


static void cheat_clev(char buf[3])
{
  int epsd, map;

  if (gamemode == commercial)
    {
      epsd = 1;
      map = (buf[0] - '0')*10 + buf[1] - '0';
    }
  else
    {
      epsd = buf[0] - '0';
      map = buf[1] - '0';
    }


  if (epsd < 1 || map < 1 ||
      (gamemode == retail     && (epsd > 4 || map > 9  )) ||
      (gamemode == registered && (epsd > 3 || map > 9  )) ||
      (gamemode == shareware  && (epsd > 1 || map > 9  )) ||
      (gamemode == commercial && (epsd > 1 || map > 32 )) )
    return;



  idmusnum = -1;

  plyr->message = STSTR_CLEV;

  G_DeferedInitNew(gameskill, epsd, map);
}



static void cheat_mypos()
{
  doom_printf("Position (%d,%d,%d)\tAngle %-.0f",
          players[consoleplayer].mo->x >> FRACBITS,
          players[consoleplayer].mo->y >> FRACBITS,
          players[consoleplayer].mo->z >> FRACBITS,
          players[consoleplayer].mo->angle * (90.0/ANG90));
}


static void cheat_rate()
{
  rendering_stats ^= 1;
}



static void cheat_comp()
{

  compatibility_level++; compatibility_level %= MAX_COMPATIBILITY_LEVEL;


  G_Compatibility();
  doom_printf("New compatibility level:\n%s",
        comp_lev_str[compatibility_level]);
}


static void cheat_friction()
{
  plyr->message =
    (variable_friction = !variable_friction) ? "Variable Friction enabled" :
                                               "Variable Friction disabled";
}




static void cheat_pushers()
{
  plyr->message =
    (allow_pushers = !allow_pushers) ? "Pushers enabled" : "Pushers disabled";
}


static void cheat_tnttran()
{
    R_InitTranMap(0);
}

static void cheat_massacre()
{






  int killcount=0;
  thinker_t *currentthinker = NULL;
  extern void A_PainDie(mobj_t *);


  uint_64_t mask = MF_FRIEND;
  P_MapStart();
  do
    while ((currentthinker = P_NextThinker(currentthinker,th_all)) != NULL)
    if (currentthinker->function == P_MobjThinker &&
  !(((mobj_t *) currentthinker)->flags & mask) &&
        (((mobj_t *) currentthinker)->flags & MF_COUNTKILL ||
         ((mobj_t *) currentthinker)->type == MT_SKULL))
      {
        if (((mobj_t *) currentthinker)->health > 0)
          {
            killcount++;
            P_DamageMobj((mobj_t *)currentthinker, NULL, NULL, 10000);
          }
        if (((mobj_t *) currentthinker)->type == MT_PAIN)
          {
            A_PainDie((mobj_t *) currentthinker);
            P_SetMobjState ((mobj_t *) currentthinker, S_PAIN_DIE6);
          }
      }
  while (!killcount && mask ? mask=0, 1 : 0);
  P_MapEnd();


  doom_printf("%d Monster%s Killed", killcount, killcount==1 ? "" : "s");
}



static void cheat_ddt()
{
  extern int ddt_cheating;
  if (automapmode & am_active)
    ddt_cheating = (ddt_cheating+1) % 3;
}


static void cheat_hom()
{
  extern int autodetect_hom;
  plyr->message = (autodetect_hom = !autodetect_hom) ? "HOM Detection On" :
    "HOM Detection Off";
}


static void cheat_fast()
{
  plyr->message = (fastparm = !fastparm) ? "Fast Monsters On" :
    "Fast Monsters Off";
  G_SetFastParms(fastparm);
}


static void cheat_tntkey()
{
  plyr->message = "Red, Yellow, Blue";
}

static void cheat_tntkeyx()
{
  plyr->message = "Card, Skull";
}

static void cheat_tntkeyxx(int key)
{
  plyr->message = (plyr->cards[key] = !plyr->cards[key]) ?
    "Key Added" : "Key Removed";
}



static void cheat_tntweap()
{
  plyr->message = gamemode==commercial ?
    "Weapon number 1-9" : "Weapon number 1-8";
}

static void cheat_tntweapx(buf)
char buf[3];
{
  int w = *buf - '1';

  if ((w==wp_supershotgun && gamemode!=commercial) ||
      ((w==wp_bfg || w==wp_plasma) && gamemode==shareware))
    return;

  if (w==wp_fist)
    cheat_pw(pw_strength);
  else
    if (w >= 0 && w < NUMWEAPONS) {
      if ((plyr->weaponowned[w] = !plyr->weaponowned[w]))
        plyr->message = "Weapon Added";
      else
        {
          plyr->message = "Weapon Removed";
          if (w==plyr->readyweapon)
            plyr->pendingweapon = P_SwitchWeapon(plyr);
        }
    }
}


static void cheat_tntammo()
{
  plyr->message = "Ammo 1-4, Backpack";
}

static void cheat_tntammox(buf)
char buf[1];
{
  int a = *buf - '1';
  if (*buf == 'b')
    if ((plyr->backpack = !plyr->backpack))
      for (plyr->message = "Backpack Added",   a=0 ; a<NUMAMMO ; a++)
        plyr->maxammo[a] <<= 1;
    else
      for (plyr->message = "Backpack Removed", a=0 ; a<NUMAMMO ; a++)
        {
          if (plyr->ammo[a] > (plyr->maxammo[a] >>= 1))
            plyr->ammo[a] = plyr->maxammo[a];
        }
  else
    if (a>=0 && a<NUMAMMO)
      {
        a = a==am_cell ? am_misl : a==am_misl ? am_cell : a;
        plyr->message = (plyr->ammo[a] = !plyr->ammo[a]) ?
          plyr->ammo[a] = plyr->maxammo[a], "Ammo Added" : "Ammo Removed";
      }
}

static void cheat_smart()
{
  plyr->message = (monsters_remember = !monsters_remember) ?
    "Smart Monsters Enabled" : "Smart Monsters Disabled";
}

static void cheat_pitch()
{
  plyr->message=(pitched_sounds = !pitched_sounds) ? "Pitch Effects Enabled" :
    "Pitch Effects Disabled";
}






#define CHEAT_ARGS_MAX 8  /* Maximum number of args at end of cheats */

boolean M_FindCheats(int key)
{
  static uint_64_t sr;
  static char argbuf[CHEAT_ARGS_MAX+1], *arg;
  static int init, argsleft, cht;
  int i, ret, matchedbefore;




  if (argsleft)
    {
      *arg++ = tolower(key);
      if (!--argsleft)
        cheat[cht].func(argbuf);
      return 1;
    }

  key = tolower(key) - 'a';
  if (key < 0 || key >= 32)
    {
      sr = 0;
      return 0;
    }

  if (!init)
    {
      init = 1;
      for (i=0;cheat[i].cheat;i++)
        {
          uint_64_t c=0, m=0;
          const char *p;

          for (p=cheat[i].cheat; *p; p++)
            {
              unsigned key = tolower(*p)-'a';
              if (key >= 32)
                continue;
              c = (c<<5) + key;
              m = (m<<5) + 31;
            }
          cheat[i].code = c;
          cheat[i].mask = m;
        }
    }

  sr = (sr<<5) + key;

  for (matchedbefore = ret = i = 0; cheat[i].cheat; i++)
    if ((sr & cheat[i].mask) == cheat[i].code) {
      if (cheat[i].arg < 0)
        {
          cht = i;
          arg = argbuf;
          argsleft = -cheat[i].arg;
          ret = 1;
        }
      else
        if (!matchedbefore)
          {
            matchedbefore = ret = 1;
            cheat[i].func(cheat[i].arg);
          }
    }
  return ret;
}
