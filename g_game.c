#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "f_finale.h"
#include "m_misc.h"
#include "m_menu.h"
#include "m_random.h"
#include "p_setup.h"
#include "p_tick.h"
#include "p_map.h"
#include "d_main.h"
#include "d_englsh.h"
#include "wi_stuff.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "p_map.h"
#include "r_data.h"
#include "p_inter.h"
#include "g_game.h"
#include "i_system.h"
#include "z_zone.h"
#include "z_bmalloc.h"

#define SKYFLATNAME                     "F_SKY1"

static int ticdup = 1;
gameaction_t gameaction;
gamestate_t gamestate;
skill_t gameskill;
boolean respawnmonsters;
int gameepisode;
int gamemap;
int starttime;
boolean playeringame[MAXPLAYERS];
player_t players[MAXPLAYERS];
int consoleplayer;
int gametic;
int basetic;
int totalkills, totallive, totalitems, totalsecret;
wbstartstruct_t wminfo;
boolean haswolflevels = false;
int autorun = false;
int totalleveltimes;
int key_right;
int key_left;
int key_up;
int key_down;
int key_menu_right;
int key_menu_left;
int key_menu_up;
int key_menu_down;
int key_menu_backspace;
int key_menu_escape;
int key_menu_enter;
int key_strafeleft;
int key_straferight;
int key_fire;
int key_use;
int key_strafe;
int key_speed;
int key_escape = KEYD_ESCAPE;
int key_autorun;
int key_reverse;
int key_backspace;
int key_enter;
int destination_keys[MAXPLAYERS];
int key_weapontoggle;
int key_weapon1;
int key_weapon2;
int key_weapon3;
int key_weapon4;
int key_weapon5;
int key_weapon6;
int key_weapon7;
int key_weapon8;
int key_weapon9;
int mousebfire;
int mousebstrafe;
int mousebforward;

#define MAXPLMOVE                       (forwardmove[1])
#define TURBOTHRESHOLD                  0x32
#define SLOWTURNTICS                    6
#define QUICKREVERSE                    (short)32768
#define NUMKEYS                         512

fixed_t forwardmove[2] = {0x19, 0x32};
fixed_t sidemove[2] = {0x18, 0x28};
fixed_t angleturn[3] = {640, 1280, 320};

static boolean gamekeydown[NUMKEYS];
static int turnheld;
static boolean mousearray[4];
static boolean *mousebuttons = &mousearray[1];
static int mousex;
static int mousey;
static int dclicktime;
static int dclickstate;
static int dclicks;
static int dclicktime2;
static int dclickstate2;
static int dclicks2;
static buttoncode_t special_event;
int defaultskill;

static inline char fudgef(char b)
{

    static int c;

    if (!b)
        return b;
        
    if (++c & 0x1f)
        return b;

    b |= 1;
    
    if (b > 2)
        b -= 2;

    return b;

}

static inline short fudgea(short b)
{

    if (!b)
        return b;

    b |= 1;
    
    if (b > 2)
        b -= 2;

    return b;

}

