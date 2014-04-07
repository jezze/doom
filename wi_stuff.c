#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_random.h"
#include "r_main.h"
#include "v_video.h"
#include "wi_stuff.h"
#include "s_sound.h"
#include "i_system.h"

#define NUMEPISODES                     4
#define NUMMAPS                         9
#define WI_TITLEY                       2
#define WI_SPACINGY                     33
#define SP_STATSX                       50
#define SP_STATSY                       50
#define SP_TIMEX                        8
#define SP_TIMEY                        160
#define FB                              0
#define SP_KILLS                        0
#define SP_ITEMS                        2
#define SP_SECRET                       4
#define SP_FRAGS                        6
#define SP_TIME                         8
#define SP_PAR                          ST_TIME
#define SP_PAUSE                        1
#define SHOWNEXTLOCDELAY                4

typedef enum
{

    ANIM_ALWAYS,
    ANIM_RANDOM,
    ANIM_LEVEL

} animenum_t;

typedef struct
{
    int x;
    int y;

} point_t;

typedef struct
{

    animenum_t type;
    int period;
    int nanims;
    point_t loc;
    int data1;
    int data2;
    patchnum_t p[3];
    int nexttic;
    int lastdrawn;
    int ctr;
    int state;

} anim_t;

static point_t lnodes[NUMEPISODES][NUMMAPS] = {
  {
    { 185, 164 },
    { 148, 143 },
    { 69, 122 },
    { 209, 102 },
    { 116, 89 },
    { 166, 55 },
    { 71, 56 },
    { 135, 29 },
    { 71, 24 }
  },
  {
    { 254, 25 },
    { 97, 50 },
    { 188, 64 },
    { 128, 78 },
    { 214, 92 },
    { 133, 130 },
    { 208, 136 },
    { 148, 140 },
    { 235, 158 }
  },
  {
    { 156, 168 },
    { 48, 154 },
    { 174, 95 },
    { 265, 75 },
    { 130, 48 },
    { 279, 23 },
    { 198, 48 },
    { 140, 25 },
    { 281, 136 }
  }
};

static anim_t epsd0animinfo[] = {
  { ANIM_ALWAYS, TICRATE/3, 3, { 224, 104 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 184, 160 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 112, 136 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 72, 112 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 88, 96 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 64, 48 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 192, 40 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 136, 16 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 80, 16 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 64, 24 } }
};

static anim_t epsd1animinfo[] = {
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 1 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 2 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 3 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 4 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 5 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 6 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 7 },
  { ANIM_LEVEL,  TICRATE/3, 3, { 192, 144 }, 8 },
  { ANIM_LEVEL,  TICRATE/3, 1, { 128, 136 }, 8 }
};

static anim_t epsd2animinfo[] = {
  { ANIM_ALWAYS, TICRATE/3, 3, { 104, 168 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 40, 136 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 160, 96 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 104, 80 } },
  { ANIM_ALWAYS, TICRATE/3, 3, { 120, 32 } },
  { ANIM_ALWAYS, TICRATE/4, 3, { 40, 0 } }
};

static int NUMANIMS[NUMEPISODES] = {
    sizeof (epsd0animinfo) / sizeof (anim_t),
    sizeof (epsd1animinfo) / sizeof (anim_t),
    sizeof (epsd2animinfo) / sizeof (anim_t)
};

static anim_t *anims[NUMEPISODES] = {
    epsd0animinfo,
    epsd1animinfo,
    epsd2animinfo
};

int acceleratestage;
static int me;
static stateenum_t  state;
static wbstartstruct_t* wbs;
static wbplayerstruct_t* plrs;
static int cnt;
static int bcnt;
static int firstrefresh;
static int cnt_time;
static int cnt_total_time;
static int cnt_par;
static int cnt_pause;
static short *cnt_kills;
static short *cnt_items;
static short *cnt_secret;
static short *cnt_frags;
static int sp_state;
static boolean snl_pointeron = false;
static const char* yah[2] = { "WIURH0", "WIURH1" };
static const char* splat = "WISPLAT";
static const char percent[] = {"WIPCNT"};
static const char colon[] = {"WICOLON"};
static patchnum_t num[10];
static const char wiminus[] = {"WIMINUS"};
static const char finished[] = {"WIF"};
static const char entering[] = {"WIENTER"};
static const char sp_secret[] = {"WISCRT2"};
static const char kills[] = {"WIOSTK"};
static const char secret[] = {"WIOSTS"};
static const char items[] = {"WIOSTI"};
static const char frags[] = {"WIFRGS"};
static const char time1[] = {"WITIME"};
static const char par[] = {"WIPAR"};
static const char sucks[] = {"WISUCKS"};
static const char total[] = {"WIMSTT"};
static const char star[] = {"STFST01"};
static const char bstar[] = {"STFDEAD0"};
static const char facebackp[] = {"STPB0"};

