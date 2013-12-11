#include "doomstat.h"
#include "hu_stuff.h"
#include "hu_lib.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "sounds.h"
#include "g_game.h"
#include "r_main.h"

int hud_active;
int hud_displayed;
int hud_nosecrets;
int hud_distributed;
int hud_graph_keys=1;

const char *const mapnames[] =
{
  HUSTR_E1M1,
  HUSTR_E1M2,
  HUSTR_E1M3,
  HUSTR_E1M4,
  HUSTR_E1M5,
  HUSTR_E1M6,
  HUSTR_E1M7,
  HUSTR_E1M8,
  HUSTR_E1M9,

  HUSTR_E2M1,
  HUSTR_E2M2,
  HUSTR_E2M3,
  HUSTR_E2M4,
  HUSTR_E2M5,
  HUSTR_E2M6,
  HUSTR_E2M7,
  HUSTR_E2M8,
  HUSTR_E2M9,

  HUSTR_E3M1,
  HUSTR_E3M2,
  HUSTR_E3M3,
  HUSTR_E3M4,
  HUSTR_E3M5,
  HUSTR_E3M6,
  HUSTR_E3M7,
  HUSTR_E3M8,
  HUSTR_E3M9,

  HUSTR_E4M1,
  HUSTR_E4M2,
  HUSTR_E4M3,
  HUSTR_E4M4,
  HUSTR_E4M5,
  HUSTR_E4M6,
  HUSTR_E4M7,
  HUSTR_E4M8,
  HUSTR_E4M9,

};

const char *const mapnames2[] =
{
  HUSTR_1,
  HUSTR_2,
  HUSTR_3,
  HUSTR_4,
  HUSTR_5,
  HUSTR_6,
  HUSTR_7,
  HUSTR_8,
  HUSTR_9,
  HUSTR_10,
  HUSTR_11,

  HUSTR_12,
  HUSTR_13,
  HUSTR_14,
  HUSTR_15,
  HUSTR_16,
  HUSTR_17,
  HUSTR_18,
  HUSTR_19,
  HUSTR_20,

  HUSTR_21,
  HUSTR_22,
  HUSTR_23,
  HUSTR_24,
  HUSTR_25,
  HUSTR_26,
  HUSTR_27,
  HUSTR_28,
  HUSTR_29,
  HUSTR_30,
  HUSTR_31,
  HUSTR_32,
};

const char *const mapnamesp[] =
{
  PHUSTR_1,
  PHUSTR_2,
  PHUSTR_3,
  PHUSTR_4,
  PHUSTR_5,
  PHUSTR_6,
  PHUSTR_7,
  PHUSTR_8,
  PHUSTR_9,
  PHUSTR_10,
  PHUSTR_11,

  PHUSTR_12,
  PHUSTR_13,
  PHUSTR_14,
  PHUSTR_15,
  PHUSTR_16,
  PHUSTR_17,
  PHUSTR_18,
  PHUSTR_19,
  PHUSTR_20,

  PHUSTR_21,
  PHUSTR_22,
  PHUSTR_23,
  PHUSTR_24,
  PHUSTR_25,
  PHUSTR_26,
  PHUSTR_27,
  PHUSTR_28,
  PHUSTR_29,
  PHUSTR_30,
  PHUSTR_31,
  PHUSTR_32,
};

const char *const mapnamest[] =
{
  THUSTR_1,
  THUSTR_2,
  THUSTR_3,
  THUSTR_4,
  THUSTR_5,
  THUSTR_6,
  THUSTR_7,
  THUSTR_8,
  THUSTR_9,
  THUSTR_10,
  THUSTR_11,

  THUSTR_12,
  THUSTR_13,
  THUSTR_14,
  THUSTR_15,
  THUSTR_16,
  THUSTR_17,
  THUSTR_18,
  THUSTR_19,
  THUSTR_20,

  THUSTR_21,
  THUSTR_22,
  THUSTR_23,
  THUSTR_24,
  THUSTR_25,
  THUSTR_26,
  THUSTR_27,
  THUSTR_28,
  THUSTR_29,
  THUSTR_30,
  THUSTR_31,
  THUSTR_32,
};

