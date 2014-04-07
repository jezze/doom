#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_map.h"
#include "p_inter.h"
#include "p_pspr.h"
#include "p_enemy.h"
#include "m_random.h"
#include "s_sound.h"
#include "d_event.h"

#define LOWERSPEED                      (FRACUNIT * 6)
#define RAISESPEED                      (FRACUNIT * 6)
#define WEAPONBOTTOM                    (FRACUNIT * 128)
#define WEAPONTOP                       (FRACUNIT * 32)

weaponinfo_t weaponinfo[NUMWEAPONS] = {
    {am_noammo, S_PUNCHUP, S_PUNCHDOWN, S_PUNCH, S_PUNCH1, S_NULL},
    {am_clip, S_PISTOLUP, S_PISTOLDOWN, S_PISTOL, S_PISTOL1, S_PISTOLFLASH},
    {am_shell, S_SGUNUP, S_SGUNDOWN, S_SGUN, S_SGUN1, S_SGUNFLASH1},
    {am_clip, S_CHAINUP, S_CHAINDOWN, S_CHAIN, S_CHAIN1, S_CHAINFLASH1},
    {am_misl, S_MISSILEUP, S_MISSILEDOWN, S_MISSILE, S_MISSILE1, S_MISSILEFLASH1},
    {am_cell, S_PLASMAUP, S_PLASMADOWN, S_PLASMA, S_PLASMA1, S_PLASMAFLASH1},
    {am_cell, S_BFGUP, S_BFGDOWN, S_BFG, S_BFG1, S_BFGFLASH1},
    {am_noammo, S_SAWUP, S_SAWDOWN, S_SAW, S_SAW1, S_NULL},
    {am_shell, S_DSGUNUP, S_DSGUNDOWN, S_DSGUN, S_DSGUN1, S_DSGUNFLASH1},
};

int weapon_preferences[2][NUMWEAPONS + 1] = {
    {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},
    {6, 9, 4, 3, 2, 8, 5, 7, 1, 0},
};

static const int recoil_values[] = {10, 10, 30, 10, 100, 20, 100, 0, 80};
static fixed_t bulletslope;

extern void P_Thrust(player_t *, angle_t, fixed_t);

static void P_SetPsprite(player_t *player, int position, statenum_t stnum)
{

    pspdef_t *psp = &player->psprites[position];

    do
    {

        state_t *state;

        if (!stnum)
        {

            psp->state = NULL;

            break;

        }

        state = &states[stnum];
        psp->state = state;
        psp->tics = state->tics;

        if (state->misc1)
        {

            psp->sx = state->misc1 << FRACBITS;
            psp->sy = state->misc2 << FRACBITS;

        }

        if (state->action)
        {

            state->action(player, psp);

            if (!psp->state)
                break;

        }

        stnum = psp->state->nextstate;

    } while (!psp->tics);

}

static void P_BringUpWeapon(player_t *player)
{

    statenum_t newstate;

    if (player->pendingweapon == wp_nochange)
        player->pendingweapon = player->readyweapon;

    if (player->pendingweapon == wp_chainsaw)
        S_StartSound(player->mo, sfx_sawup);

    newstate = weaponinfo[player->pendingweapon].upstate;
    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM + FRACUNIT * 2;

    P_SetPsprite(player, ps_weapon, newstate);

}

int P_SwitchWeapon(player_t *player)
{

    int *prefer = weapon_preferences[1];
    int currentweapon = player->readyweapon;
    int newweapon = currentweapon;
    int i = NUMWEAPONS + 1;

    do
    {

        switch (*prefer++)
        {

        case 1:
            if (!player->powers[pw_strength])
                break;

        case 0:
            newweapon = wp_fist;

            break;

        case 2:
            if (player->ammo[am_clip])
                newweapon = wp_pistol;

            break;

        case 3:
            if (player->weaponowned[wp_shotgun] && player->ammo[am_shell])
                newweapon = wp_shotgun;

            break;

        case 4:
            if (player->weaponowned[wp_chaingun] && player->ammo[am_clip])
                newweapon = wp_chaingun;

            break;

        case 5:
            if (player->weaponowned[wp_missile] && player->ammo[am_misl])
                newweapon = wp_missile;

            break;

        case 6:
            if (player->weaponowned[wp_plasma] && player->ammo[am_cell] && gamemode != shareware)
                newweapon = wp_plasma;

            break;

        case 7:
            if (player->weaponowned[wp_bfg] && gamemode != shareware && player->ammo[am_cell] >= 40)
                newweapon = wp_bfg;

            break;

        case 8:
            if (player->weaponowned[wp_chainsaw])
                newweapon = wp_chainsaw;

            break;

        case 9:
            if (player->weaponowned[wp_supershotgun] && gamemode == commercial && player->ammo[am_shell] >= 2)
                newweapon = wp_supershotgun;

            break;

        }

    } while (newweapon==currentweapon && --i);

    return newweapon;

}


