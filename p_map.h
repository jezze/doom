#ifndef __P_MAP__
#define __P_MAP__

#include "r_defs.h"
#include "d_player.h"

#define USERANGE                        (64 * FRACUNIT)
#define MELEERANGE                      (64 * FRACUNIT)
#define MISSILERANGE                    (32 * 64 * FRACUNIT)
#define MAXRADIUS                       (32 * FRACUNIT)

enum
{

    BOXTOP,
    BOXBOTTOM,
    BOXLEFT,
    BOXRIGHT

};

boolean P_TryMove(mobj_t *thing, int x, int y, boolean dropoff);
boolean P_TeleportMove(mobj_t *thing, int x, int y,boolean boss);
void P_SlideMove(mobj_t *mo);
boolean P_CheckSight(mobj_t *t1, mobj_t *t2);
void P_UseLines(player_t *player);
int P_AimLineAttack(mobj_t *t1,angle_t angle,int distance, uint_64_t mask);
void P_LineAttack(mobj_t *t1, angle_t angle, int distance, int slope, int damage);
void P_RadiusAttack(mobj_t *spot, mobj_t *source, int damage);
boolean P_CheckPosition(mobj_t *thing, int x, int y);
boolean P_ChangeSector(sector_t* sector,boolean crunch);
boolean P_CheckSector(sector_t *sector, boolean crunch);
void P_DelSeclist(msecnode_t*);
void P_CreateSecNodeList(mobj_t*,int,int);
boolean Check_Sides(mobj_t *, int, int);
int P_GetMoveFactor(const mobj_t *mo, int *friction);
int P_GetFriction(const mobj_t *mo, int *factor);
void P_ApplyTorque(mobj_t *mo);
void P_MapStart(void);
void P_MapEnd(void);

extern boolean floatok;
extern boolean felldown;
extern int tmfloorz;
extern line_t *ceilingline;
extern mobj_t *linetarget;
extern msecnode_t *sector_list;
extern int tmbbox[4];
extern line_t *blockline;

#endif
