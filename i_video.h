#ifndef __I_VIDEO__
#define __I_VIDEO__

#include "doomtype.h"
#include "v_video.h"

void I_PreInitGraphics(void);
void I_CalculateRes(unsigned int width, unsigned int height);
void I_SetRes(void);
void I_InitGraphics(void);
void I_UpdateVideoMode(void);
void I_ShutdownGraphics(void);
void I_SetPalette(int pal);
void I_UpdateNoBlit(void);
void I_FinishUpdate(void);
void I_StartTic(void);
void I_StartFrame(void);

extern int use_fullscreen;

#endif
