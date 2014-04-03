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
extern fixed_t viewcos;
extern fixed_t viewsin;
extern int viewwidth;
extern int viewheight;
extern int viewwindowx;
extern int viewwindowy;
extern int centerx;
extern int centery;
extern fixed_t centerxfrac;
extern fixed_t centeryfrac;
extern fixed_t viewheightfrac;
extern fixed_t projection;
extern fixed_t projectiony;
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
void R_InterpolateView(player_t *player, fixed_t frac);
int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node);
int R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t *line);
angle_t R_PointToAngle(fixed_t x, fixed_t y);
angle_t R_PointToAngle2(fixed_t x1, fixed_t y1, fixed_t x2, fixed_t y2);
subsector_t *R_PointInSubsector(fixed_t x, fixed_t y);
void R_RenderPlayerView(player_t *player);
void R_Init(void);

#endif
