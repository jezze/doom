#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_draw.h"
#include "r_things.h"
#include "z_zone.h"
#include "i_system.h"

#define MINZ        (FRACUNIT*4)
#define BASEYCENTER 100

typedef struct {
  int x1;
  int x2;
  int column;
  int topclip;
  int bottomclip;
} maskdraw_t;

fixed_t pspritescale;
fixed_t pspriteiscale;
fixed_t pspriteyscale;

int negonearray[MAX_SCREENWIDTH];
int screenheightarray[MAX_SCREENWIDTH];

spritedef_t *sprites;
static int numsprites;

#define MAX_SPRITE_FRAMES 29

static spriteframe_t sprtemp[MAX_SPRITE_FRAMES];
static int maxframe;

static void R_InstallSpriteLump(int lump, unsigned frame, unsigned rotation, boolean flipped)
{
  if (frame >= MAX_SPRITE_FRAMES || rotation > 8)
    I_Error("R_InstallSpriteLump: Bad frame characters in lump %i", lump);

  if ((int) frame > maxframe)
    maxframe = frame;

  if (rotation == 0)
    {
      int r;
      for (r=0 ; r<8 ; r++)
        if (sprtemp[frame].lump[r]==-1)
          {
            sprtemp[frame].lump[r] = lump - firstspritelump;
            sprtemp[frame].flip[r] = (byte) flipped;
            sprtemp[frame].rotate = false;
          }
      return;
    }

  if (sprtemp[frame].lump[--rotation] == -1)
    {
      sprtemp[frame].lump[rotation] = lump - firstspritelump;
      sprtemp[frame].flip[rotation] = (byte) flipped;
      sprtemp[frame].rotate = true;
    }
}

#define R_SpriteNameHash(s) ((unsigned)((s)[0]-((s)[1]*3-(s)[3]*2-(s)[2])*2))

static void R_InitSpriteDefs(const char * const * namelist)
{
  size_t numentries = lastspritelump-firstspritelump+1;
  struct { int index, next; } *hash;
  int i;

  if (!numentries || !*namelist)
    return;

  for (i=0; namelist[i]; i++)
    ;

  numsprites = i;
  sprites = Z_Malloc(numsprites *sizeof(*sprites), PU_STATIC, NULL);
  hash = malloc(sizeof(*hash)*numentries);

  for (i=0; (size_t)i<numentries; i++)
    hash[i].index = -1;

  for (i=0; (size_t)i<numentries; i++)
    {
      int j = R_SpriteNameHash(lumpinfo[i+firstspritelump].name) % numentries;
      hash[i].next = hash[j].index;
      hash[j].index = i;
    }

  for (i=0 ; i<numsprites ; i++)
    {
      const char *spritename = namelist[i];
      int j = hash[R_SpriteNameHash(spritename) % numentries].index;

      if (j >= 0)
        {
          memset(sprtemp, -1, sizeof(sprtemp));
          maxframe = -1;
          do
            {
              register lumpinfo_t *lump = lumpinfo + j + firstspritelump;

              if (!((lump->name[0] ^ spritename[0]) |
                    (lump->name[1] ^ spritename[1]) |
                    (lump->name[2] ^ spritename[2]) |
                    (lump->name[3] ^ spritename[3])))
                {
                  R_InstallSpriteLump(j+firstspritelump,
                                      lump->name[4] - 'A',
                                      lump->name[5] - '0',
                                      false);
                  if (lump->name[6])
                    R_InstallSpriteLump(j+firstspritelump,
                                        lump->name[6] - 'A',
                                        lump->name[7] - '0',
                                        true);
                }
            }
          while ((j = hash[j].next) >= 0);

          if ((sprites[i].numframes = ++maxframe))
            {
              int frame;
              for (frame = 0; frame < maxframe; frame++)
                switch ((int) sprtemp[frame].rotate)
                  {
                  case -1:
                    I_Error ("R_InitSprites: No patches found for %.8s frame %c", namelist[i], frame+'A');
                    break;

                  case 0:

                    break;

                  case 1:

                    {
                      int rotation;
                      for (rotation=0 ; rotation<8 ; rotation++)
                        if (sprtemp[frame].lump[rotation] == -1)
                          I_Error ("R_InitSprites: Sprite %.8s frame %c "
                                   "is missing rotations",
                                   namelist[i], frame+'A');
                      break;
                    }
                  }

              sprites[i].spriteframes =
                Z_Malloc (maxframe * sizeof(spriteframe_t), PU_STATIC, NULL);
              memcpy (sprites[i].spriteframes, sprtemp,
                      maxframe*sizeof(spriteframe_t));
            }
        }
    }
  free(hash);
}

