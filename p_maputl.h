#ifndef __P_MAPUTL__
#define __P_MAPUTL__

#define MAPBLOCKUNITS   128
#define MAPBLOCKSIZE    (MAPBLOCKUNITS*FRACUNIT)
#define MAPBLOCKSHIFT   (FRACBITS+7)
#define MAPBMASK        (MAPBLOCKSIZE-1)
#define MAPBTOFRAC      (MAPBLOCKSHIFT-FRACBITS)
#define PT_ADDLINES     1
#define PT_ADDTHINGS    2
#define PT_EARLYOUT     4

typedef struct
{

    fixed_t x;
    fixed_t y;
    fixed_t dx;
    fixed_t dy;

} divline_t;

typedef struct
{

    fixed_t frac;
    boolean isaline;
    union
    {

        mobj_t *thing;
        line_t *line;

    } d;

} intercept_t;

typedef boolean (*traverser_t)(intercept_t *in);
fixed_t P_AproxDistance (fixed_t dx, fixed_t dy);
int P_PointOnLineSide (fixed_t x, fixed_t y, const line_t *line);
int P_BoxOnLineSide (const fixed_t *tmbox, const line_t *ld);
fixed_t P_InterceptVector (const divline_t *v2, const divline_t *v1);
fixed_t P_InterceptVector2(const divline_t *v2, const divline_t *v1);
void P_LineOpening (const line_t *linedef);
void P_UnsetThingPosition(mobj_t *thing);
void P_SetThingPosition(mobj_t *thing);
boolean P_BlockLinesIterator (int x, int y, boolean func(line_t *));
boolean P_BlockThingsIterator(int x, int y, boolean func(mobj_t *));
boolean P_PathTraverse(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2, int flags, boolean trav(intercept_t *));

extern fixed_t opentop;
extern fixed_t openbottom;
extern fixed_t openrange;
extern fixed_t lowfloor;
extern divline_t trace;

#endif
