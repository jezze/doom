#include "doomstat.h"
#include "r_defs.h"
#include "r_state.h"
#include "p_spec.h"
#include "r_fps.h"

typedef fixed_t fixed2_t[2];

void R_InterpolateView(player_t *player, fixed_t frac)
{

    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle;

}

