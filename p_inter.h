#ifndef __P_INTER__
#define __P_INTER__

#define BASETHRESHOLD                   100

boolean P_GivePower(player_t *, int);
void P_TouchSpecialThing(mobj_t *special, mobj_t *toucher);
void P_DamageMobj(mobj_t *target,mobj_t *inflictor,mobj_t *source,int damage);

extern int god_health;
extern int idfa_armor;
extern int idfa_armor_class;
extern int idkfa_armor;
extern int idkfa_armor_class;
extern int mega_health;
extern int bfgcells;
extern int maxammo[];

#endif
