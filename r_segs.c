#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "r_bsp.h"
#include "r_segs.h"
#include "r_plane.h"
#include "r_things.h"
#include "r_draw.h"
#include "w_wad.h"
#include "v_video.h"

static boolean  segtextured;
static boolean  markfloor;
static boolean  markceiling;
static boolean  maskedtexture;
static int      toptexture;
static int      bottomtexture;
static int      midtexture;
static fixed_t  toptexheight, midtexheight, bottomtexheight;
angle_t         rw_normalangle;
int             rw_angle1;
fixed_t         rw_distance;
static int      rw_x;
static int      rw_stopx;
static angle_t  rw_centerangle;
static fixed_t  rw_offset;
static fixed_t  rw_scale;
static fixed_t  rw_scalestep;
static fixed_t  rw_midtexturemid;
static fixed_t  rw_toptexturemid;
static fixed_t  rw_bottomtexturemid;
static int      rw_lightlevel;
static int      worldtop;
static int      worldbottom;
static int      worldhigh;
static int      worldlow;
static fixed_t  pixhigh;
static fixed_t  pixlow;
static fixed_t  pixhighstep;
static fixed_t  pixlowstep;
static fixed_t  topfrac;
static fixed_t  topstep;
static fixed_t  bottomfrac;
static fixed_t  bottomstep;
static int      *maskedtexturecol;

static fixed_t R_ScaleFromGlobalAngle(angle_t visangle)
{
  int     anglea = ANG90 + (visangle-viewangle);
  int     angleb = ANG90 + (visangle-rw_normalangle);
  int     den = FixedMul(rw_distance, finesine[anglea>>ANGLETOFINESHIFT]);
  fixed_t num = FixedMul(projectiony, finesine[angleb>>ANGLETOFINESHIFT]);
  return den > num>>16 ? (num = FixedDiv(num, den)) > 64*FRACUNIT ?
    64*FRACUNIT : num < 256 ? 256 : num : 64*FRACUNIT;
}

void R_RenderMaskedSegRange(drawseg_t *ds, int x1, int x2)
{
  int      texnum;
  sector_t tempsec;
  const rpatch_t *patch;
  R_DrawColumn_f colfunc;
  draw_column_vars_t dcvars;
  angle_t angle;

  R_SetDefaultDrawColumnVars(&dcvars);

  curline = ds->curline;
  colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterwall, drawvars.filterz);

  frontsector = curline->frontsector;
  backsector = curline->backsector;
  texnum = curline->sidedef->midtexture;
  texnum = texturetranslation[texnum];

  rw_lightlevel = R_FakeFlat(frontsector, &tempsec, NULL, NULL, false) ->lightlevel;

  maskedtexturecol = ds->maskedtexturecol;

  rw_scalestep = ds->scalestep;
  spryscale = ds->scale1 + (x1 - ds->x1)*rw_scalestep;
  mfloorclip = ds->sprbottomclip;
  mceilingclip = ds->sprtopclip;

  if (curline->linedef->flags & ML_DONTPEGBOTTOM)
    {
      dcvars.texturemid = frontsector->floorheight > backsector->floorheight
        ? frontsector->floorheight : backsector->floorheight;
      dcvars.texturemid = dcvars.texturemid + textureheight[texnum] - viewz;
    }
  else
    {
      dcvars.texturemid =frontsector->ceilingheight<backsector->ceilingheight
        ? frontsector->ceilingheight : backsector->ceilingheight;
      dcvars.texturemid = dcvars.texturemid - viewz;
    }

  dcvars.texturemid += curline->sidedef->rowoffset;

  if (fixedcolormap) {
    dcvars.colormap = fixedcolormap;
    dcvars.nextcolormap = dcvars.colormap;
  }

  patch = R_CacheTextureCompositePatchNum(texnum);

  for (dcvars.x = x1 ; dcvars.x <= x2 ; dcvars.x++, spryscale += rw_scalestep)
    if (maskedtexturecol[dcvars.x] != INT_MAX)
      {
        angle = (ds->rw_centerangle + xtoviewangle[dcvars.x]) >> ANGLETOFINESHIFT;
        dcvars.texu = ds->rw_offset - FixedMul(finetangent[angle], ds->rw_distance);
        if (drawvars.filterwall == RDRAW_FILTER_LINEAR)
          dcvars.texu -= (FRACUNIT>>1);

        if (!fixedcolormap)
          dcvars.z = spryscale;
        dcvars.colormap = R_ColourMap(rw_lightlevel,spryscale);
        dcvars.nextcolormap = R_ColourMap(rw_lightlevel+1,spryscale);

        {
          int_64_t t = ((int_64_t) centeryfrac << FRACBITS) -
            (int_64_t) dcvars.texturemid * spryscale;
          if (t + (int_64_t) textureheight[texnum] * spryscale < 0 ||
              t > (int_64_t) MAX_SCREENHEIGHT << FRACBITS*2)
            continue;
          sprtopscreen = (long)(t >> FRACBITS);
        }

        dcvars.iscale = 0xffffffffu / (unsigned) spryscale;

        R_DrawMaskedColumn(
          patch,
          colfunc,
          &dcvars,
          R_GetPatchColumnWrapped(patch, maskedtexturecol[dcvars.x]),
          R_GetPatchColumnWrapped(patch, maskedtexturecol[dcvars.x]-1),
          R_GetPatchColumnWrapped(patch, maskedtexturecol[dcvars.x]+1)
        );

        maskedtexturecol[dcvars.x] = INT_MAX;
      }

  if (curline->linedef->tranlump > 0)
    W_UnlockLumpNum(curline->linedef->tranlump-1);

  R_UnlockTextureCompositePatchNum(texnum);

  curline = NULL;
}