void G_BuildTiccmd(ticcmd_t* cmd)
{

    boolean strafe;
    boolean bstrafe;
    int speed;
    int tspeed;
    int forward;
    int side;
    int newweapon;

    memset(cmd, 0, sizeof *cmd);

    strafe = gamekeydown[key_strafe] || mousebuttons[mousebstrafe];
    speed = (gamekeydown[key_speed] ? !autorun : autorun);
    forward = side = 0;

    if (gamekeydown[key_right] || gamekeydown[key_left])
        turnheld += ticdup;
    else
        turnheld = 0;

    if (turnheld < SLOWTURNTICS)
        tspeed = 2;
    else
        tspeed = speed;

    if (gamekeydown[key_reverse])
    {

        cmd->angleturn += QUICKREVERSE;
        gamekeydown[key_reverse] = false;

    }

    if (strafe)
    {

        if (gamekeydown[key_right])
            side += sidemove[speed];

        if (gamekeydown[key_left])
            side -= sidemove[speed];

    }

    else
    {

        if (gamekeydown[key_right])
            cmd->angleturn -= angleturn[tspeed];

        if (gamekeydown[key_left])
            cmd->angleturn += angleturn[tspeed];

    }

    if (gamekeydown[key_up])
        forward += forwardmove[speed];

    if (gamekeydown[key_down])
        forward -= forwardmove[speed];

    if (gamekeydown[key_straferight])
        side += sidemove[speed];

    if (gamekeydown[key_strafeleft])
        side -= sidemove[speed];

    if (gamekeydown[key_fire] || mousebuttons[mousebfire])
        cmd->buttons |= BT_ATTACK;

    if (gamekeydown[key_use])
    {

        cmd->buttons |= BT_USE;
        dclicks = 0;

    }

    if ((players[consoleplayer].attackdown && !P_CheckAmmo(&players[consoleplayer])) || gamekeydown[key_weapontoggle])
    {

        newweapon = P_SwitchWeapon(&players[consoleplayer]);

    }

    else
    {

        newweapon = gamekeydown[key_weapon1] ? wp_fist : gamekeydown[key_weapon2] ? wp_pistol : gamekeydown[key_weapon3] ? wp_shotgun : gamekeydown[key_weapon4] ? wp_chaingun : gamekeydown[key_weapon5] ? wp_missile : gamekeydown[key_weapon6] && gamemode != shareware ? wp_plasma : gamekeydown[key_weapon7] && gamemode != shareware ? wp_bfg : gamekeydown[key_weapon8] ? wp_chainsaw : (gamekeydown[key_weapon9] && gamemode == commercial) ? wp_supershotgun : wp_nochange;

        {

            const player_t *player = &players[consoleplayer];

            if (newweapon==wp_fist && player->weaponowned[wp_chainsaw] && player->readyweapon != wp_chainsaw && (player->readyweapon == wp_fist || !player->powers[pw_strength] || P_WeaponPreferred(wp_chainsaw, wp_fist)))
                newweapon = wp_chainsaw;

            if (newweapon == wp_shotgun && gamemode == commercial && player->weaponowned[wp_supershotgun] && (!player->weaponowned[wp_shotgun] || player->readyweapon == wp_shotgun || (player->readyweapon != wp_supershotgun && P_WeaponPreferred(wp_supershotgun, wp_shotgun))))
                newweapon = wp_supershotgun;

        }

    }

    if (newweapon != wp_nochange)
    {

        cmd->buttons |= BT_CHANGE;
        cmd->buttons |= newweapon << BT_WEAPONSHIFT;

    }

    if (mousebuttons[mousebforward])
        forward += forwardmove[speed];

    if (mousebuttons[mousebforward] != dclickstate && dclicktime > 1 )
    {

        dclickstate = mousebuttons[mousebforward];

        if (dclickstate)
            dclicks++;

        if (dclicks == 2)
        {

            cmd->buttons |= BT_USE;
            dclicks = 0;

        }

        else
        {

            dclicktime = 0;

        }

    }

    else if ((dclicktime += ticdup) > 20)
    {

        dclicks = 0;
        dclickstate = 0;

    }

    bstrafe = mousebuttons[mousebstrafe];

    if (bstrafe != dclickstate2 && dclicktime2 > 1)
    {

        dclickstate2 = bstrafe;

        if (dclickstate2)
            dclicks2++;

        if (dclicks2 == 2)
        {

            cmd->buttons |= BT_USE;
            dclicks2 = 0;

        }

        else
        {

            dclicktime2 = 0;

        }

    }

    else if ((dclicktime2 += ticdup) > 20)
    {

        dclicks2 = 0;
        dclickstate2 = 0;

    }

    forward += mousey;

    if (strafe)
        side += mousex / 4;
    else
        cmd->angleturn -= mousex;

    mousex = mousey = 0;

    if (forward > MAXPLMOVE)
        forward = MAXPLMOVE;
    else if (forward < -MAXPLMOVE)
        forward = -MAXPLMOVE;

    if (side > MAXPLMOVE)
        side = MAXPLMOVE;
    else if (side < -MAXPLMOVE)
        side = -MAXPLMOVE;

    cmd->forwardmove += fudgef(forward);
    cmd->sidemove += side;
    cmd->angleturn = fudgea(cmd->angleturn);

    if (special_event & BT_SPECIAL)
    {

        cmd->buttons = special_event;
        special_event = 0;

    }

}

void G_RestartLevel(void)
{

    special_event = BT_SPECIAL | (BTS_RESTARTLEVEL & BT_SPECIALMASK);

}

