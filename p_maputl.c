#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_main.h"
#include "p_maputl.h"
#include "p_map.h"
#include "p_setup.h"

static size_t num_intercepts;
static intercept_t *intercepts, *intercept_p;
int opentop;
int openbottom;
int openrange;
int lowfloor;
sector_t *openfrontsector;
sector_t *openbacksector;

int P_AproxDistance(int dx, int dy)
{
  dx = D_abs(dx);
  dy = D_abs(dy);
  if (dx < dy)
    return dx+dy-(dx>>1);
  return dx+dy-(dy>>1);
}

int P_PointOnLineSide(int x, int y, const line_t *line)
{
  return
    !line->dx ? x <= line->v1->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->v1->y ? line->dx < 0 : line->dx > 0 :
    FixedMul(y-line->v1->y, line->dx>>FRACBITS) >=
    FixedMul(line->dy>>FRACBITS, x-line->v1->x);
}

int P_BoxOnLineSide(const int *tmbox, const line_t *ld)
{
  switch (ld->slopetype)
    {
      int p;
    default:
    case ST_HORIZONTAL:
      return
      (tmbox[BOXBOTTOM] > ld->v1->y) == (p = tmbox[BOXTOP] > ld->v1->y) ?
        p ^ (ld->dx < 0) : -1;
    case ST_VERTICAL:
      return
        (tmbox[BOXLEFT] < ld->v1->x) == (p = tmbox[BOXRIGHT] < ld->v1->x) ?
        p ^ (ld->dy < 0) : -1;
    case ST_POSITIVE:
      return
        P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXBOTTOM], ld) ==
        (p = P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXTOP], ld)) ? p : -1;
    case ST_NEGATIVE:
      return
        (P_PointOnLineSide(tmbox[BOXLEFT], tmbox[BOXBOTTOM], ld)) ==
        (p = P_PointOnLineSide(tmbox[BOXRIGHT], tmbox[BOXTOP], ld)) ? p : -1;
    }
}

static int P_PointOnDivlineSide(int x, int y, const divline_t *line)
{
  return
    !line->dx ? x <= line->x ? line->dy > 0 : line->dy < 0 :
    !line->dy ? y <= line->y ? line->dx < 0 : line->dx > 0 :
    (line->dy^line->dx^(x -= line->x)^(y -= line->y)) < 0 ? (line->dy^x) < 0 :
    FixedMul(y>>8, line->dx>>8) >= FixedMul(line->dy>>8, x>>8);
}

static void P_MakeDivline(const line_t *li, divline_t *dl)
{
  dl->x = li->v1->x;
  dl->y = li->v1->y;
  dl->dx = li->dx;
  dl->dy = li->dy;
}


int P_InterceptVector2(const divline_t *v2, const divline_t *v1)
{
  int den;
  return (den = FixedMul(v1->dy>>8, v2->dx) - FixedMul(v1->dx>>8, v2->dy)) ?
    FixedDiv(FixedMul((v1->x - v2->x)>>8, v1->dy) +
             FixedMul((v2->y - v1->y)>>8, v1->dx), den) : 0;
}

int P_InterceptVector(const divline_t *v2, const divline_t *v1)
{
    int_64_t den = (int_64_t)v1->dy * v2->dx - (int_64_t)v1->dx * v2->dy;
    den >>= 16;
    if (!den)
      return 0;
    return (int)(((int_64_t)(v1->x - v2->x) * v1->dy - (int_64_t)(v1->y - v2->y) * v1->dx) / den);
}

void P_LineOpening(const line_t *linedef)
{
  if (linedef->sidenum[1] == NO_INDEX)
    {
      openrange = 0;
      return;
    }

  openfrontsector = linedef->frontsector;
  openbacksector = linedef->backsector;

  if (openfrontsector->ceilingheight < openbacksector->ceilingheight)
    opentop = openfrontsector->ceilingheight;
  else
    opentop = openbacksector->ceilingheight;

  if (openfrontsector->floorheight > openbacksector->floorheight)
    {
      openbottom = openfrontsector->floorheight;
      lowfloor = openbacksector->floorheight;
    }
  else
    {
      openbottom = openbacksector->floorheight;
      lowfloor = openfrontsector->floorheight;
    }
  openrange = opentop - openbottom;
}