static void WI_levelNameLump(int epis, int map, char *buf)
{

    if (gamemode == commercial)
        sprintf(buf, "CWILV%2.2d", map);
    else
        sprintf(buf, "WILV%d%d", epis, map);

}

static void WI_slamBackground(void)
{

    char name[9];

    if (gamemode == commercial || (gamemode == retail && wbs->epsd == 3))
        strcpy(name, "INTERPIC");
    else
        sprintf(name, "WIMAP%d", wbs->epsd);

    V_DrawNamePatch(0, 0, FB, name, CR_DEFAULT, VPT_STRETCH);

}

boolean WI_Responder(event_t *ev)
{

    return false;

}

static void WI_drawLF(void)
{

    int y = WI_TITLEY;
    char lname[9];

    WI_levelNameLump(wbs->epsd, wbs->last, lname);
    V_DrawNamePatch((320 - V_NamePatchWidth(lname)) / 2, y, FB, lname, CR_DEFAULT, VPT_STRETCH);

    y += (5 * V_NamePatchHeight(lname)) / 4;

    V_DrawNamePatch((320 - V_NamePatchWidth(finished)) / 2, y, FB, finished, CR_DEFAULT, VPT_STRETCH);

}

static void WI_drawEL(void)
{

    int y = WI_TITLEY;
    char lname[9];

    WI_levelNameLump(wbs->epsd, wbs->next, lname);
    V_DrawNamePatch((320 - V_NamePatchWidth(entering)) / 2, y, FB, entering, CR_DEFAULT, VPT_STRETCH);

    y += (5 * V_NamePatchHeight(lname)) / 4;

    V_DrawNamePatch((320 - V_NamePatchWidth(lname)) / 2, y, FB, lname, CR_DEFAULT, VPT_STRETCH);

}

static void WI_drawOnLnode(int n, const char *const c[])
{

    int i = 0;
    boolean fits = false;

    do
    {

        const rpatch_t *patch = R_CachePatchName(c[i]);
        int left = lnodes[wbs->epsd][n].x - patch->leftoffset;
        int top = lnodes[wbs->epsd][n].y - patch->topoffset;
        int right = left + patch->width;
        int bottom = top + patch->height;

        R_UnlockPatchName(c[i]);

        if (left >= 0 && right < 320 && top >= 0 && bottom < 200)
            fits = true;
        else
            i++;

    } while (!fits && i != 2);

    if (fits && i < 2)
        V_DrawNamePatch(lnodes[wbs->epsd][n].x, lnodes[wbs->epsd][n].y, FB, c[i], CR_DEFAULT, VPT_STRETCH);
    else
        I_Error("WI_drawOnLnode: Could not place patch on level %d", n + 1);

}

static void WI_initAnimatedBack(void)
{

    int i;
    anim_t *a;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (i=0; i < NUMANIMS[wbs->epsd]; i++)
    {

        a = &anims[wbs->epsd][i];
        a->ctr = -1;

        if (a->type == ANIM_ALWAYS)
            a->nexttic = bcnt + 1 + (P_Random(pr_misc) % a->period);
        else if (a->type == ANIM_RANDOM)
            a->nexttic = bcnt + 1 + a->data2+(P_Random(pr_misc)%a->data1);
        else if (a->type == ANIM_LEVEL)
            a->nexttic = bcnt + 1;

    }

}

