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

typedef struct
{

    unsigned int start;
    unsigned int next;
    unsigned int step;
    fixed_t frac;
    float msec;

} tic_vars_t;

extern tic_vars_t tic_vars;

void R_InitInterpolation(void);
void R_InterpolateView(player_t *player, fixed_t frac);

#endif
