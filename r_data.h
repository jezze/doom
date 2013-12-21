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

    char name[8];
    int next, index;
    unsigned widthmask;
    short width, height;
    short patchcount;
    texpatch_t patches[1];

} texture_t;

extern int numtextures;
extern texture_t **textures;

const byte *R_GetTextureColumn(const rpatch_t *texpatch, int col);
void R_InitData(void);
int R_FlatNumForName(const char *name);
int R_TextureNumForName(const char *name);
int R_SafeTextureNumForName(const char *name, int snum);
int R_CheckTextureNumForName(const char *name);
int R_ColormapNumForName(const char *name);
const lighttable_t* R_ColourMap(int lightlevel, fixed_t spryscale);
void R_SetPatchNum(patchnum_t *patchnum, const char *name);

#endif
