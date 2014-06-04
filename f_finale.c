#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_state.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "wi_stuff.h"
#include "v_video.h"
#include "i_video.h"
#include "hu_stuff.h"

#define TEXTSPEED                       3
#define TEXTWAIT                        250
#define NEWTEXTSPEED                    0.01f
#define NEWTEXTWAIT                     1000

static int finalestage;
static int finalecount;
static int midstage;
static const char *finaletext;
static const char *finaleflat;
static const char *bgflatE1 = "FLOOR4_8";
static const char *bgflatE2 = "SFLR6_1";
static const char *bgflatE3 = "MFLR8_4";
static const char *bgflatE4 = "MFLR8_3";
static const char *bgflat06 = "SLIME16";
static const char *bgflat11 = "RROCK14";
static const char *bgflat20 = "RROCK07";
static const char *bgflat30 = "RROCK17";
static const char *bgflat15 = "RROCK13";
static const char *bgflat31 = "RROCK19";
static const char pfub2[] = {"PFUB2"};
static const char pfub1[] = {"PFUB1"};

extern patchnum_t hu_font[HU_FONTSIZE];

void F_StartFinale(void)
{

    gameaction = ga_nothing;
    gamestate = GS_FINALE;
    acceleratestage = midstage = 0;

    switch (gamemode)
    {

    case shareware:
    case registered:
    case retail:
        S_ChangeMusic(mus_victor, true);

        switch (gameepisode)
        {

        case 1:
             finaleflat = bgflatE1;
             finaletext = E1TEXT;

             break;

        case 2:
             finaleflat = bgflatE2;
             finaletext = E2TEXT;

             break;

        case 3:
             finaleflat = bgflatE3;
             finaletext = E3TEXT;

             break;

        case 4:
             finaleflat = bgflatE4;
             finaletext = E4TEXT;

             break;

        default:

             break;

        }

        break;

    case commercial:
        S_ChangeMusic(mus_read_m, true);

        switch (gamemap)
        {

        case 6:
             finaleflat = bgflat06;
             finaletext = C1TEXT;

             break;

        case 11:
             finaleflat = bgflat11;
             finaletext = C2TEXT;

             break;

        case 20:
             finaleflat = bgflat20;
             finaletext = C3TEXT;

             break;

        case 30:
             finaleflat = bgflat30;
             finaletext = C4TEXT;

             break;

        case 15:
             finaleflat = bgflat15;
             finaletext = C5TEXT;

             break;

        case 31:
             finaleflat = bgflat31;
             finaletext = C6TEXT;

             break;

        default:
             break;

        }

        break;

    default:
        S_ChangeMusic(mus_read_m, true);

        finaleflat = "F_SKY1";
        finaletext = C1TEXT;

        break;

    }

    finalestage = 0;
    finalecount = 0;

}

boolean F_Responder(event_t *event)
{

    return false;

}

static float Get_TextSpeed(void)
{

    return midstage ? NEWTEXTSPEED : (midstage = acceleratestage) ? acceleratestage = 0, NEWTEXTSPEED : TEXTSPEED;

}

static void F_TextWrite(void)
{

    V_DrawBackground(finaleflat, 0);

    {

        int cx = 10;
        int cy = 10;
        const char *ch = finaletext;
        int count = (int)((float)(finalecount - 10) / Get_TextSpeed());
        int w;

        if (count < 0)
            count = 0;

        for (; count; count--)
        {

            int c = *ch++;

            if (!c)
                break;

            if (c == '\n')
            {

                cx = 10;
                cy += 11;

                continue;

            }

            c = toupper(c) - HU_FONTSTART;

            if (c < 0 || c> HU_FONTSIZE)
            {
            
                cx += 4;

                continue;

            }

            w = hu_font[c].width;

            if (cx + w > SCREENWIDTH)
                break;

            V_DrawNumPatch(cx, cy, 0, hu_font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);

            cx+=w;

        }

    }

}

void F_Ticker(void)
{

    int i;

    WI_checkForAccelerate();

    if (gamemode == commercial && finalecount > 50)
    {

        for (i = 0; i < MAXPLAYERS; i++)
        {

            if (players[i].cmd.buttons)
                goto next_level;

        }

    }

    finalecount++;

    if (!finalestage)
    {

        float speed = Get_TextSpeed();

        if (finalecount > strlen(finaletext) * speed + (midstage ? NEWTEXTWAIT : TEXTWAIT) || (midstage && acceleratestage))
        {

            if (gamemode != commercial)
            {

                finalecount = 0;
                finalestage = 1;

                if (gameepisode == 3)
                    S_StartMusic(mus_bunny);

            }

            else if (midstage)
            {

            next_level:
                    gameaction = ga_worlddone;

            }

        }

    }

}

static void F_BunnyScroll(void)
{

    char name[10];
    int stage;
    static int laststage;

    {
        int scrolled = 320 - (finalecount - 230) / 2;

        if (scrolled <= 0)
        {

            V_DrawNamePatch(0, 0, 0, pfub2, CR_DEFAULT, VPT_STRETCH);

        }
        
        else if (scrolled >= 320)
        {

            V_DrawNamePatch(0, 0, 0, pfub1, CR_DEFAULT, VPT_STRETCH);

        }
        
        else
        {

            V_DrawNamePatch(320 - scrolled, 0, 0, pfub1, CR_DEFAULT, VPT_STRETCH);
            V_DrawNamePatch(-scrolled, 0, 0, pfub2, CR_DEFAULT, VPT_STRETCH);

        }

    }

    if (finalecount < 1130)
        return;

    if (finalecount < 1180)
    {

        V_DrawNamePatch((320 - 13 * 8) / 2, (200 - 8 * 8) / 2,0, "END0", CR_DEFAULT, VPT_STRETCH);

        laststage = 0;

        return;

    }

    stage = (finalecount - 1180) / 5;

    if (stage > 6)
        stage = 6;

    if (stage > laststage)
    {

        S_StartSound (NULL, sfx_pistol);

        laststage = stage;

    }

    sprintf(name,"END%i",stage);
    V_DrawNamePatch((320 - 13 * 8) / 2, (200 - 8 * 8) / 2, 0, name, CR_DEFAULT, VPT_STRETCH);

}

void F_Drawer(void)
{

    if (!finalestage)
        F_TextWrite();

    else
    {

        switch (gameepisode)
        {

        case 1:
           if (gamemode == retail)
                V_DrawNamePatch(0, 0, 0, "CREDIT", CR_DEFAULT, VPT_STRETCH);
           else
                V_DrawNamePatch(0, 0, 0, "HELP2", CR_DEFAULT, VPT_STRETCH);

           break;

        case 2:
           V_DrawNamePatch(0, 0, 0, "VICTORY2", CR_DEFAULT, VPT_STRETCH);

           break;

        case 3:
           F_BunnyScroll();

           break;

        case 4:
           V_DrawNamePatch(0, 0, 0, "ENDPIC", CR_DEFAULT, VPT_STRETCH);

           break;

        }

    }

}

