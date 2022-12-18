#ifndef __R_MAIN__
#define __R_MAIN__

#include "d_player.h"
#include "r_data.h"

#define LIGHTLEVELS                     16
#define LIGHTSEGSHIFT                   4
#define MAXLIGHTSCALE                   48
#define LIGHTSCALESHIFT                 12
#define MAXLIGHTZ                       128
#define LIGHTZSHIFT                     20
#define NUMCOLORMAPS                    32

extern int skytexture;
extern int skytexturemid;
extern int viewcos;
extern int viewsin;
extern int viewwidth;
extern int viewheight;
extern int viewwindowx;
extern int viewwindowy;
extern int centerx;
extern int centery;
extern int centerxfrac;
extern int centeryfrac;
extern int viewheightfrac;
extern int projection;
extern int projectiony;
extern int validcount;
extern int rendered_visplanes, rendered_segs, rendered_vissprites;
extern boolean rendering_stats;
extern const lighttable_t *(*zlight)[MAXLIGHTZ];
extern const lighttable_t *fullcolormap;
extern int numcolormaps;
extern const lighttable_t **colormaps;
extern int extralight;
extern const lighttable_t *fixedcolormap;

void R_InitSkyMap(void);
void R_InterpolateView(player_t *player, int frac);
int R_PointOnSide(int x, int y, const node_t *node);
int R_PointOnSegSide(int x, int y, const seg_t *line);
angle_t R_PointToAngle(int x, int y);
angle_t R_PointToAngle2(int x1, int y1, int x2, int y2);
subsector_t *R_PointInSubsector(int x, int y);
void R_RenderPlayerView(player_t *player);
void R_Init(void);

#endif
