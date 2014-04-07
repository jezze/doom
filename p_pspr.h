#ifndef __P_PSPR__
#define __P_PSPR__

#include "m_fixed.h"
#include "tables.h"
#include "info.h"

#define FF_FULLBRIGHT                   0x8000
#define FF_FRAMEMASK                    0x7fff

struct player_s;

typedef enum
{

    ps_weapon,
    ps_flash,
    NUMPSPRITES

} psprnum_t;

typedef struct
{

    state_t *state;
    int tics;
    fixed_t sx;
    fixed_t sy;

} pspdef_t;

typedef struct
{

    ammotype_t  ammo;
    int upstate;
    int downstate;
    int readystate;
    int atkstate;
    int flashstate;

} weaponinfo_t;

extern weaponinfo_t weaponinfo[NUMWEAPONS];
extern int weapon_preferences[2][NUMWEAPONS + 1];

int P_WeaponPreferred(int w1, int w2);
int P_SwitchWeapon(struct player_s *player);
boolean P_CheckAmmo(struct player_s *player);
void P_SetupPsprites(struct player_s *curplayer);
void P_MovePsprites(struct player_s *curplayer);
void P_DropWeapon(struct player_s *player);
void A_Light0();
void A_WeaponReady();
void A_Lower();
void A_Raise();
void A_Punch();
void A_ReFire();
void A_FirePistol();
void A_Light1();
void A_FireShotgun();
void A_Light2();
void A_FireShotgun2();
void A_CheckReload();
void A_OpenShotgun2();
void A_LoadShotgun2();
void A_CloseShotgun2();
void A_FireCGun();
void A_GunFlash();
void A_FireMissile();
void A_Saw();
void A_FirePlasma();
void A_BFGsound();
void A_FireBFG();
void A_BFGSpray();

#endif
