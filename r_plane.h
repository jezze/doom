#ifndef __R_PLANE__
#define __R_PLANE__

#define PL_SKYFLAT                      0x80000000

extern int *lastopening;
extern int floorclip[], ceilingclip[];
extern int yslope[], distscale[];

void R_ClearPlanes(void);
void R_DrawPlanes(void);
visplane_t *R_FindPlane(int height, int picnum, int lightlevel, int xoffs, int yoffs);
visplane_t *R_CheckPlane(visplane_t *pl, int start, int stop);
visplane_t *R_DupPlane(const visplane_t *pl, int start, int stop);

#endif
