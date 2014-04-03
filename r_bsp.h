#ifndef __R_BSP__
#define __R_BSP__

void R_ClearClipSegs(void);
void R_ClearDrawSegs(void);
void R_RenderBSPNode(int bspnum);
sector_t *R_FakeFlat(sector_t *, sector_t *, int *, int *, boolean);

extern seg_t *curline;
extern side_t *sidedef;
extern line_t *linedef;
extern sector_t *frontsector;
extern sector_t *backsector;
extern drawseg_t *drawsegs;
extern unsigned maxdrawsegs;
extern byte solidcol[MAX_SCREENWIDTH];
extern drawseg_t *ds_p;

#endif
