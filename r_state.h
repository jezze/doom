#ifndef __R_STATE__
#define __R_STATE__

#include "d_player.h"
#include "r_data.h"

extern fixed_t *textureheight;
extern int firstflat, numflats;
extern int *flattranslation;
extern int *texturetranslation;
extern int firstspritelump;
extern int lastspritelump;
extern int numspritelumps;
extern spritedef_t *sprites;
extern int numsegs;
extern seg_t *segs;
extern int numsectors;
extern sector_t *sectors;
extern int numsubsectors;
extern subsector_t *subsectors;
extern int numnodes;
extern node_t *nodes;
extern int numlines;
extern line_t *lines;
extern int numsides;
extern side_t *sides;
extern fixed_t viewx;
extern fixed_t viewy;
extern fixed_t viewz;
extern angle_t viewangle;
extern player_t *viewplayer;
extern angle_t clipangle;
extern int viewangletox[FINEANGLES / 2];
extern angle_t xtoviewangle[MAX_SCREENWIDTH + 1];
extern fixed_t rw_distance;
extern angle_t rw_normalangle;
extern int rw_angle1;
extern visplane_t *floorplane;
extern visplane_t *ceilingplane;

#endif
