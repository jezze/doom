#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_defs.h"
#include "r_draw.h"
#include "r_main.h"
#include "i_system.h"
#include "r_bsp.h"
#include "r_things.h"
#include "p_tick.h"
#include "i_system.h"
#include "p_tick.h"
#include "z_zone.h"

typedef struct
{
  short originx;
  short originy;
  short patch;
  short stepdir;
  short colormap;
} __attribute__((packed)) mappatch_t;

typedef struct
{
  char       name[8];
  char       pad2[4];
  short      width;
  short      height;
  char       pad[4];
  short      patchcount;
  mappatch_t patches[1];
} __attribute__((packed)) maptexture_t;

int firstcolormaplump, lastcolormaplump;

int       firstflat, lastflat, numflats;
int       firstspritelump, lastspritelump, numspritelumps;
int       numtextures;
texture_t **textures;
fixed_t   *textureheight;
int       *flattranslation;
int       *texturetranslation;

const byte *R_GetTextureColumn(const rpatch_t *texpatch, int col) {
  while (col < 0)
    col += texpatch->width;
  col &= texpatch->widthmask;
  
  return texpatch->columns[col].pixels;
}

static void R_InitTextures (void)
{
  const maptexture_t *mtexture;
  texture_t    *texture;
  const mappatch_t   *mpatch;
  texpatch_t   *patch;
  int  i, j;
  int         maptex_lump[2] = {-1, -1};
  const int  *maptex;
  const int  *maptex1, *maptex2;
  char name[9];
  int names_lump;
  const char *names;
  const char *name_p;
  int  *patchlookup;
  int  totalwidth;
  int  nummappatches;
  int  offset;
  int  maxoff, maxoff2;
  int  numtextures1, numtextures2;
  const int *directory;
  int  errors = 0;

  name[8] = 0;
  names = W_CacheLumpNum(names_lump = W_GetNumForName("PNAMES"));
  nummappatches = *((const int *)names);
  name_p = names+4;
  patchlookup = malloc(nummappatches*sizeof(*patchlookup));

  for (i=0 ; i<nummappatches ; i++)
    {
      strncpy (name,name_p+i*8, 8);
      patchlookup[i] = W_CheckNumForName(name, ns_global);
      if (patchlookup[i] == -1)
        {

          patchlookup[i] = W_CheckNumForName(name, ns_sprites);

        }
    }
  W_UnlockLumpNum(names_lump);

  maptex = maptex1 = W_CacheLumpNum(maptex_lump[0] = W_GetNumForName("TEXTURE1"));
  numtextures1 = *maptex;
  maxoff = W_LumpLength(maptex_lump[0]);
  directory = maptex+1;

  if (W_CheckNumForName("TEXTURE2", ns_global) != -1)
    {
      maptex2 = W_CacheLumpNum(maptex_lump[1] = W_GetNumForName("TEXTURE2"));
      numtextures2 = *maptex2;
      maxoff2 = W_LumpLength(maptex_lump[1]);
    }
  else
    {
      maptex2 = NULL;
      numtextures2 = 0;
      maxoff2 = 0;
    }
  numtextures = numtextures1 + numtextures2;
  textures = Z_Malloc(numtextures*sizeof*textures, PU_STATIC, 0);
  textureheight = Z_Malloc(numtextures*sizeof*textureheight, PU_STATIC, 0);
  totalwidth = 0;

  for (i=0 ; i<numtextures ; i++, directory++)
    {
      if (i == numtextures1)
        {

          maptex = maptex2;
          maxoff = maxoff2;
          directory = maptex+1;
        }

      offset = *directory;

      if (offset > maxoff)
        I_Error("R_InitTextures: Bad texture directory");

      mtexture = (const maptexture_t *) ( (const byte *)maptex + offset);

      texture = textures[i] =
        Z_Malloc(sizeof(texture_t) +
                 sizeof(texpatch_t)*(mtexture->patchcount -1),
                 PU_STATIC, 0);

      texture->width = mtexture->width;
      texture->height = mtexture->height;
      texture->patchcount = mtexture->patchcount;

      {
        int j;
        for(j=0;j<sizeof(texture->name);j++)
          texture->name[j]=mtexture->name[j];
      }

      mpatch = mtexture->patches;
      patch = texture->patches;

      for (j=0 ; j<texture->patchcount ; j++, mpatch++, patch++)
        {
          patch->originx = mpatch->originx;
          patch->originy = mpatch->originy;
          patch->patch = patchlookup[mpatch->patch];
          if (patch->patch == -1)
            {

              I_Error("R_InitTextures: Missing patch %d in texture %.8s", mpatch->patch), texture->name;
              ++errors;
            }
        }

      for (j=1; j*2 <= texture->width; j<<=1)
        ;
      texture->widthmask = j-1;
      textureheight[i] = texture->height<<FRACBITS;

      totalwidth += texture->width;
    }

  free(patchlookup);

  for (i=0; i<2; i++)
    if (maptex_lump[i] != -1)
      W_UnlockLumpNum(maptex_lump[i]);

  if (errors)
    I_Error("R_InitTextures: %d errors", errors);

  texturetranslation =
    Z_Malloc((numtextures+1)*sizeof*texturetranslation, PU_STATIC, 0);

  for (i=0 ; i<numtextures ; i++)
    texturetranslation[i] = i;


  for (i = 0; i<numtextures; i++)
    textures[i]->index = -1;
  while (--i >= 0)
    {
      int j = W_LumpNameHash(textures[i]->name) % (unsigned) numtextures;
      textures[i]->next = textures[j]->index;
      textures[j]->index = i;
    }
}

