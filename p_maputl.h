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

    int x;
    int y;
    int dx;
    int dy;

} divline_t;

typedef struct
{

    int frac;
    boolean isaline;
    union
    {

        mobj_t *thing;
        line_t *line;

    } d;

} intercept_t;

typedef boolean (*traverser_t)(intercept_t *in);
int P_AproxDistance (int dx, int dy);
int P_PointOnLineSide (int x, int y, const line_t *line);
int P_BoxOnLineSide (const int *tmbox, const line_t *ld);
int P_InterceptVector (const divline_t *v2, const divline_t *v1);
int P_InterceptVector2(const divline_t *v2, const divline_t *v1);
void P_LineOpening (const line_t *linedef);
void P_UnsetThingPosition(mobj_t *thing);
void P_SetThingPosition(mobj_t *thing);
boolean P_BlockLinesIterator (int x, int y, boolean func(line_t *));
boolean P_BlockThingsIterator(int x, int y, boolean func(mobj_t *));
boolean P_PathTraverse(int x1, int y1, int x2, int y2, int flags, boolean trav(intercept_t *));

extern int opentop;
extern int openbottom;
extern int openrange;
extern int lowfloor;
extern divline_t trace;

#endif