static vissprite_t *vissprites, **vissprite_ptrs;
static size_t num_vissprite, num_vissprite_alloc, num_vissprite_ptrs;

void R_InitSprites(const char * const *namelist)
{
  int i;
  for (i=0; i<MAX_SCREENWIDTH; i++)
    negonearray[i] = -1;
  R_InitSpriteDefs(namelist);
}

void R_ClearSprites (void)
{
  num_vissprite = 0;
}

static vissprite_t *R_NewVisSprite(void)
{
  if (num_vissprite >= num_vissprite_alloc)
    {
      size_t num_vissprite_alloc_prev = num_vissprite_alloc;

      num_vissprite_alloc = num_vissprite_alloc ? num_vissprite_alloc*2 : 128;
      vissprites = realloc(vissprites,num_vissprite_alloc*sizeof(*vissprites));
      memset(vissprites + num_vissprite_alloc_prev, 0, (num_vissprite_alloc - num_vissprite_alloc_prev)*sizeof(*vissprites));
    }
 return vissprites + num_vissprite++;
}

int   *mfloorclip;
int   *mceilingclip;
fixed_t spryscale;
fixed_t sprtopscreen;

void R_DrawMaskedColumn(
  const rpatch_t *patch,
  R_DrawColumn_f colfunc,
  draw_column_vars_t *dcvars,
  const rcolumn_t *column,
  const rcolumn_t *prevcolumn,
  const rcolumn_t *nextcolumn
)
{
  int     i;
  int     topscreen;
  int     bottomscreen;
  fixed_t basetexturemid = dcvars->texturemid;

  dcvars->texheight = patch->height;
  for (i=0; i<column->numPosts; i++) {
      const rpost_t *post = &column->posts[i];

      topscreen = sprtopscreen + spryscale*post->topdelta;
      bottomscreen = topscreen + spryscale*post->length;

      dcvars->yl = (topscreen+FRACUNIT-1)>>FRACBITS;
      dcvars->yh = (bottomscreen-1)>>FRACBITS;

      if (dcvars->yh >= mfloorclip[dcvars->x])
        dcvars->yh = mfloorclip[dcvars->x]-1;

      if (dcvars->yl <= mceilingclip[dcvars->x])
        dcvars->yl = mceilingclip[dcvars->x]+1;

      if (dcvars->yl <= dcvars->yh && dcvars->yh < viewheight)
        {
          dcvars->source = column->pixels + post->topdelta;
          dcvars->prevsource = prevcolumn->pixels + post->topdelta;
          dcvars->nextsource = nextcolumn->pixels + post->topdelta;

          dcvars->texturemid = basetexturemid - (post->topdelta<<FRACBITS);

          dcvars->edgeslope = post->slope;
          dcvars->drawingmasked = 1;
          colfunc (dcvars);
          dcvars->drawingmasked = 0;
        }
    }
  dcvars->texturemid = basetexturemid;
}

static void R_DrawVisSprite(vissprite_t *vis, int x1, int x2)
{
  int      texturecolumn;
  fixed_t  frac;
  const rpatch_t *patch = R_CachePatchNum(vis->patch+firstspritelump);
  R_DrawColumn_f colfunc;
  draw_column_vars_t dcvars;
  enum draw_filter_type_e filter;
  enum draw_filter_type_e filterz;

  R_SetDefaultDrawColumnVars(&dcvars);
  if (vis->isplayersprite) {
    dcvars.edgetype = drawvars.patch_edges;
    filter = drawvars.filterpatch;
    filterz = RDRAW_FILTER_POINT;
  } else {
    dcvars.edgetype = drawvars.sprite_edges;
    filter = drawvars.filtersprite;
    filterz = drawvars.filterz;
  }

  dcvars.colormap = vis->colormap;
  dcvars.nextcolormap = dcvars.colormap;

  if (!dcvars.colormap)
    colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_FUZZ, filter, filterz);
  else
    if (vis->mobjflags & MF_TRANSLATION)
      {
        colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, filter, filterz);
        dcvars.translation = translationtables - 256 +
          ((vis->mobjflags & MF_TRANSLATION) >> (MF_TRANSSHIFT-8) );
      }
    else
        colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, filter, filterz);

  dcvars.iscale = FixedDiv (FRACUNIT, vis->scale);
  dcvars.texturemid = vis->texturemid;
  frac = vis->startfrac;
  if (filter == RDRAW_FILTER_LINEAR)
    frac -= (FRACUNIT>>1);
  spryscale = vis->scale;
  sprtopscreen = centeryfrac - FixedMul(dcvars.texturemid,spryscale);

  for (dcvars.x=vis->x1 ; dcvars.x<=vis->x2 ; dcvars.x++, frac += vis->xiscale)
    {
      texturecolumn = frac>>FRACBITS;
      dcvars.texu = frac;

      R_DrawMaskedColumn(
        patch,
        colfunc,
        &dcvars,
        R_GetPatchColumnClamped(patch, texturecolumn),
        R_GetPatchColumnClamped(patch, texturecolumn-1),
        R_GetPatchColumnClamped(patch, texturecolumn+1)
      );
    }
  R_UnlockPatchNum(vis->patch+firstspritelump);
}

