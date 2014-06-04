#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "d_main.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "r_plane.h"
#include "r_bsp.h"
#include "r_draw.h"
#include "v_video.h"
#include "st_stuff.h"
#include "i_system.h"
#include "i_video.h"
#include "g_game.h"

#define FIELDOFVIEW                     2048
#define KEEPTIMES                       10
#define DISTMAP                         2

int validcount = 1;
const lighttable_t *fixedcolormap;
int centerx, centery;
fixed_t centerxfrac, centeryfrac;
fixed_t viewheightfrac;
fixed_t projection;
fixed_t projectiony;
fixed_t viewx, viewy, viewz;
angle_t viewangle;
fixed_t viewcos, viewsin;
player_t *viewplayer;
extern lighttable_t **walllights;
angle_t clipangle;
int viewangletox[FINEANGLES / 2];
angle_t xtoviewangle[MAX_SCREENWIDTH + 1];
int numcolormaps;
const lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];
const lighttable_t *(*zlight)[MAXLIGHTZ];
const lighttable_t *fullcolormap;
const lighttable_t **colormaps;
int extralight;
int rendered_visplanes, rendered_segs, rendered_vissprites;
boolean rendering_stats;
int skyflatnum;
int skytexture;
int skytexturemid;
static mobj_t *oviewer;

void R_InitSkyMap(void)
{

    skytexturemid = 100 * FRACUNIT;

}

void R_InterpolateView(player_t *player, fixed_t frac)
{

    viewx = player->mo->x;
    viewy = player->mo->y;
    viewz = player->viewz;
    viewangle = player->mo->angle;

}

int R_PointOnSide(fixed_t x, fixed_t y, const node_t *node)
{

    if (!node->dx)
        return x <= node->x ? node->dy > 0 : node->dy < 0;

    if (!node->dy)
        return y <= node->y ? node->dx < 0 : node->dx > 0;

    x -= node->x;
    y -= node->y;


    if ((node->dy ^ node->dx ^ x ^ y) < 0)
        return (node->dy ^ x) < 0;

    return FixedMul(y, node->dx >> FRACBITS) >= FixedMul(node->dy >> FRACBITS, x);

}

int R_PointOnSegSide(fixed_t x, fixed_t y, const seg_t *line)
{

    fixed_t lx = line->v1->x;
    fixed_t ly = line->v1->y;
    fixed_t ldx = line->v2->x - lx;
    fixed_t ldy = line->v2->y - ly;

    if (!ldx)
        return x <= lx ? ldy > 0 : ldy < 0;

    if (!ldy)
        return y <= ly ? ldx < 0 : ldx > 0;

    x -= lx;
    y -= ly;

    if ((ldy ^ ldx ^ x ^ y) < 0)
        return (ldy ^ x) < 0;

    return FixedMul(y, ldx >> FRACBITS) >= FixedMul(ldy >> FRACBITS, x);

}

angle_t R_PointToAngle(fixed_t x, fixed_t y)
{

    static fixed_t oldx, oldy;
    static angle_t oldresult;

    x -= viewx; y -= viewy;

    if ((x < INT_MAX / 4 && x > -INT_MAX / 4 && y < INT_MAX / 4 && y > -INT_MAX / 4))
        return (x || y) ? x >= 0 ? y >= 0 ? (x > y) ? tantoangle[SlopeDiv(y, x)] : ANG90 - 1 - tantoangle[SlopeDiv(x,y)] : x > (y = -y) ? 0 - tantoangle[SlopeDiv(y, x)] : ANG270 + tantoangle[SlopeDiv(x, y)] : y >= 0 ? (x = -x) > y ? ANG180 - 1 - tantoangle[SlopeDiv(y, x)] : ANG90 + tantoangle[SlopeDiv(x, y)] : (x = -x) > (y = -y) ? ANG180 + tantoangle[SlopeDiv(y, x)] : ANG270 - 1 - tantoangle[SlopeDiv(x, y)] : 0;

    if (oldx != x || oldy != y)
    {

        oldx = x;
        oldy = y;
        oldresult = (int)(atan2(y, x) * ANG180 / M_PI);

    }

    return oldresult;

}

