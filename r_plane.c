#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "z_zone.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_things.h"
#include "r_plane.h"
#include "v_video.h"

#define MAXVISPLANES                    128
#define ANGLETOSKYSHIFT                 22

static visplane_t *visplanes[MAXVISPLANES];
static visplane_t *freetail;
static visplane_t **freehead = &freetail;
visplane_t *floorplane, *ceilingplane;

#define visplane_hash(picnum,lightlevel,height) \
  ((unsigned)((picnum)*3+(lightlevel)+(height)*7) & (MAXVISPLANES-1))

size_t maxopenings;
int *openings, *lastopening;
int floorclip[MAX_SCREENWIDTH], ceilingclip[MAX_SCREENWIDTH];
static int spanstart[MAX_SCREENHEIGHT];
static const lighttable_t **planezlight;
static fixed_t planeheight;
static fixed_t basexscale, baseyscale;
static fixed_t cachedheight[MAX_SCREENHEIGHT];
static fixed_t cacheddistance[MAX_SCREENHEIGHT];
static fixed_t cachedxstep[MAX_SCREENHEIGHT];
static fixed_t cachedystep[MAX_SCREENHEIGHT];
static fixed_t xoffs, yoffs;

fixed_t yslope[MAX_SCREENHEIGHT], distscale[MAX_SCREENWIDTH];

static void R_MapPlane(int y, int x1, int x2, draw_span_vars_t *dsvars)
{

    angle_t angle;
    fixed_t distance, length;
    unsigned index;

    if (planeheight != cachedheight[y])
    {

        cachedheight[y] = planeheight;
        distance = cacheddistance[y] = FixedMul(planeheight, yslope[y]);
        dsvars->xstep = cachedxstep[y] = FixedMul(distance,basexscale);
        dsvars->ystep = cachedystep[y] = FixedMul(distance,baseyscale);
    }

    else
    {

        distance = cacheddistance[y];
        dsvars->xstep = cachedxstep[y];
        dsvars->ystep = cachedystep[y];

    }

    length = FixedMul(distance, distscale[x1]);
    angle = (viewangle + xtoviewangle[x1]) >> ANGLETOFINESHIFT;
    dsvars->xfrac = viewx + FixedMul(finecosine[angle], length) + xoffs;
    dsvars->yfrac = -viewy - FixedMul(finesine[angle], length) + yoffs;

    if (drawvars.filterfloor == RDRAW_FILTER_LINEAR)
    {

        dsvars->xfrac -= (FRACUNIT >> 1);
        dsvars->yfrac -= (FRACUNIT >> 1);

    }

    if (!(dsvars->colormap = fixedcolormap))
    {

        dsvars->z = distance;
        index = distance >> LIGHTZSHIFT;

        if (index >= MAXLIGHTZ)
            index = MAXLIGHTZ - 1;

        dsvars->colormap = planezlight[index];
        dsvars->nextcolormap = planezlight[index + 1 >= MAXLIGHTZ ? MAXLIGHTZ - 1 : index + 1];

    }

    else
    {

        dsvars->z = 0;
    }

    dsvars->y = y;
    dsvars->x1 = x1;
    dsvars->x2 = x2;

    R_DrawSpan(dsvars);

}

void R_ClearPlanes(void)
{

    int i;

    for (i = 0; i < viewwidth; i++)
        floorclip[i] = viewheight, ceilingclip[i] = -1;

    for (i = 0; i < MAXVISPLANES; i++)
        for (*freehead = visplanes[i], visplanes[i] = NULL; *freehead; )
            freehead = &(*freehead)->next;

    lastopening = openings;

    memset(cachedheight, 0, sizeof(cachedheight));

    basexscale = FixedDiv(viewsin, projection);
    baseyscale = FixedDiv(viewcos, projection);

}

static visplane_t *new_visplane(unsigned hash)
{

    visplane_t *check = freetail;

    if (!check)
        check = calloc(1, sizeof *check);
    else if (!(freetail = freetail->next))
        freehead = &freetail;

    check->next = visplanes[hash];
    visplanes[hash] = check;

    return check;

}

visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop)
{

      unsigned hash = visplane_hash(pl->picnum, pl->lightlevel, pl->height);
      visplane_t *new_pl = new_visplane(hash);

      new_pl->height = pl->height;
      new_pl->picnum = pl->picnum;
      new_pl->lightlevel = pl->lightlevel;
      new_pl->xoffs = pl->xoffs;
      new_pl->yoffs = pl->yoffs;
      new_pl->minx = start;
      new_pl->maxx = stop;

      memset(new_pl->top, 0xff, sizeof new_pl->top);

      return new_pl;

}

