#include "doomdef.h"
#include "doomstat.h"
#include "m_random.h"
#include "i_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_main.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_englsh.h"
#include "r_draw.h"

#define STARTREDPALS                    1
#define STARTBONUSPALS                  9
#define NUMREDPALS                      8
#define NUMBONUSPALS                    4
#define RADIATIONPAL                    13
#define ST_X                            0
#define ST_X2                           104
#define ST_FX                           (ST_X + 143)
#define ST_FY                           (ST_Y + 1)
#define ST_TALLNUMWIDTH                 (tallnum[0]->width)
#define ST_NUMPAINFACES                 5
#define ST_NUMSTRAIGHTFACES             3
#define ST_NUMTURNFACES                 2
#define ST_NUMSPECIALFACES              3
#define ST_FACESTRIDE                   (ST_NUMSTRAIGHTFACES + ST_NUMTURNFACES + ST_NUMSPECIALFACES)
#define ST_NUMEXTRAFACES                2
#define ST_NUMFACES                     (ST_FACESTRIDE * ST_NUMPAINFACES + ST_NUMEXTRAFACES)
#define ST_TURNOFFSET                   (ST_NUMSTRAIGHTFACES)
#define ST_OUCHOFFSET                   (ST_TURNOFFSET + ST_NUMTURNFACES)
#define ST_EVILGRINOFFSET               (ST_OUCHOFFSET + 1)
#define ST_RAMPAGEOFFSET                (ST_EVILGRINOFFSET + 1)
#define ST_GODFACE                      (ST_NUMPAINFACES * ST_FACESTRIDE)
#define ST_DEADFACE                     (ST_GODFACE + 1)
#define ST_FACESX                       (ST_X + 143)
#define ST_FACESY                       (ST_Y)
#define ST_EVILGRINCOUNT                (2 * TICRATE)
#define ST_STRAIGHTFACECOUNT            (TICRATE / 2)
#define ST_TURNCOUNT                    (1 * TICRATE)
#define ST_OUCHCOUNT                    (1 * TICRATE)
#define ST_RAMPAGEDELAY                 (2 * TICRATE)
#define ST_MUCHPAIN                     20
#define ST_AMMOWIDTH                    3
#define ST_AMMOX                        (ST_X + 44)
#define ST_AMMOY                        (ST_Y + 3)
#define ST_HEALTHWIDTH                  3
#define ST_HEALTHX                      (ST_X + 90)
#define ST_HEALTHY                      (ST_Y + 3)
#define ST_ARMSX                        (ST_X + 111)
#define ST_ARMSY                        (ST_Y + 4)
#define ST_ARMSBGX                      (ST_X + 104)
#define ST_ARMSBGY                      (ST_Y)
#define ST_ARMSXSPACE                   12
#define ST_ARMSYSPACE                   10
#define ST_FRAGSX                       (ST_X + 138)
#define ST_FRAGSY                       (ST_Y + 3)
#define ST_FRAGSWIDTH                   2
#define ST_ARMORWIDTH                   3
#define ST_ARMORX                       (ST_X + 221)
#define ST_ARMORY                       (ST_Y + 3)
#define ST_KEY0WIDTH                    8
#define ST_KEY0HEIGHT                   5
#define ST_KEY0X                        (ST_X + 239)
#define ST_KEY0Y                        (ST_Y + 3)
#define ST_KEY1WIDTH                    ST_KEY0WIDTH
#define ST_KEY1X                        (ST_X + 239)
#define ST_KEY1Y                        (ST_Y + 13)
#define ST_KEY2WIDTH                    ST_KEY0WIDTH
#define ST_KEY2X                        (ST_X + 239)
#define ST_KEY2Y                        (ST_Y + 23)
#define ST_AMMO0WIDTH                   3
#define ST_AMMO0HEIGHT                  6
#define ST_AMMO0X                       (ST_X + 288)
#define ST_AMMO0Y                       (ST_Y + 5)
#define ST_AMMO1WIDTH                   ST_AMMO0WIDTH
#define ST_AMMO1X                       (ST_X + 288)
#define ST_AMMO1Y                       (ST_Y + 11)
#define ST_AMMO2WIDTH                   ST_AMMO0WIDTH
#define ST_AMMO2X                       (ST_X + 288)
#define ST_AMMO2Y                       (ST_Y + 23)
#define ST_AMMO3WIDTH                   ST_AMMO0WIDTH
#define ST_AMMO3X                       (ST_X + 288)
#define ST_AMMO3Y                       (ST_Y + 17)
#define ST_MAXAMMO0WIDTH                3
#define ST_MAXAMMO0HEIGHT               5
#define ST_MAXAMMO0X                    (ST_X + 314)
#define ST_MAXAMMO0Y                    (ST_Y + 5)
#define ST_MAXAMMO1WIDTH                ST_MAXAMMO0WIDTH
#define ST_MAXAMMO1X                    (ST_X + 314)
#define ST_MAXAMMO1Y                    (ST_Y + 11)
#define ST_MAXAMMO2WIDTH                ST_MAXAMMO0WIDTH
#define ST_MAXAMMO2X                    (ST_X + 314)
#define ST_MAXAMMO2Y                    (ST_Y + 23)
#define ST_MAXAMMO3WIDTH                ST_MAXAMMO0WIDTH
#define ST_MAXAMMO3X                    (ST_X + 314)
#define ST_MAXAMMO3Y                    (ST_Y + 17)