void P_UnsetThingPosition (mobj_t *thing)
{
  if (!(thing->flags & MF_NOSECTOR))
    {
      mobj_t **sprev = thing->sprev;
      mobj_t  *snext = thing->snext;
      if ((*sprev = snext))
        snext->sprev = sprev;

      sector_list = thing->touching_sectorlist;
      thing->touching_sectorlist = NULL;
    }

  if (!(thing->flags & MF_NOBLOCKMAP))
    {
      mobj_t *bnext, **bprev = thing->bprev;
      if (bprev && (*bprev = bnext = thing->bnext))
        bnext->bprev = bprev;
    }
}

void P_SetThingPosition(mobj_t *thing)
{
  subsector_t *ss = thing->subsector = R_PointInSubsector(thing->x, thing->y);
  if (!(thing->flags & MF_NOSECTOR))
    {

      mobj_t **link = &ss->sector->thinglist;
      mobj_t *snext = *link;
      if ((thing->snext = snext))
        snext->sprev = &thing->snext;
      thing->sprev = link;
      *link = thing;

      P_CreateSecNodeList(thing,thing->x,thing->y);
      thing->touching_sectorlist = sector_list;
      sector_list = NULL;
    }


  if (!(thing->flags & MF_NOBLOCKMAP))
    {

      int blockx = (thing->x - bmaporgx)>>MAPBLOCKSHIFT;
      int blocky = (thing->y - bmaporgy)>>MAPBLOCKSHIFT;
      if (blockx>=0 && blockx < bmapwidth && blocky>=0 && blocky < bmapheight)
        {



        mobj_t **link = &blocklinks[blocky*bmapwidth+blockx];
        mobj_t *bnext = *link;
        if ((thing->bnext = bnext))
          bnext->bprev = &thing->bnext;
        thing->bprev = link;
        *link = thing;
      }
      else
        thing->bnext = NULL, thing->bprev = NULL;
    }
}

boolean P_BlockLinesIterator(int x, int y, boolean func(line_t*))
{
  int        offset;
  const long *list;

  if (x<0 || y<0 || x>=bmapwidth || y>=bmapheight)
    return true;
  offset = y*bmapwidth+x;
  offset = *(blockmap+offset);
  list = blockmaplump+offset;
  list++;

  for ( ; *list != -1 ; list++)
    {
      line_t *ld = &lines[*list];
      if (ld->validcount == validcount)
        continue;
      ld->validcount = validcount;
      if (!func(ld))
        return false;
    }
  return true;
}

boolean P_BlockThingsIterator(int x, int y, boolean func(mobj_t*))
{
  mobj_t *mobj;
  if (!(x<0 || y<0 || x>=bmapwidth || y>=bmapheight))
    for (mobj = blocklinks[y*bmapwidth+x]; mobj; mobj = mobj->bnext)
      if (!func(mobj))
        return false;
  return true;
}

static void check_intercept(void)
{
  size_t offset = intercept_p - intercepts;
  if (offset >= num_intercepts)
    {
      num_intercepts = num_intercepts ? num_intercepts*2 : 128;
      intercepts = realloc(intercepts, sizeof(*intercepts)*num_intercepts);
      intercept_p = intercepts + offset;
    }
}

divline_t trace;

boolean PIT_AddLineIntercepts(line_t *ld)
{
  int       s1;
  int       s2;
  int   frac;
  divline_t dl;


  if (trace.dx >  FRACUNIT*16 || trace.dy >  FRACUNIT*16 ||
      trace.dx < -FRACUNIT*16 || trace.dy < -FRACUNIT*16)
    {
      s1 = P_PointOnDivlineSide (ld->v1->x, ld->v1->y, &trace);
      s2 = P_PointOnDivlineSide (ld->v2->x, ld->v2->y, &trace);
    }
  else
    {
      s1 = P_PointOnLineSide (trace.x, trace.y, ld);
      s2 = P_PointOnLineSide (trace.x+trace.dx, trace.y+trace.dy, ld);
    }

  if (s1 == s2)
    return true;


  P_MakeDivline(ld, &dl);
  frac = P_InterceptVector(&trace, &dl);

  if (frac < 0)
    return true;

  check_intercept();

  intercept_p->frac = frac;
  intercept_p->isaline = true;
  intercept_p->d.line = ld;
  intercept_p++;

  return true;
}