#define HEIGHTBITS 12
#define HEIGHTUNIT (1<<HEIGHTBITS)
static int didsolidcol;

static void R_RenderSegLoop (void)
{
  const rpatch_t *tex_patch;
  R_DrawColumn_f colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterwall, drawvars.filterz);
  draw_column_vars_t dcvars;
  fixed_t  texturecolumn = 0;

  R_SetDefaultDrawColumnVars(&dcvars);

  rendered_segs++;
  for ( ; rw_x < rw_stopx ; rw_x++)
    {

      int yh = bottomfrac>>HEIGHTBITS;
      int yl = (topfrac+HEIGHTUNIT-1)>>HEIGHTBITS;
      int bottom,top = ceilingclip[rw_x]+1;

      if (yl < top)
        yl = top;

      if (markceiling)
        {
          bottom = yl-1;

          if (bottom >= floorclip[rw_x])
            bottom = floorclip[rw_x]-1;

          if (top <= bottom)
            {
              ceilingplane->top[rw_x] = top;
              ceilingplane->bottom[rw_x] = bottom;
            }
          ceilingclip[rw_x] = bottom;
        }

      bottom = floorclip[rw_x]-1;
      if (yh > bottom)
        yh = bottom;

      if (markfloor)
        {

          top  = yh < ceilingclip[rw_x] ? ceilingclip[rw_x] : yh;

          if (++top <= bottom)
            {
              floorplane->top[rw_x] = top;
              floorplane->bottom[rw_x] = bottom;
            }
          floorclip[rw_x] = top;
        }

      if (segtextured)
        {
          angle_t angle =(rw_centerangle+xtoviewangle[rw_x])>>ANGLETOFINESHIFT;

          texturecolumn = rw_offset-FixedMul(finetangent[angle],rw_distance);
          if (drawvars.filterwall == RDRAW_FILTER_LINEAR)
            texturecolumn -= (FRACUNIT>>1);
          dcvars.texu = texturecolumn;
          texturecolumn >>= FRACBITS;

          dcvars.colormap = R_ColourMap(rw_lightlevel,rw_scale);
          dcvars.nextcolormap = R_ColourMap(rw_lightlevel+1,rw_scale);
          dcvars.z = rw_scale;

          dcvars.x = rw_x;
          dcvars.iscale = 0xffffffffu / (unsigned)rw_scale;
        }

      if (midtexture)
        {

          dcvars.yl = yl;
          dcvars.yh = yh;
          dcvars.texturemid = rw_midtexturemid;
          tex_patch = R_CacheTextureCompositePatchNum(midtexture);
          dcvars.source = R_GetTextureColumn(tex_patch, texturecolumn);
          dcvars.prevsource = R_GetTextureColumn(tex_patch, texturecolumn-1);
          dcvars.nextsource = R_GetTextureColumn(tex_patch, texturecolumn+1);
          dcvars.texheight = midtexheight;
          colfunc (&dcvars);
          R_UnlockTextureCompositePatchNum(midtexture);
          tex_patch = NULL;
          ceilingclip[rw_x] = viewheight;
          floorclip[rw_x] = -1;
        }
      else
        {

          if (toptexture)
            {
              int mid = pixhigh>>HEIGHTBITS;
              pixhigh += pixhighstep;

              if (mid >= floorclip[rw_x])
                mid = floorclip[rw_x]-1;

              if (mid >= yl)
                {
                  dcvars.yl = yl;
                  dcvars.yh = mid;
                  dcvars.texturemid = rw_toptexturemid;
                  tex_patch = R_CacheTextureCompositePatchNum(toptexture);
                  dcvars.source = R_GetTextureColumn(tex_patch,texturecolumn);
                  dcvars.prevsource = R_GetTextureColumn(tex_patch,texturecolumn-1);
                  dcvars.nextsource = R_GetTextureColumn(tex_patch,texturecolumn+1);
                  dcvars.texheight = toptexheight;
                  colfunc (&dcvars);
                  R_UnlockTextureCompositePatchNum(toptexture);
                  tex_patch = NULL;
                  ceilingclip[rw_x] = mid;
                }
              else
                ceilingclip[rw_x] = yl-1;
            }
          else
            {

            if (markceiling)
              ceilingclip[rw_x] = yl-1;
             }

          if (bottomtexture)
            {
              int mid = (pixlow+HEIGHTUNIT-1)>>HEIGHTBITS;
              pixlow += pixlowstep;

              if (mid <= ceilingclip[rw_x])
                mid = ceilingclip[rw_x]+1;

              if (mid <= yh)
                {
                  dcvars.yl = mid;
                  dcvars.yh = yh;
                  dcvars.texturemid = rw_bottomtexturemid;
                  tex_patch = R_CacheTextureCompositePatchNum(bottomtexture);
                  dcvars.source = R_GetTextureColumn(tex_patch, texturecolumn);
                  dcvars.prevsource = R_GetTextureColumn(tex_patch, texturecolumn-1);
                  dcvars.nextsource = R_GetTextureColumn(tex_patch, texturecolumn+1);
                  dcvars.texheight = bottomtexheight;
                  colfunc (&dcvars);
                  R_UnlockTextureCompositePatchNum(bottomtexture);
                  tex_patch = NULL;
                  floorclip[rw_x] = mid;
                }
              else
                floorclip[rw_x] = yh+1;
            }
          else
            {
            if (markfloor)
              floorclip[rw_x] = yh+1;
            }

    if ((markceiling || markfloor) &&
        (floorclip[rw_x] <= ceilingclip[rw_x] + 1)) {
      solidcol[rw_x] = 1; didsolidcol = 1;
    }

          if (maskedtexture)
            maskedtexturecol[rw_x] = texturecolumn;
        }

      rw_scale += rw_scalestep;
      topfrac += topstep;
      bottomfrac += bottomstep;
    }
}

