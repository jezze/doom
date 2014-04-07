#include <stdlib.h>
#include <ctype.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "g_game.h"
#include "r_data.h"
#include "p_inter.h"
#include "p_tick.h"
#include "m_cheat.h"
#include "s_sound.h"
#include "d_englsh.h"
#include "r_main.h"
#include "p_map.h"
#include "p_tick.h"

#define CHEAT_ARGS_MAX                  8
#define plyr                            (players + consoleplayer)

static void cheat_choppers();
static void cheat_god();
static void cheat_fa();
static void cheat_k();
static void cheat_kfa();
static void cheat_noclip();
static void cheat_pw();
static void cheat_behold();
static void cheat_rate();
static void cheat_megaarmour();
static void cheat_health();

static struct cheat_s
{

    const char *cheat;
    void (*const func)();
    const int arg;
    uint_64_t code, mask;

} cheat[] = {
    {"idchoppers", cheat_choppers},
    {"iddqd", cheat_god},
    {"idkfa", cheat_kfa},
    {"idfa", cheat_fa},
    {"idclip", cheat_noclip},
    {"idbeholdh", cheat_health},
    {"idbeholdm", cheat_megaarmour},
    {"idbeholdv", cheat_pw, pw_invulnerability},
    {"idbeholds", cheat_pw, pw_strength},
    {"idbeholdi", cheat_pw, pw_invisibility},
    {"idbeholdr", cheat_pw, pw_ironfeet},
    {"idbeholda", cheat_pw, pw_allmap},
    {"idbeholdl", cheat_pw, pw_infrared},
    {"idbehold",  cheat_behold},
    {"idrate", cheat_rate},
    {NULL}
};

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
    {

        plyr->message = STSTR_DQDOFF;

    }

}


static void cheat_health()
{

    if (!(plyr->cheats & CF_GODMODE))
    {

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

        for (i = 0; i < NUMAMMO; i++)
            plyr->maxammo[i] *= 2;

        plyr->backpack = true;

    }

    plyr->armorpoints = idfa_armor;
    plyr->armortype = idfa_armor_class;

    for (i = 0; i < NUMWEAPONS; i++)
        plyr->weaponowned[i] = true;

    for (i = 0; i < NUMAMMO;i++)
        plyr->ammo[i] = plyr->maxammo[i];

    plyr->message = STSTR_FAADDED;

}

static void cheat_k()
{

    int i;

    for (i = 0; i < NUMCARDS; i++)
    {

        if (!plyr->cards[i])
        {

            plyr->cards[i] = true;
            plyr->message = "Keys Added";

        }

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

    plyr->message = (plyr->cheats ^= CF_NOCLIP) & CF_NOCLIP ? STSTR_NCON : STSTR_NCOFF;
}


static void cheat_pw(int pw)
{

    if (plyr->powers[pw])
    {

        plyr->powers[pw] = pw != pw_strength && pw != pw_allmap;

    }

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

static void cheat_rate()
{

    rendering_stats ^= 1;

}

boolean M_FindCheats(int key)
{

    static uint_64_t sr;
    static char argbuf[CHEAT_ARGS_MAX + 1], *arg;
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

        for (i = 0; cheat[i]. cheat; i++)
        {

            uint_64_t c=0, m=0;
            const char *p;

            for (p = cheat[i].cheat; *p; p++)
            {

                unsigned key = tolower(*p) - 'a';

                if (key >= 32)
                    continue;

                c = (c << 5) + key;
                m = (m << 5) + 31;

            }

            cheat[i].code = c;
            cheat[i].mask = m;

        }

    }

    sr = (sr << 5) + key;

    for (matchedbefore = ret = i = 0; cheat[i].cheat; i++)
    {

        if ((sr & cheat[i].mask) == cheat[i].code)
        {

            if (cheat[i].arg < 0)
            {

                cht = i;
                arg = argbuf;
                argsleft = -cheat[i].arg;
                ret = 1;

            }

            else if (!matchedbefore)
            {

                matchedbefore = ret = 1;
                cheat[i].func(cheat[i].arg);

            }

        }

    }

    return ret;

}