int P_WeaponPreferred(int w1, int w2)
{

    return (weapon_preferences[0][0] != ++w2 && (weapon_preferences[0][0] == ++w1 || (weapon_preferences[0][1] != w2 && (weapon_preferences[0][1] == w1 || (weapon_preferences[0][2] != w2 && (weapon_preferences[0][2] == w1 || (weapon_preferences[0][3] != w2 && (weapon_preferences[0][3] == w1 || (weapon_preferences[0][4] != w2 && (weapon_preferences[0][4] == w1 || (weapon_preferences[0][5] != w2 && (weapon_preferences[0][5] == w1 || (weapon_preferences[0][6] != w2 && (weapon_preferences[0][6] == w1 || (weapon_preferences[0][7] != w2 && (weapon_preferences[0][7] == w1))))))))))))))));

}

boolean P_CheckAmmo(player_t *player)
{

    ammotype_t ammo = weaponinfo[player->readyweapon].ammo;
    int count = 1;

    if (player->readyweapon == wp_bfg)
        count = bfgcells;
    else if (player->readyweapon == wp_supershotgun)
        count = 2;

    if (ammo == am_noammo || player->ammo[ammo] >= count)
        return true;

    return false;

}

static void P_FireWeapon(player_t *player)
{

    statenum_t newstate;

    if (!P_CheckAmmo(player))
        return;

    P_SetMobjState(player->mo, S_PLAY_ATK1);

    newstate = weaponinfo[player->readyweapon].atkstate;

    P_SetPsprite(player, ps_weapon, newstate);
    P_NoiseAlert(player->mo, player->mo);

}

void P_DropWeapon(player_t *player)
{

    P_SetPsprite(player, ps_weapon, weaponinfo[player->readyweapon].downstate);

}

void A_WeaponReady(player_t *player, pspdef_t *psp)
{

    if (player->mo->state == &states[S_PLAY_ATK1] || player->mo->state == &states[S_PLAY_ATK2])
        P_SetMobjState(player->mo, S_PLAY);

    if (player->readyweapon == wp_chainsaw && psp->state == &states[S_SAW])
        S_StartSound(player->mo, sfx_sawidl);

    if (player->pendingweapon != wp_nochange || !player->health)
    {

        statenum_t newstate = weaponinfo[player->readyweapon].downstate;

        P_SetPsprite(player, ps_weapon, newstate);

        return;

    }

    if (player->cmd.buttons & BT_ATTACK)
    {

        if (!player->attackdown || (player->readyweapon != wp_missile && player->readyweapon != wp_bfg))
        {

            player->attackdown = true;

            P_FireWeapon(player);

            return;

        }

    }

    else
    {

        player->attackdown = false;

    }

    {

        int angle = (128*leveltime) & FINEMASK;

        psp->sx = FRACUNIT + FixedMul(player->bob, finecosine[angle]);
        angle &= FINEANGLES/2 - 1;
        psp->sy = WEAPONTOP + FixedMul(player->bob, finesine[angle]);

    }

}

void A_ReFire(player_t *player, pspdef_t *psp)
{

    if ((player->cmd.buttons & BT_ATTACK) && player->pendingweapon == wp_nochange && player->health)
    {

        player->refire++;

        P_FireWeapon(player);

    }

    else
    {

        player->refire = 0;
        
        P_CheckAmmo(player);

    }

}

void A_CheckReload(player_t *player, pspdef_t *psp)
{

    if (!P_CheckAmmo(player))
        P_SetPsprite(player,ps_weapon,weaponinfo[player->readyweapon].downstate);

}