static fixed_t R_PointToDist(fixed_t x, fixed_t y)
{
  fixed_t dx = D_abs(x - viewx);
  fixed_t dy = D_abs(y - viewy);

  if (dy > dx)
    {
      fixed_t t = dx;
      dx = dy;
      dy = t;
    }

  return FixedDiv(dx, finesine[(tantoangle[FixedDiv(dy,dx) >> DBITS]
                                + ANG90) >> ANGLETOFINESHIFT]);
}

void R_StoreWallRange(const int start, const int stop)
{
  fixed_t hyp;
  angle_t offsetangle;

  if (ds_p == drawsegs+maxdrawsegs)
    {
      unsigned pos = ds_p - drawsegs;
      unsigned newmax = maxdrawsegs ? maxdrawsegs*2 : 128;
      drawsegs = realloc(drawsegs,newmax*sizeof(*drawsegs));
      ds_p = drawsegs + pos;
      maxdrawsegs = newmax;
    }

  if(curline->miniseg == false)
    curline->linedef->flags |= ML_MAPPED;

  sidedef = curline->sidedef;
  linedef = curline->linedef;
  linedef->flags |= ML_MAPPED;
  rw_normalangle = curline->angle + ANG90;
  offsetangle = rw_normalangle-rw_angle1;

  if (D_abs(offsetangle) > ANG90)
    offsetangle = ANG90;

  hyp = (viewx==curline->v1->x && viewy==curline->v1->y) ? 0 : R_PointToDist (curline->v1->x, curline->v1->y);
  rw_distance = FixedMul(hyp, finecosine[offsetangle>>ANGLETOFINESHIFT]);
  ds_p->x1 = rw_x = start;
  ds_p->x2 = stop;
  ds_p->curline = curline;
  rw_stopx = stop+1;

  {
    extern int *openings;
    extern size_t maxopenings;
    size_t pos = lastopening - openings;
    size_t need = (rw_stopx - start)*4 + pos;
    if (need > maxopenings)
      {
        drawseg_t *ds;
        int *oldopenings = openings;
        int *oldlast = lastopening;

        do
          maxopenings = maxopenings ? maxopenings*2 : 16384;
        while (need > maxopenings);
        openings = realloc(openings, maxopenings * sizeof(*openings));
        lastopening = openings + pos;

      for (ds = drawsegs; ds < ds_p; ds++)
        {
#define ADJUST(p) if (ds->p + ds->x1 >= oldopenings && ds->p + ds->x1 <= oldlast)\
            ds->p = ds->p - oldopenings + openings;
          ADJUST (maskedtexturecol);
          ADJUST (sprtopclip);
          ADJUST (sprbottomclip);
        }
#undef ADJUST
      }
  }

  ds_p->scale1 = rw_scale = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[start]);

  if (stop > start)
    {
      ds_p->scale2 = R_ScaleFromGlobalAngle (viewangle + xtoviewangle[stop]);
      ds_p->scalestep = rw_scalestep = (ds_p->scale2-rw_scale) / (stop-start);
    }
  else
    ds_p->scale2 = ds_p->scale1;

  worldtop = frontsector->ceilingheight - viewz;
  worldbottom = frontsector->floorheight - viewz;

  midtexture = toptexture = bottomtexture = maskedtexture = 0;
  ds_p->maskedtexturecol = NULL;

  if (!backsector)
    {
      midtexture = texturetranslation[sidedef->midtexture];
      midtexheight = (linedef->r_flags & RF_MID_TILE) ? 0 : textureheight[midtexture] >> FRACBITS;

      markfloor = markceiling = true;

      if (linedef->flags & ML_DONTPEGBOTTOM)
        {
          fixed_t vtop = frontsector->floorheight +
            textureheight[sidedef->midtexture];
          rw_midtexturemid = vtop - viewz;
        }
      else
        rw_midtexturemid = worldtop;

      rw_midtexturemid += FixedMod(sidedef->rowoffset, textureheight[midtexture]);

      ds_p->silhouette = SIL_BOTH;
      ds_p->sprtopclip = screenheightarray;
      ds_p->sprbottomclip = negonearray;
      ds_p->bsilheight = INT_MAX;
      ds_p->tsilheight = INT_MIN;
    }
  else
    {
      ds_p->sprtopclip = ds_p->sprbottomclip = NULL;
      ds_p->silhouette = 0;

      if (linedef->r_flags & RF_CLOSED) {

  ds_p->silhouette = SIL_BOTH;
  ds_p->sprbottomclip = negonearray;
  ds_p->bsilheight = INT_MAX;
  ds_p->sprtopclip = screenheightarray;
  ds_p->tsilheight = INT_MIN;

      } else {

  if (frontsector->floorheight > backsector->floorheight)
    {
      ds_p->silhouette = SIL_BOTTOM;
      ds_p->bsilheight = frontsector->floorheight;
    }
  else
    if (backsector->floorheight > viewz)
      {
        ds_p->silhouette = SIL_BOTTOM;
        ds_p->bsilheight = INT_MAX;
      }

  if (frontsector->ceilingheight < backsector->ceilingheight)
    {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = frontsector->ceilingheight;
    }
  else
    if (backsector->ceilingheight < viewz)
      {
        ds_p->silhouette |= SIL_TOP;
        ds_p->tsilheight = INT_MIN;
      }
      }

      worldhigh = backsector->ceilingheight - viewz;
      worldlow = backsector->floorheight - viewz;

      if (frontsector->ceilingpic == skyflatnum
          && backsector->ceilingpic == skyflatnum)
        worldtop = worldhigh;

      markfloor = worldlow != worldbottom
        || backsector->floorpic != frontsector->floorpic
        || backsector->lightlevel != frontsector->lightlevel
        || backsector->floor_xoffs != frontsector->floor_xoffs
        || backsector->floor_yoffs != frontsector->floor_yoffs
        || frontsector->heightsec != -1
        || backsector->floorlightsec != frontsector->floorlightsec
        ;

      markceiling = worldhigh != worldtop
        || backsector->ceilingpic != frontsector->ceilingpic
        || backsector->lightlevel != frontsector->lightlevel
        || backsector->ceiling_xoffs != frontsector->ceiling_xoffs
        || backsector->ceiling_yoffs != frontsector->ceiling_yoffs
        || (frontsector->heightsec != -1 && frontsector->ceilingpic!=skyflatnum)
        || backsector->ceilinglightsec != frontsector->ceilinglightsec
        ;

      if (backsector->ceilingheight <= frontsector->floorheight
          || backsector->floorheight >= frontsector->ceilingheight)
        markceiling = markfloor = true;

      if (worldhigh < worldtop)
        {
          toptexture = texturetranslation[sidedef->toptexture];
    toptexheight = (linedef->r_flags & RF_TOP_TILE) ? 0 : textureheight[toptexture] >> FRACBITS;
          rw_toptexturemid = linedef->flags & ML_DONTPEGTOP ? worldtop :
            backsector->ceilingheight+textureheight[sidedef->toptexture]-viewz;
    rw_toptexturemid += FixedMod(sidedef->rowoffset, textureheight[toptexture]);
        }

      if (worldlow > worldbottom)
        {
          bottomtexture = texturetranslation[sidedef->bottomtexture];
    bottomtexheight = (linedef->r_flags & RF_BOT_TILE) ? 0 : textureheight[bottomtexture] >> FRACBITS;
          rw_bottomtexturemid = linedef->flags & ML_DONTPEGBOTTOM ? worldtop :
            worldlow;
    rw_bottomtexturemid += FixedMod(sidedef->rowoffset, textureheight[bottomtexture]);
        }

      if (sidedef->midtexture)
        {
          maskedtexture = true;
          ds_p->maskedtexturecol = maskedtexturecol = lastopening - rw_x;
          lastopening += rw_stopx - rw_x;
        }
    }

  segtextured = midtexture | toptexture | bottomtexture | maskedtexture;

  if (segtextured)
    {
      rw_offset = FixedMul (hyp, -finesine[offsetangle >>ANGLETOFINESHIFT]);

      rw_offset += sidedef->textureoffset + curline->offset;

      rw_centerangle = ANG90 + viewangle - rw_normalangle;

      rw_lightlevel = frontsector->lightlevel;
    }

  ds_p->rw_offset = rw_offset;
  ds_p->rw_distance = rw_distance;
  ds_p->rw_centerangle = rw_centerangle;
      
  if (frontsector->heightsec == -1)
    {
      if (frontsector->floorheight >= viewz)
        markfloor = false;
      if (frontsector->ceilingheight <= viewz &&
          frontsector->ceilingpic != skyflatnum)
        markceiling = false;
    }

  worldtop >>= 4;
  worldbottom >>= 4;

  topstep = -FixedMul (rw_scalestep, worldtop);
  topfrac = (centeryfrac>>4) - FixedMul (worldtop, rw_scale);

  bottomstep = -FixedMul (rw_scalestep,worldbottom);
  bottomfrac = (centeryfrac>>4) - FixedMul (worldbottom, rw_scale);

  if (backsector)
    {
      worldhigh >>= 4;
      worldlow >>= 4;

      if (worldhigh < worldtop)
        {
          pixhigh = (centeryfrac>>4) - FixedMul (worldhigh, rw_scale);
          pixhighstep = -FixedMul (rw_scalestep,worldhigh);
        }
      if (worldlow > worldbottom)
        {
          pixlow = (centeryfrac>>4) - FixedMul (worldlow, rw_scale);
          pixlowstep = -FixedMul (rw_scalestep,worldlow);
        }
    }

  if (markceiling) {
    if (ceilingplane)
      ceilingplane = R_CheckPlane (ceilingplane, rw_x, rw_stopx-1);
    else
      markceiling = 0;
  }

  if (markfloor) {
    if (floorplane)
      if (markceiling && ceilingplane == floorplane)
    floorplane = R_DupPlane (floorplane, rw_x, rw_stopx-1);
      else
    floorplane = R_CheckPlane (floorplane, rw_x, rw_stopx-1);
    else
      markfloor = 0;
  }

  didsolidcol = 0;
  R_RenderSegLoop();

  if (backsector && didsolidcol) {
    if (!(ds_p->silhouette & SIL_BOTTOM)) {
      ds_p->silhouette |= SIL_BOTTOM;
      ds_p->bsilheight = backsector->floorheight;
    }
    if (!(ds_p->silhouette & SIL_TOP)) {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = backsector->ceilingheight;
    }
  }

  if ((ds_p->silhouette & SIL_TOP || maskedtexture) && !ds_p->sprtopclip)
    {
      memcpy (lastopening, ceilingclip+start, sizeof(int)*(rw_stopx-start));
      ds_p->sprtopclip = lastopening - start;
      lastopening += rw_stopx - start;
    }
  if ((ds_p->silhouette & SIL_BOTTOM || maskedtexture) && !ds_p->sprbottomclip)
    {
      memcpy (lastopening, floorclip+start, sizeof(int)*(rw_stopx-start));
      ds_p->sprbottomclip = lastopening - start;
      lastopening += rw_stopx - start;
    }
  if (maskedtexture && !(ds_p->silhouette & SIL_TOP))
    {
      ds_p->silhouette |= SIL_TOP;
      ds_p->tsilheight = INT_MIN;
    }
  if (maskedtexture && !(ds_p->silhouette & SIL_BOTTOM))
    {
      ds_p->silhouette |= SIL_BOTTOM;
      ds_p->bsilheight = INT_MAX;
    }
  ds_p++;
}
