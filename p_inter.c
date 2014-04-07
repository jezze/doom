#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "d_event.h"
#include "d_englsh.h"
#include "m_random.h"
#include "r_main.h"
#include "s_sound.h"
#include "p_tick.h"
#include "p_inter.h"
#include "p_enemy.h"
#include "i_system.h"

#define BONUSADD                        6

static int maxhealth = 100;
static int maxarmor = 200;
static int maxsoul = 200;
static int green_armor_class = 1;
static int blue_armor_class = 2;
static int soul_health = 100;
static int clipammo[NUMAMMO] = {10, 4, 20, 1};
int mega_health = 200;
int god_health = 100;
int idfa_armor = 200;
int idfa_armor_class = 2;
int idkfa_armor = 200;
int idkfa_armor_class = 2;
int bfgcells = 40;
int maxammo[NUMAMMO] = {200, 50, 300, 50};

static boolean P_GiveAmmo(player_t *player, ammotype_t ammo, int num)
{

    int oldammo;

    if (ammo == am_noammo)
        return false;

    if (player->ammo[ammo] == player->maxammo[ammo])
        return false;

    if (num)
        num *= clipammo[ammo];
    else
        num = clipammo[ammo]/2;

    if (gameskill == sk_baby || gameskill == sk_nightmare)
        num <<= 1;

    oldammo = player->ammo[ammo];
    player->ammo[ammo] += num;

    if (player->ammo[ammo] > player->maxammo[ammo])
        player->ammo[ammo] = player->maxammo[ammo];

    if (oldammo)
        return true;

    switch (ammo)
    {

    case am_clip:
        if (player->readyweapon == wp_fist)
        {

            if (player->weaponowned[wp_chaingun])
                player->pendingweapon = wp_chaingun;
            else
                player->pendingweapon = wp_pistol;

        }

        break;

    case am_shell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
            if (player->weaponowned[wp_shotgun])
                player->pendingweapon = wp_shotgun;

        break;

    case am_cell:
        if (player->readyweapon == wp_fist || player->readyweapon == wp_pistol)
            if (player->weaponowned[wp_plasma])
                player->pendingweapon = wp_plasma;

        break;

    case am_misl:
        if (player->readyweapon == wp_fist)
            if (player->weaponowned[wp_missile])
                player->pendingweapon = wp_missile;
    default:
        break;

    }

    return true;

}

static boolean P_GiveWeapon(player_t *player, weapontype_t weapon, boolean dropped)
{

    boolean gaveammo;
    boolean gaveweapon;

    if (weaponinfo[weapon].ammo != am_noammo)
        gaveammo = P_GiveAmmo (player, weaponinfo[weapon].ammo, dropped ? 1 : 2);
    else
        gaveammo = false;

    if (player->weaponowned[weapon])
        gaveweapon = false;
    else
    {

        gaveweapon = true;
        player->weaponowned[weapon] = true;
        player->pendingweapon = weapon;

    }

    return gaveweapon || gaveammo;

}

static boolean P_GiveBody(player_t *player, int num)
{

    if (player->health >= maxhealth)
        return false;

    player->health += num;

    if (player->health > maxhealth)
        player->health = maxhealth;

    player->mo->health = player->health;

    return true;

}

static boolean P_GiveArmor(player_t *player, int armortype)
{

    int hits = armortype * 100;

    if (player->armorpoints >= hits)
        return false;

    player->armortype = armortype;
    player->armorpoints = hits;

    return true;

}

static void P_GiveCard(player_t *player, card_t card)
{

    if (player->cards[card])
        return;

    player->bonuscount = BONUSADD;
    player->cards[card] = 1;

}

boolean P_GivePower(player_t *player, int power)
{

    static const int tics[NUMPOWERS] = {
        INVULNTICS, 1, INVISTICS,
        IRONTICS, 1, INFRATICS,
    };

    switch (power)
    {
    case pw_invisibility:
        player->mo->flags |= MF_SHADOW;

        break;

    case pw_allmap:
        if (player->powers[pw_allmap])
            return false;

        break;

    case pw_strength:
        P_GiveBody(player, 100);

        break;

    }

    if (player->powers[power] >= 0)
        player->powers[power] = tics[power];

    return true;

}

