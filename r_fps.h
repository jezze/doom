#ifndef __R_FPS__
#define __R_FPS__

#include "doomstat.h"

typedef struct
{

    fixed_t viewx;
    fixed_t viewy;
    fixed_t viewz;
    angle_t viewangle;
    angle_t viewpitch;

} view_vars_t;

void R_InterpolateView(player_t *player, fixed_t frac);

#endif