angle_t R_PointToAngle2(fixed_t viewx, fixed_t viewy, fixed_t x, fixed_t y)
{

    return (y -= viewy, (x -= viewx) || y) ? x >= 0 ? y >= 0 ? (x > y) ? tantoangle[SlopeDiv(y, x)] : ANG90 - 1 - tantoangle[SlopeDiv(x, y)] : x > (y = -y) ? 0 - tantoangle[SlopeDiv(y, x)] : ANG270 + tantoangle[SlopeDiv(x, y)] : y >= 0 ? (x = -x) > y ? ANG180 - 1 - tantoangle[SlopeDiv(y, x)] : ANG90 + tantoangle[SlopeDiv(x, y)] : (x = -x) > (y = -y) ? ANG180 + tantoangle[SlopeDiv(y, x)] : ANG270 - 1 - tantoangle[SlopeDiv(x, y)] : 0;

}

static void R_InitTextureMapping (void)
{

    register int i, x;
    fixed_t focallength = FixedDiv(centerxfrac, finetangent[FINEANGLES / 4 + FIELDOFVIEW / 2]);

    for (i = 0; i < FINEANGLES / 2; i++)
    {

        int t;

        if (finetangent[i] > FRACUNIT * 2)
        {

            t = -1;

        }

        else if (finetangent[i] < -FRACUNIT * 2)
        {

            t = viewwidth + 1;

        }

        else
        {

            t = FixedMul(finetangent[i], focallength);
            t = (centerxfrac - t + FRACUNIT-1) >> FRACBITS;

            if (t < -1)
                t = -1;
            else if (t > viewwidth + 1)
                t = viewwidth + 1;

        }

        viewangletox[i] = t;

    }

    for (x = 0; x <= viewwidth; x++)
    {

        for (i = 0; viewangletox[i] > x; i++);

        xtoviewangle[x] = (i << ANGLETOFINESHIFT) - ANG90;

    }

    for (i = 0; i < FINEANGLES / 2; i++)
    {

        if (viewangletox[i] == -1)
            viewangletox[i] = 0;
        else if (viewangletox[i] == viewwidth + 1)
            viewangletox[i] = viewwidth;

    }

    clipangle = xtoviewangle[0];

}

static void R_InitLightTables(void)
{

    int i;

    c_zlight = malloc(sizeof (*c_zlight) * numcolormaps);

    for (i = 0; i< LIGHTLEVELS; i++)
    {

        int j, startmap = ((LIGHTLEVELS - 1 - i) * 2) * NUMCOLORMAPS / LIGHTLEVELS;

        for (j = 0; j < MAXLIGHTZ; j++)
        {

            int scale = FixedDiv ((320 / 2 * FRACUNIT), (j + 1) << LIGHTZSHIFT);
            int t, level = startmap - (scale >>= LIGHTSCALESHIFT)/DISTMAP;

            if (level < 0)
                level = 0;
            else if (level >= NUMCOLORMAPS)
                level = NUMCOLORMAPS-1;

            level *= 256;

            for (t = 0; t < numcolormaps; t++)
                c_zlight[t][i][j] = colormaps[t] + level;

        }

    }

}

