#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "d_event.h"
#include "v_video.h"
#include "w_wad.h"
#include "s_sound.h"
#include "sounds.h"
#include "d_englsh.h"
#include "r_state.h"
#include "f_finale.h"
#include "wi_stuff.h"
#include "i_video.h"

#define TEXTSPEED                       3
#define TEXTWAIT                        250
#define NEWTEXTSPEED                    0.01f
#define NEWTEXTWAIT                     1000

static int finalestage;
static int finalecount;
static const char *finaletext;
static const char *finaleflat;
const char *bgflatE1 = "FLOOR4_8";
const char *bgflatE2 = "SFLR6_1";
const char *bgflatE3 = "MFLR8_4";
const char *bgflatE4 = "MFLR8_3";
const char *bgflat06 = "SLIME16";
const char *bgflat11 = "RROCK14";
const char *bgflat20 = "RROCK07";
const char *bgflat30 = "RROCK17";
const char *bgflat15 = "RROCK13";
const char *bgflat31 = "RROCK19";
const char *bgcastcall = "BOSSBACK";

void F_StartCast(void);
void F_CastTicker(void);
boolean F_CastResponder(event_t *ev);
void F_CastDrawer(void);

extern int acceleratestage;
static int midstage;

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

boolean F_Responder (event_t *event)
{

    if (finalestage == 2)
        return F_CastResponder(event);

    return false;

}

static float Get_TextSpeed(void)
{

    return midstage ? NEWTEXTSPEED : (midstage = acceleratestage) ? acceleratestage = 0, NEWTEXTSPEED : TEXTSPEED;

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

    if (finalestage == 2)
        F_CastTicker();

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
                if (gamemap == 30)
                    F_StartCast();
                else
                    gameaction = ga_worlddone;

            }

        }

    }

}

#include "hu_stuff.h"

extern patchnum_t hu_font[HU_FONTSIZE];

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

typedef struct
{

    const char *name;
    mobjtype_t   type;

} castinfo_t;

#define MAX_CASTORDER 18

static const castinfo_t castorder[] = {
    {CC_ZOMBIE, MT_POSSESSED},
    {CC_SHOTGUN, MT_SHOTGUY},
    {CC_HEAVY, MT_CHAINGUY},
    {CC_IMP, MT_TROOP},
    {CC_DEMON, MT_SERGEANT},
    {CC_LOST, MT_SKULL},
    {CC_CACO, MT_HEAD},
    {CC_HELL, MT_KNIGHT},
    {CC_BARON, MT_BRUISER},
    {CC_ARACH, MT_BABY},
    {CC_PAIN, MT_PAIN},
    {CC_REVEN, MT_UNDEAD},
    {CC_MANCU, MT_FATSO},
    {CC_ARCH, MT_VILE},
    {CC_SPIDER, MT_SPIDER},
    {CC_CYBER, MT_CYBORG},
    {CC_HERO, MT_PLAYER},
    {NULL, 0}
};

int castnum;
int casttics;
state_t *caststate;
boolean castdeath;
int castframes;
int castonmelee;
boolean castattacking;

void F_StartCast(void)
{

    castnum = 0;
    caststate = &states[mobjinfo[castorder[castnum].type].seestate];
    casttics = caststate->tics;
    castdeath = false;
    finalestage = 2;
    castframes = 0;
    castonmelee = 0;
    castattacking = false;

    S_ChangeMusic(mus_evil, true);

}