static player_t *plyr;
static unsigned int st_clock;
static int st_msgcounter = 0;
static st_chatstateenum_t st_chatstate;
static st_stateenum_t st_gamestate;
static boolean st_statusbaron;
static boolean st_chat;
static boolean st_oldchat;
static boolean st_notdeathmatch;
static boolean st_armson;
static patchnum_t tallnum[10];
static patchnum_t tallpercent;
static patchnum_t shortnum[10];
static patchnum_t keys[NUMCARDS];
static patchnum_t faces[ST_NUMFACES];
static patchnum_t faceback;
static patchnum_t stbarbg;
static patchnum_t armsbg;
static patchnum_t arms[6][2];
static st_number_t w_ready;
int ammo_red;
int ammo_yellow;
int health_red;
int health_yellow;
int health_green;
int armor_red;
int armor_yellow;
int armor_green;
static st_percent_t w_health;
static st_binicon_t w_armsbg;
static st_multicon_t w_arms[6];
static st_multicon_t w_faces;
static st_multicon_t w_keyboxes[3];
static st_percent_t w_armor;
static st_number_t w_ammo[4];
static st_number_t w_maxammo[4];
static int st_oldhealth = -1;
static boolean oldweaponsowned[NUMWEAPONS];
static int st_facecount = 0;
static int st_faceindex = 0;
static int keyboxes[3];
static int st_randomnumber;
extern char *mapnames[];

static void ST_Stop(void);

static void ST_refreshBackground(void)
{

    int y = 0;

    if (st_statusbaron)
    {

        V_DrawNumPatch(ST_X, y, BG, stbarbg.lumpnum, CR_DEFAULT, VPT_STRETCH);

        if (st_armson)
            V_DrawNumPatch(ST_ARMSBGX, y, BG, armsbg.lumpnum, CR_DEFAULT, VPT_STRETCH);

        V_CopyRect(ST_X, y, BG, ST_SCALED_WIDTH, ST_SCALED_HEIGHT, ST_X, ST_SCALED_Y, FG, VPT_NONE);

    }

}

boolean ST_Responder(event_t *ev)
{

    if (ev->type == ev_keydown)
        return M_FindCheats(ev->data1);

    return false;

}

static int ST_calcPainOffset(void)
{

    static int lastcalc;
    static int oldhealth = -1;
    int health = plyr->health > 100 ? 100 : plyr->health;

    if (health != oldhealth)
    {

        lastcalc = ST_FACESTRIDE * (((100 - health) * ST_NUMPAINFACES) / 101);
        oldhealth = health;

    }

    return lastcalc;

}