static void G_DoLoadLevel(void)
{

    int i;

    skyflatnum = R_FlatNumForName(SKYFLATNAME);

    if (gamemode == commercial)
    {

        skytexture = R_TextureNumForName("SKY3");

        if (gamemap < 12)
            skytexture = R_TextureNumForName("SKY1");
        else if (gamemap < 21)
            skytexture = R_TextureNumForName("SKY2");

    }

    else
    {

        switch (gameepisode)
        {

        case 1:
            skytexture = R_TextureNumForName("SKY1");

            break;

        case 2:
            skytexture = R_TextureNumForName("SKY2");

            break;

        case 3:
            skytexture = R_TextureNumForName("SKY3");

            break;

        case 4:
            skytexture = R_TextureNumForName("SKY4");

            break;

        }

    }

    gamestate = GS_LEVEL;

    for (i = 0; i < MAXPLAYERS; i++)
    {

        if (playeringame[i] && players[i].playerstate == PST_DEAD)
            players[i].playerstate = PST_REBORN;

        memset(players[i].frags, 0, sizeof (players[i].frags));

    }

    DECLARE_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);
    NULL_BLOCK_MEMORY_ALLOC_ZONE(secnodezone);

    P_SetupLevel(gameepisode, gamemap, 0, gameskill);

    gameaction = ga_nothing;

    memset(gamekeydown, 0, sizeof (gamekeydown));

    mousex = mousey = 0;
    special_event = 0;

    memset(mousebuttons, 0, sizeof (mousebuttons));

    ST_Start();
    HU_Start();

}

boolean G_Responder(event_t* ev)
{

    if (gamestate == GS_FINALE && F_Responder(ev))
        return true;

    switch (ev->type)
    {

        case ev_keydown:
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = true;

            return true;

        case ev_keyup:
            if (ev->data1 < NUMKEYS)
                gamekeydown[ev->data1] = false;

            return false;

        case ev_mouse:
            mousebuttons[0] = ev->data1 & 1;
            mousebuttons[1] = ev->data1 & 2;
            mousebuttons[2] = ev->data1 & 4;
            mousex += (ev->data2 * (mouseSensitivity_horiz)) / 10;
            mousey += (ev->data3 * (mouseSensitivity_vert)) / 10;

            return true;

        default:
            break;

    }

    return false;

}

void G_Ticker(void)
{

    int i;
    static gamestate_t prevgamestate;

    P_MapStart();

    for (i = 0; i < MAXPLAYERS; i++)
    {

        if (playeringame[i] && players[i].playerstate == PST_REBORN)
            G_DoReborn(i);

    }

    P_MapEnd();

    while (gameaction != ga_nothing)
    {

        switch (gameaction)
        {

        case ga_loadlevel:
            for (i = 0; i < MAXPLAYERS; i++)
                players[i].playerstate = PST_REBORN;

            G_DoLoadLevel();

            break;

        case ga_newgame:
            G_DoNewGame();

            break;

        case ga_completed:
            G_DoCompleted();

            break;

        case ga_victory:
            F_StartFinale();

            break;

        case ga_worlddone:
            G_DoWorldDone();

            break;

        case ga_nothing:
            break;

        }

    }

    {

        int buf = (gametic / ticdup) % BACKUPTICS;

        for (i = 0; i < MAXPLAYERS; i++)
        {

            if (playeringame[i])
            {

                ticcmd_t *cmd = &players[i].cmd;

                memcpy(cmd, &netcmds[i][buf], sizeof *cmd);

            }

        }

        for (i = 0; i < MAXPLAYERS; i++)
        {

            if (playeringame[i])
            {

                if (players[i].cmd.buttons & BT_SPECIAL)
                    players[i].cmd.buttons = 0;

            }

        }

    }

    if (gamestate != prevgamestate)
    {

        switch (prevgamestate)
        {

        case GS_LEVEL:
            break;

        case GS_INTERMISSION:
            WI_End();

        default:
            break;

        }

        prevgamestate = gamestate;

    }

    switch (gamestate)
    {

    case GS_LEVEL:
        P_Ticker();
        ST_Ticker();
        HU_Ticker();

        break;

    case GS_INTERMISSION:
        WI_Ticker();

        break;

    case GS_FINALE:
        F_Ticker();

        break;

    }

}