void F_CastTicker (void)
{

    int st;
    int sfx;

    if (--casttics > 0)
        return;

    if (caststate->tics == -1 || caststate->nextstate == S_NULL)
    {

        castnum++;
        castdeath = false;

        if (castorder[castnum].name == NULL)
            castnum = 0;
            
        if (mobjinfo[castorder[castnum].type].seesound)
            S_StartSound (NULL, mobjinfo[castorder[castnum].type].seesound);

        caststate = &states[mobjinfo[castorder[castnum].type].seestate];
        castframes = 0;

    }

    else
    {

        if (caststate == &states[S_PLAY_ATK1])
            goto stopattack;

        st = caststate->nextstate;
        caststate = &states[st];
        castframes++;

        switch (st)
        {

        case S_PLAY_ATK1:     sfx = sfx_dshtgn; break;
        case S_POSS_ATK2:     sfx = sfx_pistol; break;
        case S_SPOS_ATK2:     sfx = sfx_shotgn; break;
        case S_VILE_ATK2:     sfx = sfx_vilatk; break;
        case S_SKEL_FIST2:    sfx = sfx_skeswg; break;
        case S_SKEL_FIST4:    sfx = sfx_skepch; break;
        case S_SKEL_MISS2:    sfx = sfx_skeatk; break;
        case S_FATT_ATK8:
        case S_FATT_ATK5:
        case S_FATT_ATK2:     sfx = sfx_firsht; break;
        case S_CPOS_ATK2:
        case S_CPOS_ATK3:
        case S_CPOS_ATK4:     sfx = sfx_shotgn; break;
        case S_TROO_ATK3:     sfx = sfx_claw; break;
        case S_SARG_ATK2:     sfx = sfx_sgtatk; break;
        case S_BOSS_ATK2:
        case S_BOS2_ATK2:
        case S_HEAD_ATK2:     sfx = sfx_firsht; break;
        case S_SKULL_ATK2:    sfx = sfx_sklatk; break;
        case S_SPID_ATK2:
        case S_SPID_ATK3:     sfx = sfx_shotgn; break;
        case S_BSPI_ATK2:     sfx = sfx_plasma; break;
        case S_CYBER_ATK2:
        case S_CYBER_ATK4:
        case S_CYBER_ATK6:    sfx = sfx_rlaunc; break;
        case S_PAIN_ATK3:     sfx = sfx_sklatk; break;
        default: sfx = 0; break;

        }

        if (sfx)
            S_StartSound (NULL, sfx);

    }

    if (castframes == 12)
    {

        castattacking = true;

        if (castonmelee)
            caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
        else
            caststate = &states[mobjinfo[castorder[castnum].type].missilestate];

        castonmelee ^= 1;

        if (caststate == &states[S_NULL])
        {

            if (castonmelee)
                caststate = &states[mobjinfo[castorder[castnum].type].meleestate];
            else
                caststate = &states[mobjinfo[castorder[castnum].type].missilestate];

        }

    }

    if (castattacking)
    {

        if (castframes == 24 || caststate == &states[mobjinfo[castorder[castnum].type].seestate])
        {

        stopattack:
            castattacking = false;
            castframes = 0;
            caststate = &states[mobjinfo[castorder[castnum].type].seestate];

        }

    }

    casttics = caststate->tics;

    if (casttics == -1)
        casttics = 15;

}

boolean F_CastResponder (event_t* ev)
{

    if (ev->type != ev_keydown)
        return false;

    if (castdeath)
        return true;

    castdeath = true;
    caststate = &states[mobjinfo[castorder[castnum].type].deathstate];
    casttics = caststate->tics;
    castframes = 0;
    castattacking = false;

    if (mobjinfo[castorder[castnum].type].deathsound)
        S_StartSound(NULL, mobjinfo[castorder[castnum].type].deathsound);

    return true;

}

static void F_CastPrint(const char *text)
{

    const char *ch = text;
    int c;
    int cx;
    int w;
    int width = 0;

    while (ch)
    {

        c = *ch++;

        if (!c)
            break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c> HU_FONTSIZE)
        {

            width += 4;

            continue;

        }

        w = hu_font[c].width;
        width += w;

    }

    cx = 160 - width / 2;
    ch = text;

    while (ch)
    {

        c = *ch++;

        if (!c)
            break;

        c = toupper(c) - HU_FONTSTART;

        if (c < 0 || c > HU_FONTSIZE)
        {

            cx += 4;

            continue;

        }

        w = hu_font[c].width;

        V_DrawNumPatch(cx, 180, 0, hu_font[c].lumpnum, CR_DEFAULT, VPT_STRETCH);

        cx += w;

    }

}

void F_CastDrawer(void)
{

    spritedef_t *sprdef;
    spriteframe_t *sprframe;
    int lump;
    boolean flip;

    V_DrawNamePatch(0,0,0, bgcastcall, CR_DEFAULT, VPT_STRETCH);
    F_CastPrint(castorder[castnum].name);

    sprdef = &sprites[caststate->sprite];
    sprframe = &sprdef->spriteframes[caststate->frame & FF_FRAMEMASK];
    lump = sprframe->lump[0];
    flip = (boolean)sprframe->flip[0];

    V_DrawNumPatch(160, 170, 0, lump+firstspritelump, CR_DEFAULT, VPT_STRETCH | (flip ? VPT_FLIP : 0));

}

static const char pfub2[] = {"PFUB2"};
static const char pfub1[] = {"PFUB1"};

static void F_BunnyScroll (void)
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

void F_Drawer (void)
{

    if (finalestage == 2)
    {

        F_CastDrawer();

        return;

    }

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