static void ST_updateFaceWidget(void)
{

    int i;
    angle_t badguyangle;
    angle_t diffang;
    static int lastattackdown = -1;
    static int priority = 0;
    boolean doevilgrin;

    if (priority < 10)
    {

        if (!plyr->health)
        {

            priority = 9;
            st_faceindex = ST_DEADFACE;
            st_facecount = 1;

        }
    }

    if (priority < 9)
    {

        if (plyr->bonuscount)
        {

            doevilgrin = false;

            for (i = 0; i < NUMWEAPONS; i++)
            {

                if (oldweaponsowned[i] != plyr->weaponowned[i])
                {

                    doevilgrin = true;
                    oldweaponsowned[i] = plyr->weaponowned[i];

                }

            }

            if (doevilgrin)
            {

                priority = 8;
                st_facecount = ST_EVILGRINCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_EVILGRINOFFSET;

            }

        }

    }

    if (priority < 8)
    {

        if (plyr->damagecount && plyr->attacker && plyr->attacker != plyr->mo)
        {

            priority = 7;

            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {

                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;

            }

            else
            {

                badguyangle = R_PointToAngle2(plyr->mo->x, plyr->mo->y, plyr->attacker->x, plyr->attacker->y);

                if (badguyangle > plyr->mo->angle)
                {

                    diffang = badguyangle - plyr->mo->angle;
                    i = diffang > ANG180;

                }

                else
                {

                    diffang = plyr->mo->angle - badguyangle;
                    i = diffang <= ANG180;

                }

                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset();

                if (diffang < ANG45)
                {

                    st_faceindex += ST_RAMPAGEOFFSET;

                }

                else if (i)
                {

                    st_faceindex += ST_TURNOFFSET;

                }

                else
                {

                  st_faceindex += ST_TURNOFFSET + 1;

                }

            }

        }

    }

    if (priority < 7)
    {

        if (plyr->damagecount)
        {

            if (st_oldhealth - plyr->health > ST_MUCHPAIN)
            {

                priority = 7;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_OUCHOFFSET;

            }

            else
            {

                priority = 6;
                st_facecount = ST_TURNCOUNT;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;

            }

        }

    }

    if (priority < 6)
    {

        if (plyr->attackdown)
        {

            if (lastattackdown == -1)
            {

                lastattackdown = ST_RAMPAGEDELAY;

            }

            else if (!--lastattackdown)
            {

                priority = 5;
                st_faceindex = ST_calcPainOffset() + ST_RAMPAGEOFFSET;
                st_facecount = 1;
                lastattackdown = 1;

            }

        }

        else
        {

            lastattackdown = -1;

        }

    }

    if (priority < 5)
    {

        if ((plyr->cheats & CF_GODMODE) || plyr->powers[pw_invulnerability])
        {

            priority = 4;
            st_faceindex = ST_GODFACE;
            st_facecount = 1;

        }

    }

    if (!st_facecount)
    {

        st_faceindex = ST_calcPainOffset() + (st_randomnumber % 3);
        st_facecount = ST_STRAIGHTFACECOUNT;
        priority = 0;

    }

    st_facecount--;

}

int sts_traditional_keys;

static void ST_updateWidgets(void)
{

    static int largeammo = 1994;
    int i;

    if (weaponinfo[plyr->readyweapon].ammo == am_noammo)
        w_ready.num = &largeammo;
    else
        w_ready.num = &plyr->ammo[weaponinfo[plyr->readyweapon].ammo];

    w_ready.data = plyr->readyweapon;

    for (i = 0; i < 3; i++)
        keyboxes[i] = plyr->cards[i + 3] ? i + 3 : plyr->cards[i] ? i : -1;

    ST_updateFaceWidget();

    st_notdeathmatch = true;
    st_armson = st_statusbaron;

    if (!--st_msgcounter)
        st_chat = st_oldchat;

}

void ST_Ticker(void)
{

    st_clock++;
    st_randomnumber = P_Random(pr_misc);

    ST_updateWidgets();

    st_oldhealth = plyr->health;

}

int st_palette = 0;

static void ST_doPaletteStuff(void)
{

    int palette;
    int cnt = plyr->damagecount;

    if (plyr->powers[pw_strength])
    {

        int bzc = 12 - (plyr->powers[pw_strength] >> 6);

        if (bzc > cnt)
            cnt = bzc;

    }

    if (cnt)
    {

        palette = (cnt+7) >> 3;

        if (palette >= NUMREDPALS)
            palette = NUMREDPALS-1;

        if (menuactive)
            palette >>=1;

        palette += STARTREDPALS;

    }

    else if (plyr->bonuscount)
    {

        palette = (plyr->bonuscount + 7) >> 3;

        if (palette >= NUMBONUSPALS)
            palette = NUMBONUSPALS - 1;

        palette += STARTBONUSPALS;

    }

    else if (plyr->powers[pw_ironfeet] > 4 * 32 || plyr->powers[pw_ironfeet] & 8)
        palette = RADIATIONPAL;
    else
        palette = 0;

    if (palette != st_palette)
        V_SetPalette(st_palette = palette);

}