void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher)
{

    player_t *player;
    int i;
    int sound;
    fixed_t delta = special->z - toucher->z;

    if (delta > toucher->height || delta < -8 * FRACUNIT)
        return;

    sound = sfx_itemup;
    player = toucher->player;

    if (toucher->health <= 0)
        return;

    switch (special->sprite)
    {

    case SPR_ARM1:
        if (!P_GiveArmor(player, green_armor_class))
            return;

        player->message = GOTARMOR;

        break;

    case SPR_ARM2:
        if (!P_GiveArmor(player, blue_armor_class))
            return;

        player->message = GOTMEGA;

        break;

    case SPR_BON1:
        player->health++;

        if (player->health > (maxhealth * 2))
            player->health = (maxhealth * 2);

        player->mo->health = player->health;
        player->message = GOTHTHBONUS;

        break;

    case SPR_BON2:
        player->armorpoints++;

        if (player->armorpoints > maxarmor)
            player->armorpoints = maxarmor;

        if (!player->armortype)
            player->armortype = green_armor_class;

        player->message = GOTARMBONUS;

        break;

    case SPR_SOUL:
        player->health += soul_health;

        if (player->health > maxsoul)
            player->health = maxsoul;

        player->mo->health = player->health;
        player->message = GOTSUPER;
        sound = sfx_getpow;

        break;

    case SPR_MEGA:
        if (gamemode != commercial)
            return;

        player->health = mega_health;
        player->mo->health = player->health;

        P_GiveArmor(player,blue_armor_class);

        player->message = GOTMSPHERE;
        sound = sfx_getpow;

        break;

    case SPR_BKEY:
        if (!player->cards[it_bluecard])
            player->message = GOTBLUECARD;

        P_GiveCard(player, it_bluecard);

        break;

    case SPR_YKEY:
        if (!player->cards[it_yellowcard])
            player->message = GOTYELWCARD;

        P_GiveCard(player, it_yellowcard);

        break;

    case SPR_RKEY:
        if (!player->cards[it_redcard])
            player->message = GOTREDCARD;

        P_GiveCard(player, it_redcard);

        break;

    case SPR_BSKU:
        if (!player->cards[it_blueskull])
            player->message = GOTBLUESKUL;

        P_GiveCard(player, it_blueskull);

        break;

    case SPR_YSKU:
        if (!player->cards[it_yellowskull])
            player->message = GOTYELWSKUL;

        P_GiveCard(player, it_yellowskull);

        break;

    case SPR_RSKU:
        if (!player->cards[it_redskull])
            player->message = GOTREDSKULL;

        P_GiveCard(player, it_redskull);

        break;

    case SPR_STIM:
        if (!P_GiveBody(player, 10))
            return;

        player->message = GOTSTIM;

        break;

    case SPR_MEDI:
        if (!P_GiveBody (player, 25))
            return;

        if (player->health < 50)
            player->message = GOTMEDINEED;
        else
            player->message = GOTMEDIKIT;

        break;

    case SPR_PINV:
        if (!P_GivePower(player, pw_invulnerability))
            return;

        player->message = GOTINVUL;
        sound = sfx_getpow;

        break;

    case SPR_PSTR:
        if (!P_GivePower(player, pw_strength))
            return;

        player->message = GOTBERSERK;

        if (player->readyweapon != wp_fist)
            player->pendingweapon = wp_fist;

        sound = sfx_getpow;

        break;

    case SPR_PINS:
        if (!P_GivePower(player, pw_invisibility))
            return;

        player->message = GOTINVIS;
        sound = sfx_getpow;

        break;

    case SPR_SUIT:
        if (!P_GivePower(player, pw_ironfeet))
            return;

        player->message = GOTSUIT;
        sound = sfx_getpow;

        break;

    case SPR_PMAP:
        if (!P_GivePower(player, pw_allmap))
            return;

        player->message = GOTMAP;
        sound = sfx_getpow;

        break;

    case SPR_PVIS:
        if (!P_GivePower(player, pw_infrared))
            return;

        player->message = GOTVISOR;
        sound = sfx_getpow;

        break;

    case SPR_CLIP:
        if (special->flags & MF_DROPPED)
        {

            if (!P_GiveAmmo(player,am_clip,0))
                return;

        }

        else
        {

            if (!P_GiveAmmo(player,am_clip,1))
                return;

        }

        player->message = GOTCLIP;

        break;

    case SPR_AMMO:
        if (!P_GiveAmmo(player, am_clip,5))
            return;

        player->message = GOTCLIPBOX;

        break;

    case SPR_ROCK:
        if (!P_GiveAmmo(player, am_misl,1))
            return;

        player->message = GOTROCKET;

        break;

    case SPR_BROK:
        if (!P_GiveAmmo(player, am_misl,5))
            return;

        player->message = GOTROCKBOX;

        break;

    case SPR_CELL:
        if (!P_GiveAmmo(player, am_cell,1))
            return;

        player->message = GOTCELL;

        break;

    case SPR_CELP:
        if (!P_GiveAmmo(player, am_cell,5))
            return;
            
        player->message = GOTCELLBOX;

        break;

    case SPR_SHEL:
        if (!P_GiveAmmo(player, am_shell,1))
            return;

        player->message = GOTSHELLS;

        break;

    case SPR_SBOX:
        if (!P_GiveAmmo(player, am_shell,5))
            return;

        player->message = GOTSHELLBOX;

        break;

    case SPR_BPAK:
        if (!player->backpack)
        {

            for (i = 0; i < NUMAMMO; i++)
                player->maxammo[i] *= 2;

            player->backpack = true;

        }

        for (i = 0; i < NUMAMMO; i++)
            P_GiveAmmo(player, i, 1);

        player->message = GOTBACKPACK;

        break;

    case SPR_BFUG:
        if (!P_GiveWeapon(player, wp_bfg, false))
            return;

        player->message = GOTBFG9000;
        sound = sfx_wpnup;

        break;

    case SPR_MGUN:
        if (!P_GiveWeapon(player, wp_chaingun, (special->flags & MF_DROPPED) != 0))
            return;

        player->message = GOTCHAINGUN;
        sound = sfx_wpnup;

        break;

    case SPR_CSAW:
        if (!P_GiveWeapon(player, wp_chainsaw, false))
            return;

        player->message = GOTCHAINSAW;
        sound = sfx_wpnup;

        break;

    case SPR_LAUN:
        if (!P_GiveWeapon(player, wp_missile, false))
            return;

      player->message = GOTLAUNCHER;
      sound = sfx_wpnup;
      break;

    case SPR_PLAS:
        if (!P_GiveWeapon(player, wp_plasma, false))
            return;

        player->message = GOTPLASMA;
        sound = sfx_wpnup;

        break;

    case SPR_SHOT:
        if (!P_GiveWeapon(player, wp_shotgun, (special->flags & MF_DROPPED) != 0))
            return;

        player->message = GOTSHOTGUN;
        sound = sfx_wpnup;

        break;

    case SPR_SGN2:
        if (!P_GiveWeapon(player, wp_supershotgun, (special->flags & MF_DROPPED) != 0))
            return;

        player->message = GOTSHOTGUN2;
        sound = sfx_wpnup;

        break;

    default:
        I_Error ("P_SpecialThing: Unknown gettable thing");

    }

    if (special->flags & MF_COUNTITEM)
        player->itemcount++;

    P_RemoveMobj(special);

    player->bonuscount += BONUSADD;

    S_StartSound(player->mo, sound | PICKUP_SOUND);

}