#define HU_TITLE  (mapnames[(gameepisode-1)*9+gamemap-1])
#define HU_TITLE2 (mapnames2[gamemap-1])
#define HU_TITLEP (mapnamesp[gamemap-1])
#define HU_TITLET (mapnamest[gamemap-1])
#define HU_TITLEHEIGHT  1
#define HU_TITLEX 0
#define HU_TITLEY ((200-ST_HEIGHT) - 1 - hu_font[0].height)
#define HU_COORDX (320 - 13*hu_font2['A'-HU_FONTSTART].width)
#define HU_COORDX_Y (1 + 0*hu_font['A'-HU_FONTSTART].height)
#define HU_COORDY_Y (2 + 1*hu_font['A'-HU_FONTSTART].height)
#define HU_COORDZ_Y (3 + 2*hu_font['A'-HU_FONTSTART].height)
#define HU_GAPY 8
#define HU_HUDHEIGHT (6*HU_GAPY)
#define HU_HUDX 2
#define HU_HUDY (200-HU_HUDHEIGHT-1)
#define HU_MONSECX (HU_HUDX)
#define HU_MONSECY (HU_HUDY+0*HU_GAPY)
#define HU_KEYSX   (HU_HUDX)
#define HU_KEYSGX  (HU_HUDX+4*hu_font2['A'-HU_FONTSTART].width)
#define HU_KEYSY   (HU_HUDY+1*HU_GAPY)
#define HU_WEAPX   (HU_HUDX)
#define HU_WEAPY   (HU_HUDY+2*HU_GAPY)
#define HU_AMMOX   (HU_HUDX)
#define HU_AMMOY   (HU_HUDY+3*HU_GAPY)
#define HU_HEALTHX (HU_HUDX)
#define HU_HEALTHY (HU_HUDY+4*HU_GAPY)
#define HU_ARMORX  (HU_HUDX)
#define HU_ARMORY  (HU_HUDY+5*HU_GAPY)
#define HU_HUDX_LL 2
#define HU_HUDY_LL (200-2*HU_GAPY-1)
#define HU_HUDX_LR (320-120)
#define HU_HUDY_LR (200-2*HU_GAPY-1)
#define HU_HUDX_UR (320-96)
#define HU_HUDY_UR 2
#define HU_MONSECX_D (HU_HUDX_LL)
#define HU_MONSECY_D (HU_HUDY_LL+0*HU_GAPY)
#define HU_KEYSX_D   (HU_HUDX_LL)
#define HU_KEYSGX_D  (HU_HUDX_LL+4*hu_font2['A'-HU_FONTSTART].width)
#define HU_KEYSY_D   (HU_HUDY_LL+1*HU_GAPY)
#define HU_WEAPX_D   (HU_HUDX_LR)
#define HU_WEAPY_D   (HU_HUDY_LR+0*HU_GAPY)
#define HU_AMMOX_D   (HU_HUDX_LR)
#define HU_AMMOY_D   (HU_HUDY_LR+1*HU_GAPY)
#define HU_HEALTHX_D (HU_HUDX_UR)
#define HU_HEALTHY_D (HU_HUDY_UR+0*HU_GAPY)
#define HU_ARMORX_D  (HU_HUDX_UR)
#define HU_ARMORY_D  (HU_HUDY_UR+1*HU_GAPY)
#define HU_INPUTX HU_MSGX
#define HU_INPUTY (HU_MSGY + HU_MSGHEIGHT*(hu_font[0].height) +1)
#define HU_INPUTWIDTH 64
#define HU_INPUTHEIGHT  1
#define key_alt KEYD_RALT
#define key_shift KEYD_RSHIFT

const char* player_names[] =


{
  HUSTR_PLRGREEN,
  HUSTR_PLRINDIGO,
  HUSTR_PLRBROWN,
  HUSTR_PLRRED
};


int plyrcoltran[MAXPLAYERS]={CR_GREEN,CR_GRAY,CR_BROWN,CR_RED};

char chat_char;
static player_t*  plr;