void A_Lower(player_t *player, pspdef_t *psp)
{

    psp->sy += LOWERSPEED;

    if (psp->sy < WEAPONBOTTOM)
        return;

    if (player->playerstate == PST_DEAD)
    {

        psp->sy = WEAPONBOTTOM;

        return;

    }

    if (!player->health)
    {

        P_SetPsprite(player, ps_weapon, S_NULL);

        return;

    }

    player->readyweapon = player->pendingweapon;

    P_BringUpWeapon(player);

}

void A_Raise(player_t *player, pspdef_t *psp)
{

    statenum_t newstate;

    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP)
        return;

    psp->sy = WEAPONTOP;
    newstate = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite(player, ps_weapon, newstate);

}

static void A_FireSomething(player_t* player,int adder)
{

    P_SetPsprite(player, ps_flash, weaponinfo[player->readyweapon].flashstate + adder);

    if (!(player->mo->flags & MF_NOCLIP))
    {

        if (weapon_recoil)
            P_Thrust(player, ANG180 + player->mo->angle, 2048 * recoil_values[player->readyweapon]);

    }

}

void A_GunFlash(player_t *player, pspdef_t *psp)
{

    P_SetMobjState(player->mo, S_PLAY_ATK2);
    A_FireSomething(player, 0);

}

void A_Punch(player_t *player, pspdef_t *psp)
{

    angle_t angle;
    int t, slope, damage = (P_Random(pr_punch) % 10 + 1) << 1;

    if (player->powers[pw_strength])
        damage *= 10;

    angle = player->mo->angle;
    t = P_Random(pr_punchangle);
    angle += (t - P_Random(pr_punchangle)) << 18;

    if ((slope = P_AimLineAttack(player->mo, angle, MELEERANGE, MF_FRIEND), !linetarget))
        slope = P_AimLineAttack(player->mo, angle, MELEERANGE, 0);

    P_LineAttack(player->mo, angle, MELEERANGE, slope, damage);

    if (!linetarget)
        return;

    S_StartSound(player->mo, sfx_punch);

    player->mo->angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);

}

void A_Saw(player_t *player, pspdef_t *psp)
{

    int slope, damage = 2 * (P_Random(pr_saw) % 10 + 1);
    angle_t angle = player->mo->angle;
    int t = P_Random(pr_saw);

    angle += (t - P_Random(pr_saw)) << 18;

    if ((slope = P_AimLineAttack(player->mo, angle, MELEERANGE + 1, MF_FRIEND), !linetarget))
        slope = P_AimLineAttack(player->mo, angle, MELEERANGE + 1, 0);

    P_LineAttack(player->mo, angle, MELEERANGE + 1, slope, damage);

    if (!linetarget)
    {

        S_StartSound(player->mo, sfx_sawful);

        return;

    }

    S_StartSound(player->mo, sfx_sawhit);

    angle = R_PointToAngle2(player->mo->x, player->mo->y, linetarget->x, linetarget->y);

    if (angle - player->mo->angle > ANG180)
    {

        if (angle - player->mo->angle < -ANG90 / 20)
            player->mo->angle = angle + ANG90 / 21;
        else
            player->mo->angle -= ANG90 / 20;

    }
    
    else
    {

        if (angle - player->mo->angle > ANG90 / 20)
            player->mo->angle = angle - ANG90 / 21;
        else
            player->mo->angle += ANG90 / 20;

    }

    player->mo->flags |= MF_JUSTATTACKED;

}

void A_FireMissile(player_t *player, pspdef_t *psp)
{

    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    
    P_SpawnPlayerMissile(player->mo, MT_ROCKET);

}

void A_FireBFG(player_t *player, pspdef_t *psp)
{

    player->ammo[weaponinfo[player->readyweapon].ammo] -= bfgcells;

    P_SpawnPlayerMissile(player->mo, MT_BFG);

}

void A_FirePlasma(player_t *player, pspdef_t *psp)
{

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    A_FireSomething(player,P_Random(pr_plasma) & 1);
    P_SpawnPlayerMissile(player->mo, MT_PLASMA);

}

static void P_BulletSlope(mobj_t *mo)
{

    angle_t an = mo->angle;
    uint_64_t mask = MF_FRIEND;

    do
    {

        bulletslope = P_AimLineAttack(mo, an, 16 * 64 * FRACUNIT, mask);

        if (!linetarget)
            bulletslope = P_AimLineAttack(mo, an += 1 << 26, 16 * 64 * FRACUNIT, mask);

        if (!linetarget)
            bulletslope = P_AimLineAttack(mo, an -= 2 << 26, 16 * 64 * FRACUNIT, mask);

    } while (mask && (mask=0, !linetarget));

}