static void P_KillMobj(mobj_t *source, mobj_t *target)
{

    mobjtype_t item;
    mobj_t *mo;

    target->flags &= ~(MF_SHOOTABLE|MF_FLOAT|MF_SKULLFLY);

    if (target->type != MT_SKULL)
        target->flags &= ~MF_NOGRAVITY;

    target->flags |= MF_CORPSE|MF_DROPOFF;
    target->height >>= 2;

    if (!((target->flags ^ MF_COUNTKILL) & (MF_FRIEND | MF_COUNTKILL)))
        totallive--;

    if (source && source->player)
    {

        if (target->flags & MF_COUNTKILL)
            source->player->killcount++;

        if (target->player)
            source->player->frags[target->player-players]++;

    }

    else if (target->flags & MF_COUNTKILL)
    {

        if (target->lastenemy && target->lastenemy->health > 0 && target->lastenemy->player)
        {

            target->lastenemy->player->killcount++;

        }

        else
        {

            unsigned int activeplayers = 0, player, i;

            for (player = 0; player < MAXPLAYERS; player++)
                if (playeringame[player])
                    activeplayers++;

            if (activeplayers)
            {

                player = P_Random(pr_friends) % activeplayers;

                for (i = 0; i < MAXPLAYERS; i++)
                    if (playeringame[i])
                        if (!player--)
                            players[i].killcount++;

            }
        }

    }

    if (target->player)
    {

        if (!source)
            target->player->frags[target->player-players]++;

        target->flags &= ~MF_SOLID;
        target->player->playerstate = PST_DEAD;

        P_DropWeapon(target->player);

    }

    if (target->health < -target->info->spawnhealth && target->info->xdeathstate)
        P_SetMobjState(target, target->info->xdeathstate);
    else
        P_SetMobjState(target, target->info->deathstate);

    target->tics -= P_Random(pr_killtics)&3;

    if (target->tics < 1)
        target->tics = 1;

    switch (target->type)
    {

    case MT_WOLFSS:
    case MT_POSSESSED:
        item = MT_CLIP;
        break;

    case MT_SHOTGUY:
        item = MT_SHOTGUN;
        break;

    case MT_CHAINGUY:
        item = MT_CHAINGUN;
        break;

    default:
        return;

    }

    mo = P_SpawnMobj(target->x, target->y, ONFLOORZ, item);
    mo->flags |= MF_DROPPED;

}