static void G_PlayerFinishLevel(int player)
{

    player_t *p = &players[player];

    memset(p->powers, 0, sizeof p->powers);
    memset(p->cards, 0, sizeof p->cards);

    p->mo = NULL;
    p->extralight = 0;
    p->fixedcolormap = 0;
    p->damagecount = 0;
    p->bonuscount = 0;

}

void G_PlayerReborn(int player)
{

    player_t *p;
    int i;
    int frags[MAXPLAYERS];
    int killcount;
    int itemcount;
    int secretcount;

    memcpy(frags, players[player].frags, sizeof frags);

    killcount = players[player].killcount;
    itemcount = players[player].itemcount;
    secretcount = players[player].secretcount;
    p = &players[player];

    {

        int cheats = p->cheats;

        memset(p, 0, sizeof(*p));

        p->cheats = cheats;

    }

    memcpy(players[player].frags, frags, sizeof (players[player].frags));

    players[player].killcount = killcount;
    players[player].itemcount = itemcount;
    players[player].secretcount = secretcount;
    p->usedown = p->attackdown = true;
    p->playerstate = PST_LIVE;
    p->health = 100;
    p->readyweapon = p->pendingweapon = wp_pistol;
    p->weaponowned[wp_fist] = true;
    p->weaponowned[wp_pistol] = true;
    p->ammo[am_clip] = 50;

    for (i = 0; i < NUMAMMO; i++)
        p->maxammo[i] = maxammo[i];

}

void G_DoReborn(int playernum)
{

    gameaction = ga_loadlevel;

}

int pars[4][10] = {
    {0},
    {0,30,75,120,90,165,180,180,30,165},
    {0,90,90,90,120,90,360,240,30,170},
    {0,90,45,90,150,90,90,165,30,135}
};


int cpars[32] = {
    30,90,120,120,90,150,120,120,270,90,
    210,150,150,150,210,150,420,150,210,150,
    240,150,180,150,150,300,330,420,300,180,
    120,30
};

static boolean secretexit;

void G_ExitLevel (void)
{

    secretexit = false;
    gameaction = ga_completed;

}

void G_SecretExitLevel(void)
{

    if (gamemode != commercial || haswolflevels)
        secretexit = true;
    else
        secretexit = false;

    gameaction = ga_completed;

}

void G_DoCompleted(void)
{

    int i;

    gameaction = ga_nothing;

    for (i = 0; i < MAXPLAYERS; i++)
    {

        if (playeringame[i])
            G_PlayerFinishLevel(i);

    }

    if (gamemode != commercial)
    {

        switch (gamemap)
        {

        case 9:
            for (i = 0; i < MAXPLAYERS; i++)
                players[i].didsecret = true;

            break;

        }

    }

    wminfo.didsecret = players[consoleplayer].didsecret;
    wminfo.epsd = gameepisode -1;
    wminfo.last = gamemap -1;

    if (gamemode == commercial)
    {

        if (secretexit)
        {

            switch (gamemap)
            {

            case 15:
                wminfo.next = 30;

                break;

            case 31:
                wminfo.next = 31;

                break;

            }

        }

        else
        {

            switch(gamemap)
            {

            case 31:
            case 32:
                wminfo.next = 15;

                break;

            default:
                wminfo.next = gamemap;

            }

        }

    }

    else
    {

        if (secretexit)
        {

            wminfo.next = 8;

        }

        else if (gamemap == 9)
        {

            switch (gameepisode)
            {

            case 1:
                wminfo.next = 3;

                break;

            case 2:
                wminfo.next = 5;

                break;

            case 3:
                wminfo.next = 6;

                break;

            case 4:
                wminfo.next = 2;

                break;

            }

        }

        else
        {

            wminfo.next = gamemap;

        }

    }

    wminfo.maxkills = totalkills;
    wminfo.maxitems = totalitems;
    wminfo.maxsecret = totalsecret;
    wminfo.maxfrags = 0;

    if (gamemode == commercial)
        wminfo.partime = TICRATE * cpars[gamemap - 1];
    else
        wminfo.partime = TICRATE * pars[gameepisode][gamemap];

    wminfo.pnum = consoleplayer;

    for (i = 0; i < MAXPLAYERS; i++)
    {

        wminfo.plyr[i].in = playeringame[i];
        wminfo.plyr[i].skills = players[i].killcount;
        wminfo.plyr[i].sitems = players[i].itemcount;
        wminfo.plyr[i].ssecret = players[i].secretcount;
        wminfo.plyr[i].stime = leveltime;
        memcpy(wminfo.plyr[i].frags, players[i].frags, sizeof (wminfo.plyr[i].frags));

    }

    wminfo.totaltimes = (totalleveltimes += (leveltime - leveltime % 35));
    gamestate = GS_INTERMISSION;

    WI_Start(&wminfo);

}

