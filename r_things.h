#ifndef __R_THINGS__
#define __R_THINGS__

#include "r_draw.h"

extern int negonearray[MAX_SCREENWIDTH];
extern int screenheightarray[MAX_SCREENWIDTH];

extern int     *mfloorclip;
extern int     *mceilingclip;
extern fixed_t spryscale;
extern fixed_t sprtopscreen;
extern fixed_t pspritescale;
extern fixed_t pspriteiscale;
extern fixed_t pspriteyscale;

void R_DrawMaskedColumn(const rpatch_t *patch, R_DrawColumn_f colfunc, draw_column_vars_t *dcvars, const rcolumn_t *column, const rcolumn_t *prevcolumn, const rcolumn_t *nextcolumn);
void R_SortVisSprites(void);
void R_AddSprites(subsector_t* subsec, int lightlevel);
void R_DrawPlayerSprites(void);
void R_InitSprites(const char * const * namelist);
void R_ClearSprites(void);
void R_DrawMasked(void);

#endif