visplane_t *R_FindPlane(fixed_t height, int picnum, int lightlevel, fixed_t xoffs, fixed_t yoffs)
{

    visplane_t *check;
    unsigned hash;

    if (picnum == skyflatnum || picnum & PL_SKYFLAT)
        height = lightlevel = 0;

    hash = visplane_hash(picnum,lightlevel,height);

    for (check=visplanes[hash]; check; check=check->next)
        if (height == check->height && picnum == check->picnum && lightlevel == check->lightlevel && xoffs == check->xoffs && yoffs == check->yoffs)
            return check;

    check = new_visplane(hash);
    check->height = height;
    check->picnum = picnum;
    check->lightlevel = lightlevel;
    check->minx = viewwidth;
    check->maxx = -1;
    check->xoffs = xoffs;
    check->yoffs = yoffs;

    memset(check->top, 0xff, sizeof check->top);

    return check;

}

visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop)
{

    int intrl, intrh, unionl, unionh, x;

    if (start < pl->minx)
        intrl = pl->minx, unionl = start;
    else
        unionl = pl->minx, intrl = start;

    if (stop > pl->maxx)
        intrh = pl->maxx, unionh = stop;
    else
        unionh = pl->maxx, intrh = stop;

    for (x = intrl; x <= intrh && pl->top[x] == 0xffffffffu; x++);

    if (x > intrh)
    {

        pl->minx = unionl;
        pl->maxx = unionh;

        return pl;

    }

    else
        return R_DupPlane(pl, start, stop);

}

static void R_MakeSpans(int x, unsigned int t1, unsigned int b1, unsigned int t2, unsigned int b2, draw_span_vars_t *dsvars)
{

    for (; t1 < t2 && t1 <= b1; t1++)
        R_MapPlane(t1, spanstart[t1], x - 1, dsvars);

    for (; b1 > b2 && b1 >= t1; b1--)
        R_MapPlane(b1, spanstart[b1], x - 1, dsvars);

    while (t2 < t1 && t2 <= b2)
        spanstart[t2++] = x;

    while (b2 > b1 && b2 >= t2)
        spanstart[b2--] = x;

}

static void R_DoDrawPlane(visplane_t *pl)
{

    register int x;
    draw_column_vars_t dcvars;
    R_DrawColumn_f colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterwall, drawvars.filterz);

    R_SetDefaultDrawColumnVars(&dcvars);

    if (pl->minx <= pl->maxx)
    {

        if (pl->picnum == skyflatnum || pl->picnum & PL_SKYFLAT)
        {

            int texture;
            const rpatch_t *tex_patch;
            angle_t flip, an = viewangle;

            if (pl->picnum & PL_SKYFLAT)
            {

                const line_t *l = &lines[pl->picnum & ~PL_SKYFLAT];
                const side_t *s = *l->sidenum + sides;

                texture = texturetranslation[s->toptexture];
                an += s->textureoffset;
                dcvars.texturemid = s->rowoffset - 28 * FRACUNIT;
                flip = l->special == 272 ? 0u : ~0u;

            }

            else
            {

                dcvars.texturemid = skytexturemid;
                texture = skytexture;
                flip = 0;

            }

            if (!(dcvars.colormap = fixedcolormap))
                dcvars.colormap = fullcolormap;

            dcvars.nextcolormap = dcvars.colormap;
            dcvars.texheight = textureheight[skytexture] >> FRACBITS;
            dcvars.iscale = FRACUNIT * 200 / viewheight;
            tex_patch = R_CacheTextureCompositePatchNum(texture);

            for (x = pl->minx; (dcvars.x = x) <= pl->maxx; x++)
                if ((dcvars.yl = pl->top[x]) != -1 && dcvars.yl <= (dcvars.yh = pl->bottom[x]))
                {

                    dcvars.source = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x]) ^ flip) >> ANGLETOSKYSHIFT);
                    dcvars.prevsource = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x - 1]) ^ flip) >> ANGLETOSKYSHIFT);
                    dcvars.nextsource = R_GetTextureColumn(tex_patch, ((an + xtoviewangle[x + 1]) ^ flip) >> ANGLETOSKYSHIFT);

                    colfunc(&dcvars);

                }

            R_UnlockTextureCompositePatchNum(texture);

        }
        
        else
        {

            int stop, light;
            draw_span_vars_t dsvars;

            dsvars.source = W_CacheLumpNum(firstflat + flattranslation[pl->picnum]);
            xoffs = pl->xoffs;
            yoffs = pl->yoffs;
            planeheight = D_abs(pl->height-viewz);
            light = (pl->lightlevel >> LIGHTSEGSHIFT) + extralight;

            if (light >= LIGHTLEVELS)
                light = LIGHTLEVELS - 1;

            if (light < 0)
                light = 0;

            stop = pl->maxx + 1;
            planezlight = zlight[light];
            pl->top[pl->minx - 1] = pl->top[stop] = 0xffffffffu;

            for (x = pl->minx; x <= stop; x++)
                R_MakeSpans(x, pl->top[x - 1], pl->bottom[x - 1], pl->top[x], pl->bottom[x], &dsvars);

            W_UnlockLumpNum(firstflat + flattranslation[pl->picnum]);

        }

    }

}

void R_DrawPlanes(void)
{

    visplane_t *pl;
    int i;

    for (i = 0; i < MAXVISPLANES; i++)
        for (pl = visplanes[i]; pl; pl = pl->next, rendered_visplanes++)
            R_DoDrawPlane(pl);

}

