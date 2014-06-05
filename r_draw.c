#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_filter.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "z_zone.h"
#include "i_system.h"
#include "i_video.h"

#define FUZZTABLE                       50
#define FUZZOFF                         1
#define RDC_STANDARD                    1
#define RDC_TRANSLATED                  4
#define RDC_FUZZ                        8
#define RDC_NOCOLMAP                    16
#define RDC_DITHERZ                     32
#define RDC_BILINEAR                    64
#define RDC_ROUNDED                     128

typedef enum
{

    COL_NONE,

} columntype_e;

int viewwidth;
int viewheight;
int viewwindowx;
int viewwindowy;
byte *translationtables;
extern lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];

static int temp_x = 0;
static int tempyl[4], tempyh[4];
static byte byte_tempbuf[MAX_SCREENHEIGHT * 4];
static int startx = 0;
static int temptype = COL_NONE;
static int commontop, commonbot;
static const byte *tempfuzzmap;

static const int fuzzoffset_org[FUZZTABLE] = {
    FUZZOFF, -FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF,
    FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF,
    FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF, FUZZOFF, -FUZZOFF, -FUZZOFF,
    FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF,
    FUZZOFF, -FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, -FUZZOFF, -FUZZOFF,
    -FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, FUZZOFF, -FUZZOFF, FUZZOFF, FUZZOFF,
    -FUZZOFF, FUZZOFF
};

static int fuzzoffset[FUZZTABLE];
static int fuzzpos = 0;

draw_vars_t drawvars = { 
    NULL, 0,
    RDRAW_FILTER_POINT,
    RDRAW_FILTER_POINT,
    RDRAW_FILTER_POINT,
    RDRAW_FILTER_POINT,
    RDRAW_FILTER_POINT,
    RDRAW_MASKEDCOLUMNEDGE_SQUARE,
    RDRAW_MASKEDCOLUMNEDGE_SQUARE,
    49152
};

static void R_FlushWholeError(void)
{

    I_Error("R_FlushWholeColumns called without being initialized.\n");

}

static void R_FlushHTError(void)
{

    I_Error("R_FlushHTColumns called without being initialized.\n");

}

static void R_QuadFlushError(void)
{

    I_Error("R_FlushQuadColumn called without being initialized.\n");

}

static void (*R_FlushWholeColumns)(void) = R_FlushWholeError;
static void (*R_FlushHTColumns)(void) = R_FlushHTError;
static void (*R_FlushQuadColumn)(void) = R_QuadFlushError;

static void R_FlushColumns(void)
{

    if (temp_x != 4 || commontop >= commonbot)
    {

        R_FlushWholeColumns();

    }

    else
    {

        R_FlushHTColumns();
        R_FlushQuadColumn();

    }

    temp_x = 0;

}

void R_ResetColumnBuffer(void)
{

    if (temp_x)
        R_FlushColumns();

    temptype = COL_NONE;
    R_FlushWholeColumns = R_FlushWholeError;
    R_FlushHTColumns = R_FlushHTError;
    R_FlushQuadColumn = R_QuadFlushError;
}

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawflush.inl"
#undef R_DRAWCOLUMN_PIPELINE
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawflush.inl"
#undef R_DRAWCOLUMN_PIPELINE
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
#undef R_DRAWCOLUMN_FUNCNAME_COMPOSITE
#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLATED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
#undef R_DRAWCOLUMN_FUNCNAME_COMPOSITE
#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawcolpipeline.inl"
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
#undef R_DRAWCOLUMN_FUNCNAME_COMPOSITE
#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS][RDC_PIPELINE_MAXPIPELINES] = {
    {
        {NULL, NULL, NULL,},
        {R_DrawColumn8_PointUV, R_DrawTranslatedColumn8_PointUV, R_DrawFuzzColumn8_PointUV,},
        {R_DrawColumn8_LinearUV, R_DrawTranslatedColumn8_LinearUV, R_DrawFuzzColumn8_LinearUV,},
        {R_DrawColumn8_RoundedUV, R_DrawTranslatedColumn8_RoundedUV, R_DrawFuzzColumn8_RoundedUV,},
    },
    {
        {NULL, NULL, NULL,},
        {R_DrawColumn8_PointUV_PointZ, R_DrawTranslatedColumn8_PointUV_PointZ, R_DrawFuzzColumn8_PointUV_PointZ,},
        {R_DrawColumn8_LinearUV_PointZ, R_DrawTranslatedColumn8_LinearUV_PointZ, R_DrawFuzzColumn8_LinearUV_PointZ,},
        {R_DrawColumn8_RoundedUV_PointZ, R_DrawTranslatedColumn8_RoundedUV_PointZ, R_DrawFuzzColumn8_RoundedUV_PointZ,},
    },
    {
        {NULL, NULL, NULL,},
        {R_DrawColumn8_PointUV_LinearZ, R_DrawTranslatedColumn8_PointUV_LinearZ, R_DrawFuzzColumn8_PointUV_LinearZ,},
        {R_DrawColumn8_LinearUV_LinearZ, R_DrawTranslatedColumn8_LinearUV_LinearZ, R_DrawFuzzColumn8_LinearUV_LinearZ,},
        {R_DrawColumn8_RoundedUV_LinearZ, R_DrawTranslatedColumn8_RoundedUV_LinearZ, R_DrawFuzzColumn8_RoundedUV_LinearZ,},
    },
};

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filter, enum draw_filter_type_e filterz)
{

    R_DrawColumn_f result = drawcolumnfuncs[filterz][filter][type];

    if (result == NULL)
        I_Error("R_GetDrawColumnFunc: undefined function (%d, %d, %d)", type, filter, filterz);

    return result;

}

