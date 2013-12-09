#ifndef __R_FPS__
#define __R_FPS__

#include "doomstat.h"

extern int movement_smooth;

typedef struct {
  fixed_t viewx;
  fixed_t viewy;
  fixed_t viewz;
  angle_t viewangle;
  angle_t viewpitch;
} view_vars_t;

extern view_vars_t original_view_vars;

typedef struct {
  unsigned int start;
  unsigned int next;
  unsigned int step;
  fixed_t frac;
  float msec;
} tic_vars_t;

extern tic_vars_t tic_vars;
extern boolean WasRenderedInTryRunTics;

void R_InitInterpolation(void);
void R_InterpolateView(player_t *player, fixed_t frac);
void R_ResetViewInterpolation();
void R_UpdateInterpolations();
void R_StopAllInterpolations(void);
void R_DoInterpolations(fixed_t smoothratio);
void R_RestoreInterpolations();
void R_ActivateSectorInterpolations();
void R_ActivateThinkerInterpolations(thinker_t *th);
void R_StopInterpolationIfNeeded(thinker_t *th);

#endif