static void WI_updateAnimatedBack(void)
{

    int i;
    anim_t *a;

    if (gamemode == commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (i = 0; i < NUMANIMS[wbs->epsd]; i++)
    {

        a = &anims[wbs->epsd][i];

        if (bcnt == a->nexttic)
        {

            switch (a->type)
            {

            case ANIM_ALWAYS:
                if (++a->ctr >= a->nanims)
                    a->ctr = 0;

                a->nexttic = bcnt + a->period;

                break;

            case ANIM_RANDOM:
                a->ctr++;

                if (a->ctr == a->nanims)
                {

                    a->ctr = -1;
                    a->nexttic = bcnt+a->data2+(P_Random(pr_misc)%a->data1);

                }

                else
                {

                    a->nexttic = bcnt + a->period;

                }

                break;

            case ANIM_LEVEL:
                if (!(state == StatCount && i == 7) && wbs->next == a->data1)
                {

                    a->ctr++;

                    if (a->ctr == a->nanims)
                        a->ctr--;

                    a->nexttic = bcnt + a->period;

                }

                break;

            }

        }

    }

}

static void WI_drawAnimatedBack(void)
{

    int i;
    anim_t *a;

    if (gamemode==commercial)
        return;

    if (wbs->epsd > 2)
        return;

    for (i = 0; i < NUMANIMS[wbs->epsd]; i++)
    {

        a = &anims[wbs->epsd][i];

        if (a->ctr >= 0)
            V_DrawNumPatch(a->loc.x, a->loc.y, FB, a->p[a->ctr].lumpnum, CR_DEFAULT, VPT_STRETCH);

    }

}

static int WI_drawNum(int x, int y, int n, int digits)
{

    int fontwidth = num[0].width;
    int neg;
    int temp;

    if (digits < 0)
    {

        if (!n)
        {

            digits = 1;

        }

        else
        {

            digits = 0;
            temp = n;

            while (temp)
            {

                temp /= 10;
                digits++;

            }

        }

    }

    neg = n < 0;

    if (neg)
        n = -n;

    if (n == 1994)
        return 0;

    while (digits--)
    {

        x -= fontwidth;

        V_DrawNumPatch(x, y, FB, num[n % 10].lumpnum, CR_DEFAULT, VPT_STRETCH);

        n /= 10;

    }

    if (neg)
        V_DrawNamePatch(x -= 8, y, FB, wiminus, CR_DEFAULT, VPT_STRETCH);

    return x;

}

static void WI_drawPercent(int x, int y, int p)
{

    if (p < 0)
        return;

    V_DrawNamePatch(x, y, FB, percent, CR_DEFAULT, VPT_STRETCH);
    WI_drawNum(x, y, p, -1);

}

static void WI_drawTime(int x, int y, int t)
{

    int n;

    if (t < 0)
        return;

    if (t < 100 * 60 * 60)
        for (;;)
        {

            n = t % 60;
            t /= 60;
            x = WI_drawNum(x, y, n, (t || n > 9) ? 2 : 1) - V_NamePatchWidth(colon);

            if (t)
                V_DrawNamePatch(x, y, FB, colon, CR_DEFAULT, VPT_STRETCH);
            else
                break;
        }
    else
        V_DrawNamePatch(x - V_NamePatchWidth(sucks), y, FB, sucks, CR_DEFAULT, VPT_STRETCH);

}

static void WI_initNoState(void)
{

    state = NoState;
    acceleratestage = 0;
    cnt = 10;

}

static void WI_drawTimeStats(int cnt_time, int cnt_total_time, int cnt_par)
{

    V_DrawNamePatch(SP_TIMEX, SP_TIMEY, FB, time1, CR_DEFAULT, VPT_STRETCH);
    WI_drawTime(320 / 2 - SP_TIMEX, SP_TIMEY, cnt_time);
    V_DrawNamePatch(SP_TIMEX, (SP_TIMEY + 200) / 2, FB, total, CR_DEFAULT, VPT_STRETCH);
    WI_drawTime(320 / 2 - SP_TIMEX, (SP_TIMEY + 200) / 2, cnt_total_time);

}

static void WI_updateNoState(void)
{

    WI_updateAnimatedBack();

    if (!--cnt)
        G_WorldDone();

}


static void WI_initShowNextLoc(void)
{

    if ((gamemode != commercial) && (gamemap == 8))
    {

        G_WorldDone();

        return;

    }

    state = ShowNextLoc;
    acceleratestage = 0;
    cnt = SHOWNEXTLOCDELAY * TICRATE;

    WI_initAnimatedBack();

}

static void WI_updateShowNextLoc(void)
{

    WI_updateAnimatedBack();

    if (!--cnt || acceleratestage)
        WI_initNoState();
    else
        snl_pointeron = (cnt & 31) < 20;

}

static void WI_drawShowNextLoc(void)
{

    int i;
    int last;

    WI_slamBackground();
    WI_drawAnimatedBack();

    if (gamemode != commercial)
    {

        if (wbs->epsd > 2)
        {

            WI_drawEL();

            return;

        }

        last = (wbs->last == 8) ? wbs->next - 1 : wbs->last;

        for (i = 0; i <= last; i++)
            WI_drawOnLnode(i, &splat);

        if (wbs->didsecret)
            WI_drawOnLnode(8, &splat);

        if (snl_pointeron)
            WI_drawOnLnode(wbs->next, yah);

    }

    if ((gamemode != commercial) || wbs->next != 30)
        WI_drawEL();

}

void WI_drawNoState(void)
{

    snl_pointeron = true;

    WI_drawShowNextLoc();

}

static void WI_endStats(void)
{

    free(cnt_frags);
    free(cnt_secret);
    free(cnt_items);
    free(cnt_kills);

    cnt_frags = NULL;
    cnt_secret = NULL;
    cnt_items = NULL;
    cnt_kills = NULL;

}

static void WI_initStats(void)
{

    state = StatCount;
    acceleratestage = 0;
    sp_state = 1;

    *(cnt_kills = malloc(sizeof(*cnt_kills))) = *(cnt_items = malloc(sizeof(*cnt_items))) = *(cnt_secret = malloc(sizeof(*cnt_secret))) = -1;
    cnt_time = cnt_par = cnt_total_time = -1;
    cnt_pause = TICRATE;

    WI_initAnimatedBack();

}

static void WI_updateStats(void)
{

    WI_updateAnimatedBack();

    if (acceleratestage && sp_state != 10)
    {

        acceleratestage = 0;

        cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;
        cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;
        cnt_secret[0] = (wbs->maxsecret ? (plrs[me].ssecret * 100) / wbs->maxsecret : 100);
        cnt_total_time = wbs->totaltimes / TICRATE;
        cnt_time = plrs[me].stime / TICRATE;
        cnt_par = wbs->partime / TICRATE;

        S_StartSound(0, sfx_barexp);

        sp_state = 10;

    }

    if (sp_state == 2)
    {

        cnt_kills[0] += 2;

        if (!(bcnt & 3))
            S_StartSound(0, sfx_pistol);

        if (cnt_kills[0] >= (plrs[me].skills * 100) / wbs->maxkills)
        {

            cnt_kills[0] = (plrs[me].skills * 100) / wbs->maxkills;

            S_StartSound(0, sfx_barexp);

            sp_state++;

        }

    }

    else if (sp_state == 4)
    {

        cnt_items[0] += 2;

        if (!(bcnt & 3))
            S_StartSound(0, sfx_pistol);

        if (cnt_items[0] >= (plrs[me].sitems * 100) / wbs->maxitems)
        {

            cnt_items[0] = (plrs[me].sitems * 100) / wbs->maxitems;

            S_StartSound(0, sfx_barexp);

            sp_state++;

        }

    }

    else if (sp_state == 6)
    {

        cnt_secret[0] += 2;

        if (!(bcnt & 3))
            S_StartSound(0, sfx_pistol);

        if (cnt_secret[0] >= (wbs->maxsecret ? (plrs[me].ssecret * 100) / wbs->maxsecret : 100))
        {

            cnt_secret[0] = (wbs->maxsecret ? (plrs[me].ssecret * 100) / wbs->maxsecret : 100);

            S_StartSound(0, sfx_barexp);

            sp_state++;

        }

    }

    else if (sp_state == 8)
    {

        if (!(bcnt & 3))
            S_StartSound(0, sfx_pistol);

        cnt_time += 3;

        if (cnt_time >= plrs[me].stime / TICRATE)
            cnt_time = plrs[me].stime / TICRATE;

        cnt_total_time += 3;

        if (cnt_total_time >= wbs->totaltimes / TICRATE)
            cnt_total_time = wbs->totaltimes / TICRATE;

        cnt_par += 3;

        if (cnt_par >= wbs->partime / TICRATE)
        {

            cnt_par = wbs->partime / TICRATE;

            if ((cnt_time >= plrs[me].stime / TICRATE) && (cnt_total_time >= wbs->totaltimes / TICRATE))
            {

                S_StartSound(0, sfx_barexp);
                sp_state++;

            }

        }

    }

    else if (sp_state == 10)
    {

        if (acceleratestage)
        {

            S_StartSound(0, sfx_sgcock);

            if (gamemode == commercial)
                WI_initNoState();
            else
                WI_initShowNextLoc();

        }

    }

    else if (sp_state & 1)
    {

        if (!--cnt_pause)
        {

            sp_state++;
            cnt_pause = TICRATE;

        }

    }

}

static void WI_drawStats(void)
{

    int lh = (3 * num[0].height) / 2;

    WI_slamBackground();
    WI_drawAnimatedBack();
    WI_drawLF();
    V_DrawNamePatch(SP_STATSX, SP_STATSY, FB, kills, CR_DEFAULT, VPT_STRETCH);

    if (cnt_kills)
        WI_drawPercent(320 - SP_STATSX, SP_STATSY, cnt_kills[0]);

    V_DrawNamePatch(SP_STATSX, SP_STATSY+lh, FB, items, CR_DEFAULT, VPT_STRETCH);

    if (cnt_items)
        WI_drawPercent(320 - SP_STATSX, SP_STATSY + lh, cnt_items[0]);

    V_DrawNamePatch(SP_STATSX, SP_STATSY + 2 * lh, FB, sp_secret, CR_DEFAULT, VPT_STRETCH);

    if (cnt_secret)
        WI_drawPercent(320 - SP_STATSX, SP_STATSY + 2 * lh, cnt_secret[0]);

    WI_drawTimeStats(cnt_time, cnt_total_time, cnt_par);

}

void WI_checkForAccelerate(void)
{

    int i;
    player_t  *player;

    for (i = 0, player = players; i < MAXPLAYERS; i++, player++)
    {

        if (playeringame[i])
        {

            if (player->cmd.buttons & BT_ATTACK)
            {

                if (!player->attackdown)
                    acceleratestage = 1;

                player->attackdown = true;

            }

            else
                player->attackdown = false;

            if (player->cmd.buttons & BT_USE)
            {

                if (!player->usedown)
                    acceleratestage = 1;

                player->usedown = true;

            }

            else
                player->usedown = false;

        }

    }

}

void WI_Ticker(void)
{

    bcnt++;

    if (bcnt == 1)
    {

        if (gamemode == commercial)
            S_ChangeMusic(mus_dm2int, true);
        else
            S_ChangeMusic(mus_inter, true);

    }

    WI_checkForAccelerate();

    switch (state)
    {

    case StatCount:
        WI_updateStats();

        break;

    case ShowNextLoc:
        WI_updateShowNextLoc();

        break;

    case NoState:
        WI_updateNoState();

        break;

    }

}

static void WI_loadData(void)
{

    int i;
    int j;
    char name[9];
    anim_t *a;

    if (gamemode != commercial)
    {

        if (wbs->epsd < 3)
        {

            for (j = 0; j < NUMANIMS[wbs->epsd]; j++)
            {

                a = &anims[wbs->epsd][j];

                for (i = 0; i < a->nanims; i++)
                {

                    if (wbs->epsd != 1 || j != 8)
                    {

                        sprintf(name, "WIA%d%.2d%.2d", wbs->epsd, j, i);
                        R_SetPatchNum(&a->p[i], name);

                    }

                    else
                    {

                        a->p[i] = anims[1][4].p[i];

                    }

                }
                
            }

        }

    }

    for (i = 0; i < 10; i++)
    {

        sprintf(name, "WINUM%d", i);
        R_SetPatchNum(&num[i], name);

    }

}

void WI_Drawer(void)
{

    switch (state)
    {

    case StatCount:
        WI_drawStats();

        break;

    case ShowNextLoc:
        WI_drawShowNextLoc();

        break;

    case NoState:
        WI_drawNoState();

        break;

    }

}

static void WI_initVariables(wbstartstruct_t* wbstartstruct)
{

    wbs = wbstartstruct;
    acceleratestage = 0;
    cnt = bcnt = 0;
    firstrefresh = 1;
    me = wbs->pnum;
    plrs = wbs->plyr;

    if (!wbs->maxkills)
        wbs->maxkills = 1;

    if (!wbs->maxitems)
        wbs->maxitems = 1;

    if (gamemode != retail)
    {

        if (wbs->epsd > 2)
            wbs->epsd -= 3;

    }

}

void WI_Start(wbstartstruct_t* wbstartstruct)
{

    WI_initVariables(wbstartstruct);
    WI_loadData();
    WI_initStats();

}

void WI_End(void)
{

    WI_endStats();

}

