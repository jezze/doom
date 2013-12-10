#include <stdio.h>
#include <fcntl.h>
#include "doomdef.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "d_main.h"
#include "v_video.h"
#include "w_wad.h"
#include "r_main.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "sounds.h"
#include "m_menu.h"
#include "m_misc.h"
#include "lprintf.h"
#include "am_map.h"
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "r_demo.h"
#include "r_fps.h"

extern patchnum_t hu_font[HU_FONTSIZE];
extern boolean  message_dontfuckwithme;
extern boolean chat_on;
int mouseSensitivity_horiz;
int mouseSensitivity_vert;
int showMessages;
int hide_setup=1;
int screenblocks;
int screenSize;
int quickSaveSlot;
int messageToPrint;
static const char* messageString;
int     messx;
int     messy;
int     messageLastMenuActive;
boolean messageNeedsInput;

void (*messageRoutine)(int response);

#define SAVESTRINGSIZE  24

static int allow_changes(void)
{
 return !(demoplayback || demorecording);
}

static void M_UpdateCurrent(default_t* def)
{
  if (def->current) {
    if (allow_changes())
  *def->current = *def->location.pi;
    else if (*def->current != *def->location.pi)
  warn_about_changes(S_LEVWARN);
  }
}

int warning_about_changes, print_warning_about_changes;

#define M_DrawBackground V_DrawBackground

int saveStringEnter;
int saveSlot;
int saveCharIndex;

char saveOldString[SAVESTRINGSIZE];

boolean inhelpscreens;

boolean menuactive;

#define SKULLXOFF  -32
#define LINEHEIGHT  16

char savegamestrings[10][SAVESTRINGSIZE];

typedef struct
{
  short status;
  char  name[10];
  void  (*routine)(int choice);
} menuitem_t;

typedef struct menu_s
{
  short           numitems;
  struct menu_s*  prevMenu;
  menuitem_t*     menuitems;
  void            (*routine)();
  short           x;
  short           y;
  short           lastOn;
} menu_t;

short itemOn;
short skullAnimCounter;
short whichSkull;
const char skullName[2][9] = {"M_SKULL1","M_SKULL2"};
menu_t* currentMenu;
extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;
extern int mousebbackward;
int mapcolor_me;
extern int map_point_coordinates;
extern char* chat_macros[];
extern const char* shiftxform;
extern default_t defaults[];
extern int numdefaults;

void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_LoadGame(int choice);
void M_SaveGame(int choice);
void M_Options(int choice);
void M_QuitDOOM(int choice);
void M_ChangeMessages(int choice);
void M_ChangeSensitivity(int choice);
void M_SfxVol(int choice);
void M_MusicVol(int choice);
void M_SizeDisplay(int choice);
void M_StartGame(int choice);
void M_Sound(int choice);
void M_Mouse(int choice, int *sens);
void M_MouseVert(int choice);
void M_MouseHoriz(int choice);
void M_DrawMouse(void);
void M_LoadSelect(int choice);
void M_SaveSelect(int choice);
void M_ReadSaveStrings(void);
void M_QuickSave(void);
void M_QuickLoad(void);
void M_DrawMainMenu(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_DrawOptions(void);
void M_DrawSound(void);
void M_DrawLoad(void);
void M_DrawSave(void);
void M_DrawSetup(void);
void M_DrawSaveLoadBorder(int x,int y);
void M_SetupNextMenu(menu_t *menudef);
void M_DrawThermo(int x,int y,int width,int dot);
void M_DrawEmptyCell(menu_t *menu,int item);
void M_DrawSelCell(menu_t *menu,int item);
void M_WriteText(int x, int y, const char *string);
int  M_StringWidth(const char *string);
int  M_StringHeight(const char *string);
void M_StartMessage(const char *string,void *routine,boolean input);
void M_StopMessage(void);
void M_ClearMenus (void);
int  M_GetKeyString(int,int);
void M_Setup(int choice);
void M_KeyBindings(int choice);
void M_Weapons(int);
void M_StatusBar(int);
void M_Automap(int);
void M_Enemy(int);
void M_Messages(int);
void M_ChatStrings(int);
static int M_GetPixelWidth(const char*);
void M_DrawKeybnd(void);
void M_DrawWeapons(void);
static void M_DrawMenuString(int,int,int);
static void M_DrawStringCentered(int,int,int,const char*);
void M_DrawStatusHUD(void);
void M_DrawAutoMap(void);
void M_DrawEnemy(void);
void M_DrawMessages(void);
void M_DrawChatStrings(void);
void M_Compat(int);
void M_ChangeDemoSmoothTurns(void);
void M_General(int);
void M_DrawCompat(void);
void M_DrawGeneral(void);
void M_FullScreen(void);

menu_t NewDef;

enum
{

    newgame = 0,
    loadgame,
    savegame,
    options,
    quitdoom,
    main_end

} main_e;

menuitem_t MainMenu[] = {
    {1, "M_NGAME", M_NewGame},
    {1, "M_OPTION", M_Options},
    {1, "M_LOADG", M_LoadGame},
    {1, "M_SAVEG", M_SaveGame},
    {1, "M_QUITG", M_QuitDOOM}
};

menu_t MainDef = {
    main_end,
    NULL,
    MainMenu,
    M_DrawMainMenu,
    97,64,
    0
};

void M_DrawMainMenu(void)
{

    V_DrawNamePatch(94, 2, 0, "M_DOOM", CR_DEFAULT, VPT_STRETCH);

}

enum
{

  ep1,
  ep2,
  ep3,
  ep4,
  ep_end

} episodes_e;

menuitem_t EpisodeMenu[] = {
    {1, "M_EPI1", M_Episode},
    {1, "M_EPI2", M_Episode},
    {1, "M_EPI3", M_Episode},
    {1, "M_EPI4", M_Episode}
};

menu_t EpiDef = {
    ep_end,
    &MainDef,
    EpisodeMenu,
    M_DrawEpisode,
    48,63,
    ep1
};

int epi;

void M_DrawEpisode(void)
{

    V_DrawNamePatch(54, 38, 0, "M_EPISOD", CR_DEFAULT, VPT_STRETCH);

}

void M_Episode(int choice)
{

    epi = choice;
    M_SetupNextMenu(&NewDef);

}

enum
{

    killthings,
    toorough,
    hurtme,
    violence,
    nightmare,
    newg_end

} newgame_e;

menuitem_t NewGameMenu[] = {
    {1,"M_JKILL", M_ChooseSkill},
    {1,"M_ROUGH", M_ChooseSkill},
    {1,"M_HURT",  M_ChooseSkill},
    {1,"M_ULTRA", M_ChooseSkill},
    {1,"M_NMARE", M_ChooseSkill}
};

menu_t NewDef = {
    newg_end,
    &EpiDef,
    NewGameMenu,
    M_DrawNewGame,
    48,63,
    hurtme
};

void M_DrawNewGame(void)
{

    V_DrawNamePatch(96, 14, 0, "M_NEWG", CR_DEFAULT, VPT_STRETCH);
    V_DrawNamePatch(54, 38, 0, "M_SKILL",CR_DEFAULT, VPT_STRETCH);

}

void M_NewGame(int choice)
{

    if (gamemode == commercial)
        M_SetupNextMenu(&NewDef);
    else
        M_SetupNextMenu(&EpiDef);

}

void M_ChooseSkill(int choice)
{

    G_DeferedInitNew(choice,epi+1,1);
    M_ClearMenus ();

}

enum
{
    load1,
    load2,
    load3,
    load4,
    load5,
    load6,
    load7,
    load8,
    load_end

} load_e;

menuitem_t LoadMenue[] = {
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect},
    {1, "", M_LoadSelect}
};

menu_t LoadDef =
{
  load_end,
  &MainDef,
  LoadMenue,
  M_DrawLoad,
  80,34,
  0
};

#define LOADGRAPHIC_Y 8

void M_DrawLoad(void)
{
  int i;



  V_DrawNamePatch(72 ,LOADGRAPHIC_Y, 0, "M_LOADG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++) {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
  }
}





void M_DrawSaveLoadBorder(int x,int y)
{
  int i;

  V_DrawNamePatch(x-8, y+7, 0, "M_LSLEFT", CR_DEFAULT, VPT_STRETCH);

  for (i = 0 ; i < 24 ; i++)
    {
      V_DrawNamePatch(x, y+7, 0, "M_LSCNTR", CR_DEFAULT, VPT_STRETCH);
      x += 8;
    }

  V_DrawNamePatch(x, y+7, 0, "M_LSRGHT", CR_DEFAULT, VPT_STRETCH);
}

void M_LoadSelect(int choice)
{

  G_LoadGame(choice, false);
  M_ClearMenus ();

}

void M_LoadGame (int choice)
{

  if (demorecording && (compatibility_level < prboom_2_compatibility))
    {
    M_StartMessage("you can't load a game\n"
       "while recording an old demo!\n\n"PRESSKEY,
       NULL, false);
    return;
    }

  M_SetupNextMenu(&LoadDef);
  M_ReadSaveStrings();
}

menuitem_t SaveMenu[] = {
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect},
    {1, "", M_SaveSelect}
};

menu_t SaveDef =
{
  load_end,
  &MainDef,
  SaveMenu,
  M_DrawSave,
  80,34,
  0
};

void M_ReadSaveStrings(void)
{
  int i;

  for (i = 0 ; i < load_end ; i++) {
    char name[PATH_MAX+1];
    FILE *fp;

    /* killough 3/22/98
     * cph - add not-demoplayback parameter */
    G_SaveGameName(name,sizeof(name),i,false);
    fp = fopen(name,"rb");
    if (!fp) {
      strcpy(&savegamestrings[i][0],EMPTYSTRING);
      LoadMenue[i].status = 0;
      continue;
    }
    fread(&savegamestrings[i], SAVESTRINGSIZE, 1, fp);
    fclose(fp);
    LoadMenue[i].status = 1;
  }
}




void M_DrawSave(void)
{
  int i;



  V_DrawNamePatch(72, LOADGRAPHIC_Y, 0, "M_SAVEG", CR_DEFAULT, VPT_STRETCH);
  for (i = 0 ; i < load_end ; i++)
    {
    M_DrawSaveLoadBorder(LoadDef.x,LoadDef.y+LINEHEIGHT*i);
    M_WriteText(LoadDef.x,LoadDef.y+LINEHEIGHT*i,savegamestrings[i]);
    }

  if (saveStringEnter)
    {
    i = M_StringWidth(savegamestrings[saveSlot]);
    M_WriteText(LoadDef.x + i,LoadDef.y+LINEHEIGHT*saveSlot,"_");
    }
}




static void M_DoSave(int slot)
{
  G_SaveGame (slot,savegamestrings[slot]);
  M_ClearMenus ();


  if (quickSaveSlot == -2)
    quickSaveSlot = slot;
}




void M_SaveSelect(int choice)
{

  saveStringEnter = 1;

  saveSlot = choice;
  strcpy(saveOldString,savegamestrings[choice]);
  if (!strcmp(savegamestrings[choice],EMPTYSTRING))
    savegamestrings[choice][0] = 0;
  saveCharIndex = strlen(savegamestrings[choice]);
}




void M_SaveGame (int choice)
{

  if (!usergame && (!demoplayback))
    {
    M_StartMessage(SAVEDEAD,NULL,false);
    return;
    }

  if (gamestate != GS_LEVEL)
    return;

  M_SetupNextMenu(&SaveDef);
  M_ReadSaveStrings();
}

enum
{

    general,
    setup,
    messages,
    scrnsize,
    option_empty1,
    mousesens,
    soundvol,
    opt_end

} options_e;

menuitem_t OptionsMenu[] = {
    {1, "M_GENERL", M_General},
    {1, "M_SETUP", M_Setup},
    {1, "M_MESSG", M_ChangeMessages},
    {2, "M_SCRNSZ", M_SizeDisplay},
    {-1, "", 0},
    {1, "M_MSENS", M_ChangeSensitivity},
    {1, "M_SVOL", M_Sound}
};

menu_t OptionsDef = {
    opt_end,
    &MainDef,
    OptionsMenu,
    M_DrawOptions,
    60,37,
    0
};

char detailNames[2][9] = {"M_GDHIGH","M_GDLOW"};
char msgNames[2][9]  = {"M_MSGOFF","M_MSGON"};

void M_DrawOptions(void)
{

    V_DrawNamePatch(108, 15, 0, "M_OPTTTL", CR_DEFAULT, VPT_STRETCH);
    V_DrawNamePatch(OptionsDef.x + 120, OptionsDef.y+LINEHEIGHT*messages, 0, msgNames[showMessages], CR_DEFAULT, VPT_STRETCH);
    M_DrawThermo(OptionsDef.x, OptionsDef.y + LINEHEIGHT * (scrnsize + 1), 9, screenSize);

}

void M_Options(int choice)
{

    M_SetupNextMenu(&OptionsDef);

}

void M_QuitDOOM(int choice)
{

    exit(0);

}

