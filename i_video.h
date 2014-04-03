#ifndef __I_VIDEO__
#define __I_VIDEO__

void I_PreInitGraphics(void);
void I_CalculateRes(unsigned int width, unsigned int height);
void I_InitGraphics(void);
void I_SetPalette(int pal);
void I_FinishUpdate(void);
void I_StartTic(void);

extern int SCREENWIDTH;
extern int SCREENHEIGHT;
extern int SCREENPITCH;

#endif