patchnum_t hu_font[HU_FONTSIZE];
patchnum_t hu_font2[HU_FONTSIZE];
patchnum_t hu_fontk[HU_FONTSIZE];
patchnum_t hu_msgbg[9];

static hu_textline_t  w_title;
static hu_stext_t     w_message;
static hu_itext_t     w_chat;
static hu_itext_t     w_inputbuffer[MAXPLAYERS];
static hu_textline_t  w_coordx;
static hu_textline_t  w_coordy;
static hu_textline_t  w_coordz;
static hu_textline_t  w_ammo;
static hu_textline_t  w_health;
static hu_textline_t  w_armor;
static hu_textline_t  w_weapon;
static hu_textline_t  w_keys;
static hu_textline_t  w_gkeys;
static hu_textline_t  w_monsec;
static hu_mtext_t     w_rtext;

static boolean    always_off = false;
static char       chat_dest[MAXPLAYERS];
boolean           chat_on;
static boolean    message_on;
static boolean    message_list;
boolean           message_dontfuckwithme;
static boolean    message_nottobefuckedwith;
static int        message_counter;
extern int        showMessages;
extern boolean    automapactive;
static boolean    headsupactive = false;


int hudcolor_titl;
int hudcolor_xyco;

int hudcolor_mesg;
int hudcolor_chat;
int hud_msg_lines;

int hudcolor_list;
int hud_list_bgon;


static char hud_coordstrx[32];
static char hud_coordstry[32];
static char hud_coordstrz[32];
static char hud_ammostr[80];
static char hud_healthstr[80];
static char hud_armorstr[80];
static char hud_weapstr[80];
static char hud_keysstr[80];
static char hud_gkeysstr[80];
static char hud_monsecstr[80];

extern int map_point_coordinates;

const char* shiftxform;

const char english_shiftxform[] =
{
  0,
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
  11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31,
  ' ', '!', '"', '#', '$', '%', '&',
  '"',
  '(', ')', '*', '+',
  '<',
  '_',
  '>',
  '?',
  ')',
  '!',
  '@',
  '#',
  '$',
  '%',
  '^',
  '&',
  '*',
  '(',
  ':',
  ':',
  '<',
  '+',
  '>', '?', '@',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '[',
  '!',
  ']',
  '"', '_',
  '\'',
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
  'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  '{', '|', '}', '~', 127
};