static void R_InitFlats(void)
{
  int i;

  firstflat = W_GetNumForName("F_START") + 1;
  lastflat  = W_GetNumForName("F_END") - 1;
  numflats  = lastflat - firstflat + 1;
  flattranslation =
    Z_Malloc((numflats+1)*sizeof(*flattranslation), PU_STATIC, 0);

  for (i=0 ; i<numflats ; i++)
    flattranslation[i] = i;
}

static void R_InitSpriteLumps(void)
{
  firstspritelump = W_GetNumForName("S_START") + 1;
  lastspritelump = W_GetNumForName("S_END") - 1;
  numspritelumps = lastspritelump - firstspritelump + 1;
}

static void R_InitColormaps(void)
{
  int i;
  numcolormaps = 1;
  colormaps = Z_Malloc(sizeof(*colormaps) * numcolormaps, PU_STATIC, 0);
  colormaps[0] = (const lighttable_t *)W_CacheLumpName("COLORMAP");
}

int R_ColormapNumForName(const char *name)
{
  return 0;
}

static inline int between(int l,int u,int x)
{ return (l > x ? l : x > u ? u : x); }

const lighttable_t* R_ColourMap(int lightlevel, fixed_t spryscale)
{
  if (fixedcolormap) return fixedcolormap;
  else {
    if (curline)
      if (curline->v1->y == curline->v2->y)
        lightlevel -= 1 << LIGHTSEGSHIFT;
      else
        if (curline->v1->x == curline->v2->x)
          lightlevel += 1 << LIGHTSEGSHIFT;

    lightlevel += extralight << LIGHTSEGSHIFT;

    return fullcolormap + between(0,NUMCOLORMAPS-1,
          ((256-lightlevel)*2*NUMCOLORMAPS/256) - 4
          - (FixedMul(spryscale,pspriteiscale)/2 >> LIGHTSCALESHIFT)
          )*256;
  }
}

void R_InitData(void)
{
  R_InitTextures();
  R_InitFlats();
  R_InitSpriteLumps();
  R_InitColormaps();
}

int R_FlatNumForName(const char *name)
{
  int i = W_CheckNumForName(name, ns_flats);
  if (i == -1)
    I_Error("R_FlatNumForName: %.8s not found", name);
  return i - firstflat;
}

int R_CheckTextureNumForName(const char *name)
{
  int i = 0;
  if (*name != '-')
    {
      i = textures[W_LumpNameHash(name) % (unsigned) numtextures]->index;
      while (i >= 0 && strncasecmp(textures[i]->name,name,8))
        i = textures[i]->next;
    }
  return i;
}

int R_TextureNumForName(const char *name)
{
  int i = R_CheckTextureNumForName(name);
  if (i == -1)
    I_Error("R_TextureNumForName: %.8s not found", name);
  return i;
}

int R_SafeTextureNumForName(const char *name, int snum)
{
  int i = R_CheckTextureNumForName(name);
  if (i == -1) {
    i = 0;
    I_Error("R_SafeTextureNumForName: Bad texture '%s' in sidedef %d\n",name,snum);
  }
  return i;
}

void R_SetPatchNum(patchnum_t *patchnum, const char *name)
{
  const rpatch_t *patch = R_CachePatchName(name);
  patchnum->width = patch->width;
  patchnum->height = patch->height;
  patchnum->leftoffset = patch->leftoffset;
  patchnum->topoffset = patch->topoffset;
  patchnum->lumpnum = W_GetNumForName(name);
  R_UnlockPatchName(name);
}