static void ST_drawWidgets(boolean refresh)
{

    int i;

    st_armson = st_statusbaron;

    if (*w_ready.num * 100 < ammo_red * plyr->maxammo[weaponinfo[w_ready.data].ammo])
        STlib_updateNum(&w_ready, CR_RED, refresh);
    else if (*w_ready.num * 100 < ammo_yellow * plyr->maxammo[weaponinfo[w_ready.data].ammo])
        STlib_updateNum(&w_ready, CR_GOLD, refresh);
    else
        STlib_updateNum(&w_ready, CR_GREEN, refresh);

    for (i = 0; i < 4; i++)
    {

        STlib_updateNum(&w_ammo[i], CR_DEFAULT, refresh);
        STlib_updateNum(&w_maxammo[i], CR_DEFAULT, refresh);

    }

    if (*w_health.n.num < health_red)
        STlib_updatePercent(&w_health, CR_RED, refresh);
    else if (*w_health.n.num < health_yellow)
        STlib_updatePercent(&w_health, CR_GOLD, refresh);
    else if (*w_health.n.num <= health_green)
        STlib_updatePercent(&w_health, CR_GREEN, refresh);
    else
        STlib_updatePercent(&w_health, CR_BLUE2, refresh);

    if (*w_armor.n.num < armor_red)
        STlib_updatePercent(&w_armor, CR_RED, refresh);
    else if (*w_armor.n.num < armor_yellow)
        STlib_updatePercent(&w_armor, CR_GOLD, refresh);
    else if (*w_armor.n.num <= armor_green)
        STlib_updatePercent(&w_armor, CR_GREEN, refresh);
    else
        STlib_updatePercent(&w_armor, CR_BLUE2, refresh);

    for (i = 0; i < 6; i++)
        STlib_updateMultIcon(&w_arms[i], refresh);

    STlib_updateMultIcon(&w_faces, refresh);

    for (i = 0; i < 3; i++)
        STlib_updateMultIcon(&w_keyboxes[i], refresh);

}

void ST_Drawer(void)
{

    ST_doPaletteStuff();
    ST_refreshBackground();
    ST_drawWidgets(true);

}

static void ST_loadGraphics(boolean doload)
{

    int i, facenum;
    char namebuf[9];

    for (i = 0; i < 10; i++)
    {

        sprintf(namebuf, "STTNUM%d", i);
        R_SetPatchNum(&tallnum[i],namebuf);
        sprintf(namebuf, "STYSNUM%d", i);
        R_SetPatchNum(&shortnum[i],namebuf);

    }

    R_SetPatchNum(&tallpercent,"STTPRCNT");

    for (i = 0; i < NUMCARDS; i++)
    {

        sprintf(namebuf, "STKEYS%d", i);
        R_SetPatchNum(&keys[i], namebuf);

    }

    R_SetPatchNum(&stbarbg, "STBAR");
    R_SetPatchNum(&armsbg, "STARMS");

    for (i = 0; i < 6; i++)
    {

        sprintf(namebuf, "STGNUM%d", i + 2);
        R_SetPatchNum(&arms[i][0], namebuf);

        arms[i][1] = shortnum[i + 2];

    }

    R_SetPatchNum(&faceback, "STFB0");

    facenum = 0;

    for (i = 0; i < ST_NUMPAINFACES; i++)
    {

        int j;

        for (j = 0; j < ST_NUMSTRAIGHTFACES; j++)
        {

            sprintf(namebuf, "STFST%d%d", i, j);
            R_SetPatchNum(&faces[facenum++], namebuf);

        }

        sprintf(namebuf, "STFTR%d0", i);
        R_SetPatchNum(&faces[facenum++], namebuf);
        sprintf(namebuf, "STFTL%d0", i);
        R_SetPatchNum(&faces[facenum++], namebuf);
        sprintf(namebuf, "STFOUCH%d", i);
        R_SetPatchNum(&faces[facenum++], namebuf);
        sprintf(namebuf, "STFEVL%d", i);
        R_SetPatchNum(&faces[facenum++], namebuf);
        sprintf(namebuf, "STFKILL%d", i);
        R_SetPatchNum(&faces[facenum++], namebuf);

    }

    R_SetPatchNum(&faces[facenum++], "STFGOD0");
    R_SetPatchNum(&faces[facenum++], "STFDEAD0");

}

static void ST_loadData(void)
{

    ST_loadGraphics(true);

}

