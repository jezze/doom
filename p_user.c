#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "d_event.h"
#include "r_main.h"
#include "p_map.h"
#include "p_spec.h"
#include "p_user.h"

#define INVERSECOLORMAP                 32
#define MAXBOB                          0x100000

static boolean onground;

void P_Thrust(player_t *player, angle_t angle, fixed_t move)
{

    angle >>= ANGLETOFINESHIFT;
    player->mo->momx += FixedMul(move, finecosine[angle]);
    player->mo->momy += FixedMul(move, finesine[angle]);

}

static void P_Bob(player_t *player, angle_t angle, fixed_t move)
{

    player->momx += FixedMul(move, finecosine[angle >>= ANGLETOFINESHIFT]);
    player->momy += FixedMul(move, finesine[angle]);

}

void P_CalcHeight(player_t *player)
{

    int angle;
    fixed_t bob;

    player->bob = false ? (FixedMul(player->mo->momx, player->mo->momx) + FixedMul(player->mo->momy,player->mo->momy)) >> 2 : player_bobbing ? (FixedMul(player->momx, player->momx) + FixedMul(player->momy, player->momy)) >> 2 : 0;

    if (player->bob > MAXBOB)
    {

        player->bob = MAXBOB;

    }

    if (!onground || player->cheats & CF_NOMOMENTUM)
    {

        player->viewz = player->mo->z + VIEWHEIGHT;

        if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
            player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

        return;

    }

    angle = (FINEANGLES / 20 * leveltime) & FINEMASK;
    bob = FixedMul(player->bob / 2, finesine[angle]);

    if (player->playerstate == PST_LIVE)
    {

        player->viewheight += player->deltaviewheight;

        if (player->viewheight > VIEWHEIGHT)
        {

            player->viewheight = VIEWHEIGHT;
            player->deltaviewheight = 0;

        }

        if (player->viewheight < VIEWHEIGHT / 2)
        {

            player->viewheight = VIEWHEIGHT / 2;

            if (player->deltaviewheight <= 0)
                player->deltaviewheight = 1;

        }

        if (player->deltaviewheight)
        {

            player->deltaviewheight += FRACUNIT / 4;

            if (!player->deltaviewheight)
                player->deltaviewheight = 1;

        }
    }

    player->viewz = player->mo->z + player->viewheight + bob;

    if (player->viewz > player->mo->ceilingz - 4 * FRACUNIT)
        player->viewz = player->mo->ceilingz - 4 * FRACUNIT;

}

void P_MovePlayer(player_t *player)
{

    ticcmd_t *cmd = &player->cmd;
    mobj_t *mo = player->mo;

    mo->angle += cmd->angleturn << 16;
    onground = mo->z <= mo->floorz;

    if ((cmd->forwardmove | cmd->sidemove))
    {

        if (onground || mo->flags & MF_BOUNCES)
        {

            int friction, movefactor = P_GetMoveFactor(mo, &friction);
            int bobfactor = friction < ORIG_FRICTION ? movefactor : ORIG_FRICTION_FACTOR;

            if (cmd->forwardmove)
            {

                P_Bob(player,mo->angle,cmd->forwardmove * bobfactor);
                P_Thrust(player,mo->angle,cmd->forwardmove * movefactor);

            }

            if (cmd->sidemove)
            {

                P_Bob(player, mo->angle - ANG90, cmd->sidemove * bobfactor);
                P_Thrust(player, mo->angle - ANG90, cmd->sidemove * movefactor);

            }

        }

        if (mo->state == states + S_PLAY)
            P_SetMobjState(mo, S_PLAY_RUN1);

    }

}

#define ANG5 (ANG90/18)

void P_DeathThink(player_t *player)
{

    angle_t angle;
    angle_t delta;

    P_MovePsprites(player);

    if (player->viewheight > 6 * FRACUNIT)
        player->viewheight -= FRACUNIT;

    if (player->viewheight < 6 * FRACUNIT)
        player->viewheight = 6 * FRACUNIT;

    player->deltaviewheight = 0;
    onground = (player->mo->z <= player->mo->floorz);
    P_CalcHeight(player);

    if (player->attacker && player->attacker != player->mo)
    {

        angle = R_PointToAngle2(player->mo->x, player->mo->y, player->attacker->x, player->attacker->y);
        delta = angle - player->mo->angle;

        if (delta < ANG5 || delta > (unsigned)-ANG5)
        {

            player->mo->angle = angle;

            if (player->damagecount)
                player->damagecount--;

        }

        else if (delta < ANG180)
        {

            player->mo->angle += ANG5;

        }

        else
        {

            player->mo->angle -= ANG5;

        }

    }

    else if (player->damagecount)
    {

        player->damagecount--;

    }

    if (player->cmd.buttons & BT_USE)
        player->playerstate = PST_REBORN;

}

void P_PlayerThink(player_t *player)
{

    ticcmd_t *cmd;
    weapontype_t newweapon;

    if (player->cheats & CF_NOCLIP)
        player->mo->flags |= MF_NOCLIP;
    else
        player->mo->flags &= ~MF_NOCLIP;

    cmd = &player->cmd;

    if (player->mo->flags & MF_JUSTATTACKED)
    {

        cmd->angleturn = 0;
        cmd->forwardmove = 0xc800 / 512;
        cmd->sidemove = 0;
        player->mo->flags &= ~MF_JUSTATTACKED;

    }

    if (player->playerstate == PST_DEAD)
    {

        P_DeathThink(player);

        return;

    }

    if (player->mo->reactiontime)
        player->mo->reactiontime--;
    else
        P_MovePlayer(player);

    P_CalcHeight(player);

    if (player->mo->subsector->sector->special)
        P_PlayerInSpecialSector(player);

    if (cmd->buttons & BT_CHANGE)
    {

        newweapon = (cmd->buttons & BT_WEAPONMASK) >> BT_WEAPONSHIFT;

        if (player->weaponowned[newweapon] && newweapon != player->readyweapon)
            if ((newweapon != wp_plasma && newweapon != wp_bfg) || (gamemode != shareware))
                player->pendingweapon = newweapon;

    }

    if (cmd->buttons & BT_USE)
    {

        if (!player->usedown)
        {

            P_UseLines(player);

            player->usedown = true;

        }

    }

    else
    {

        player->usedown = false;

    }

    P_MovePsprites(player);

    if (player->powers[pw_strength])
        player->powers[pw_strength]++;

    if (player->powers[pw_invulnerability] > 0)
        player->powers[pw_invulnerability]--;

    if (player->powers[pw_invisibility] > 0)
        if (!--player->powers[pw_invisibility])
            player->mo->flags &= ~MF_SHADOW;

    if (player->powers[pw_infrared] > 0)
        player->powers[pw_infrared]--;

    if (player->powers[pw_ironfeet] > 0)
        player->powers[pw_ironfeet]--;

    if (player->damagecount)
        player->damagecount--;

    if (player->bonuscount)
        player->bonuscount--;

    player->fixedcolormap = player->powers[pw_invulnerability] > 4 * 32 || player->powers[pw_invulnerability] & 8 ? INVERSECOLORMAP : player->powers[pw_infrared] > 4 * 32 || player->powers[pw_infrared] & 8;

}