enum
{
  sfx_vol,
  sfx_empty1,
  music_vol,
  sfx_empty2,
  sound_end
} sound_e;

menuitem_t SoundMenu[] = {
    {2, "M_SFXVOL", M_SfxVol},
    {-1, "", 0},
    {2, "M_MUSVOL", M_MusicVol},
    {-1, "", 0}
};

menu_t SoundDef =
{
  sound_end,
  &OptionsDef,
  SoundMenu,
  M_DrawSound,
  80,64,
  0
};

void M_DrawSound(void)
{

  V_DrawNamePatch(60, 38, 0, "M_SVOL", CR_DEFAULT, VPT_STRETCH);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(sfx_vol+1),16,snd_SfxVolume);

  M_DrawThermo(SoundDef.x,SoundDef.y+LINEHEIGHT*(music_vol+1),16,snd_MusicVolume);
}

void M_Sound(int choice)
{
  M_SetupNextMenu(&SoundDef);
}

void M_SfxVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_SfxVolume)
        snd_SfxVolume--;
      break;
    case 1:
      if (snd_SfxVolume < 15)
        snd_SfxVolume++;
      break;
    }

  S_SetSfxVolume(snd_SfxVolume /* *8 */);
}

void M_MusicVol(int choice)
{
  switch(choice)
    {
    case 0:
      if (snd_MusicVolume)
        snd_MusicVolume--;
      break;
    case 1:
      if (snd_MusicVolume < 15)
        snd_MusicVolume++;
      break;
    }

  S_SetMusicVolume(snd_MusicVolume /* *8 */);
}

enum
{
  mouse_horiz,
  mouse_empty1,
  mouse_vert,
  mouse_empty2,
  mouse_end
} mouse_e;

menuitem_t MouseMenu[] = {
    {2, "M_HORSEN", M_MouseHoriz},
    {-1, "", 0},
    {2, "M_VERSEN", M_MouseVert},
    {-1, "", 0}
};

menu_t MouseDef =
{
  mouse_end,
  &OptionsDef,
  MouseMenu,
  M_DrawMouse,
  60,64,
  0
};

#define MOUSE_SENS_MAX 100

void M_DrawMouse(void)
{
  int mhmx,mvmx;


  V_DrawNamePatch(60, 38, 0, "M_MSENS", CR_DEFAULT, VPT_STRETCH);


  mhmx = mouseSensitivity_horiz>99? 99 : mouseSensitivity_horiz; /*mead*/
  M_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_horiz+1),100,mhmx);

  mvmx = mouseSensitivity_vert>99? 99 : mouseSensitivity_vert; /*mead*/
  M_DrawThermo(MouseDef.x,MouseDef.y+LINEHEIGHT*(mouse_vert+1),100,mvmx);
}

void M_ChangeSensitivity(int choice)
{
  M_SetupNextMenu(&MouseDef);
}

void M_MouseHoriz(int choice)
{
  M_Mouse(choice, &mouseSensitivity_horiz);
}

void M_MouseVert(int choice)
{
  M_Mouse(choice, &mouseSensitivity_vert);
}

void M_Mouse(int choice, int *sens)
{
  switch(choice)
    {
    case 0:
      if (*sens)
        --*sens;
      break;
    case 1:
      if (*sens < 99)
        ++*sens;
      break;
    }
}

char tempstring[80];

static void M_QuickSaveResponse(int ch)
{
  if (ch == 'y')  {
    M_DoSave(quickSaveSlot);
    S_StartSound(NULL,sfx_swtchx);
  }
}

void M_QuickSave(void)
{
  if (!usergame && (!demoplayback)) {
    S_StartSound(NULL,sfx_oof);
    return;
  }

  if (gamestate != GS_LEVEL)
    return;

  if (quickSaveSlot < 0) {
    M_StartControlPanel();
    M_ReadSaveStrings();
    M_SetupNextMenu(&SaveDef);
    quickSaveSlot = -2;
    return;
  }
  sprintf(tempstring,QSPROMPT,savegamestrings[quickSaveSlot]);
  M_StartMessage(tempstring,M_QuickSaveResponse,true);
}

static void M_QuickLoadResponse(int ch)
{
  if (ch == 'y') {
    M_LoadSelect(quickSaveSlot);
    S_StartSound(NULL,sfx_swtchx);
  }
}

void M_QuickLoad(void)
{


  if (demorecording) {
    M_StartMessage("you can't quickload\n"
       "while recording a demo!\n\n"PRESSKEY,
       NULL, false);
    return;
  }

  if (quickSaveSlot < 0) {
    M_StartMessage(QSAVESPOT,NULL,false);
    return;
  }
  sprintf(tempstring,QLPROMPT,savegamestrings[quickSaveSlot]);
  M_StartMessage(tempstring,M_QuickLoadResponse,true);
}

void M_ChangeMessages(int choice)
{

  choice = 0;
  showMessages = 1 - showMessages;

  if (!showMessages)
    players[consoleplayer].message = MSGOFF;
  else
    players[consoleplayer].message = MSGON ;

  message_dontfuckwithme = true;
}

void M_SizeDisplay(int choice)
{
  switch(choice) {
  case 0:
    if (screenSize > 0) {
      screenblocks--;
      screenSize--;
      hud_displayed = 0;
    }
    break;
  case 1:
    if (screenSize < 8) {
      screenblocks++;
      screenSize++;
    }
    else
      hud_displayed = !hud_displayed;
    break;
  }
  R_SetViewSize (screenblocks);
}

boolean setup_active      = false;
boolean set_keybnd_active = false;
boolean set_weapon_active = false;
boolean set_status_active = false;
boolean set_auto_active   = false;
boolean set_enemy_active  = false;
boolean set_mess_active   = false;
boolean set_chat_active   = false;
boolean setup_select      = false;
boolean setup_gather      = false;
boolean colorbox_active   = false;
boolean default_verify    = false;
boolean set_general_active = false;
boolean set_compat_active = false;

static int set_menu_itemon;
setup_menu_t* current_setup_menu;
static char menu_buffer[64];

enum
{
  set_compat,
  set_key_bindings,
  set_weapons,
  set_statbar,
  set_automap,
  set_enemy,
  set_messages,
  set_chatstrings,
  set_setup_end
} setup_e;

int setup_screen;

menuitem_t SetupMenu[] = {
    {1, "M_COMPAT", M_Compat},
    {1, "M_KEYBND", M_KeyBindings},
    {1, "M_WEAP", M_Weapons},
    {1, "M_STAT", M_StatusBar},
    {1, "M_AUTO", M_Automap},
    {1, "M_ENEM", M_Enemy},
    {1, "M_MESS", M_Messages},
    {1, "M_CHAT", M_ChatStrings}
};

static void M_DoNothing(int choice)
{
}

enum
{
  generic_setupempty1,
  generic_setup_end
} generic_setup_e;

menuitem_t Generic_Setup[] = {
    {1, "", M_DoNothing}
};

menu_t  SetupDef =
{
  set_setup_end,
  &OptionsDef,
  SetupMenu,
  M_DrawSetup,
  59,37,
  0
};

menu_t KeybndDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawKeybnd,
  34,5,
  0
};

menu_t WeaponDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawWeapons,
  34,5,
  0
};

menu_t StatusHUDDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawStatusHUD,
  34,5,
  0
};

menu_t AutoMapDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawAutoMap,
  34,5,
  0
};

menu_t EnemyDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawEnemy,
  34,5,
  0
};

menu_t MessageDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawMessages,
  34,5,
  0
};

menu_t ChatStrDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawChatStrings,
  34,5,
  0
};

menu_t GeneralDef =
{
  generic_setup_end,
  &OptionsDef,
  Generic_Setup,
  M_DrawGeneral,
  34,5,
  0
};

menu_t CompatDef =
{
  generic_setup_end,
  &SetupDef,
  Generic_Setup,
  M_DrawCompat,
  34,5,
  0
};

void M_DrawSetup(void)
{

  V_DrawNamePatch(124, 15, 0, "M_SETUP", CR_DEFAULT, VPT_STRETCH);
}

void M_Setup(int choice)
{
  M_SetupNextMenu(&SetupDef);
}

#define CR_TITLE  CR_GOLD
#define CR_SET    CR_GREEN
#define CR_ITEM   CR_RED
#define CR_HILITE CR_ORANGE
#define CR_SELECT CR_GRAY
#define CHIP_SIZE 7
#define COLORPALXORIG ((320 - 16*(CHIP_SIZE+1))/2)
#define COLORPALYORIG ((200 - 16*(CHIP_SIZE+1))/2)
#define PAL_BLACK   0
#define PAL_WHITE   4
#define CHAT_STRING_BFR_SIZE 128
#define MAXCHATWIDTH         272

int   chat_index;
char* chat_string_buffer;
char ResetButtonName[2][8] = {"M_BUTT1","M_BUTT2"};

static void M_DrawItem(const setup_menu_t* s)
{
  int x = s->m_x;
  int y = s->m_y;
  int flags = s->m_flags;
  if (flags & S_RESET)

    V_DrawNamePatch(x, y, 0, ResetButtonName[(flags & (S_HILITE|S_SELECT)) ? whichSkull : 0],
        CR_DEFAULT, VPT_STRETCH);

  else {
    char *p, *t;
    int w = 0;
    int color =
  flags & S_SELECT ? CR_SELECT :
  flags & S_HILITE ? CR_HILITE :
  flags & (S_TITLE|S_NEXT|S_PREV) ? CR_TITLE : CR_ITEM;

    /* killough 10/98:
     * Enhance to support multiline text separated by newlines.
     * This supports multiline items on horizontally-crowded menus.
     */

    for (p = t = strdup(s->m_text); (p = strtok(p,"\n")); y += 8, p = NULL)
      {      /* killough 10/98: support left-justification: */
  strcpy(menu_buffer,p);
  if (!(flags & S_LEFTJUST))
    w = M_GetPixelWidth(menu_buffer) + 4;
  M_DrawMenuString(x - w, y ,color);
      }
    free(t);
  }
}

#define MAXGATHER 5
int  gather_count;
char gather_buffer[MAXGATHER+1];

static void M_DrawSetting(const setup_menu_t* s)
{
  int x = s->m_x, y = s->m_y, flags = s->m_flags, color;

  color = flags & S_SELECT ? CR_SELECT : flags & S_HILITE ? CR_HILITE : CR_SET;

  if (flags & S_YESNO) {
    strcpy(menu_buffer,*s->var.def->location.pi ? "YES" : "NO");
    M_DrawMenuString(x,y,color);
    return;
  }



  if (flags & S_NUM) {

    if (flags & (S_HILITE|S_SELECT) && setup_gather) {
      gather_buffer[gather_count] = 0;
      strcpy(menu_buffer, gather_buffer);
    }
    else
      sprintf(menu_buffer,"%d",*s->var.def->location.pi);
    M_DrawMenuString(x,y,color);
    return;
  }



  if (flags & S_KEY) {
    int *key = s->var.m_key;



    if (key) {
      M_GetKeyString(*key,0);
      if (key == &key_use) {


  if (s->m_mouse)
    sprintf(menu_buffer+strlen(menu_buffer), "/DBL-CLK MB%d",*s->m_mouse+1);
      }
      else if (key == &key_up   || key == &key_speed ||
         key == &key_fire || key == &key_strafe)
  {
    if (s->m_mouse)
      sprintf(menu_buffer+strlen(menu_buffer), "/MB%d",
        *s->m_mouse+1);
  }
      M_DrawMenuString(x,y,color);
    }
    return;
  }

  if (flags & (S_WEAP|S_CRITEM))
    {
      sprintf(menu_buffer,"%d", *s->var.def->location.pi);
      M_DrawMenuString(x,y, flags & S_CRITEM ? *s->var.def->location.pi : color);
      return;
    }



  if (flags & S_COLOR)
    {
      int ch;

      ch = *s->var.def->location.pi;


      V_FillRect(0, x*SCREENWIDTH/320, (y-1)*SCREENHEIGHT/200,
                    8*SCREENWIDTH/320, 8*SCREENHEIGHT/200,
                 PAL_BLACK);
      V_FillRect(0, (x+1)*SCREENWIDTH/320, y*SCREENHEIGHT/200,
                        6*SCREENWIDTH/320, 6*SCREENHEIGHT/200,
                 (byte)ch);

      if (!ch)
  V_DrawNamePatch(x+1,y,0,"M_PALNO", CR_DEFAULT, VPT_STRETCH);
      return;
    }




  if (flags & S_STRING) {
    /* cph - cast to char* as it's really a Z_Strdup'd string (see m_misc.h) */
    char *text = (char*)*s->var.def->location.ppsz;




    if (setup_select && (s->m_flags & (S_HILITE|S_SELECT))) {
      int cursor_start, char_width;
      char c[2];

      while (M_GetPixelWidth(text) >= MAXCHATWIDTH) {
  int len = strlen(text);
  text[--len] = 0;
  if (chat_index > len)
    chat_index--;
      }





      *c = text[chat_index];
      c[1] = 0;
      char_width = M_GetPixelWidth(c);
      if (char_width == 1)
  char_width = 7;
      text[chat_index] = 0;
      cursor_start = M_GetPixelWidth(text);
      text[chat_index] = *c;



      V_FillRect(0, ((x+cursor_start-1)*SCREENWIDTH)/320, (y*SCREENHEIGHT)/200,
      (char_width*SCREENWIDTH)/320, 9*SCREENHEIGHT/200, PAL_WHITE);
    }



    strcpy(menu_buffer,text);
    M_DrawMenuString(x,y,color);
    return;
  }



  if (flags & S_CHOICE) {
    if (s->var.def->type == def_int) {
      if (s->selectstrings == NULL) {
        sprintf(menu_buffer,"%d",*s->var.def->location.pi);
      } else {
        strcpy(menu_buffer,s->selectstrings[*s->var.def->location.pi]);
      }
    }

    if (s->var.def->type == def_str) {
      sprintf(menu_buffer,"%s", *s->var.def->location.ppsz);
    }

    M_DrawMenuString(x,y,color);
    return;
  }
}