static void R_ProjectSprite (mobj_t* thing, int lightlevel)
{
  fixed_t   gzt;
  fixed_t   tx;
  fixed_t   xscale;
  int       x1;
  int       x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int       lump;
  boolean   flip;
  vissprite_t *vis;
  fixed_t   iscale;
  int heightsec;

  fixed_t tr_x, tr_y;
  fixed_t fx, fy, fz;
  fixed_t gxt, gyt;
  fixed_t tz;
  int width;

  fx = thing->x;
  fy = thing->y;
  fz = thing->z;
  tr_x = fx - viewx;
  tr_y = fy - viewy;

  gxt = FixedMul(tr_x,viewcos);
  gyt = -FixedMul(tr_y,viewsin);

  tz = gxt-gyt;

  if (tz < MINZ)
    return;

  xscale = FixedDiv(projection, tz);

  gxt = -FixedMul(tr_x,viewsin);
  gyt = FixedMul(tr_y,viewcos);
  tx = -(gyt+gxt);

  if (D_abs(tx)>(tz<<2))
    return;

  sprdef = &sprites[thing->sprite];

  if (!sprdef->spriteframes)
    I_Error ("R_ProjectSprite: Missing spriteframes %i : %i", thing->sprite, thing->frame);

  sprframe = &sprdef->spriteframes[thing->frame & FF_FRAMEMASK];

  if (sprframe->rotate)
    {
      angle_t ang = R_PointToAngle(fx, fy);
      unsigned rot = (ang-thing->angle+(unsigned)(ANG45/2)*9)>>29;
      lump = sprframe->lump[rot];
      flip = (boolean) sprframe->flip[rot];
    }
  else
    {
      lump = sprframe->lump[0];
      flip = (boolean) sprframe->flip[0];
    }

  {
    const rpatch_t* patch = R_CachePatchNum(lump+firstspritelump);

    if (flip) {
      tx -= (patch->width - patch->leftoffset) << FRACBITS;
    } else {
      tx -= patch->leftoffset << FRACBITS;
    }
    x1 = (centerxfrac + FixedMul(tx,xscale)) >> FRACBITS;

    tx += patch->width<<FRACBITS;
    x2 = ((centerxfrac + FixedMul (tx,xscale) ) >> FRACBITS) - 1;

    gzt = fz + (patch->topoffset << FRACBITS);
    width = patch->width;
    R_UnlockPatchNum(lump+firstspritelump);
  }

  if (x1 > viewwidth || x2 < 0)
    return;

  if (fz  > viewz + FixedDiv(viewheightfrac, xscale) ||
      gzt < viewz - FixedDiv(viewheightfrac-viewheight, xscale))
    return;

  heightsec = thing->subsector->sector->heightsec;

  if (heightsec != -1)
    {
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if (phs != -1 && viewz < sectors[phs].floorheight ?
          fz >= sectors[heightsec].floorheight :
          gzt < sectors[heightsec].floorheight)
        return;
      if (phs != -1 && viewz > sectors[phs].ceilingheight ?
          gzt < sectors[heightsec].ceilingheight &&
          viewz >= sectors[heightsec].ceilingheight :
          fz >= sectors[heightsec].ceilingheight)
        return;
    }

  vis = R_NewVisSprite ();

  vis->heightsec = heightsec;

  vis->mobjflags = thing->flags;
  vis->scale = FixedDiv(projectiony, tz);
  vis->gx = fx;
  vis->gy = fy;
  vis->gz = fz;
  vis->gzt = gzt;
  vis->texturemid = vis->gzt - viewz;
  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
  iscale = FixedDiv (FRACUNIT, xscale);

  if (flip)
    {
      vis->startfrac = (width<<FRACBITS)-1;
      vis->xiscale = -iscale;
    }
  else
    {
      vis->startfrac = 0;
      vis->xiscale = iscale;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);
  vis->patch = lump;

  if (thing->flags & MF_SHADOW)
      vis->colormap = NULL;
  else if (fixedcolormap)
    vis->colormap = fixedcolormap;
  else if (thing->frame & FF_FULLBRIGHT)
    vis->colormap = fullcolormap;
  else
    {
      vis->colormap = R_ColourMap(lightlevel,xscale);
    }
}

