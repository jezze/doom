#ifndef __V_VIDEO__
#define __V_VIDEO__

#include "doomtype.h"
#include "doomdef.h"
#include "r_data.h"

#define CR_DEFAULT                      CR_RED
#define NUM_SCREENS                     6

extern const byte *colrngs[];

typedef enum
{

    CR_BRICK,
    CR_TAN,
    CR_GRAY,
    CR_GREEN,
    CR_BROWN,
    CR_GOLD,
    CR_RED,
    CR_BLUE,
    CR_ORANGE,
    CR_YELLOW,
    CR_BLUE2,
    CR_LIMIT

} crange_idx_e;

typedef struct
{

    byte *data;
    boolean not_on_heap;
    int width;
    int height;
    int byte_pitch;
    int short_pitch;
    int int_pitch;

} screeninfo_t;

typedef struct
{

    int x, y;

} fpoint_t;

typedef struct
{
    fpoint_t a, b;

} fline_t;

typedef enum
{

    VID_MODE8,
    VID_MODE15,
    VID_MODE16,
    VID_MODE32,
    VID_MODEMAX

} video_mode_t;

extern screeninfo_t screens[NUM_SCREENS];
extern int usegamma;

#define VID_NUMCOLORWEIGHTS 64
#define VID_COLORWEIGHTMASK (VID_NUMCOLORWEIGHTS-1)
#define VID_COLORWEIGHTBITS 6

extern unsigned short *V_Palette15;
extern unsigned short *V_Palette16;
extern unsigned int *V_Palette32;

#define VID_PAL15(color, weight) V_Palette15[ (color)*VID_NUMCOLORWEIGHTS + (weight) ]
#define VID_PAL16(color, weight) V_Palette16[ (color)*VID_NUMCOLORWEIGHTS + (weight) ]
#define VID_PAL32(color, weight) V_Palette32[ (color)*VID_NUMCOLORWEIGHTS + (weight) ]

void V_InitMode(video_mode_t mode);
video_mode_t V_GetMode(void);
int V_GetModePixelDepth(video_mode_t mode);
int V_GetNumPixelBits(void);
int V_GetPixelDepth(void);
void V_InitColorTranslation(void);
void V_Init (void);

typedef void (*V_CopyRect_f)(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, enum patch_translation_e flags);
extern V_CopyRect_f V_CopyRect;

typedef void (*V_FillRect_f)(int scrn, int x, int y, int width, int height, byte colour);
extern V_FillRect_f V_FillRect;

typedef void (*V_DrawNumPatch_f)(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags);
extern V_DrawNumPatch_f V_DrawNumPatch;

#define V_DrawNamePatch(x,y,s,n,t,f) V_DrawNumPatch(x,y,s,W_GetNumForName(n),t,f)
#define V_NamePatchWidth(name) R_NumPatchWidth(W_GetNumForName(name))
#define V_NamePatchHeight(name) R_NumPatchHeight(W_GetNumForName(name))

typedef void (*V_DrawBackground_f)(const char* flatname, int scrn);
extern V_DrawBackground_f V_DrawBackground;

void V_DestroyUnusedTrueColorPalettes(void);
void V_SetPalette(int pal);

typedef void (*V_PlotPixel_f)(int,int,int,byte);
extern V_PlotPixel_f V_PlotPixel;

typedef void (*V_DrawLine_f)(fline_t* fl, int color);
extern V_DrawLine_f V_DrawLine;
void V_AllocScreen(screeninfo_t *scrn);
void V_AllocScreens();
void V_FreeScreen(screeninfo_t *scrn);
void V_FreeScreens();

#endif
