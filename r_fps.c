#include "doomstat.h"
#include "r_defs.h"
#include "r_state.h"
#include "p_spec.h"
#include "r_fps.h"

tic_vars_t tic_vars;
extern int realtic_clock_rate;
typedef fixed_t fixed2_t[2];

void R_InitInterpolation(void)
{

    tic_vars.msec = realtic_clock_rate * TICRATE / 100000.0f;

}


void R_InterpolateView(player_t *player, fixed_t frac)
{

    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle;

}