static void P_GunShot(mobj_t *mo, boolean accurate)
{

    int damage = 5 * (P_Random(pr_gunshot) % 3 + 1);
    angle_t angle = mo->angle;

    if (!accurate)
    {

        int t = P_Random(pr_misfire);

        angle += (t - P_Random(pr_misfire)) << 18;

    }

    P_LineAttack(mo, angle, MISSILERANGE, bulletslope, damage);

}

void A_FirePistol(player_t *player, pspdef_t *psp)
{

    S_StartSound(player->mo, sfx_pistol);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    A_FireSomething(player, 0);
    P_BulletSlope(player->mo);
    P_GunShot(player->mo, !player->refire);

}

void A_FireShotgun(player_t *player, pspdef_t *psp)
{

    int i;

    S_StartSound(player->mo, sfx_shotgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    A_FireSomething(player, 0);
    P_BulletSlope(player->mo);

    for (i = 0; i < 7; i++)
        P_GunShot(player->mo, false);

}

void A_FireShotgun2(player_t *player, pspdef_t *psp)
{

    int i;

    S_StartSound(player->mo, sfx_dshtgn);
    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo] -= 2;

    A_FireSomething(player, 0);
    P_BulletSlope(player->mo);

    for (i = 0; i < 20; i++)
    {

        int damage = 5 * (P_Random(pr_shotgun) % 3 + 1);
        angle_t angle = player->mo->angle;
        int t = P_Random(pr_shotgun);

        angle += (t - P_Random(pr_shotgun)) << 19;
        t = P_Random(pr_shotgun);

        P_LineAttack(player->mo, angle, MISSILERANGE, bulletslope + ((t - P_Random(pr_shotgun)) << 5), damage);

    }

}

void A_FireCGun(player_t *player, pspdef_t *psp)
{

    if (player->ammo[weaponinfo[player->readyweapon].ammo])
        S_StartSound(player->mo, sfx_pistol);

    if (!player->ammo[weaponinfo[player->readyweapon].ammo])
        return;

    P_SetMobjState(player->mo, S_PLAY_ATK2);

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    A_FireSomething(player,psp->state - &states[S_CHAIN1]);
    P_BulletSlope(player->mo);
    P_GunShot(player->mo, !player->refire);

}

void A_Light0(player_t *player, pspdef_t *psp)
{

    player->extralight = 0;

}

void A_Light1 (player_t *player, pspdef_t *psp)
{

    player->extralight = 1;

}

void A_Light2 (player_t *player, pspdef_t *psp)
{

    player->extralight = 2;

}

void A_BFGSpray(mobj_t *mo)
{

    int i;

    for (i = 0; i < 40; i++)
    {

        int j, damage;
        angle_t an = mo->angle - ANG90 / 2 + ANG90 / 40 * i;

        if ((P_AimLineAttack(mo->target, an, 16 * 64 * FRACUNIT, MF_FRIEND), !linetarget))
            P_AimLineAttack(mo->target, an, 16 * 64 * FRACUNIT, 0);

        if (!linetarget)
            continue;

        P_SpawnMobj(linetarget->x, linetarget->y, linetarget->z + (linetarget->height >> 2), MT_EXTRABFG);

        for (damage = j = 0; j < 15; j++)
            damage += (P_Random(pr_bfg) & 7) + 1;

        P_DamageMobj(linetarget, mo->target, mo->target, damage);

    }

}

void A_BFGsound(player_t *player, pspdef_t *psp)
{

    S_StartSound(player->mo, sfx_bfg);

}

void P_SetupPsprites(player_t *player)
{

    int i;

    for (i = 0; i < NUMPSPRITES; i++)
        player->psprites[i].state = NULL;

    player->pendingweapon = player->readyweapon;

    P_BringUpWeapon(player);

}

void P_MovePsprites(player_t *player)
{

    pspdef_t *psp = player->psprites;
    int i;

    for (i = 0; i < NUMPSPRITES; i++, psp++)
    {

        if (psp->state && psp->tics != -1 && !--psp->tics)
            P_SetPsprite(player, i, psp->state->nextstate);

    }

    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;

}