void R_AddSprites(subsector_t* subsec, int lightlevel)
{
  sector_t* sec=subsec->sector;
  mobj_t *thing;

  if (sec->validcount == validcount)
    return;

  sec->validcount = validcount;

  for (thing = sec->thinglist; thing; thing = thing->snext)
    R_ProjectSprite(thing, lightlevel);
}

static void R_DrawPSprite (pspdef_t *psp, int lightlevel)
{
  int           x1, x2;
  spritedef_t   *sprdef;
  spriteframe_t *sprframe;
  int           lump;
  boolean       flip;
  vissprite_t   *vis;
  vissprite_t   avis;
  int           width;
  fixed_t       topoffset;

  avis.isplayersprite = true;
  sprdef = &sprites[psp->state->sprite];
  sprframe = &sprdef->spriteframes[psp->state->frame & FF_FRAMEMASK];
  lump = sprframe->lump[0];
  flip = (boolean) sprframe->flip[0];

  {
    const rpatch_t* patch = R_CachePatchNum(lump+firstspritelump);
    fixed_t       tx;
    tx = psp->sx-160*FRACUNIT;

    tx -= patch->leftoffset<<FRACBITS;
    x1 = (centerxfrac + FixedMul (tx,pspritescale))>>FRACBITS;

    tx += patch->width<<FRACBITS;
    x2 = ((centerxfrac + FixedMul (tx, pspritescale) ) >>FRACBITS) - 1;

    width = patch->width;
    topoffset = patch->topoffset<<FRACBITS;
    R_UnlockPatchNum(lump+firstspritelump);
  }

  if (x2 < 0 || x1 > viewwidth)
    return;

  vis = &avis;
  vis->mobjflags = 0;
  vis->texturemid = (BASEYCENTER<<FRACBITS) - (psp->sy-topoffset);
  vis->x1 = x1 < 0 ? 0 : x1;
  vis->x2 = x2 >= viewwidth ? viewwidth-1 : x2;
  vis->scale = pspriteyscale;

  if (flip)
    {
      vis->xiscale = -pspriteiscale;
      vis->startfrac = (width<<FRACBITS)-1;
    }
  else
    {
      vis->xiscale = pspriteiscale;
      vis->startfrac = 0;
    }

  if (vis->x1 > x1)
    vis->startfrac += vis->xiscale*(vis->x1-x1);

  vis->patch = lump;

  if (viewplayer->powers[pw_invisibility] > 4*32
      || viewplayer->powers[pw_invisibility] & 8)
    vis->colormap = NULL;
  else if (fixedcolormap)
    vis->colormap = fixedcolormap;
  else if (psp->state->frame & FF_FULLBRIGHT)
    vis->colormap = fullcolormap;
  else
    vis->colormap = R_ColourMap(lightlevel, FixedMul(pspritescale, 0x2b000));

    R_DrawVisSprite(vis, vis->x1, vis->x2);
}

void R_DrawPlayerSprites(void)
{
  int i, lightlevel = viewplayer->mo->subsector->sector->lightlevel;
  pspdef_t *psp;

  mfloorclip = screenheightarray;
  mceilingclip = negonearray;

  for (i=0, psp=viewplayer->psprites; i<NUMPSPRITES; i++,psp++)
    if (psp->state)
      R_DrawPSprite (psp, lightlevel);
}

#define bcopyp(d, s, n) memcpy(d, s, (n) * sizeof(void *))

static void msort(vissprite_t **s, vissprite_t **t, int n)
{
  if (n >= 16)
    {
      int n1 = n/2, n2 = n - n1;
      vissprite_t **s1 = s, **s2 = s + n1, **d = t;

      msort(s1, t, n1);
      msort(s2, t, n2);

      while ((*s1)->scale > (*s2)->scale ?
             (*d++ = *s1++, --n1) : (*d++ = *s2++, --n2));

      if (n2)
        bcopyp(d, s2, n2);
      else
        bcopyp(d, s1, n1);

      bcopyp(s, t, n);
    }
  else
    {
      int i;
      for (i = 1; i < n; i++)
        {
          vissprite_t *temp = s[i];
          if (s[i-1]->scale < temp->scale)
            {
              int j = i;
              while ((s[j] = s[j-1])->scale < temp->scale && --j);
              s[j] = temp;
            }
        }
    }
}