void HU_Init(void)
{

  int   i;
  int   j;
  char  buffer[9];

  shiftxform = english_shiftxform;


  j = HU_FONTSTART;
  for (i=0;i<HU_FONTSIZE;i++,j++)
  {
    if ('0'<=j && j<='9')
    {
      sprintf(buffer, "DIG%.1d",j-48);
      R_SetPatchNum(&hu_font2[i], buffer);
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if ('A'<=j && j<='Z')
    {
      sprintf(buffer, "DIG%c",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else if (j=='-')
    {
      R_SetPatchNum(&hu_font2[i], "DIG45");
      R_SetPatchNum(&hu_font[i], "STCFN045");
    }
    else if (j=='/')
    {
      R_SetPatchNum(&hu_font2[i], "DIG47");
      R_SetPatchNum(&hu_font[i], "STCFN047");
    }
    else if (j==':')
    {
      R_SetPatchNum(&hu_font2[i], "DIG58");
      R_SetPatchNum(&hu_font[i], "STCFN058");
    }
    else if (j=='[')
    {
      R_SetPatchNum(&hu_font2[i], "DIG91");
      R_SetPatchNum(&hu_font[i], "STCFN091");
    }
    else if (j==']')
    {
      R_SetPatchNum(&hu_font2[i], "DIG93");
      R_SetPatchNum(&hu_font[i], "STCFN093");
    }
    else if (j<97)
    {
      sprintf(buffer, "STCFN%.3d",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      R_SetPatchNum(&hu_font[i], buffer);

    }
    else if (j>122)
    {
      sprintf(buffer, "STBR%.3d",j);
      R_SetPatchNum(&hu_font2[i], buffer);
      R_SetPatchNum(&hu_font[i], buffer);
    }
    else
      hu_font[i] = hu_font[0];
  }


  for (i=0; i<9; i++) {
    sprintf(buffer, "BOX%c%c", "UCL"[i/3], "LCR"[i%3]);
    R_SetPatchNum(&hu_msgbg[i], buffer);
  }


  for (i=0; i<6; i++) {
    sprintf(buffer, "STKEYS%d", i);
    R_SetPatchNum(&hu_fontk[i], buffer);
  }
}

static void HU_Stop(void)
{
  headsupactive = false;
}

void HU_Start(void)
{

  int   i;
  const char* s; /* cph - const */

  if (headsupactive)
    HU_Stop();

  plr = &players[displayplayer];
  message_on = false;
  message_dontfuckwithme = false;
  message_nottobefuckedwith = false;
  chat_on = false;



  HUlib_initSText
  (
    &w_message,
    HU_MSGX,
    HU_MSGY,
    HU_MSGHEIGHT,
    hu_font,
    HU_FONTSTART,
    hudcolor_mesg,
    &message_on
  );

  HUlib_initTextLine
  (
    &w_title,
    HU_TITLEX,
    HU_TITLEY,
    hu_font,
    HU_FONTSTART,
    hudcolor_titl
  );

  HUlib_initTextLine
  (
    &w_health,
    hud_distributed? HU_HEALTHX_D : HU_HEALTHX,
    hud_distributed? HU_HEALTHY_D : HU_HEALTHY,
    hu_font2,
    HU_FONTSTART,
    CR_GREEN
  );

  HUlib_initTextLine
  (
    &w_armor,
    hud_distributed? HU_ARMORX_D : HU_ARMORX,
    hud_distributed? HU_ARMORY_D : HU_ARMORY,
    hu_font2,
    HU_FONTSTART,
    CR_GREEN
  );

  HUlib_initTextLine
  (
    &w_ammo,
    hud_distributed? HU_AMMOX_D : HU_AMMOX,
    hud_distributed? HU_AMMOY_D : HU_AMMOY,
    hu_font2,
    HU_FONTSTART,
    CR_GOLD
  );

  HUlib_initTextLine
  (
    &w_weapon,
    hud_distributed? HU_WEAPX_D : HU_WEAPX,
    hud_distributed? HU_WEAPY_D : HU_WEAPY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  HUlib_initTextLine
  (
    &w_keys,
    hud_distributed? HU_KEYSX_D : HU_KEYSX,
    hud_distributed? HU_KEYSY_D : HU_KEYSY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  HUlib_initTextLine
  (
    &w_gkeys,
    hud_distributed? HU_KEYSGX_D : HU_KEYSGX,
    hud_distributed? HU_KEYSY_D : HU_KEYSY,
    hu_fontk,
    HU_FONTSTART,
    CR_RED
  );

  HUlib_initTextLine
  (
    &w_monsec,
    hud_distributed? HU_MONSECX_D : HU_MONSECX,
    hud_distributed? HU_MONSECY_D : HU_MONSECY,
    hu_font2,
    HU_FONTSTART,
    CR_GRAY
  );

  if (hud_msg_lines>HU_MAXMESSAGES)
    hud_msg_lines=HU_MAXMESSAGES;

  message_list = hud_msg_lines > 1;

  HUlib_initMText
  (
    &w_rtext,
    0,
    0,
    320,

    (hud_msg_lines+2)*HU_REFRESHSPACING,
    hu_font,
    HU_FONTSTART,
    hudcolor_list,
    hu_msgbg,
    &message_list
  );


  if (gamestate == GS_LEVEL) /* cph - stop SEGV here when not in level */
  switch (gamemode)
  {
    case shareware:
    case registered:
    case retail:
      s = HU_TITLE;
      break;

    case commercial:
    default:
      s = (gamemission==pack_tnt)  ? HU_TITLET :
          (gamemission==pack_plut) ? HU_TITLEP : HU_TITLE2;
      break;
  } else s = "";
  while (*s)
    HUlib_addCharToTextLine(&w_title, *(s++));




  HUlib_initTextLine
  (
    &w_coordx,
    HU_COORDX,
    HU_COORDX_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );
  HUlib_initTextLine
  (
    &w_coordy,
    HU_COORDX,
    HU_COORDY_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );
  HUlib_initTextLine
  (
    &w_coordz,
    HU_COORDX,
    HU_COORDZ_Y,
    hu_font,
    HU_FONTSTART,
    hudcolor_xyco
  );

  if (map_point_coordinates)
  {
    sprintf(hud_coordstrx,"X: %-5d",0);
    s = hud_coordstrx;
    while (*s)
      HUlib_addCharToTextLine(&w_coordx, *(s++));
    sprintf(hud_coordstry,"Y: %-5d",0);
    s = hud_coordstry;
    while (*s)
      HUlib_addCharToTextLine(&w_coordy, *(s++));
    sprintf(hud_coordstrz,"Z: %-5d",0);
    s = hud_coordstrz;
    while (*s)
      HUlib_addCharToTextLine(&w_coordz, *(s++));
  }


  strcpy(hud_ammostr,"AMM ");
  s = hud_ammostr;
  while (*s)
    HUlib_addCharToTextLine(&w_ammo, *(s++));


  strcpy(hud_healthstr,"HEL ");
  s = hud_healthstr;
  while (*s)
    HUlib_addCharToTextLine(&w_health, *(s++));


  strcpy(hud_armorstr,"ARM ");
  s = hud_armorstr;
  while (*s)
    HUlib_addCharToTextLine(&w_armor, *(s++));


  strcpy(hud_weapstr,"WEA ");
  s = hud_weapstr;
  while (*s)
    HUlib_addCharToTextLine(&w_weapon, *(s++));


  strcpy(hud_keysstr,"KEY ");
  s = hud_keysstr;
  while (*s)
    HUlib_addCharToTextLine(&w_keys, *(s++));


  strcpy(hud_gkeysstr," ");
  s = hud_gkeysstr;
  while (*s)
    HUlib_addCharToTextLine(&w_gkeys, *(s++));


  strcpy(hud_monsecstr,"STS ");
  s = hud_monsecstr;
  while (*s)
    HUlib_addCharToTextLine(&w_monsec, *(s++));


  HUlib_initIText
  (
    &w_chat,
    HU_INPUTX,
    HU_INPUTY,
    hu_font,
    HU_FONTSTART,
    hudcolor_chat,
    &chat_on
  );


  for (i=0 ; i<MAXPLAYERS ; i++)
    HUlib_initIText
    (
      &w_inputbuffer[i],
      0,
      0,
      0,
      0,
      hudcolor_chat,
      &always_off
    );

  headsupactive = true;
}

void HU_MoveHud(void)
{
  static int ohud_distributed=-1;

  if (hud_distributed!=ohud_distributed)
  {
    w_ammo.x =    hud_distributed? HU_AMMOX_D   : HU_AMMOX;
    w_ammo.y =    hud_distributed? HU_AMMOY_D   : HU_AMMOY;
    w_weapon.x =  hud_distributed? HU_WEAPX_D   : HU_WEAPX;
    w_weapon.y =  hud_distributed? HU_WEAPY_D   : HU_WEAPY;
    w_keys.x =    hud_distributed? HU_KEYSX_D   : HU_KEYSX;
    w_keys.y =    hud_distributed? HU_KEYSY_D   : HU_KEYSY;
    w_gkeys.x =   hud_distributed? HU_KEYSGX_D  : HU_KEYSGX;
    w_gkeys.y =   hud_distributed? HU_KEYSY_D   : HU_KEYSY;
    w_monsec.x =  hud_distributed? HU_MONSECX_D : HU_MONSECX;
    w_monsec.y =  hud_distributed? HU_MONSECY_D : HU_MONSECY;
    w_health.x =  hud_distributed? HU_HEALTHX_D : HU_HEALTHX;
    w_health.y =  hud_distributed? HU_HEALTHY_D : HU_HEALTHY;
    w_armor.x =   hud_distributed? HU_ARMORX_D  : HU_ARMORX;
    w_armor.y =   hud_distributed? HU_ARMORY_D  : HU_ARMORY;
  }
  ohud_distributed = hud_distributed;
}

void HU_Drawer(void)
{
  char *s;
  player_t *plr;
  char ammostr[80];
  char healthstr[80];
  char armorstr[80];
  int i,doit;

  plr = &players[displayplayer];

  if (hud_active>0 && hud_displayed && viewheight==SCREENHEIGHT)
  {
    doit = !(gametic&1);
    if (doit)
    {
      HU_MoveHud();

      HUlib_clearTextLine(&w_ammo);
      strcpy(hud_ammostr,"AMM ");
      if (weaponinfo[plr->readyweapon].ammo == am_noammo)
      {
        strcat(hud_ammostr,"\x7f\x7f\x7f\x7f\x7f\x7f\x7f N/A");
        w_ammo.cm = CR_GRAY;
      }
      else
      {
        int ammo = plr->ammo[weaponinfo[plr->readyweapon].ammo];
        int fullammo = plr->maxammo[weaponinfo[plr->readyweapon].ammo];
        int ammopct = (100*ammo)/fullammo;
        int ammobars = ammopct/4;

        sprintf(ammostr,"%d/%d",ammo,fullammo);

        for (i=4;i<4+ammobars/4;)
          hud_ammostr[i++] = 123;

        switch(ammobars%4)
        {
          case 0:
            break;
          case 1:
            hud_ammostr[i++] = 126;
            break;
          case 2:
            hud_ammostr[i++] = 125;
            break;
          case 3:
            hud_ammostr[i++] = 124;
            break;
        }

        while(i<4+7)
          hud_ammostr[i++] = 127;
        hud_ammostr[i] = '\0';
        strcat(hud_ammostr,ammostr);


        if (ammopct<ammo_red)
          w_ammo.cm = CR_RED;
        else if (ammopct<ammo_yellow)
          w_ammo.cm = CR_GOLD;
        else
          w_ammo.cm = CR_GREEN;
      }

      s = hud_ammostr;
      while (*s)
        HUlib_addCharToTextLine(&w_ammo, *(s++));
    }

    HUlib_drawTextLine(&w_ammo, false);

    if (doit)
    {
      int health = plr->health;
      int healthbars = health>100? 25 : health/4;

      HUlib_clearTextLine(&w_health);

      sprintf(healthstr,"%3d",health);

      for (i=4;i<4+healthbars/4;)
        hud_healthstr[i++] = 123;

      switch(healthbars%4)
      {
        case 0:
          break;
        case 1:
          hud_healthstr[i++] = 126;
          break;
        case 2:
          hud_healthstr[i++] = 125;
          break;
        case 3:
          hud_healthstr[i++] = 124;
          break;
      }

      while(i<4+7)
        hud_healthstr[i++] = 127;
      hud_healthstr[i] = '\0';
      strcat(hud_healthstr,healthstr);


      if (health<health_red)
        w_health.cm = CR_RED;
      else if (health<health_yellow)
        w_health.cm = CR_GOLD;
      else if (health<=health_green)
        w_health.cm = CR_GREEN;
      else
        w_health.cm = CR_BLUE;


      s = hud_healthstr;
      while (*s)
        HUlib_addCharToTextLine(&w_health, *(s++));
    }

    HUlib_drawTextLine(&w_health, false);


    if (doit)
    {
      int armor = plr->armorpoints;
      int armorbars = armor>100? 25 : armor/4;


      HUlib_clearTextLine(&w_armor);

      sprintf(armorstr,"%3d",armor);


      for (i=4;i<4+armorbars/4;)
        hud_armorstr[i++] = 123;

      switch(armorbars%4)
      {
        case 0:
          break;
        case 1:
          hud_armorstr[i++] = 126;
          break;
        case 2:
          hud_armorstr[i++] = 125;
          break;
        case 3:
          hud_armorstr[i++] = 124;
          break;
      }

      while(i<4+7)
        hud_armorstr[i++] = 127;
      hud_armorstr[i] = '\0';
      strcat(hud_armorstr,armorstr);


      if (armor<armor_red)
        w_armor.cm = CR_RED;
      else if (armor<armor_yellow)
        w_armor.cm = CR_GOLD;
      else if (armor<=armor_green)
        w_armor.cm = CR_GREEN;
      else
        w_armor.cm = CR_BLUE;


      s = hud_armorstr;
      while (*s)
        HUlib_addCharToTextLine(&w_armor, *(s++));
    }

    HUlib_drawTextLine(&w_armor, false);


    if (doit)
    {
      int w;
      int ammo,fullammo,ammopct;


      HUlib_clearTextLine(&w_weapon);
      i=4; hud_weapstr[i] = '\0';


      for (w=0;w<=wp_supershotgun;w++)
      {
        int ok=1;

        switch (gamemode)
        {
          case shareware:
            if (w>=wp_plasma && w!=wp_chainsaw)
              ok=0;
            break;
          case retail:
          case registered:
            if (w>=wp_supershotgun)
              ok=0;
            break;
          default:
          case commercial:
            break;
        }
        if (!ok) continue;

        ammo = plr->ammo[weaponinfo[w].ammo];
        fullammo = plr->maxammo[weaponinfo[w].ammo];
        ammopct=0;


        if (!plr->weaponowned[w])
          continue;

        ammopct = fullammo? (100*ammo)/fullammo : 100;


        hud_weapstr[i++] = '\x1b';
        if (weaponinfo[w].ammo==am_noammo)
          hud_weapstr[i++] = plr->powers[pw_strength]? '0'+CR_GREEN : '0'+CR_GRAY;
        else if (ammopct<ammo_red)
          hud_weapstr[i++] = '0'+CR_RED;
        else if (ammopct<ammo_yellow)
          hud_weapstr[i++] = '0'+CR_GOLD;
        else
          hud_weapstr[i++] = '0'+CR_GREEN;
        hud_weapstr[i++] = '0'+w+1;
        hud_weapstr[i++] = ' ';
        hud_weapstr[i] = '\0';
      }


      s = hud_weapstr;
      while (*s)
        HUlib_addCharToTextLine(&w_weapon, *(s++));
    }

    HUlib_drawTextLine(&w_weapon, false);

    if (doit && hud_active>1)
    {
      int k;

      hud_keysstr[4] = '\0';

      if (hud_graph_keys)
      {
        i=0;
        hud_gkeysstr[i] = '\0';

        for (k=0;k<6;k++)
        {

          if (!plr->cards[k])
            continue;

          hud_gkeysstr[i++] = '!'+k;
          hud_gkeysstr[i++] = ' ';
          hud_gkeysstr[i++] = ' ';
        }
        hud_gkeysstr[i]='\0';
      }
      else
      {
        i=4;
        hud_keysstr[i] = '\0';

        {

          for (k=0;k<6;k++)
          {

            if (!plr->cards[k])
              continue;


            hud_keysstr[i++] = '\x1b';
            switch(k)
            {
              case 0:
                hud_keysstr[i++] = '0'+CR_BLUE;
                hud_keysstr[i++] = 'B';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 1:
                hud_keysstr[i++] = '0'+CR_GOLD;
                hud_keysstr[i++] = 'Y';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 2:
                hud_keysstr[i++] = '0'+CR_RED;
                hud_keysstr[i++] = 'R';
                hud_keysstr[i++] = 'C';
                hud_keysstr[i++] = ' ';
                break;
              case 3:
                hud_keysstr[i++] = '0'+CR_BLUE;
                hud_keysstr[i++] = 'B';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
            case 4:
                hud_keysstr[i++] = '0'+CR_GOLD;
                hud_keysstr[i++] = 'Y';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
              case 5:
                hud_keysstr[i++] = '0'+CR_RED;
                hud_keysstr[i++] = 'R';
                hud_keysstr[i++] = 'S';
                hud_keysstr[i++] = ' ';
                break;
            }
            hud_keysstr[i]='\0';
          }
        }
      }
    }

    if (hud_active>1)
    {
      HUlib_clearTextLine(&w_keys);
      HUlib_clearTextLine(&w_gkeys);


      s = hud_keysstr;
      while (*s)
        HUlib_addCharToTextLine(&w_keys, *(s++));
      HUlib_drawTextLine(&w_keys, false);

        s = hud_gkeysstr;
        while (*s)
          HUlib_addCharToTextLine(&w_gkeys, *(s++));

        HUlib_drawTextLine(&w_gkeys, false);
    }

    if (!hud_nosecrets)
    {
      if (hud_active>1 && doit)
      {

        HUlib_clearTextLine(&w_monsec);


        sprintf
        (
          hud_monsecstr,
          "STS \x1b\x36K \x1b\x33%d \x1b\x36M \x1b\x33%d \x1b\x37I \x1b\x33%d/%d \x1b\x35S \x1b\x33%d/%d",
          plr->killcount,totallive,
          plr->itemcount,totalitems,
          plr->secretcount,totalsecret
        );

        s = hud_monsecstr;
        while (*s)
          HUlib_addCharToTextLine(&w_monsec, *(s++));
      }

      if (hud_active>1)
        HUlib_drawTextLine(&w_monsec, false);
    }
  }


  HU_Erase();



  if (hud_msg_lines<=1)
    message_list = false;


  if (!message_list)
    HUlib_drawSText(&w_message);


  if (hud_msg_lines>1 && message_list)
    HUlib_drawMText(&w_rtext);


  HUlib_drawIText(&w_chat);
}

void HU_Erase(void)
{

  if (!message_list)
    HUlib_eraseSText(&w_message);
  else
    HUlib_eraseMText(&w_rtext);


  HUlib_eraseIText(&w_chat);


  HUlib_eraseTextLine(&w_title);
}

static boolean bsdown;
static int bscounter;

void HU_Ticker(void)
{
  int i, rc;
  char c;


  if (message_counter && !--message_counter)
  {
    message_on = false;
    message_nottobefuckedwith = false;
  }
  if (bsdown && bscounter++ > 9) {
    HUlib_keyInIText(&w_chat, (unsigned char)key_backspace);
    bscounter = 8;
  }

  if (showMessages || message_dontfuckwithme)
  {

    if ((plr->message && !message_nottobefuckedwith)
        || (plr->message && message_dontfuckwithme))
    {

      HUlib_addMessageToSText(&w_message, 0, plr->message);

      HUlib_addMessageToMText(&w_rtext, 0, plr->message);


      plr->message = 0;

      message_on = true;

      message_counter = HU_MSGTIMEOUT;

      message_nottobefuckedwith = message_dontfuckwithme;

      message_dontfuckwithme = 0;
    }
  }

}

#define QUEUESIZE   128

static char chatchars[QUEUESIZE];
static int  head = 0;
static int  tail = 0;

char HU_dequeueChatChar(void)
{
  char c;

  if (head != tail)
  {
    c = chatchars[tail];
    tail = (tail + 1) & (QUEUESIZE-1);
  }
  else
  {
    c = 0;
  }
  return c;
}

boolean HU_Responder(event_t *ev)
{

  static char   lastmessage[HU_MAXLINELENGTH+1];
  boolean   eatkey = false;
  static boolean  shiftdown = false;
  static boolean  altdown = false;
  unsigned char   c;
  int     i;
  int     numplayers;

  static int    num_nobrainers = 0;

  numplayers = 0;
  for (i=0 ; i<MAXPLAYERS ; i++)
    numplayers += playeringame[i];

  if (ev->data1 == key_shift)
  {
    shiftdown = ev->type == ev_keydown;
    return false;
  }
  else if (ev->data1 == key_alt)
  {
    altdown = ev->type == ev_keydown;
    return false;
  }
  else if (ev->data1 == key_backspace)
  {
    bsdown = ev->type == ev_keydown;
    bscounter = 0;
  }

  if (ev->type != ev_keydown)
    return false;

  if (!chat_on)
  {
    if (ev->data1 == key_enter)
    {
      if (hud_msg_lines>1)
      {
        if (message_list) HU_Erase();
        message_list = !message_list;
      }
      if (!message_list)
      {
        message_on = true;
        message_counter = HU_MSGTIMEOUT;
      }
      eatkey = true;
    }

  }
  return eatkey;
}
