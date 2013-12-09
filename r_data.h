#ifndef __R_DATA__
#define __R_DATA__

#include "r_defs.h"
#include "r_state.h"
#include "r_patch.h"

typedef struct
{
  int originx, originy;
  int patch;
} texpatch_t;

typedef struct
{
  char  name[8];
  int   next, index;

  unsigned  widthmask;

  short width, height;
  short patchcount;
  texpatch_t patches[1];
} texture_t;

extern int numtextures;
extern texture_t **textures;

const byte *R_GetTextureColumn(const rpatch_t *texpatch, int col);
void R_InitData (void);
void R_PrecacheLevel (void);
int R_FlatNumForName (const char* name);

#define NO_TEXTURE 0
int R_TextureNumForName (const char *name);
int R_SafeTextureNumForName (const char *name, int snum);
int R_CheckTextureNumForName (const char *name);

void R_InitTranMap(int);
int R_ColormapNumForName(const char *name);
/* cph 2001/11/17 - new func to do lighting calcs and get suitable colour map */
const lighttable_t* R_ColourMap(int lightlevel, fixed_t spryscale);

extern const byte *main_tranmap, *tranmap;

/* Proff - Added for OpenGL - cph - const char* param */
void R_SetPatchNum(patchnum_t *patchnum, const char *name);

#endif
