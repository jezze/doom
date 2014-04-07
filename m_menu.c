#include <stdio.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "hu_stuff.h"
#include "g_game.h"
#include "s_sound.h"
#include "m_menu.h"
#include "v_video.h"
#include "i_system.h"
#include "i_video.h"
#include "i_sound.h"

extern patchnum_t hu_font[HU_FONTSIZE];
int mouseSensitivity_horiz;
int mouseSensitivity_vert;

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
void M_DrawMainMenu(void);
void M_DrawNewGame(void);
void M_DrawEpisode(void);

menu_t NewDef;

static void M_ClearMenus(void)
{

    menuactive = 0;

}

static void M_SetupNextMenu(menu_t *menudef)
{

    currentMenu = menudef;
    itemOn = currentMenu->lastOn;

}

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

    I_Exit(0);

}

boolean M_Responder(event_t *ev)
{

    if (ev->type != ev_keydown)
        return false;

    if (!menuactive)
    {

        if (ev->data1 == key_escape)
        {

            M_StartControlPanel();
            S_StartSound(NULL, sfx_swtchn);

            return true;

        }

        return false;

    }

    if (ev->data1 == key_menu_down)
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

    if (ev->data1 == key_menu_up)
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

    if (ev->data1 == key_menu_enter)
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

    if (ev->data1 == key_menu_escape)
    {

        currentMenu->lastOn = itemOn;

        M_ClearMenus();
        S_StartSound(NULL, sfx_swtchx);

        return true;

    }

    if (ev->data1 == key_menu_backspace)
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