void P_DamageMobj(mobj_t *target, mobj_t *inflictor, mobj_t *source, int damage)
{

    player_t *player;
    boolean justhit = false;

    if (!(target->flags & (MF_SHOOTABLE | MF_BOUNCES)))
        return;

    if (target->health <= 0)
        return;

    if (target->flags & MF_SKULLFLY)
        target->momx = target->momy = target->momz = 0;

    player = target->player;

    if (player && gameskill == sk_baby)
        damage >>= 1;

    if (inflictor && !(target->flags & MF_NOCLIP) && (!source || !source->player || source->player->readyweapon != wp_chainsaw))
    {

        unsigned ang = R_PointToAngle2 (inflictor->x, inflictor->y, target->x, target->y);
        fixed_t thrust = damage * (FRACUNIT >> 3) * 100 / target->info->mass;

        if (damage < 40 && damage > target->health && target->z - inflictor->z > 64 * FRACUNIT && P_Random(pr_damagemobj) & 1)
        {

            ang += ANG180;
            thrust *= 4;

        }

        ang >>= ANGLETOFINESHIFT;
        target->momx += FixedMul (thrust, finecosine[ang]);
        target->momy += FixedMul (thrust, finesine[ang]);

        if (target->intflags & MIF_FALLING && target->gear >= MAXGEAR)
            target->gear = 0;

    }

    if (player)
    {

        if (target->subsector->sector->special == 11 && damage >= target->health)
            damage = target->health - 1;

        if ((damage < 1000 || ((player->cheats & CF_GODMODE))) && (player->cheats&CF_GODMODE || player->powers[pw_invulnerability]))
            return;

        if (player->armortype)
        {

            int saved = player->armortype == 1 ? damage / 3 : damage / 2;
            
            if (player->armorpoints <= saved)
            {

                saved = player->armorpoints;
                player->armortype = 0;

            }

            player->armorpoints -= saved;
            damage -= saved;

        }

        player->health -= damage;

        if (player->health < 0)
            player->health = 0;

        player->attacker = source;
        player->damagecount += damage;

        if (player->damagecount > 100)
            player->damagecount = 100;

    }

    target->health -= damage;

    if (target->health <= 0)
    {

        P_KillMobj(source, target);

        return;

    }


        if (player)
            P_SetTarget(&target->target, source);

        if (target->health * 2 < target->info->spawnhealth)
        {

            thinker_t *cap = &thinkerclasscap[target->flags & MF_FRIEND ? th_friends : th_enemies];
            (target->thinker.cprev->cnext = target->thinker.cnext)->cprev = target->thinker.cprev;
            (target->thinker.cnext = cap->cnext)->cprev = &target->thinker;
            (target->thinker.cprev = cap)->cnext = &target->thinker;

        }

    if (P_Random (pr_painchance) < target->info->painchance && !(target->flags & MF_SKULLFLY))
    {

        justhit = true;

        P_SetMobjState(target, target->info->painstate);

    }

    target->reactiontime = 0;

    if (source && source != target && source->type != MT_VILE && (!target->threshold || target->type == MT_VILE) && ((source->flags ^ target->flags) & MF_FRIEND || monster_infighting))
    {

        if (!target->lastenemy || target->lastenemy->health <= 0 || (!((target->flags ^ target->lastenemy->flags) & MF_FRIEND) && target->target != source))
            P_SetTarget(&target->lastenemy, target->target);

        P_SetTarget(&target->target, source);

        target->threshold = BASETHRESHOLD;

        if (target->state == &states[target->info->spawnstate] && target->info->seestate != S_NULL)
            P_SetMobjState (target, target->info->seestate);

    }

    if (justhit && (target->target == source || !target->target || !(target->flags & target->target->flags & MF_FRIEND)))
        target->flags |= MF_JUSTHIT;

}