static void ST_initData(void)
{

    int i;

    plyr = &players[displayplayer];
    st_clock = 0;
    st_chatstate = StartChatState;
    st_gamestate = FirstPersonState;
    st_statusbaron = true;
    st_oldchat = st_chat = false;
    st_faceindex = 0;
    st_palette = -1;
    st_oldhealth = -1;

    for (i = 0; i < NUMWEAPONS; i++)
        oldweaponsowned[i] = plyr->weaponowned[i];

    for (i = 0; i < 3; i++)
        keyboxes[i] = -1;

}

static void ST_createWidgets(void)
{

    int i;

    STlib_initNum(&w_ready, ST_AMMOX, ST_AMMOY, tallnum, &plyr->ammo[weaponinfo[plyr->readyweapon].ammo], &st_statusbaron, ST_AMMOWIDTH);

    w_ready.data = plyr->readyweapon;

    STlib_initPercent(&w_health, ST_HEALTHX, ST_HEALTHY, tallnum, &plyr->health, &st_statusbaron, &tallpercent);
    STlib_initBinIcon(&w_armsbg, ST_ARMSBGX, ST_ARMSBGY, &armsbg, &st_notdeathmatch, &st_statusbaron);

    for (i = 0; i < 6; i++)
        STlib_initMultIcon(&w_arms[i], ST_ARMSX + (i % 3) * ST_ARMSXSPACE, ST_ARMSY + (i / 3) * ST_ARMSYSPACE, arms[i], (int *)&plyr->weaponowned[i + 1], &st_armson);

    STlib_initMultIcon(&w_faces, ST_FACESX, ST_FACESY, faces, &st_faceindex, &st_statusbaron);
    STlib_initPercent(&w_armor, ST_ARMORX, ST_ARMORY, tallnum, &plyr->armorpoints, &st_statusbaron, &tallpercent);
    STlib_initMultIcon(&w_keyboxes[0], ST_KEY0X, ST_KEY0Y, keys, &keyboxes[0], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes[1], ST_KEY1X, ST_KEY1Y, keys, &keyboxes[1], &st_statusbaron);
    STlib_initMultIcon(&w_keyboxes[2], ST_KEY2X, ST_KEY2Y, keys, &keyboxes[2], &st_statusbaron);
    STlib_initNum(&w_ammo[0], ST_AMMO0X, ST_AMMO0Y, shortnum, &plyr->ammo[0], &st_statusbaron, ST_AMMO0WIDTH);
    STlib_initNum(&w_ammo[1], ST_AMMO1X, ST_AMMO1Y, shortnum, &plyr->ammo[1], &st_statusbaron, ST_AMMO1WIDTH);
    STlib_initNum(&w_ammo[2], ST_AMMO2X, ST_AMMO2Y, shortnum, &plyr->ammo[2], &st_statusbaron, ST_AMMO2WIDTH);
    STlib_initNum(&w_ammo[3], ST_AMMO3X, ST_AMMO3Y, shortnum, &plyr->ammo[3], &st_statusbaron, ST_AMMO3WIDTH);
    STlib_initNum(&w_maxammo[0], ST_MAXAMMO0X, ST_MAXAMMO0Y, shortnum, &plyr->maxammo[0], &st_statusbaron, ST_MAXAMMO0WIDTH);
    STlib_initNum(&w_maxammo[1], ST_MAXAMMO1X, ST_MAXAMMO1Y, shortnum, &plyr->maxammo[1], &st_statusbaron, ST_MAXAMMO1WIDTH);
    STlib_initNum(&w_maxammo[2], ST_MAXAMMO2X, ST_MAXAMMO2Y, shortnum, &plyr->maxammo[2], &st_statusbaron, ST_MAXAMMO2WIDTH);
    STlib_initNum(&w_maxammo[3], ST_MAXAMMO3X, ST_MAXAMMO3Y, shortnum, &plyr->maxammo[3], &st_statusbaron, ST_MAXAMMO3WIDTH);

}

static boolean st_stopped = true;

void ST_Start(void)
{

    if (!st_stopped)
        ST_Stop();

    ST_initData();
    ST_createWidgets();

    st_stopped = false;

}

static void ST_Stop(void)
{

    if (st_stopped)
        return;

    V_SetPalette(0);

    st_stopped = true;

}

void ST_Init(void)
{

    ST_loadData();

}