static void R_InitView()
{

    int i;

    viewwidth = SCREENWIDTH;
    viewheight = SCREENHEIGHT - ST_SCALED_HEIGHT;
    viewheightfrac = viewheight << FRACBITS;
    centery = viewheight / 2;
    centerx = viewwidth / 2;
    centerxfrac = centerx << FRACBITS;
    centeryfrac = centery << FRACBITS;
    projection = centerxfrac;
    projectiony = ((SCREENHEIGHT * centerx * 320) / 200) / SCREENWIDTH * FRACUNIT;

    R_InitBuffer(viewwidth, viewheight);
    R_InitTextureMapping();

    pspritescale = FRACUNIT * viewwidth / 320;
    pspriteiscale = FRACUNIT * 320 / viewwidth;
    pspriteyscale = (((SCREENHEIGHT * viewwidth) / SCREENWIDTH) << FRACBITS) / 200;

    for (i = 0; i < viewwidth; i++)
        screenheightarray[i] = viewheight;

    for (i = 0; i < viewheight; i++)
    {

        fixed_t dy = D_abs(((i - viewheight / 2) << FRACBITS) + FRACUNIT / 2);
        yslope[i] = FixedDiv(projectiony, dy);

    }

    for (i = 0; i < viewwidth; i++)
    {

        fixed_t cosadj = D_abs(finecosine[xtoviewangle[i] >> ANGLETOFINESHIFT]);
        distscale[i] = FixedDiv(FRACUNIT, cosadj);

    }

}

void R_Init(void)
{

    R_LoadTrigTables();
    R_InitData();
    R_InitView();
    R_InitLightTables();
    R_InitSkyMap();
    R_InitTranslationTables();
    R_InitPatches();

}

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{

    int nodenum = numnodes - 1;

    if (numnodes == 0)
        return subsectors;

    while (!(nodenum & NF_SUBSECTOR))
        nodenum = nodes[nodenum].children[R_PointOnSide(x, y, nodes + nodenum)];

    return &subsectors[nodenum & ~NF_SUBSECTOR];

}

static void R_SetupFrame(player_t *player)
{

    int cm;
    boolean NoInterpolate = menuactive;

    viewplayer = player;

    if (player->mo != oviewer || NoInterpolate)
        oviewer = player->mo;

    tic_vars.frac = I_GetTimeFrac();

    if (NoInterpolate)
        tic_vars.frac = FRACUNIT;

    R_InterpolateView(player, tic_vars.frac);

    extralight = player->extralight;
    viewsin = finesine[viewangle >> ANGLETOFINESHIFT];
    viewcos = finecosine[viewangle >> ANGLETOFINESHIFT];

    if (player->mo->subsector->sector->heightsec != -1)
    {

        const sector_t *s = player->mo->subsector->sector->heightsec + sectors;

        cm = viewz < s->floorheight ? s->bottommap : viewz > s->ceilingheight ? s->topmap : s->midmap;

        if (cm < 0 || cm > numcolormaps)
            cm = 0;

    }

    else
    {

        cm = 0;

    }

    fullcolormap = colormaps[cm];
    zlight = c_zlight[cm];

    if (player->fixedcolormap)
    {

        fixedcolormap = fullcolormap + player->fixedcolormap * 256 * sizeof (lighttable_t);

    }

    else
    {

        fixedcolormap = 0;

    }

    validcount++;

}

static void R_ShowStats(void)
{

    static int keeptime[KEEPTIMES], showtime;
    int now = I_GetTime();

    if (now - showtime > 35)
    {

        I_Print("Frame rate %d fps\nSegs %d, Visplanes %d, Sprites %d", (35*KEEPTIMES)/(now - keeptime[0]), rendered_segs, rendered_visplanes, rendered_vissprites);

        showtime = now;

    }

    memmove(keeptime, keeptime + 1, sizeof (keeptime[0]) * (KEEPTIMES - 1));
    keeptime[KEEPTIMES - 1] = now;

}

void R_RenderPlayerView(player_t *player)
{

    R_SetupFrame(player);
    R_ClearClipSegs();
    R_ClearDrawSegs();
    R_ClearPlanes();
    R_ClearSprites();

    rendered_segs = rendered_visplanes = 0;

    R_RenderBSPNode(numnodes - 1);
    R_ResetColumnBuffer();
    R_DrawPlanes();
    R_DrawMasked();
    R_ResetColumnBuffer();

    if (rendering_stats)
        R_ShowStats();

}

