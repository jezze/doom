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
#include "i_main.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"
#include "r_fps.h"

extern patchnum_t hu_font[HU_FONTSIZE];
int mouseSensitivity_horiz;
int mouseSensitivity_vert;
int showMessages;
int screenblocks;

boolean menuactive;

#define SKULLXOFF  -32
#define LINEHEIGHT  16

typedef struct
{

    short status;
    char name[10];
    void (*routine)(int choice);

} menuitem_t;

typedef struct menu_s
{

    short numitems;
    struct menu_s *prevMenu;
    menuitem_t *menuitems;
    void  (*routine)();
    short x;
    short y;
    short lastOn;

} menu_t;

short itemOn;
short skullAnimCounter;
short whichSkull;
const char skullName[2][9] = {"M_SKULL1","M_SKULL2"};
menu_t* currentMenu;

void M_NewGame(int choice);
void M_Episode(int choice);
void M_ChooseSkill(int choice);
void M_QuitDOOM(int choice);
void M_StartGame(int choice);
void M_DrawMainMenu(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);
void M_SetupNextMenu(menu_t *menudef);
void M_ClearMenus (void);
void M_Setup(int choice);

menu_t NewDef;

enum
{

    newgame = 0,
    quitdoom,
    main_end

} main_e;

menuitem_t MainMenu[] = {
    {1, "M_NGAME", M_NewGame},
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

    G_DeferedInitNew(choice, epi + 1, 1);
    M_ClearMenus();

}

void M_QuitDOOM(int choice)
{

    exit(0);

}

static boolean shiftdown = false;

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

    if (!menuactive)
    {

        if (ch == key_autorun)
        {

            autorun = !autorun;

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

    if (ch == key_menu_down)
    {

        do
        {

            if (itemOn + 1 > currentMenu->numitems - 1)
                itemOn = 0;
            else
                itemOn++;

            S_StartSound(NULL, sfx_pstop);

        } while (currentMenu->menuitems[itemOn].status == -1);

        return true;

    }

    if (ch == key_menu_up)
    {

        do
        {

            if (!itemOn)
                itemOn = currentMenu->numitems - 1;
            else
                itemOn--;

            S_StartSound(NULL, sfx_pstop);

        } while (currentMenu->menuitems[itemOn].status == -1);

        return true;

    }

    if (ch == key_menu_left)
    {

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
        {

            S_StartSound(NULL, sfx_stnmov);

            currentMenu->menuitems[itemOn].routine(0);

        }

        return true;

    }

    if (ch == key_menu_right)
    {

        if (currentMenu->menuitems[itemOn].routine && currentMenu->menuitems[itemOn].status == 2)
        {

            S_StartSound(NULL, sfx_stnmov);

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
    menuactive = 1;
    currentMenu = &MainDef;
    itemOn = currentMenu->lastOn;

}

void M_Drawer(void)
{

    if (menuactive)
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

void M_Init(void)
{

    currentMenu = &MainDef;
    menuactive = 0;
    itemOn = currentMenu->lastOn;
    whichSkull = 0;
    skullAnimCounter = 10;

}

