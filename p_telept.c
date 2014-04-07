#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "p_spec.h"
#include "p_maputl.h"
#include "p_map.h"
#include "r_main.h"
#include "p_tick.h"
#include "s_sound.h"
#include "p_user.h"

static mobj_t* P_TeleportDestination(line_t* line)
{
  int i;
  for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;) {
    register thinker_t* th = NULL;
    while ((th = P_NextThinker(th,th_misc)) != NULL)
      if (th->function == P_MobjThinker) {
        register mobj_t* m = (mobj_t*)th;
        if (m->type == MT_TELEPORTMAN  &&
            m->subsector->sector-sectors == i)
            return m;
      }
  }
  return NULL;
}

int EV_Teleport(line_t *line, int side, mobj_t *thing)
{
  mobj_t    *m;

  if (side || thing->flags & MF_MISSILE)
    return 0;

  if ((m = P_TeleportDestination(line)) != NULL)
        {
          fixed_t oldx = thing->x, oldy = thing->y, oldz = thing->z;
          player_t *player = thing->player;


          if (player && player->mo != thing)
            player = NULL;

          if (!P_TeleportMove(thing, m->x, m->y, false))
            return 0;

          thing->z = thing->floorz;

          if (player)
            player->viewz = thing->z + player->viewheight;


          S_StartSound(P_SpawnMobj(oldx, oldy, oldz, MT_TFOG), sfx_telept);


          S_StartSound(P_SpawnMobj(m->x +
                                    20*finecosine[m->angle>>ANGLETOFINESHIFT],
                                   m->y +
                                    20*finesine[m->angle>>ANGLETOFINESHIFT],
                                   thing->z, MT_TFOG),
                       sfx_telept);

          if (thing->player)
            thing->reactiontime = 18;

          thing->angle = m->angle;

          thing->momx = thing->momy = thing->momz = 0;

    if (player)
      player->momx = player->momy = 0;

          return 1;
        }
  return 0;
}

int EV_SilentTeleport(line_t *line, int side, mobj_t *thing)
{
  mobj_t    *m;

  if (side || thing->flags & MF_MISSILE)
    return 0;

  if ((m = P_TeleportDestination(line)) != NULL)
        {

          fixed_t z = thing->z - thing->floorz;

          angle_t angle =
            R_PointToAngle2(0, 0, line->dx, line->dy) - m->angle + ANG90;


          fixed_t s = finesine[angle>>ANGLETOFINESHIFT];
          fixed_t c = finecosine[angle>>ANGLETOFINESHIFT];


          fixed_t momx = thing->momx;
          fixed_t momy = thing->momy;


          player_t *player = thing->player;


          if (!P_TeleportMove(thing, m->x, m->y, false)) /* killough 8/9/98 */
            return 0;


          thing->angle += angle;


          thing->z = z + thing->floorz;


          thing->momx = FixedMul(momx, c) - FixedMul(momy, s);
          thing->momy = FixedMul(momy, c) + FixedMul(momx, s);



          if (player && player->mo == thing)
            {

              fixed_t deltaviewheight = player->deltaviewheight;


              player->deltaviewheight = 0;


              P_CalcHeight(player);


              player->deltaviewheight = deltaviewheight;
            }
          

          return 1;
        }
  return 0;
}

#define FUDGEFACTOR 10

int EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing,
                          boolean reverse)
{
  int i;
  line_t *l;

  if (side || thing->flags & MF_MISSILE)
    return 0;

  for (i = -1; (i = P_FindLineFromLineTag(line, i)) >= 0;)
    if ((l=lines+i) != line && l->backsector)
      {

        fixed_t pos = D_abs(line->dx) > D_abs(line->dy) ?
          FixedDiv(thing->x - line->v1->x, line->dx) :
          FixedDiv(thing->y - line->v1->y, line->dy) ;

        angle_t angle = (reverse ? pos = FRACUNIT-pos, 0 : ANG180) +
          R_PointToAngle2(0, 0, l->dx, l->dy) -
          R_PointToAngle2(0, 0, line->dx, line->dy);

        fixed_t x = l->v2->x - FixedMul(pos, l->dx);
        fixed_t y = l->v2->y - FixedMul(pos, l->dy);
        fixed_t s = finesine[angle>>ANGLETOFINESHIFT];
        fixed_t c = finecosine[angle>>ANGLETOFINESHIFT];

        int fudge = FUDGEFACTOR;

        player_t *player = thing->player && thing->player->mo == thing ?
          thing->player : NULL;

        int stepdown =
          l->frontsector->floorheight < l->backsector->floorheight;

        fixed_t z = thing->z - thing->floorz;

        int side = reverse || (player && stepdown);

        while (P_PointOnLineSide(x, y, l) != side && --fudge>=0)
          if (D_abs(l->dx) > D_abs(l->dy))
            y -= l->dx < 0 != side ? -1 : 1;
          else
            x += l->dy < 0 != side ? -1 : 1;


        if (!P_TeleportMove(thing, x, y, false))
          return 0;

        thing->z = z + sides[l->sidenum[stepdown]].sector->floorheight;
        thing->angle += angle;
        x = thing->momx;
        y = thing->momy;
        thing->momx = FixedMul(x, c) - FixedMul(y, s);
        thing->momy = FixedMul(y, c) + FixedMul(x, s);

        if (player)
          {

            fixed_t deltaviewheight = player->deltaviewheight;

            player->deltaviewheight = 0;

            P_CalcHeight(player);

            player->deltaviewheight = deltaviewheight;
          }

        return 1;
      }
  return 0;
}