boolean PIT_AddThingIntercepts(mobj_t *thing)
{
  int   x1, y1;
  int   x2, y2;
  int       s1, s2;
  divline_t dl;
  int   frac;


  if ((trace.dx ^ trace.dy) > 0)
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y + thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y - thing->radius;
    }
  else
    {
      x1 = thing->x - thing->radius;
      y1 = thing->y - thing->radius;
      x2 = thing->x + thing->radius;
      y2 = thing->y + thing->radius;
    }

  s1 = P_PointOnDivlineSide (x1, y1, &trace);
  s2 = P_PointOnDivlineSide (x2, y2, &trace);

  if (s1 == s2)
    return true;

  dl.x = x1;
  dl.y = y1;
  dl.dx = x2-x1;
  dl.dy = y2-y1;

  frac = P_InterceptVector (&trace, &dl);

  if (frac < 0)
    return true;

  check_intercept();

  intercept_p->frac = frac;
  intercept_p->isaline = false;
  intercept_p->d.thing = thing;
  intercept_p++;

  return true;
}

boolean P_TraverseIntercepts(traverser_t func, int maxfrac)
{
  intercept_t *in = NULL;
  int count = intercept_p - intercepts;
  while (count--)
    {
      int dist = INT_MAX;
      intercept_t *scan;
      for (scan = intercepts; scan < intercept_p; scan++)
        if (scan->frac < dist)
          dist = (in=scan)->frac;
      if (dist > maxfrac)
        return true;
      if (!func(in))
        return false;
      in->frac = INT_MAX;
    }
  return true;
}

boolean P_PathTraverse(int x1, int y1, int x2, int y2, int flags, boolean trav(intercept_t *))
{
  int xt1, yt1;
  int xt2, yt2;
  int xstep, ystep;
  int partial;
  int xintercept, yintercept;
  int     mapx, mapy;
  int     mapxstep, mapystep;
  int     count;

  validcount++;
  intercept_p = intercepts;

  if (!((x1-bmaporgx)&(MAPBLOCKSIZE-1)))
    x1 += FRACUNIT;

  if (!((y1-bmaporgy)&(MAPBLOCKSIZE-1)))
    y1 += FRACUNIT;

  trace.x = x1;
  trace.y = y1;
  trace.dx = x2 - x1;
  trace.dy = y2 - y1;

  x1 -= bmaporgx;
  y1 -= bmaporgy;
  xt1 = x1>>MAPBLOCKSHIFT;
  yt1 = y1>>MAPBLOCKSHIFT;

  x2 -= bmaporgx;
  y2 -= bmaporgy;
  xt2 = x2>>MAPBLOCKSHIFT;
  yt2 = y2>>MAPBLOCKSHIFT;

  if (xt2 > xt1)
    {
      mapxstep = 1;
      partial = FRACUNIT - ((x1>>MAPBTOFRAC)&(FRACUNIT-1));
      ystep = FixedDiv (y2-y1,D_abs(x2-x1));
    }
  else
    if (xt2 < xt1)
      {
        mapxstep = -1;
        partial = (x1>>MAPBTOFRAC)&(FRACUNIT-1);
        ystep = FixedDiv (y2-y1,D_abs(x2-x1));
      }
    else
      {
        mapxstep = 0;
        partial = FRACUNIT;
        ystep = 256*FRACUNIT;
      }

  yintercept = (y1>>MAPBTOFRAC) + FixedMul(partial, ystep);

  if (yt2 > yt1)
    {
      mapystep = 1;
      partial = FRACUNIT - ((y1>>MAPBTOFRAC)&(FRACUNIT-1));
      xstep = FixedDiv (x2-x1,D_abs(y2-y1));
    }
  else
    if (yt2 < yt1)
      {
        mapystep = -1;
        partial = (y1>>MAPBTOFRAC)&(FRACUNIT-1);
        xstep = FixedDiv (x2-x1,D_abs(y2-y1));
      }
    else
      {
        mapystep = 0;
        partial = FRACUNIT;
        xstep = 256*FRACUNIT;
      }

  xintercept = (x1>>MAPBTOFRAC) + FixedMul (partial, xstep);
  mapx = xt1;
  mapy = yt1;

  for (count = 0; count < 64; count++)
    {
      if (flags & PT_ADDLINES)
        if (!P_BlockLinesIterator(mapx, mapy,PIT_AddLineIntercepts))
          return false;

      if (flags & PT_ADDTHINGS)
        if (!P_BlockThingsIterator(mapx, mapy,PIT_AddThingIntercepts))
          return false;

      if (mapx == xt2 && mapy == yt2)
        break;

      if ((yintercept >> FRACBITS) == mapy)
        {
          yintercept += ystep;
          mapx += mapxstep;
        }
      else
        if ((xintercept >> FRACBITS) == mapx)
          {
            xintercept += xstep;
            mapy += mapystep;
          }
    }

  return P_TraverseIntercepts(trav, FRACUNIT);
}