void R_SetDefaultDrawColumnVars(draw_column_vars_t *dcvars)
{

    dcvars->x = dcvars->yl = dcvars->yh = dcvars->z = 0;
    dcvars->iscale = dcvars->texturemid = dcvars->texheight = dcvars->texu = 0;
    dcvars->source = dcvars->prevsource = dcvars->nextsource = NULL;
    dcvars->colormap = dcvars->nextcolormap = colormaps[0];
    dcvars->translation = NULL;
    dcvars->edgeslope = dcvars->drawingmasked = 0;
    dcvars->edgetype = drawvars.sprite_edges;

}

void R_InitTranslationTables(void)
{
    int i;
#define MAXTRANS 3
    byte transtocolour[MAXTRANS];

    if (translationtables == NULL)
        translationtables = Z_Malloc(256 * MAXTRANS, PU_STATIC, 0);

    for (i = 0; i < MAXTRANS; i++)
        transtocolour[i] = 255;

    for (i = 0; i < 256; i++)
    {

        if (i >= 0x70 && i<= 0x7f)
        {

            translationtables[i] = colormaps[0][((i & 0xf) << 9) + transtocolour[0]];
            translationtables[i + 256] = colormaps[0][((i & 0xf) << 9) + transtocolour[1]];
            translationtables[i + 512] = colormaps[0][((i & 0xf) << 9) + transtocolour[2]];

        }

        else
        {

            translationtables[i] = translationtables[i + 256] = translationtables[i + 512] = i;

        }

    }

}

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"
#undef R_DRAWSPAN_PIPELINE
#undef R_DRAWSPAN_FUNCNAME


static R_DrawSpan_f drawspanfuncs[RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS] = {
    {NULL, NULL, NULL, NULL,},
    {NULL, R_DrawSpan8_PointUV_PointZ, R_DrawSpan8_LinearUV_PointZ, R_DrawSpan8_RoundedUV_PointZ,},
    {NULL, R_DrawSpan8_PointUV_LinearZ, R_DrawSpan8_LinearUV_LinearZ, R_DrawSpan8_RoundedUV_LinearZ,},
    {NULL, NULL, NULL, NULL,},
};

R_DrawSpan_f R_GetDrawSpanFunc(enum draw_filter_type_e filter, enum draw_filter_type_e filterz)
{

    R_DrawSpan_f result = drawspanfuncs[filterz][filter];

    if (result == NULL)
        I_Error("R_GetDrawSpanFunc: undefined function (%d, %d)", filter, filterz);

    return result;

}

void R_DrawSpan(draw_span_vars_t *dsvars)
{

    R_GetDrawSpanFunc(drawvars.filterfloor, drawvars.filterz)(dsvars);

}

void R_InitBuffer(int width, int height)
{

    int i = 0;

    viewwindowx = (SCREENWIDTH-width) >> 1;
    viewwindowy = width==SCREENWIDTH ? 0 : (SCREENHEIGHT-(ST_SCALED_HEIGHT-1)-height)>>1;

    drawvars.byte_topleft = screens[0].data + viewwindowy*screens[0].byte_pitch + viewwindowx;
    drawvars.byte_pitch = screens[0].byte_pitch;

    for (i = 0; i < FUZZTABLE; i++)
        fuzzoffset[i] = fuzzoffset_org[i] * screens[0].byte_pitch;

}

void R_VideoErase(int x, int y, int count)
{

    memcpy(screens[0].data + y * screens[0].byte_pitch + x, screens[1].data + y * screens[1].byte_pitch + x, count);

}