void G_WorldDone(void)
{

    gameaction = ga_worlddone;

    if (secretexit)
        players[consoleplayer].didsecret = true;

    if (gamemode == commercial)
    {

        switch (gamemap)
        {

        case 15:
        case 31:
            if (!secretexit)
                break;

        case 6:
        case 11:
        case 20:
        case 30:
            F_StartFinale();

            break;

        }

    }

    else if (gamemap == 8)
    {

        gameaction = ga_victory;

    }

}

void G_DoWorldDone(void)
{

    gamestate = GS_LEVEL;
    gamemap = wminfo.next + 1;

    G_DoLoadLevel();

    gameaction = ga_nothing;

}

#define MIN_MAXPLAYERS 32
#define VERSIONSIZE   16

static skill_t d_skill;
static int d_episode;
static int d_map;

void G_DeferedInitNew(skill_t skill, int episode, int map)
{

    d_skill = skill;
    d_episode = episode;
    d_map = map;
    gameaction = ga_newgame;

}

void G_ReloadDefaults(void)
{

    weapon_recoil = default_weapon_recoil;
    player_bobbing = default_player_bobbing;
    monsters_remember = default_monsters_remember;
    monster_infighting = default_monster_infighting;
    monster_backing = default_monster_backing;
    monster_avoid_hazards = default_monster_avoid_hazards;
    monster_friction = default_monster_friction;

    memset(playeringame + 1, 0, sizeof (*playeringame) * (MAXPLAYERS - 1));

    consoleplayer = 0;

}

void G_DoNewGame(void)
{

    G_ReloadDefaults();
    G_InitNew(d_skill, d_episode, d_map);

    gameaction = ga_nothing;

    ST_Start();

}

void G_SetFastParms(int fast_pending)
{

    static int fast = 0;
    int i;

    if (fast != fast_pending)
    {

        if ((fast = fast_pending))
        {

            for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
            {

                if (states[i].tics != 1)
                    states[i].tics >>= 1;

            }

            mobjinfo[MT_BRUISERSHOT].speed = 20 * FRACUNIT;
            mobjinfo[MT_HEADSHOT].speed = 20 * FRACUNIT;
            mobjinfo[MT_TROOPSHOT].speed = 20 * FRACUNIT;

        }

        else
        {

            for (i = S_SARG_RUN1; i <= S_SARG_PAIN2; i++)
                states[i].tics <<= 1;

            mobjinfo[MT_BRUISERSHOT].speed = 15 * FRACUNIT;
            mobjinfo[MT_HEADSHOT].speed = 10 * FRACUNIT;
            mobjinfo[MT_TROOPSHOT].speed = 10 * FRACUNIT;

        }

    }

}

void G_InitNew(skill_t skill, int episode, int map)
{

    int i;

    if (skill > sk_nightmare)
        skill = sk_nightmare;

    if (episode < 1)
        episode = 1;

    if (gamemode == retail)
    {

        if (episode > 4)
            episode = 4;

    }

    else if (gamemode == shareware)
    {

        if (episode > 1)
          episode = 1;

    }

    else if (episode > 3)
        episode = 3;

    if (map < 1)
        map = 1;

    if (map > 9 && gamemode != commercial)
        map = 9;

    G_SetFastParms(skill == sk_nightmare);
    M_ClearRandom(I_GetRandomTimeSeed());

    for (i = 0; i < MAXPLAYERS; i++)
        players[i].playerstate = PST_REBORN;

    gameepisode = episode;
    gamemap = map;
    gameskill = skill;
    totalleveltimes = 0;
    respawnmonsters = skill == sk_nightmare;

    G_DoLoadLevel();

}