static void M_DrawScreenItems(const setup_menu_t* src)
{
  if (print_warning_about_changes > 0) { /* killough 8/15/98: print warning */
    if (warning_about_changes & S_BADVAL) {
  strcpy(menu_buffer, "Value out of Range");
  M_DrawMenuString(100,176,CR_RED);
    } else if (warning_about_changes & S_PRGWARN) {
        strcpy(menu_buffer, "Warning: Program must be restarted to see changes");
  M_DrawMenuString(3, 176, CR_RED);
    } else if (warning_about_changes & S_BADVID) {
        strcpy(menu_buffer, "Video mode not supported");
  M_DrawMenuString(80,176,CR_RED);
    } else {
  strcpy(menu_buffer, "Warning: Changes are pending until next game");
        M_DrawMenuString(18,184,CR_RED);
    }
  }

  while (!(src->m_flags & S_END)) {



    if (src->m_flags & S_SHOWDESC)
      M_DrawItem(src);



    if (src->m_flags & S_SHOWSET)
      M_DrawSetting(src);
    src++;
  }
}






#define VERIFYBOXXORG 66
#define VERIFYBOXYORG 88
#define PAL_GRAY1  91
#define PAL_GRAY2  98
#define PAL_GRAY3 105



static void M_DrawDefVerify(void)
{

  V_DrawNamePatch(VERIFYBOXXORG,VERIFYBOXYORG,0,"M_VBOX",CR_DEFAULT,VPT_STRETCH);



  if (whichSkull) {
    strcpy(menu_buffer,"Reset to defaults? (Y or N)");
    M_DrawMenuString(VERIFYBOXXORG+8,VERIFYBOXYORG+8,CR_RED);
  }
}

static void M_DrawInstructions(void)
{
  int flags = current_setup_menu[set_menu_itemon].m_flags;




  if (setup_select) {
    switch (flags & (S_KEY | S_YESNO | S_WEAP | S_NUM | S_COLOR | S_CRITEM | S_CHAT | S_RESET | S_FILE | S_CHOICE)) {
      case S_KEY:


        if (current_setup_menu[set_menu_itemon].m_mouse)
          M_DrawStringCentered(160, 20, CR_SELECT, "Press key or button for this action");
        else
          M_DrawStringCentered(160, 20, CR_SELECT, "Press key for this action");
        break;

    case S_YESNO:
      M_DrawStringCentered(160, 20, CR_SELECT, "Press ENTER key to toggle");
      break;
    case S_WEAP:
      M_DrawStringCentered(160, 20, CR_SELECT, "Enter weapon number");
      break;
    case S_NUM:
      M_DrawStringCentered(160, 20, CR_SELECT, "Enter value. Press ENTER when finished.");
      break;
    case S_COLOR:
      M_DrawStringCentered(160, 20, CR_SELECT, "Select color and press enter");
      break;
    case S_CRITEM:
      M_DrawStringCentered(160, 20, CR_SELECT, "Enter value");
      break;
    case S_CHAT:
      M_DrawStringCentered(160, 20, CR_SELECT, "Type/edit chat string and Press ENTER");
      break;
    case S_FILE:
      M_DrawStringCentered(160, 20, CR_SELECT, "Type/edit filename and Press ENTER");
      break;
    case S_CHOICE:
      M_DrawStringCentered(160, 20, CR_SELECT, "Press left or right to choose");
      break;
    case S_RESET:
      break;
    }
  } else {
    if (flags & S_RESET)
      M_DrawStringCentered(160, 20, CR_HILITE, "Press ENTER key to reset to defaults");
    else
      M_DrawStringCentered(160, 20, CR_HILITE, "Press Enter to Change");
  }
}






#define KB_X  160
#define KB_PREV  57
#define KB_NEXT 310
#define KB_Y   31





#define X_BUTTON 301
#define Y_BUTTON   3



setup_menu_t keys_settings1[];
setup_menu_t keys_settings2[];
setup_menu_t keys_settings3[];
setup_menu_t keys_settings4[];



setup_menu_t* keys_settings[] =
{
  keys_settings1,
  keys_settings2,
  keys_settings3,
  keys_settings4,
  NULL
};

int mult_screens_index;

setup_menu_t keys_settings1[] =
{
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FORWARD"     ,S_KEY       ,m_scrn,KB_X,KB_Y+1*8,{&key_up},&mousebforward},
  {"BACKWARD"    ,S_KEY       ,m_scrn,KB_X,KB_Y+2*8,{&key_down}},
  {"TURN LEFT"   ,S_KEY       ,m_scrn,KB_X,KB_Y+3*8,{&key_left}},
  {"TURN RIGHT"  ,S_KEY       ,m_scrn,KB_X,KB_Y+4*8,{&key_right}},
  {"RUN"         ,S_KEY       ,m_scrn,KB_X,KB_Y+5*8,{&key_speed}},
  {"STRAFE LEFT" ,S_KEY       ,m_scrn,KB_X,KB_Y+6*8,{&key_strafeleft}},
  {"STRAFE RIGHT",S_KEY       ,m_scrn,KB_X,KB_Y+7*8,{&key_straferight}},
  {"STRAFE"      ,S_KEY       ,m_scrn,KB_X,KB_Y+8*8,{&key_strafe},&mousebstrafe},
  {"AUTORUN"     ,S_KEY       ,m_scrn,KB_X,KB_Y+9*8,{&key_autorun}},
  {"180 TURN"    ,S_KEY       ,m_scrn,KB_X,KB_Y+10*8,{&key_reverse}},
  {"USE"         ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_use},&mousebforward},

  {"MENUS"       ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+12*8},
  {"NEXT ITEM"   ,S_KEY       ,m_menu,KB_X,KB_Y+13*8,{&key_menu_down}},
  {"PREV ITEM"   ,S_KEY       ,m_menu,KB_X,KB_Y+14*8,{&key_menu_up}},
  {"LEFT"        ,S_KEY       ,m_menu,KB_X,KB_Y+15*8,{&key_menu_left}},
  {"RIGHT"       ,S_KEY       ,m_menu,KB_X,KB_Y+16*8,{&key_menu_right}},
  {"BACKSPACE"   ,S_KEY       ,m_menu,KB_X,KB_Y+17*8,{&key_menu_backspace}},
  {"SELECT ITEM" ,S_KEY       ,m_menu,KB_X,KB_Y+18*8,{&key_menu_enter}},
  {"EXIT"        ,S_KEY       ,m_menu,KB_X,KB_Y+19*8,{&key_menu_escape}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings2}},


  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings2[] =
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"HELP"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEEP ,m_scrn,0   ,0    ,{&key_escape}},

  {"SETUP"       ,S_KEY       ,m_scrn,KB_X,KB_Y+ 1*8,{&key_setup}},
  {"PAUSE"       ,S_KEY       ,m_scrn,KB_X,KB_Y+ 2*8,{&key_pause}},
  {"AUTOMAP"     ,S_KEY       ,m_scrn,KB_X,KB_Y+ 3*8,{&key_map}},
  {"VOLUME"      ,S_KEY       ,m_scrn,KB_X,KB_Y+ 4*8,{&key_soundvolume}},
  {"HUD"         ,S_KEY       ,m_scrn,KB_X,KB_Y+ 5*8,{&key_hud}},
  {"MESSAGES"    ,S_KEY       ,m_scrn,KB_X,KB_Y+ 6*8,{&key_messages}},
  {"GAMMA FIX"   ,S_KEY       ,m_scrn,KB_X,KB_Y+ 7*8,{&key_gamma}},
  {"SPY"         ,S_KEY       ,m_scrn,KB_X,KB_Y+ 8*8,{&key_spy}},
  {"LARGER VIEW" ,S_KEY       ,m_scrn,KB_X,KB_Y+ 9*8,{&key_zoomin}},
  {"SMALLER VIEW",S_KEY       ,m_scrn,KB_X,KB_Y+10*8,{&key_zoomout}},
  {"SCREENSHOT"  ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_screenshot}},
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+12*8},
  {"SAVE"        ,S_KEY       ,m_scrn,KB_X,KB_Y+13*8,{&key_savegame}},
  {"LOAD"        ,S_KEY       ,m_scrn,KB_X,KB_Y+14*8,{&key_loadgame}},
  {"QUICKSAVE"   ,S_KEY       ,m_scrn,KB_X,KB_Y+15*8,{&key_quicksave}},
  {"QUICKLOAD"   ,S_KEY       ,m_scrn,KB_X,KB_Y+16*8,{&key_quickload}},
  {"END GAME"    ,S_KEY       ,m_scrn,KB_X,KB_Y+17*8,{&key_endgame}},
  {"QUIT"        ,S_KEY       ,m_scrn,KB_X,KB_Y+18*8,{&key_quit}},
  {"<- PREV", S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings1}},
  {"NEXT ->", S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings3}},



  {0,S_SKIP|S_END,m_null}
};

setup_menu_t keys_settings3[] =
{
  {"WEAPONS" ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FIST"    ,S_KEY       ,m_scrn,KB_X,KB_Y+ 1*8,{&key_weapon1}},
  {"PISTOL"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 2*8,{&key_weapon2}},
  {"SHOTGUN" ,S_KEY       ,m_scrn,KB_X,KB_Y+ 3*8,{&key_weapon3}},
  {"CHAINGUN",S_KEY       ,m_scrn,KB_X,KB_Y+ 4*8,{&key_weapon4}},
  {"ROCKET"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 5*8,{&key_weapon5}},
  {"PLASMA"  ,S_KEY       ,m_scrn,KB_X,KB_Y+ 6*8,{&key_weapon6}},
  {"BFG",     S_KEY       ,m_scrn,KB_X,KB_Y+ 7*8,{&key_weapon7}},
  {"CHAINSAW",S_KEY       ,m_scrn,KB_X,KB_Y+ 8*8,{&key_weapon8}},
  {"SSG"     ,S_KEY       ,m_scrn,KB_X,KB_Y+ 9*8,{&key_weapon9}},
  {"BEST"    ,S_KEY       ,m_scrn,KB_X,KB_Y+10*8,{&key_weapontoggle}},
  {"FIRE"    ,S_KEY       ,m_scrn,KB_X,KB_Y+11*8,{&key_fire},&mousebfire},

  {"<- PREV",S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings2}},
  {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y+20*8, {keys_settings4}},



  {0,S_SKIP|S_END,m_null}

};

setup_menu_t keys_settings4[] =
{
  {"AUTOMAP"    ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y},
  {"FOLLOW"     ,S_KEY       ,m_map ,KB_X,KB_Y+ 1*8,{&key_map_follow}},
  {"ZOOM IN"    ,S_KEY       ,m_map ,KB_X,KB_Y+ 2*8,{&key_map_zoomin}},
  {"ZOOM OUT"   ,S_KEY       ,m_map ,KB_X,KB_Y+ 3*8,{&key_map_zoomout}},
  {"SHIFT UP"   ,S_KEY       ,m_map ,KB_X,KB_Y+ 4*8,{&key_map_up}},
  {"SHIFT DOWN" ,S_KEY       ,m_map ,KB_X,KB_Y+ 5*8,{&key_map_down}},
  {"SHIFT LEFT" ,S_KEY       ,m_map ,KB_X,KB_Y+ 6*8,{&key_map_left}},
  {"SHIFT RIGHT",S_KEY       ,m_map ,KB_X,KB_Y+ 7*8,{&key_map_right}},
  {"MARK PLACE" ,S_KEY       ,m_map ,KB_X,KB_Y+ 8*8,{&key_map_mark}},
  {"CLEAR MARKS",S_KEY       ,m_map ,KB_X,KB_Y+ 9*8,{&key_map_clear}},
  {"FULL/ZOOM"  ,S_KEY       ,m_map ,KB_X,KB_Y+10*8,{&key_map_gobig}},
  {"GRID"       ,S_KEY       ,m_map ,KB_X,KB_Y+11*8,{&key_map_grid}},

  {"CHATTING"   ,S_SKIP|S_TITLE,m_null,KB_X,KB_Y+12*8},
  {"BEGIN CHAT" ,S_KEY       ,m_scrn,KB_X,KB_Y+13*8,{&key_chat}},
  {"PLAYER 1"   ,S_KEY       ,m_scrn,KB_X,KB_Y+14*8,{&destination_keys[0]}},
  {"PLAYER 2"   ,S_KEY       ,m_scrn,KB_X,KB_Y+15*8,{&destination_keys[1]}},
  {"PLAYER 3"   ,S_KEY       ,m_scrn,KB_X,KB_Y+16*8,{&destination_keys[2]}},
  {"PLAYER 4"   ,S_KEY       ,m_scrn,KB_X,KB_Y+17*8,{&destination_keys[3]}},
  {"BACKSPACE"  ,S_KEY       ,m_scrn,KB_X,KB_Y+18*8,{&key_backspace}},
  {"ENTER"      ,S_KEY       ,m_scrn,KB_X,KB_Y+19*8,{&key_enter}},

  {"<- PREV" ,S_SKIP|S_PREV,m_null,KB_PREV,KB_Y+20*8, {keys_settings3}},



  {0,S_SKIP|S_END,m_null}

};