void R_SortVisSprites (void)
{
  if (num_vissprite)
    {
      int i = num_vissprite;

      if (num_vissprite_ptrs < num_vissprite*2)
        {
          free(vissprite_ptrs);
          vissprite_ptrs = malloc((num_vissprite_ptrs = num_vissprite_alloc*2)
                                  * sizeof *vissprite_ptrs);
        }

      while (--i>=0)
        vissprite_ptrs[i] = vissprites+i;

      msort(vissprite_ptrs, vissprite_ptrs + num_vissprite, num_vissprite);
    }
}

static void R_DrawSprite (vissprite_t* spr)
{
  drawseg_t *ds;
  int     clipbot[MAX_SCREENWIDTH];
  int     cliptop[MAX_SCREENWIDTH];
  int     x;
  int     r1;
  int     r2;
  fixed_t scale;
  fixed_t lowscale;

  for (x = spr->x1 ; x<=spr->x2 ; x++)
    clipbot[x] = cliptop[x] = -2;

  for (ds=ds_p ; ds-- > drawsegs ; )
    {
      if (ds->x1 > spr->x2 || ds->x2 < spr->x1 ||
          (!ds->silhouette && !ds->maskedtexturecol))
        continue;

      r1 = ds->x1 < spr->x1 ? spr->x1 : ds->x1;
      r2 = ds->x2 > spr->x2 ? spr->x2 : ds->x2;

      if (ds->scale1 > ds->scale2)
        {
          lowscale = ds->scale2;
          scale = ds->scale1;
        }
      else
        {
          lowscale = ds->scale1;
          scale = ds->scale2;
        }

      if (scale < spr->scale || (lowscale < spr->scale &&
                    !R_PointOnSegSide (spr->gx, spr->gy, ds->curline)))
        {
          if (ds->maskedtexturecol)
            R_RenderMaskedSegRange(ds, r1, r2);
          continue;
        }

      if (ds->silhouette&SIL_BOTTOM && spr->gz < ds->bsilheight)
        for (x=r1 ; x<=r2 ; x++)
          if (clipbot[x] == -2)
            clipbot[x] = ds->sprbottomclip[x];

      if (ds->silhouette&SIL_TOP && spr->gzt > ds->tsilheight)
        for (x=r1 ; x<=r2 ; x++)
          if (cliptop[x] == -2)
            cliptop[x] = ds->sprtopclip[x];
    }

  if (spr->heightsec != -1)
    {
      fixed_t h,mh;
      int phs = viewplayer->mo->subsector->sector->heightsec;
      if ((mh = sectors[spr->heightsec].floorheight) > spr->gz &&
          (h = centeryfrac - FixedMul(mh-=viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight) {
        if (mh <= 0 || (phs != -1 && viewz > sectors[phs].floorheight))
          {
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else
    if (phs != -1 && viewz <= sectors[phs].floorheight)
      for (x=spr->x1 ; x<=spr->x2 ; x++)
        if (cliptop[x] == -2 || h > cliptop[x])
    cliptop[x] = h;
      }

      if ((mh = sectors[spr->heightsec].ceilingheight) < spr->gzt &&
          (h = centeryfrac - FixedMul(mh-viewz, spr->scale)) >= 0 &&
          (h >>= FRACBITS) < viewheight) {
        if (phs != -1 && viewz >= sectors[phs].ceilingheight)
          {
            for (x=spr->x1 ; x<=spr->x2 ; x++)
              if (clipbot[x] == -2 || h < clipbot[x])
                clipbot[x] = h;
          }
        else
          for (x=spr->x1 ; x<=spr->x2 ; x++)
            if (cliptop[x] == -2 || h > cliptop[x])
              cliptop[x] = h;
      }
    }

  for (x = spr->x1 ; x<=spr->x2 ; x++) {
    if (clipbot[x] == -2)
      clipbot[x] = viewheight;

    if (cliptop[x] == -2)
      cliptop[x] = -1;
  }

  mfloorclip = clipbot;
  mceilingclip = cliptop;
  R_DrawVisSprite (spr, spr->x1, spr->x2);
}

void R_DrawMasked(void)
{
  int i;
  drawseg_t *ds;

  R_SortVisSprites();

  rendered_vissprites = num_vissprite;
  for (i = num_vissprite ;--i>=0; )
    R_DrawSprite(vissprite_ptrs[i]);

  for (ds=ds_p ; ds-- > drawsegs ; )
    if (ds->maskedtexturecol)
      R_RenderMaskedSegRange(ds, ds->x1, ds->x2);

  R_DrawPlayerSprites ();
}