void M_KeyBindings(int choice)
{
  M_SetupNextMenu(&KeybndDef);

  setup_active = true;
  setup_screen = ss_keys;
  set_keybnd_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = keys_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawKeybnd(void)

{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(84, 2, 0, "M_KEYBND", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

#define WP_X 203
#define WP_Y  33

enum {
  weap_recoil,
  weap_bobbing,
  weap_bfg,
  weap_stub1,
  weap_pref1,
  weap_pref2,
  weap_pref3,
  weap_pref4,
  weap_pref5,
  weap_pref6,
  weap_pref7,
  weap_pref8,
  weap_pref9,
  weap_stub2,
  weap_toggle,
  weap_toggle2,
};

setup_menu_t weap_settings1[];

setup_menu_t* weap_settings[] =
{
  weap_settings1,
  NULL
};

setup_menu_t weap_settings1[] =
{
  {"ENABLE RECOIL", S_YESNO,m_null,WP_X, WP_Y+ weap_recoil*8, {"weapon_recoil"}},
  {"ENABLE BOBBING",S_YESNO,m_null,WP_X, WP_Y+weap_bobbing*8, {"player_bobbing"}},

  {"1ST CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref1*8, {"weapon_choice_1"}},
  {"2nd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref2*8, {"weapon_choice_2"}},
  {"3rd CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref3*8, {"weapon_choice_3"}},
  {"4th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref4*8, {"weapon_choice_4"}},
  {"5th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref5*8, {"weapon_choice_5"}},
  {"6th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref6*8, {"weapon_choice_6"}},
  {"7th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref7*8, {"weapon_choice_7"}},
  {"8th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref8*8, {"weapon_choice_8"}},
  {"9th CHOICE WEAPON",S_WEAP,m_null,WP_X,WP_Y+weap_pref9*8, {"weapon_choice_9"}},

  {"Enable Fist/Chainsaw\n& SG/SSG toggle", S_YESNO, m_null, WP_X,
   WP_Y+ weap_toggle*8, {"doom_weapon_toggles"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},


  {0,S_SKIP|S_END,m_null}

};

void M_Weapons(int choice)
{
  M_SetupNextMenu(&WeaponDef);

  setup_active = true;
  setup_screen = ss_weap;
  set_weapon_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = weap_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawWeapons(void)
{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(109, 2, 0, "M_WEAP", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

#define ST_X 203
#define ST_Y  31



setup_menu_t stat_settings1[];

setup_menu_t* stat_settings[] =
{
  stat_settings1,
  NULL
};

setup_menu_t stat_settings1[] =
{
  {"STATUS BAR"        ,S_SKIP|S_TITLE,m_null,ST_X,ST_Y+ 1*8 },

  {"USE RED NUMBERS"   ,S_YESNO, m_null,ST_X,ST_Y+ 2*8, {"sts_always_red"}},
  {"GRAY %"            ,S_YESNO, m_null,ST_X,ST_Y+ 3*8, {"sts_pct_always_gray"}},
  {"SINGLE KEY DISPLAY",S_YESNO, m_null,ST_X,ST_Y+ 4*8, {"sts_traditional_keys"}},

  {"HEADS-UP DISPLAY"  ,S_SKIP|S_TITLE,m_null,ST_X,ST_Y+ 6*8},

  {"HIDE SECRETS"      ,S_YESNO     ,m_null,ST_X,ST_Y+ 7*8, {"hud_nosecrets"}},
  {"HEALTH LOW/OK"     ,S_NUM       ,m_null,ST_X,ST_Y+ 8*8, {"health_red"}},
  {"HEALTH OK/GOOD"    ,S_NUM       ,m_null,ST_X,ST_Y+ 9*8, {"health_yellow"}},
  {"HEALTH GOOD/EXTRA" ,S_NUM       ,m_null,ST_X,ST_Y+10*8, {"health_green"}},
  {"ARMOR LOW/OK"      ,S_NUM       ,m_null,ST_X,ST_Y+11*8, {"armor_red"}},
  {"ARMOR OK/GOOD"     ,S_NUM       ,m_null,ST_X,ST_Y+12*8, {"armor_yellow"}},
  {"ARMOR GOOD/EXTRA"  ,S_NUM       ,m_null,ST_X,ST_Y+13*8, {"armor_green"}},
  {"AMMO LOW/OK"       ,S_NUM       ,m_null,ST_X,ST_Y+14*8, {"ammo_red"}},
  {"AMMO OK/GOOD"      ,S_NUM       ,m_null,ST_X,ST_Y+15*8, {"ammo_yellow"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},


  {0,S_SKIP|S_END,m_null}
};

void M_StatusBar(int choice)
{
  M_SetupNextMenu(&StatusHUDDef);

  setup_active = true;
  setup_screen = ss_stat;
  set_status_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = stat_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawStatusHUD(void)
{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(59, 2, 0, "M_STAT", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

#define AU_X    250
#define AU_Y     31
#define AU_PREV KB_PREV
#define AU_NEXT KB_NEXT

setup_menu_t auto_settings1[];
setup_menu_t auto_settings2[];

setup_menu_t* auto_settings[] =
{
  auto_settings1,
  auto_settings2,
  NULL
};

setup_menu_t auto_settings1[] =
{
  {"background", S_COLOR, m_null, AU_X, AU_Y, {"mapcolor_back"}},
  {"grid lines", S_COLOR, m_null, AU_X, AU_Y + 1*8, {"mapcolor_grid"}},
  {"normal 1s wall", S_COLOR, m_null,AU_X,AU_Y+ 2*8, {"mapcolor_wall"}},
  {"line at floor height change", S_COLOR, m_null, AU_X, AU_Y+ 3*8, {"mapcolor_fchg"}},
  {"line at ceiling height change"      ,S_COLOR,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_cchg"}},
  {"line at sector with floor = ceiling",S_COLOR,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_clsd"}},
  {"red key"                            ,S_COLOR,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_rkey"}},
  {"blue key"                           ,S_COLOR,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_bkey"}},
  {"yellow key"                         ,S_COLOR,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_ykey"}},
  {"red door"                           ,S_COLOR,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_rdor"}},
  {"blue door"                          ,S_COLOR,m_null,AU_X,AU_Y+10*8, {"mapcolor_bdor"}},
  {"yellow door"                        ,S_COLOR,m_null,AU_X,AU_Y+11*8, {"mapcolor_ydor"}},

  {"AUTOMAP LEVEL TITLE COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+13*8, {"hudcolor_titl"}},
  {"AUTOMAP COORDINATES COLOR"      ,S_CRITEM,m_null,AU_X,AU_Y+14*8, {"hudcolor_xyco"}},

  {"Show Secrets only after entering",S_YESNO,m_null,AU_X,AU_Y+15*8, {"map_secret_after"}},

  {"Show coordinates of automap pointer",S_YESNO,m_null,AU_X,AU_Y+16*8, {"map_point_coord"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT,m_null,AU_NEXT,AU_Y+20*8, {auto_settings2}},


  {0,S_SKIP|S_END,m_null}

};

setup_menu_t auto_settings2[] =
{
  {"teleporter line"                ,S_COLOR ,m_null,AU_X,AU_Y, {"mapcolor_tele"}},
  {"secret sector boundary"         ,S_COLOR ,m_null,AU_X,AU_Y+ 1*8, {"mapcolor_secr"}},

  {"exit line"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 2*8, {"mapcolor_exit"}},
  {"computer map unseen line"       ,S_COLOR ,m_null,AU_X,AU_Y+ 3*8, {"mapcolor_unsn"}},
  {"line w/no floor/ceiling changes",S_COLOR ,m_null,AU_X,AU_Y+ 4*8, {"mapcolor_flat"}},
  {"general sprite"                 ,S_COLOR ,m_null,AU_X,AU_Y+ 5*8, {"mapcolor_sprt"}},
  {"countable enemy sprite"         ,S_COLOR ,m_null,AU_X,AU_Y+ 6*8, {"mapcolor_enemy"}},
  {"countable item sprite"          ,S_COLOR ,m_null,AU_X,AU_Y+ 7*8, {"mapcolor_item"}},
  {"crosshair"                      ,S_COLOR ,m_null,AU_X,AU_Y+ 8*8, {"mapcolor_hair"}},
  {"single player arrow"            ,S_COLOR ,m_null,AU_X,AU_Y+ 9*8, {"mapcolor_sngl"}},
  {"your colour in multiplayer"     ,S_COLOR ,m_null,AU_X,AU_Y+10*8, {"mapcolor_me"}},

  {"friends"                        ,S_COLOR ,m_null,AU_X,AU_Y+12*8, {"mapcolor_frnd"}},

  {"<- PREV",S_SKIP|S_PREV,m_null,AU_PREV,AU_Y+20*8, {auto_settings1}},
  {0,S_SKIP|S_END,m_null}

};

void M_Automap(int choice)
{
  M_SetupNextMenu(&AutoMapDef);

  setup_active = true;
  setup_screen = ss_auto;
  set_auto_active = true;
  setup_select = false;
  colorbox_active = false;
  default_verify = false;
  setup_gather = false;
  set_menu_itemon = 0;
  mult_screens_index = 0;
  current_setup_menu = auto_settings[0];
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

int color_palette_x;
int color_palette_y;
byte palette_background[16*(CHIP_SIZE+1)+8];

static void M_DrawColPal(void)
{
  int cpx, cpy;

  V_DrawNamePatch(COLORPALXORIG-5, COLORPALYORIG-5, 0, "M_COLORS", CR_DEFAULT, VPT_STRETCH);

  cpx = COLORPALXORIG+color_palette_x*(CHIP_SIZE+1)-1;
  cpy = COLORPALYORIG+color_palette_y*(CHIP_SIZE+1)-1;

  V_DrawNamePatch(cpx,cpy,0,"M_PALSEL",CR_DEFAULT,VPT_STRETCH);
}

void M_DrawAutoMap(void)

{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(109, 2, 0, "M_AUTO", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (colorbox_active)
    M_DrawColPal();
  else if (default_verify)
    M_DrawDefVerify();
}

#define E_X 250
#define E_Y  31

setup_menu_t enem_settings1[];

setup_menu_t* enem_settings[] =
{
  enem_settings1,
  NULL
};

enum {
  enem_infighting,
  enem_remember = 1,
  enem_backing,
  enem_avoid_hazards,
  enem_friction,
  enem_help_friends,
  enem_distfriend,
  enem_end
};

setup_menu_t enem_settings1[] =
{

  {"Monster Infighting When Provoked",S_YESNO,m_null,E_X,E_Y+ enem_infighting*8, {"monster_infighting"}},
  {"Remember Previous Enemy",S_YESNO,m_null,E_X,E_Y+ enem_remember*8, {"monsters_remember"}},
  {"Monster Backing Out",S_YESNO,m_null,E_X,E_Y+ enem_backing*8, {"monster_backing"}},
  {"Intelligently Avoid Hazards",S_YESNO,m_null,E_X,E_Y+ enem_avoid_hazards*8, {"monster_avoid_hazards"}},
  {"Affected by Friction",S_YESNO,m_null,E_X,E_Y+ enem_friction*8, {"monster_friction"}},
  {"Rescue Dying Friends",S_YESNO,m_null,E_X,E_Y+ enem_help_friends*8, {"help_friends"}},
  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},
  {0,S_SKIP|S_END,m_null}

};

void M_Enemy(int choice)
{
  M_SetupNextMenu(&EnemyDef);

  setup_active = true;
  setup_screen = ss_enem;
  set_enemy_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = enem_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawEnemy(void)

{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(114, 2, 0, "M_ENEM", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

extern int usemouse, default_mus_card, default_snd_card;
extern int detect_voices, realtic_clock_rate;

setup_menu_t gen_settings1[], gen_settings2[], gen_settings3[];

setup_menu_t* gen_settings[] =
{
  gen_settings1,
  gen_settings2,
  gen_settings3,
  NULL
};

enum {
  general_sndchan,
  general_pitch
};

#define G_X 250
#define G_YA  44
#define G_YA2 (G_YA+9*8)
#define G_YA3 (G_YA2+5*8)
#define GF_X 76

setup_menu_t gen_settings1[] = {
    {"Sound & Music", S_SKIP|S_TITLE, m_null, G_X, G_YA3 - 12},
    {"Number of Sound Channels", S_NUM|S_PRGWARN, m_null, G_X, G_YA3 + general_sndchan * 8, {"snd_channels"}},
    {"Enable v1.1 Pitch Effects", S_YESNO, m_null, G_X, G_YA3 + general_pitch*8, {"pitched_sounds"}},
    {0, S_RESET, m_null, X_BUTTON, Y_BUTTON},
    {"NEXT ->", S_SKIP | S_NEXT, m_null, KB_NEXT, KB_Y +20 * 8, {gen_settings2}},
    {0, S_SKIP|S_END,m_null}
};

enum {
  general_mouse,
  general_leds
};

enum {
  general_wad1,
  general_wad2,
  general_deh1,
  general_deh2
};

enum {
  general_corpse,
  general_realtic,
  general_smooth,
  general_smoothfactor,
  general_defskill,
};

#define G_YB  44
#define G_YB1 (G_YB+44)
#define G_YB2 (G_YB1+52)

static const char *gen_skillstrings[] = {
    "", "ITYTD", "HNTR", "HMP", "UV", "NM", NULL
};

setup_menu_t gen_settings2[] = {
    {"Input Devices"     ,S_SKIP|S_TITLE, m_null, G_X, G_YB - 12},
    {"Enable Mouse", S_YESNO, m_null, G_X, G_YB + general_mouse * 8, {"use_mouse"}},
    {"Files Preloaded at Game Startup",S_SKIP|S_TITLE, m_null, G_X, G_YB1 - 12},
    {"WAD # 1", S_FILE, m_null, GF_X, G_YB1 + general_wad1 * 8, {"wadfile_1"}},
    {"WAD #2", S_FILE, m_null, GF_X, G_YB1 + general_wad2 * 8, {"wadfile_2"}},
    {"Miscellaneous"  ,S_SKIP|S_TITLE, m_null, G_X, G_YB2 - 12},
    {"Maximum number of player corpses", S_NUM|S_PRGWARN, m_null, G_X, G_YB2 + general_corpse*8, {"max_player_corpse"}},
    {"Game speed, percentage of normal", S_NUM|S_PRGWARN, m_null, G_X, G_YB2 + general_realtic*8, {"realtic_clock_rate"}},
    {"Smooth Demo Playback", S_YESNO, m_null, G_X, G_YB2 + general_smooth * 8, {"demo_smoothturns"}, 0, 0, M_ChangeDemoSmoothTurns},
    {"Smooth Demo Playback Factor", S_NUM, m_null, G_X, G_YB2 + general_smoothfactor * 8, {"demo_smoothturnsfactor"}, 0, 0, M_ChangeDemoSmoothTurns},
    {"Default skill level", S_CHOICE, m_null, G_X, G_YB2 + general_defskill * 8, {"default_skill"}, 0, 0, NULL, gen_skillstrings},
    {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y + 20 * 8, {gen_settings1}},
    {"NEXT ->",S_SKIP|S_NEXT,m_null,KB_NEXT,KB_Y + 20 * 8, {gen_settings3}},
    {0,S_SKIP|S_END,m_null}
};

enum {
  general_filterwall,
  general_filterfloor,
  general_filtersprite,
  general_filterpatch,
  general_filterz,
  general_filter_threshold,
  general_spriteedges,
  general_patchedges,
  general_hom,
};

#define G_YC  44

static const char *renderfilters[] = {"none", "point", "linear", "rounded"};
static const char *edgetypes[] = {"jagged", "sloped"};

setup_menu_t gen_settings3[] = {

  {"Renderer settings"     ,S_SKIP|S_TITLE, m_null, G_X, G_YB - 12},

  {"Filter for walls", S_CHOICE, m_null, G_X,
   G_YC + general_filterwall*8, {"filter_wall"}, 0, 0, NULL, renderfilters},

  {"Filter for floors/ceilings", S_CHOICE, m_null, G_X,
   G_YC + general_filterfloor*8, {"filter_floor"}, 0, 0, NULL, renderfilters},

  {"Filter for sprites", S_CHOICE, m_null, G_X,
   G_YC + general_filtersprite*8, {"filter_sprite"}, 0, 0, NULL, renderfilters},

  {"Filter for patches", S_CHOICE, m_null, G_X,
   G_YC + general_filterpatch*8, {"filter_patch"}, 0, 0, NULL, renderfilters},

  {"Filter for lighting", S_CHOICE, m_null, G_X,
   G_YC + general_filterz*8, {"filter_z"}, 0, 0, NULL, renderfilters},

  {"Drawing of sprite edges", S_CHOICE, m_null, G_X,
   G_YC + general_spriteedges*8, {"sprite_edges"}, 0, 0, NULL, edgetypes},

  {"Drawing of patch edges", S_CHOICE, m_null, G_X,
   G_YC + general_patchedges*8, {"patch_edges"}, 0, 0, NULL, edgetypes},

  {"Flashing HOM indicator", S_YESNO, m_null, G_X,
   G_YC + general_hom*8, {"flashing_hom"}},

  {"<- PREV",S_SKIP|S_PREV, m_null, KB_PREV, KB_Y+20*8, {gen_settings2}},



  {0,S_SKIP|S_END,m_null}
};

void M_Trans(void)
{
    R_InitTranMap(0);
}

void M_FullScreen(void)
{
  I_UpdateVideoMode();
  V_SetPalette(0);
}

void M_ChangeDemoSmoothTurns(void)
{
  if (demo_smoothturns)
    gen_settings2[12].m_flags &= ~(S_SKIP|S_SELECT);
  else
    gen_settings2[12].m_flags |= (S_SKIP|S_SELECT);

  R_SmoothPlaying_Reset(NULL);
}

void M_General(int choice)
{
  M_SetupNextMenu(&GeneralDef);

  setup_active = true;
  setup_screen = ss_gen;
  set_general_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = gen_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawGeneral(void)
{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(114, 2, 0, "M_GENERL", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

#define C_X  284
#define C_Y  32
#define COMP_SPC 12
#define C_NEXTPREV 131

setup_menu_t comp_settings1[], comp_settings2[], comp_settings3[];

setup_menu_t* comp_settings[] =
{
  comp_settings1,
  comp_settings2,
  comp_settings3,
  NULL
};

enum
{
  compat_telefrag,
  compat_dropoff,
  compat_falloff,
  compat_staylift,
  compat_doorstuck,
  compat_pursuit,
  compat_vile,
  compat_pain,
  compat_skull,
  compat_blazing,
  compat_doorlight = 0,
  compat_god,
  compat_infcheat,
  compat_zombie,
  compat_skymap,
  compat_stairs,
  compat_floors,
  compat_moveblock,
  compat_model,
  compat_zerotags,
  compat_666 = 0,
  compat_soul,
  compat_maskedanim,
};

setup_menu_t comp_settings1[] =
{
  {"Any monster can telefrag on MAP30", S_YESNO, m_null, C_X,
   C_Y + compat_telefrag * COMP_SPC, {"comp_telefrag"}},

  {"Some objects never hang over tall ledges", S_YESNO, m_null, C_X,
   C_Y + compat_dropoff * COMP_SPC, {"comp_dropoff"}},

  {"Objects don't fall under their own weight", S_YESNO, m_null, C_X,
   C_Y + compat_falloff * COMP_SPC, {"comp_falloff"}},

  {"Monsters randomly walk off of moving lifts", S_YESNO, m_null, C_X,
   C_Y + compat_staylift * COMP_SPC, {"comp_staylift"}},

  {"Monsters get stuck on doortracks", S_YESNO, m_null, C_X,
   C_Y + compat_doorstuck * COMP_SPC, {"comp_doorstuck"}},

  {"Monsters don't give up pursuit of targets", S_YESNO, m_null, C_X,
   C_Y + compat_pursuit * COMP_SPC, {"comp_pursuit"}},

  {"Arch-Vile resurrects invincible ghosts", S_YESNO, m_null, C_X,
   C_Y + compat_vile * COMP_SPC, {"comp_vile"}},

  {"Pain Elementals limited to 21 lost souls", S_YESNO, m_null, C_X,
   C_Y + compat_pain * COMP_SPC, {"comp_pain"}},

  {"Lost souls get stuck behind walls", S_YESNO, m_null, C_X,
   C_Y + compat_skull * COMP_SPC, {"comp_skull"}},

  {"Blazing doors make double closing sounds", S_YESNO, m_null, C_X,
   C_Y + compat_blazing * COMP_SPC, {"comp_blazing"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},

  {"NEXT ->",S_SKIP|S_NEXT, m_null, KB_NEXT, C_Y+C_NEXTPREV, {comp_settings2}},


  {0,S_SKIP|S_END,m_null}
};

setup_menu_t comp_settings2[] =
{
  {"Tagged doors don't trigger special lighting", S_YESNO, m_null, C_X,
   C_Y + compat_doorlight * COMP_SPC, {"comp_doorlight"}},

  {"God mode isn't absolute", S_YESNO, m_null, C_X,
   C_Y + compat_god * COMP_SPC, {"comp_god"}},

  {"Powerup cheats are not infinite duration", S_YESNO, m_null, C_X,
   C_Y + compat_infcheat * COMP_SPC, {"comp_infcheat"}},

  {"Zombie players can exit levels", S_YESNO, m_null, C_X,
   C_Y + compat_zombie * COMP_SPC, {"comp_zombie"}},

  {"Sky is unaffected by invulnerability", S_YESNO, m_null, C_X,
   C_Y + compat_skymap * COMP_SPC, {"comp_skymap"}},

  {"Use exactly Doom's stairbuilding method", S_YESNO, m_null, C_X,
   C_Y + compat_stairs * COMP_SPC, {"comp_stairs"}},

  {"Use exactly Doom's floor motion behavior", S_YESNO, m_null, C_X,
   C_Y + compat_floors * COMP_SPC, {"comp_floors"}},

  {"Use exactly Doom's movement clipping code", S_YESNO, m_null, C_X,
   C_Y + compat_moveblock * COMP_SPC, {"comp_moveblock"}},

  {"Use exactly Doom's linedef trigger model", S_YESNO, m_null, C_X,
   C_Y + compat_model * COMP_SPC, {"comp_model"}},

  {"Linedef effects work with sector tag = 0", S_YESNO, m_null, C_X,
   C_Y + compat_zerotags * COMP_SPC, {"comp_zerotags"}},

  {"<- PREV", S_SKIP|S_PREV, m_null, KB_PREV, C_Y+C_NEXTPREV,{comp_settings1}},

  {"NEXT ->",S_SKIP|S_NEXT, m_null, KB_NEXT, C_Y+C_NEXTPREV, {comp_settings3}},



  {0,S_SKIP|S_END,m_null}
};

setup_menu_t comp_settings3[] =
{
  {"All boss types can trigger tag 666 at ExM8", S_YESNO, m_null, C_X,
   C_Y + compat_666 * COMP_SPC, {"comp_666"}},
  {"Lost souls don't bounce off flat surfaces", S_YESNO, m_null, C_X,
   C_Y + compat_soul * COMP_SPC, {"comp_soul"}},
  {"2S middle textures do not animate", S_YESNO, m_null, C_X,
   C_Y + compat_maskedanim * COMP_SPC, {"comp_maskedanim"}},
  {"<- PREV", S_SKIP|S_PREV, m_null, KB_PREV, C_Y+C_NEXTPREV,{comp_settings2}},
  {0,S_SKIP|S_END,m_null}
};

void M_Compat(int choice)
{
  M_SetupNextMenu(&CompatDef);

  setup_active = true;
  setup_screen = ss_comp;
  set_general_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = comp_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawCompat(void)
{
  inhelpscreens = true;

  M_DrawBackground("FLOOR4_6", 0);
  V_DrawNamePatch(52,2,0,"M_COMPAT", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

#define M_X 230
#define M_Y  39

enum {
  mess_color_play,
  mess_timer,
  mess_color_chat,
  mess_chat_timer,
  mess_color_review,
  mess_timed,
  mess_hud_timer,
  mess_lines,
  mess_scrollup,
  mess_background,
};

setup_menu_t mess_settings1[];

setup_menu_t* mess_settings[] =
{
  mess_settings1,
  NULL
};

setup_menu_t mess_settings1[] =
{
  {"Message Color During Play", S_CRITEM, m_null, M_X,
   M_Y + mess_color_play*8, {"hudcolor_mesg"}},

  {"Chat Message Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_chat*8, {"hudcolor_chat"}},

  {"Message Review Color", S_CRITEM, m_null, M_X,
   M_Y + mess_color_review*8, {"hudcolor_list"}},

  {"Number of Review Message Lines", S_NUM, m_null,  M_X,
   M_Y + mess_lines*8, {"hud_msg_lines"}},

  {"Message Background",  S_YESNO,  m_null,  M_X,
   M_Y + mess_background*8, {"hud_list_bgon"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},



  {0,S_SKIP|S_END,m_null}
};

void M_Messages(int choice)
{
  M_SetupNextMenu(&MessageDef);

  setup_active = true;
  setup_screen = ss_mess;
  set_mess_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = mess_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawMessages(void)

{
  inhelpscreens = true;
  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(103, 2, 0, "M_MESS", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);
  if (default_verify)
    M_DrawDefVerify();
}

#define CS_X 20
#define CS_Y (31+8)

setup_menu_t chat_settings1[];

setup_menu_t* chat_settings[] =
{
  chat_settings1,
  NULL
};

setup_menu_t chat_settings1[] =
{
  {"1",S_CHAT,m_null,CS_X,CS_Y+ 1*8, {"chatmacro1"}},
  {"2",S_CHAT,m_null,CS_X,CS_Y+ 2*8, {"chatmacro2"}},
  {"3",S_CHAT,m_null,CS_X,CS_Y+ 3*8, {"chatmacro3"}},
  {"4",S_CHAT,m_null,CS_X,CS_Y+ 4*8, {"chatmacro4"}},
  {"5",S_CHAT,m_null,CS_X,CS_Y+ 5*8, {"chatmacro5"}},
  {"6",S_CHAT,m_null,CS_X,CS_Y+ 6*8, {"chatmacro6"}},
  {"7",S_CHAT,m_null,CS_X,CS_Y+ 7*8, {"chatmacro7"}},
  {"8",S_CHAT,m_null,CS_X,CS_Y+ 8*8, {"chatmacro8"}},
  {"9",S_CHAT,m_null,CS_X,CS_Y+ 9*8, {"chatmacro9"}},
  {"0",S_CHAT,m_null,CS_X,CS_Y+10*8, {"chatmacro0"}},


  {0,S_RESET,m_null,X_BUTTON,Y_BUTTON},


  {0,S_SKIP|S_END,m_null}

};

void M_ChatStrings(int choice)
{
  M_SetupNextMenu(&ChatStrDef);
  setup_active = true;
  setup_screen = ss_chat;
  set_chat_active = true;
  setup_select = false;
  default_verify = false;
  setup_gather = false;
  mult_screens_index = 0;
  current_setup_menu = chat_settings[0];
  set_menu_itemon = 0;
  while (current_setup_menu[set_menu_itemon++].m_flags & S_SKIP);
  current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
}

void M_DrawChatStrings(void)

{
  inhelpscreens = true;
  M_DrawBackground("FLOOR4_6", 0);

  V_DrawNamePatch(83, 2, 0, "M_CHAT", CR_DEFAULT, VPT_STRETCH);
  M_DrawInstructions();
  M_DrawScreenItems(current_setup_menu);

  if (default_verify)
    M_DrawDefVerify();
}

static boolean shiftdown = false;

static void M_SelectDone(setup_menu_t* ptr)
{
  ptr->m_flags &= ~S_SELECT;
  ptr->m_flags |= S_HILITE;
  S_StartSound(NULL,sfx_itemup);
  setup_select = false;
  colorbox_active = false;
  if (print_warning_about_changes)
    print_warning_about_changes--;
}

static setup_menu_t **setup_screens[] =
{
  keys_settings,
  weap_settings,
  stat_settings,
  auto_settings,
  enem_settings,
  mess_settings,
  chat_settings,
  gen_settings,
  comp_settings,
};

static void M_ResetDefaults(void)
{
  int i;

  default_t *dp;
  int warn = 0;

  for (i = 0; i < numdefaults ; i++)
  {
    dp = &defaults[i];

    if (dp->setupscreen == setup_screen)
      {
  setup_menu_t **l, *p;
  for (l = setup_screens[setup_screen-1]; *l; l++)
    for (p = *l; !(p->m_flags & S_END); p++)
      if (p->m_flags & S_HASDEFPTR ? p->var.def == dp :
    p->var.m_key == dp->location.pi ||
    p->m_mouse == dp->location.pi)
        {
    if (IS_STRING(*dp))
      free((char*)*dp->location.ppsz),
        *dp->location.ppsz = strdup(dp->defaultvalue.psz);
    else
      *dp->location.pi = dp->defaultvalue.i;

    if (p->action)
      p->action();

    goto end;
        }
      end:;
      }
  }

  if (warn)
    warn_about_changes(warn);
}

static void M_InitDefaults(void)
{
  setup_menu_t *const *p, *t;
  default_t *dp;
  int i;
  for (i = 0; i < ss_max-1; i++)
    for (p = setup_screens[i]; *p; p++)
      for (t = *p; !(t->m_flags & S_END); t++)
  if (t->m_flags & S_HASDEFPTR) {
    if (!(dp = M_LookupDefault(t->var.name)))
      I_Error("M_InitDefaults: Couldn't find config variable %s", t->var.name);
    else
      (t->var.def = dp)->setup_menu = t;
  }
}

int M_GetKeyString(int c,int offset)
{
  const char* s;

  if (c >= 33 && c <= 126) {

    if (c == '=')
      c = '+';
    else if (c == ',')
      c = '<';
    else if (c == '.')
      c = '>';
    menu_buffer[offset++] = c;
    menu_buffer[offset] = 0;

  } else {

    if ((0x100 <= c) && (c < 0x200)) {
      if (c == KEYD_KEYPADENTER)
  s = "PADE";
      else {
  strcpy(&menu_buffer[offset], "PAD");
  offset+=4;
  menu_buffer[offset-1] = c & 0xff;
  menu_buffer[offset] = 0;
      }
    } else if ((KEYD_F1 <= c) && (c < KEYD_F10)) {
      menu_buffer[offset++] = 'F';
      menu_buffer[offset++] = '1' + c - KEYD_F1;
      menu_buffer[offset]   = 0;
    } else {
      switch(c) {
      case KEYD_TAB:      s = "TAB";  break;
      case KEYD_ENTER:      s = "ENTR"; break;
      case KEYD_ESCAPE:     s = "ESC";  break;
      case KEYD_SPACEBAR:   s = "SPAC"; break;
      case KEYD_BACKSPACE:  s = "BACK"; break;
      case KEYD_RCTRL:      s = "CTRL"; break;
      case KEYD_LEFTARROW:  s = "LARR"; break;
      case KEYD_UPARROW:    s = "UARR"; break;
      case KEYD_RIGHTARROW: s = "RARR"; break;
      case KEYD_DOWNARROW:  s = "DARR"; break;
      case KEYD_RSHIFT:     s = "SHFT"; break;
      case KEYD_RALT:       s = "ALT";  break;
      case KEYD_CAPSLOCK:   s = "CAPS"; break;
      case KEYD_SCROLLLOCK: s = "SCRL"; break;
      case KEYD_HOME:       s = "HOME"; break;
      case KEYD_PAGEUP:     s = "PGUP"; break;
      case KEYD_END:        s = "END";  break;
      case KEYD_PAGEDOWN:   s = "PGDN"; break;
      case KEYD_INSERT:     s = "INST"; break;
      case KEYD_DEL:        s = "DEL"; break;
      case KEYD_F10:        s = "F10";  break;
      case KEYD_F11:        s = "F11";  break;
      case KEYD_F12:        s = "F12";  break;
      case KEYD_PAUSE:      s = "PAUS"; break;
      default:              s = "JUNK"; break;
      }

      if (s) {
  strcpy(&menu_buffer[offset],s);
  offset += strlen(s);
      }
    }
  }
  return offset;
}

#define KT_X1 283
#define KT_X2 172
#define KT_X3  87

#define KT_Y1   2
#define KT_Y2 118
#define KT_Y3 102

setup_menu_t helpstrings[] =
{
  {"SCREEN"      ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y1},
  {"HELP"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 1*8,{&key_help}},
  {"MENU"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 2*8,{&key_escape}},
  {"SETUP"       ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 3*8,{&key_setup}},
  {"PAUSE"       ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 4*8,{&key_pause}},
  {"AUTOMAP"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 5*8,{&key_map}},
  {"SOUND VOLUME",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 6*8,{&key_soundvolume}},
  {"HUD"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 7*8,{&key_hud}},
  {"MESSAGES"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 8*8,{&key_messages}},
  {"GAMMA FIX"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+ 9*8,{&key_gamma}},
  {"SPY"         ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+10*8,{&key_spy}},
  {"LARGER VIEW" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+11*8,{&key_zoomin}},
  {"SMALLER VIEW",S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+12*8,{&key_zoomout}},
  {"SCREENSHOT"  ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y1+13*8,{&key_screenshot}},
  {"AUTOMAP"     ,S_SKIP|S_TITLE,m_null,KT_X1,KT_Y2},
  {"FOLLOW MODE" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 1*8,{&key_map_follow}},
  {"ZOOM IN"     ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 2*8,{&key_map_zoomin}},
  {"ZOOM OUT"    ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 3*8,{&key_map_zoomout}},
  {"MARK PLACE"  ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 4*8,{&key_map_mark}},
  {"CLEAR MARKS" ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 5*8,{&key_map_clear}},
  {"FULL/ZOOM"   ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 6*8,{&key_map_gobig}},
  {"GRID"        ,S_SKIP|S_KEY,m_null,KT_X1,KT_Y2+ 7*8,{&key_map_grid}},
  {"WEAPONS"     ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y1},
  {"FIST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 1*8,{&key_weapon1}},
  {"PISTOL"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 2*8,{&key_weapon2}},
  {"SHOTGUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 3*8,{&key_weapon3}},
  {"CHAINGUN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 4*8,{&key_weapon4}},
  {"ROCKET"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 5*8,{&key_weapon5}},
  {"PLASMA"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 6*8,{&key_weapon6}},
  {"BFG 9000"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 7*8,{&key_weapon7}},
  {"CHAINSAW"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 8*8,{&key_weapon8}},
  {"SSG"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+ 9*8,{&key_weapon9}},
  {"BEST"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+10*8,{&key_weapontoggle}},
  {"FIRE"        ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y1+11*8,{&key_fire},&mousebfire},
  {"MOVEMENT"    ,S_SKIP|S_TITLE,m_null,KT_X3,KT_Y3},
  {"FORWARD"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 1*8,{&key_up},&mousebforward},
  {"BACKWARD"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 2*8,{&key_down}},
  {"TURN LEFT"   ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 3*8,{&key_left}},
  {"TURN RIGHT"  ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 4*8,{&key_right}},
  {"RUN"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 5*8,{&key_speed}},
  {"STRAFE LEFT" ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 6*8,{&key_strafeleft}},
  {"STRAFE RIGHT",S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 7*8,{&key_straferight}},
  {"STRAFE"      ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 8*8,{&key_strafe},&mousebstrafe},
  {"AUTORUN"     ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+ 9*8,{&key_autorun}},
  {"180 TURN"    ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+10*8,{&key_reverse}},
  {"USE"         ,S_SKIP|S_KEY,m_null,KT_X3,KT_Y3+11*8,{&key_use},&mousebforward},
  {"GAME"        ,S_SKIP|S_TITLE,m_null,KT_X2,KT_Y1},
  {"SAVE"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 1*8,{&key_savegame}},
  {"LOAD"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 2*8,{&key_loadgame}},
  {"QUICKSAVE"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 3*8,{&key_quicksave}},
  {"END GAME"    ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 4*8,{&key_endgame}},
  {"QUICKLOAD"   ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 5*8,{&key_quickload}},
  {"QUIT"        ,S_SKIP|S_KEY,m_null,KT_X2,KT_Y1+ 6*8,{&key_quit}},
  {0,S_SKIP|S_END,m_null}
};

#define SPACEWIDTH 4

static void M_DrawString(int cx, int cy, int color, const char* ch)
{
  int   w;
  int   c;

  while (*ch) {
    c = *ch++;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c> HU_FONTSIZE)
      {
      cx += SPACEWIDTH;
      continue;
      }
    w = hu_font[c].width;
    if (cx + w > 320)
      break;

    V_DrawNumPatch(cx, cy, 0, hu_font[c].lumpnum, color, VPT_STRETCH | VPT_TRANS);


    cx += w - 1;
  }
}



static void M_DrawMenuString(int cx, int cy, int color)
{
    M_DrawString(cx, cy, color, menu_buffer);
}

static int M_GetPixelWidth(const char* ch)
{
  int len = 0;
  int c;

  while (*ch) {
    c = *ch++;
    c = toupper(c) - HU_FONTSTART;
    if (c < 0 || c > HU_FONTSIZE)
      {
      len += SPACEWIDTH;
      continue;
      }
    len += hu_font[c].width;
    len--;
  }
  len++;
  return len;
}

static void M_DrawStringCentered(int cx, int cy, int color, const char* ch)
{
    M_DrawString(cx - M_GetPixelWidth(ch)/2, cy, color, ch);
}

enum {
  prog,
  prog_stub,
  prog_stub1,
  prog_stub2,
  adcr
};

enum {
  cr_prog=0,
  cr_adcr=2,
};

#define CR_S 9
#define CR_X 20
#define CR_X2 50
#define CR_Y 32
#define CR_SH 9

setup_menu_t cred_settings[]={

  {"Programmers",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*prog + CR_SH*cr_prog},
  {"Florian 'Proff' Schulze",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+1) + CR_SH*cr_prog},
  {"Colin Phipps",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+2) + CR_SH*cr_prog},
  {"Neil Stevens",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+3) + CR_SH*cr_prog},
  {"Andrey Budko",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(prog+4) + CR_SH*cr_prog},
  {"Additional Credit To",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X, CR_Y + CR_S*adcr + CR_SH*cr_adcr},
  {"id Software for DOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+1)+CR_SH*cr_adcr},
  {"TeamTNT for BOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+2)+CR_SH*cr_adcr},
  {"Lee Killough for MBF",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+3)+CR_SH*cr_adcr},
  {"The DOSDoom-Team for DOSDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+4)+CR_SH*cr_adcr},
  {"Randy Heit for ZDOOM",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+5)+CR_SH*cr_adcr},
  {"Michael 'Kodak' Ryssen for DOOMGL",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+6)+CR_SH*cr_adcr},
  {"Jess Haas for lSDLDoom",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+7) + CR_SH*cr_adcr},
  {"all others who helped (see AUTHORS file)",S_SKIP|S_CREDIT|S_LEFTJUST,m_null, CR_X2, CR_Y + CR_S*(adcr+8)+CR_SH*cr_adcr},
  {0,S_SKIP|S_END,m_null}
};

void M_DrawCredits(void)
{
  inhelpscreens = true;
  M_DrawBackground(gamemode==shareware ? "CEIL5_1" : "MFLR8_4", 0);
  V_DrawNamePatch(115,9,0, "PRBOOM",CR_GOLD, VPT_TRANS | VPT_STRETCH);
  M_DrawScreenItems(cred_settings);
}

static int M_IndexInChoices(const char *str, const char **choices) {
  int i = 0;

  while (*choices != NULL) {
    if (!strcmp(str, *choices))
      return i;
    i++;
    choices++;
  }
  return 0;
}

boolean M_Responder(event_t* ev)
{

    int ch = -1;
    int i;

    if (ev->type == ev_keydown)
    {

        ch = ev->data1;

        if (ch == KEYD_RSHIFT)
            shiftdown = true;

    }

    else if (ev->type == ev_keyup)
    {

        if (ev->data1 == KEYD_RSHIFT)
            shiftdown = false;

    }

    if (ch == -1)
        return false;

    if (saveStringEnter)
    {

        if (ch == key_menu_backspace)
        {

            if (saveCharIndex > 0)
            {

                saveCharIndex--;
                savegamestrings[saveSlot][saveCharIndex] = 0;

            }
        }

        else if (ch == key_menu_escape)
        {

            saveStringEnter = 0;
            strcpy(&savegamestrings[saveSlot][0],saveOldString);

        }

        else if (ch == key_menu_enter)
        {

            saveStringEnter = 0;

            if (savegamestrings[saveSlot][0])
                M_DoSave(saveSlot);

        }

        else
        {

            ch = toupper(ch);

            if (ch >= 32 && ch <= 127 && saveCharIndex < SAVESTRINGSIZE - 1 && M_StringWidth(savegamestrings[saveSlot]) < (SAVESTRINGSIZE - 2) * 8)
            {
            
                savegamestrings[saveSlot][saveCharIndex++] = ch;
                savegamestrings[saveSlot][saveCharIndex] = 0;

            }

        }

        return true;

    }

    if (messageToPrint)
    {

        if (messageNeedsInput == true && !(ch == ' ' || ch == 'n' || ch == 'y' || ch == key_escape))
            return false;

        menuactive = messageLastMenuActive;
        messageToPrint = 0;

        if (messageRoutine)
            messageRoutine(ch);

        menuactive = false;
        S_StartSound(NULL, sfx_swtchx);

        return true;

    }

    if (!menuactive)
    {

        if (ch == key_autorun)
        {

            autorun = !autorun;

            return true;

        }

        if (ch == key_hud)
        {

            if ((automapmode & am_active) || chat_on)
                return false;

            if (screenSize < 8)
            {

                while (screenSize < 8 || !hud_displayed)
                    M_SizeDisplay(1);

            }

            else
            {

                hud_displayed = 1;
                hud_active = (hud_active + 1) % 3;

                if (!hud_active)
                {

                    hud_distributed = !hud_distributed;
                    HU_MoveHud();

                }
            }

            return true;
        }

        if (ch == key_escape)
        {

            M_StartControlPanel ();
            S_StartSound(NULL, sfx_swtchn);

            return true;

        }

        return false;

    }

    if (setup_active)
    {

        setup_menu_t *ptr1= current_setup_menu + set_menu_itemon;
        setup_menu_t *ptr2 = NULL;

        if (default_verify)
        {

            if (toupper(ch) == 'Y')
            {

                M_ResetDefaults();

                default_verify = false;

                M_SelectDone(ptr1);

            }

            else if (toupper(ch) == 'N')
            {

                default_verify = false;

                M_SelectDone(ptr1);

            }

            return true;

        }

        if (setup_select)
        {

            if (ch == key_menu_escape)
            {

                M_SelectDone(ptr1);

                setup_gather = false;

                return true;

            }

            if (ptr1->m_flags & S_YESNO)
            {

                if (ch == key_menu_enter)
                {

                    *ptr1->var.def->location.pi = !*ptr1->var.def->location.pi;

                    if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
                        warn_about_changes(ptr1->m_flags & (S_LEVWARN | S_PRGWARN));
                    else
                        M_UpdateCurrent(ptr1->var.def);

                    if (ptr1->action)
                        ptr1->action();

                }

                M_SelectDone(ptr1);

                return true;

            }

            if (ptr1->m_flags & S_CRITEM)
            {

                if (ch != key_menu_enter)
                {

                    ch -= 0x30;

                    if (ch < 0 || ch > 9)
                        return true;

                    *ptr1->var.def->location.pi = ch;

                }

                if (ptr1->action)
                    ptr1->action();

                M_SelectDone(ptr1);

                return true;

            }

  if (ptr1->m_flags & S_NUM)
    {
      if (setup_gather) {
        if (ch == key_menu_enter) {
    if (gather_count) {
      int value;

      gather_buffer[gather_count] = 0;
      value = atoi(gather_buffer);

      if ((ptr1->var.def->minvalue != UL &&
           value < ptr1->var.def->minvalue) ||
          (ptr1->var.def->maxvalue != UL &&
           value > ptr1->var.def->maxvalue))
        warn_about_changes(S_BADVAL);
      else {
        *ptr1->var.def->location.pi = value;

        if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
          warn_about_changes(ptr1->m_flags &
           (S_LEVWARN | S_PRGWARN));
        else
          M_UpdateCurrent(ptr1->var.def);

        if (ptr1->action)
          ptr1->action();
      }
    }
    M_SelectDone(ptr1);
    setup_gather = false;
    return true;
        }

        if (ch == key_menu_backspace && gather_count) {
    gather_count--;
    return true;
        }

        if (gather_count >= MAXGATHER)
    return true;

        if (!isdigit(ch) && ch != '-')
    return true;

        /* killough 10/98: character-based numerical input */
        gather_buffer[gather_count++] = ch;
      }
      return true;
    }

  if (ptr1->m_flags & S_CHOICE)
    {
    if (ch == key_menu_left) {
      if (ptr1->var.def->type == def_int) {
        int value = *ptr1->var.def->location.pi;

        value = value - 1;
        if ((ptr1->var.def->minvalue != UL &&
             value < ptr1->var.def->minvalue))
          value = ptr1->var.def->minvalue;
        if ((ptr1->var.def->maxvalue != UL &&
             value > ptr1->var.def->maxvalue))
          value = ptr1->var.def->maxvalue;
        if (*ptr1->var.def->location.pi != value)
          S_StartSound(NULL,sfx_pstop);
        *ptr1->var.def->location.pi = value;
      }
      if (ptr1->var.def->type == def_str) {
        int old_value, value;

        old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz,
                                     ptr1->selectstrings);
        value = old_value - 1;
        if (value < 0)
          value = 0;
        if (old_value != value)
          S_StartSound(NULL,sfx_pstop);
        *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
      }
    }
    if (ch == key_menu_right) {
      if (ptr1->var.def->type == def_int) {
        int value = *ptr1->var.def->location.pi;

        value = value + 1;
        if ((ptr1->var.def->minvalue != UL &&
             value < ptr1->var.def->minvalue))
          value = ptr1->var.def->minvalue;
        if ((ptr1->var.def->maxvalue != UL &&
             value > ptr1->var.def->maxvalue))
          value = ptr1->var.def->maxvalue;
        if (*ptr1->var.def->location.pi != value)
          S_StartSound(NULL,sfx_pstop);
        *ptr1->var.def->location.pi = value;
      }
      if (ptr1->var.def->type == def_str) {
        int old_value, value;

        old_value = M_IndexInChoices(*ptr1->var.def->location.ppsz,
                                     ptr1->selectstrings);
        value = old_value + 1;
        if (ptr1->selectstrings[value] == NULL)
          value = old_value;
        if (old_value != value)
          S_StartSound(NULL,sfx_pstop);
        *ptr1->var.def->location.ppsz = ptr1->selectstrings[value];
      }
    }
    if (ch == key_menu_enter) {

      if (ptr1->m_flags & (S_LEVWARN | S_PRGWARN))
        warn_about_changes(ptr1->m_flags &
         (S_LEVWARN | S_PRGWARN));
      else
        M_UpdateCurrent(ptr1->var.def);

      if (ptr1->action)
        ptr1->action();
      M_SelectDone(ptr1);
    }
    return true;
    }

      }

      if (set_keybnd_active)
  if (setup_select)
    {
      if (ev->type == ev_mouse)
        {
    int i,oldbutton,group;
    boolean search = true;

    if (!ptr1->m_mouse)
      return true;

    oldbutton = *ptr1->m_mouse;
    group  = ptr1->m_group;
    if (ev->data1 & 1)
      ch = 0;
    else if (ev->data1 & 2)
      ch = 1;
    else if (ev->data1 & 4)
      ch = 2;
    else
      return true;
    for (i = 0 ; keys_settings[i] && search ; i++)
      for (ptr2 = keys_settings[i] ; !(ptr2->m_flags & S_END) ; ptr2++)
        if (ptr2->m_group == group && ptr1 != ptr2)
          if (ptr2->m_flags & S_KEY && ptr2->m_mouse)
      if (*ptr2->m_mouse == ch)
        {
          *ptr2->m_mouse = oldbutton;
          search = false;
          break;
        }
    *ptr1->m_mouse = ch;
        }
      else
        {
    int i,oldkey,group;
    boolean search = true;













    oldkey = *ptr1->var.m_key;
    group  = ptr1->m_group;
    for (i = 0 ; keys_settings[i] && search ; i++)
      for (ptr2 = keys_settings[i] ; !(ptr2->m_flags & S_END) ; ptr2++)
        if (ptr2->m_flags & (S_KEY|S_KEEP) &&
      ptr2->m_group == group &&
      ptr1 != ptr2)
          if (*ptr2->var.m_key == ch)
      {
        if (ptr2->m_flags & S_KEEP)
          return true;
        *ptr2->var.m_key = oldkey;
        search = false;
        break;
      }
    *ptr1->var.m_key = ch;
        }

      M_SelectDone(ptr1);
      return true;
    }



      if (set_weapon_active)
  if (setup_select)
    {
      if (ch != key_menu_enter)
        {
    ch -= '0';
    if (ch < 1 || ch > 9)
      return true;

    for (i = 0; (ptr2 = weap_settings[i]); i++)
      for (; !(ptr2->m_flags & S_END); ptr2++)
        if (ptr2->m_flags & S_WEAP &&
      *ptr2->var.def->location.pi == ch && ptr1 != ptr2)
          {
      *ptr2->var.def->location.pi = *ptr1->var.def->location.pi;
      goto end;
          }
        end:
    *ptr1->var.def->location.pi = ch;
        }

      M_SelectDone(ptr1);
      return true;
    }

      if (set_auto_active)
  if (setup_select)
    {
      if (ch == key_menu_down)
        {
    if (++color_palette_y == 16)
      color_palette_y = 0;
    S_StartSound(NULL,sfx_itemup);
    return true;
        }

      if (ch == key_menu_up)
        {
    if (--color_palette_y < 0)
      color_palette_y = 15;
    S_StartSound(NULL,sfx_itemup);
    return true;
        }

      if (ch == key_menu_left)
        {
    if (--color_palette_x < 0)
      color_palette_x = 15;
    S_StartSound(NULL,sfx_itemup);
    return true;
        }

      if (ch == key_menu_right)
        {
    if (++color_palette_x == 16)
      color_palette_x = 0;
    S_StartSound(NULL,sfx_itemup);
    return true;
        }

      if (ch == key_menu_enter)
        {
    *ptr1->var.def->location.pi = color_palette_x + 16*color_palette_y;
    M_SelectDone(ptr1);
    colorbox_active = false;
    return true;
        }
    }


      if (setup_select &&
    set_enemy_active | set_general_active | set_chat_active |
    set_mess_active | set_status_active | set_compat_active)
  {
    if (ptr1->m_flags & S_STRING)
      {
        if (ch == key_menu_backspace)
    {
      if (chat_string_buffer[chat_index] == 0)
        {
          if (chat_index > 0)
      chat_string_buffer[--chat_index] = 0;
        }

      else
        strcpy(&chat_string_buffer[chat_index],
         &chat_string_buffer[chat_index+1]);
    }
        else if (ch == key_menu_left)
    {
      if (chat_index > 0)
        chat_index--;
    }
        else if (ch == key_menu_right)
    {
      if (chat_string_buffer[chat_index] != 0)
        chat_index++;
    }
        else if ((ch == key_menu_enter) ||
           (ch == key_menu_escape))
    {
      *ptr1->var.def->location.ppsz = chat_string_buffer;
      M_SelectDone(ptr1);
    }

        else if ((ch >= 32) && (ch <= 126))
    if ((chat_index+1) < CHAT_STRING_BFR_SIZE)
      {
        if (shiftdown)
          ch = shiftxform[ch];
        if (chat_string_buffer[chat_index] == 0)
          {
      chat_string_buffer[chat_index++] = ch;
      chat_string_buffer[chat_index] = 0;
          }
        else
          chat_string_buffer[chat_index++] = ch;
      }
        return true;
      }

    M_SelectDone(ptr1);
    return true;
  }

      if (ch == key_menu_down)
  {
    ptr1->m_flags &= ~S_HILITE;
    do
      if (ptr1->m_flags & S_END)
        {
    set_menu_itemon = 0;
    ptr1 = current_setup_menu;
        }
      else
        {
    set_menu_itemon++;
    ptr1++;
        }
    while (ptr1->m_flags & S_SKIP);
    M_SelectDone(ptr1);
    return true;
  }

      if (ch == key_menu_up)
  {
    ptr1->m_flags &= ~S_HILITE;
    do
      {
        if (set_menu_itemon == 0)
    do
      set_menu_itemon++;
    while(!((current_setup_menu + set_menu_itemon)->m_flags & S_END));
        set_menu_itemon--;
      }
    while((current_setup_menu + set_menu_itemon)->m_flags & S_SKIP);
    M_SelectDone(current_setup_menu + set_menu_itemon);
    return true;
  }

      if (ch == key_menu_enter)
  {
    int flags = ptr1->m_flags;







    if (flags & S_NUM)
      {
        setup_gather = true;
        print_warning_about_changes = false;
        gather_count = 0;
      }
    else if (flags & S_COLOR)
      {
        int color = *ptr1->var.def->location.pi;

        if (color < 0 || color > 255)
    color = 0;

        color_palette_x = *ptr1->var.def->location.pi & 15;
        color_palette_y = *ptr1->var.def->location.pi >> 4;
        colorbox_active = true;
      }
    else if (flags & S_STRING)
      {






        chat_string_buffer = malloc(CHAT_STRING_BFR_SIZE);
        strncpy(chat_string_buffer,
          *ptr1->var.def->location.ppsz, CHAT_STRING_BFR_SIZE);


        chat_string_buffer[CHAT_STRING_BFR_SIZE-1] = 0;




        free((char*)*ptr1->var.def->location.ppsz);
        *ptr1->var.def->location.ppsz = chat_string_buffer;
        chat_index = 0;
      }
    else if (flags & S_RESET)
      default_verify = true;

    ptr1->m_flags |= S_SELECT;
    setup_select = true;
    S_StartSound(NULL,sfx_itemup);
    return true;
  }

      if ((ch == key_menu_escape) || (ch == key_menu_backspace))
  {
    if (ch == key_menu_escape)
      M_ClearMenus();
    else
      if (currentMenu->prevMenu)
        {
    currentMenu = currentMenu->prevMenu;
    itemOn = currentMenu->lastOn;
    S_StartSound(NULL,sfx_swtchn);
        }
    ptr1->m_flags &= ~(S_HILITE|S_SELECT);
    setup_active = false;
    set_keybnd_active = false;
    set_weapon_active = false;
    set_status_active = false;
    set_auto_active = false;
    set_enemy_active = false;
    set_mess_active = false;
    set_chat_active = false;
    colorbox_active = false;
    default_verify = false;
    set_general_active = false;
          set_compat_active = false;
    HU_Start();
    S_StartSound(NULL,sfx_swtchx);
    return true;
  }

      if (ch == key_menu_left)
  {
    ptr2 = ptr1;
    do
      {
        ptr2++;
        if (ptr2->m_flags & S_PREV)
    {
      ptr1->m_flags &= ~S_HILITE;
      mult_screens_index--;
      current_setup_menu = ptr2->var.menu;
      set_menu_itemon = 0;
      print_warning_about_changes = false;
      while (current_setup_menu[set_menu_itemon++].m_flags&S_SKIP);
      current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
      S_StartSound(NULL,sfx_pstop);
      return true;
    }
      }
    while (!(ptr2->m_flags & S_END));
  }

      if (ch == key_menu_right)
  {
    ptr2 = ptr1;
    do
      {
        ptr2++;
        if (ptr2->m_flags & S_NEXT)
    {
      ptr1->m_flags &= ~S_HILITE;
      mult_screens_index++;
      current_setup_menu = ptr2->var.menu;
      set_menu_itemon = 0;
      print_warning_about_changes = false;
      while (current_setup_menu[set_menu_itemon++].m_flags&S_SKIP);
      current_setup_menu[--set_menu_itemon].m_flags |= S_HILITE;
      S_StartSound(NULL,sfx_pstop);
      return true;
    }
      }
    while (!(ptr2->m_flags & S_END));
  }

    }




  if (ch == key_menu_down)
    {
      do
  {
    if (itemOn+1 > currentMenu->numitems-1)
      itemOn = 0;
    else
      itemOn++;
    S_StartSound(NULL,sfx_pstop);
  }
      while(currentMenu->menuitems[itemOn].status==-1);
      return true;
    }

  if (ch == key_menu_up)
    {
      do
  {
    if (!itemOn)
      itemOn = currentMenu->numitems-1;
    else
      itemOn--;
    S_StartSound(NULL,sfx_pstop);
  }
      while(currentMenu->menuitems[itemOn].status==-1);
      return true;
    }

  if (ch == key_menu_left)
    {
      if (currentMenu->menuitems[itemOn].routine &&
    currentMenu->menuitems[itemOn].status == 2)
  {
    S_StartSound(NULL,sfx_stnmov);
    currentMenu->menuitems[itemOn].routine(0);
  }
      return true;
    }

  if (ch == key_menu_right)
    {
      if (currentMenu->menuitems[itemOn].routine &&
    currentMenu->menuitems[itemOn].status == 2)
  {
    S_StartSound(NULL,sfx_stnmov);
    currentMenu->menuitems[itemOn].routine(1);
  }
      return true;
    }

    if (ch == key_menu_enter)
    {

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status)
        {

            currentMenu->lastOn = itemOn;

            if (currentMenu->menuitems[itemOn].status == 2)
            {

                currentMenu->menuitems[itemOn].routine(1);
                S_StartSound(NULL,sfx_stnmov);

            }

            else
            {

                currentMenu->menuitems[itemOn].routine(itemOn);
                S_StartSound(NULL,sfx_pistol);

            }

        }

        return true;

    }

    if (ch == key_menu_escape)
    {

        currentMenu->lastOn = itemOn;

        M_ClearMenus();
        S_StartSound(NULL, sfx_swtchx);

        return true;

    }

    if (ch == key_menu_backspace)
    {

        currentMenu->lastOn = itemOn;

        if (currentMenu->prevMenu)
        {

            currentMenu = currentMenu->prevMenu;

            itemOn = currentMenu->lastOn;
            S_StartSound(NULL,sfx_swtchn);

        }

        return true;
    }

    return false;

}


void M_StartControlPanel (void)
{

    if (menuactive)
        return;

    NewDef.lastOn = defaultskill - 1;
    default_verify = 0;
    menuactive = 1;
    currentMenu = &MainDef;
    itemOn = currentMenu->lastOn;
    print_warning_about_changes = false;

}

void M_Drawer(void)
{

    inhelpscreens = false;

    if (messageToPrint)
    {

        char *ms = strdup(messageString);
        char *p = ms;
        int y = 100 - M_StringHeight(messageString) / 2;

        while (*p)
        {

            char *string = p, c;

            while ((c = *p) && *p != '\n')
                p++;

            *p = 0;

            M_WriteText(160 - M_StringWidth(string) / 2, y, string);

            y += hu_font[0].height;

            if ((*p = c))
                p++;

        }

        free(ms);

    }

    else if (menuactive)
    {

        int x, y, max, i;

        if (currentMenu->routine)
            currentMenu->routine();

        x = currentMenu->x;
        y = currentMenu->y;
        max = currentMenu->numitems;

        for (i = 0; i < max; i++)
        {

            if (currentMenu->menuitems[i].name[0])
                V_DrawNamePatch(x, y, 0, currentMenu->menuitems[i].name, CR_DEFAULT, VPT_STRETCH);

            y += LINEHEIGHT;

        }

        V_DrawNamePatch(x + SKULLXOFF, currentMenu->y - 5 + itemOn * LINEHEIGHT, 0, skullName[whichSkull], CR_DEFAULT, VPT_STRETCH);

    }

}

void M_ClearMenus(void)
{

    menuactive = 0;
    print_warning_about_changes = 0;
    default_verify = 0;

}

void M_SetupNextMenu(menu_t *menudef)
{

    currentMenu = menudef;
    itemOn = currentMenu->lastOn;

}

void M_Ticker(void)
{

    if (--skullAnimCounter <= 0)
    {

        whichSkull ^= 1;
        skullAnimCounter = 8;

    }

}

void M_StartMessage(const char *string, void *routine, boolean input)
{

    messageLastMenuActive = menuactive;
    messageToPrint = 1;
    messageString = string;
    messageRoutine = routine;
    messageNeedsInput = input;
    menuactive = true;

    return;

}

void M_StopMessage(void)
{

    menuactive = messageLastMenuActive;
    messageToPrint = 0;

}

void M_DrawThermo(int x, int y, int width, int dot)
{

    int xx;
    int i;
    int scaler;

    width = (width > 200) ? 200 : width;
    scaler = (width > 23) ? (200 / width) : 8;
    xx = x;

    V_DrawNamePatch(xx, y, 0, "M_THERML", CR_DEFAULT, VPT_STRETCH);

    xx += 8;

    for (i = 0; i < width; i++)
    {

        V_DrawNamePatch(xx, y, 0, "M_THERMM", CR_DEFAULT, VPT_STRETCH);

        xx += scaler;

    }

    xx += (8 - scaler);

    V_DrawNamePatch(xx, y, 0, "M_THERMR", CR_DEFAULT, VPT_STRETCH);
    V_DrawNamePatch((x + 8) + dot * scaler, y, 0, "M_THERMO", CR_DEFAULT, VPT_STRETCH);

}

void M_DrawEmptyCell (menu_t* menu,int item)
{

    V_DrawNamePatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0, "M_CELL1", CR_DEFAULT, VPT_STRETCH);

}

void M_DrawSelCell (menu_t* menu,int item)
{

    V_DrawNamePatch(menu->x - 10, menu->y + item * LINEHEIGHT - 1, 0, "M_CELL2", CR_DEFAULT, VPT_STRETCH);

}

int M_StringWidth(const char* string)
{

    int i, c, w = 0;

    for (i = 0; (size_t)i < strlen(string); i++)
        w += (c = toupper(string[i]) - HU_FONTSTART) < 0 || c >= HU_FONTSIZE ? 4 : hu_font[c].width;

    return w;

}

int M_StringHeight(const char* string)
{

    int i, h, height = h = hu_font[0].height;

    for (i = 0; string[i]; i++)
    {

        if (string[i] == '\n')
            h += height;
    }

    return h;

}

void M_WriteText (int x,int y,const char* string)
{

    int w;
    const char *ch = string;
    int c;
    int cx = x;
    int cy = y;

    while (1)
    {

        c = *ch++;

        if (!c)
            break;

        if (c == '\n')
        {

            cx = x;
            cy += 12;

            continue;

        }

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c>= HU_FONTSIZE)
        {

            cx += 4;

            continue;

        }

        w = hu_font[c].width;

        if (cx + w > SCREENWIDTH)
            break;

        V_DrawNumPatch(cx, cy, 0, hu_font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);

        cx += w;

    }

}

void M_Init(void)
{

    M_InitDefaults();
    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;
    screenSize = screenblocks - 3;
    messageToPrint = 0;
    messageString = NULL;
    messageLastMenuActive = menuactive;
    quickSaveSlot = -1;

    M_ChangeDemoSmoothTurns();

}

