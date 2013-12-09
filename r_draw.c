#include "doomstat.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_filter.h"
#include "v_video.h"
#include "st_stuff.h"
#include "g_game.h"
#include "am_map.h"
#include "lprintf.h"

#define FUZZTABLE                       50
#define FUZZOFF                         1
#define RDC_STANDARD                    1
#define RDC_TRANSLUCENT                 2
#define RDC_TRANSLATED                  4
#define RDC_FUZZ                        8
#define RDC_NOCOLMAP                    16
#define RDC_DITHERZ                     32
#define RDC_BILINEAR                    64
#define RDC_ROUNDED                     128

typedef enum
{

    COL_NONE,
    COL_OPAQUE,
    COL_TRANS,
    COL_FLEXTRANS,
    COL_FUZZ,
    COL_FLEXADD

} columntype_e;

byte *viewimage;
int viewwidth;
int scaledviewwidth;
int viewheight;
int viewwindowx;
int viewwindowy;
const byte *tranmap;
const byte *main_tranmap;
static int temp_x = 0;
static int tempyl[4], tempyh[4];
static byte byte_tempbuf[MAX_SCREENHEIGHT * 4];
static unsigned short short_tempbuf[MAX_SCREENHEIGHT * 4];
static unsigned int int_tempbuf[MAX_SCREENHEIGHT * 4];
static int startx = 0;
static int temptype = COL_NONE;
static int commontop, commonbot;
static const byte *temptranmap = NULL;
static const byte *tempfuzzmap;

static const int fuzzoffset_org[FUZZTABLE] = {
    FUZZOFF,-FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,
    FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,
    FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,
    FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF,FUZZOFF,-FUZZOFF,FUZZOFF
};

static int fuzzoffset[FUZZTABLE];
static int fuzzpos = 0;

draw_vars_t drawvars = { 
  NULL, NULL, NULL, 0, 0, 0,
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
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL8
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz15
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz16
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL32
#include "r_drawflush.inl"

#define R_DRAWCOLUMN_PIPELINE RDC_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz32
#include "r_drawflush.inl"

byte *translationtables;

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_STANDARD
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_STANDARD

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLUCENT
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLUCENT

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL8
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTLColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeTL32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTTL32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadTL32
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_TRANSLATED
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_TRANSLATED

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad8
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawTranslatedColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWhole32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHT32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuad32
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

#define R_DRAWCOLUMN_PIPELINE_TYPE RDC_PIPELINE_FUZZ
#define R_DRAWCOLUMN_PIPELINE_BASE RDC_FUZZ

#define R_DRAWCOLUMN_PIPELINE_BITS 8
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn8 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz8
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz8
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz8
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 15
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn15 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz15
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz15
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz15
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 16
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn16 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz16
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz16
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz16
#include "r_drawcolpipeline.inl"

#define R_DRAWCOLUMN_PIPELINE_BITS 32
#define R_DRAWCOLUMN_FUNCNAME_COMPOSITE(postfix) R_DrawFuzzColumn32 ## postfix
#define R_FLUSHWHOLE_FUNCNAME R_FlushWholeFuzz32
#define R_FLUSHHEADTAIL_FUNCNAME R_FlushHTFuzz32
#define R_FLUSHQUAD_FUNCNAME R_FlushQuadFuzz32
#include "r_drawcolpipeline.inl"

#undef R_DRAWCOLUMN_PIPELINE_BASE
#undef R_DRAWCOLUMN_PIPELINE_TYPE

static R_DrawColumn_f drawcolumnfuncs[VID_MODEMAX][RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS][RDC_PIPELINE_MAXPIPELINES] = {
    {
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn8_PointUV, R_DrawTLColumn8_PointUV, R_DrawTranslatedColumn8_PointUV, R_DrawFuzzColumn8_PointUV,},
            {R_DrawColumn8_LinearUV, R_DrawTLColumn8_LinearUV, R_DrawTranslatedColumn8_LinearUV, R_DrawFuzzColumn8_LinearUV,},
            {R_DrawColumn8_RoundedUV, R_DrawTLColumn8_RoundedUV, R_DrawTranslatedColumn8_RoundedUV, R_DrawFuzzColumn8_RoundedUV,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn8_PointUV_PointZ, R_DrawTLColumn8_PointUV_PointZ, R_DrawTranslatedColumn8_PointUV_PointZ, R_DrawFuzzColumn8_PointUV_PointZ,},
            {R_DrawColumn8_LinearUV_PointZ, R_DrawTLColumn8_LinearUV_PointZ, R_DrawTranslatedColumn8_LinearUV_PointZ, R_DrawFuzzColumn8_LinearUV_PointZ,},
            {R_DrawColumn8_RoundedUV_PointZ, R_DrawTLColumn8_RoundedUV_PointZ, R_DrawTranslatedColumn8_RoundedUV_PointZ, R_DrawFuzzColumn8_RoundedUV_PointZ,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn8_PointUV_LinearZ, R_DrawTLColumn8_PointUV_LinearZ, R_DrawTranslatedColumn8_PointUV_LinearZ, R_DrawFuzzColumn8_PointUV_LinearZ,},
            {R_DrawColumn8_LinearUV_LinearZ, R_DrawTLColumn8_LinearUV_LinearZ, R_DrawTranslatedColumn8_LinearUV_LinearZ, R_DrawFuzzColumn8_LinearUV_LinearZ,},
            {R_DrawColumn8_RoundedUV_LinearZ, R_DrawTLColumn8_RoundedUV_LinearZ, R_DrawTranslatedColumn8_RoundedUV_LinearZ, R_DrawFuzzColumn8_RoundedUV_LinearZ,},
        },
    },
    {
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn15_PointUV, R_DrawTLColumn15_PointUV, R_DrawTranslatedColumn15_PointUV, R_DrawFuzzColumn15_PointUV,},
            {R_DrawColumn15_LinearUV, R_DrawTLColumn15_LinearUV, R_DrawTranslatedColumn15_LinearUV, R_DrawFuzzColumn15_LinearUV,},
            {R_DrawColumn15_RoundedUV, R_DrawTLColumn15_RoundedUV, R_DrawTranslatedColumn15_RoundedUV, R_DrawFuzzColumn15_RoundedUV,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn15_PointUV_PointZ, R_DrawTLColumn15_PointUV_PointZ, R_DrawTranslatedColumn15_PointUV_PointZ, R_DrawFuzzColumn15_PointUV_PointZ,},
            {R_DrawColumn15_LinearUV_PointZ, R_DrawTLColumn15_LinearUV_PointZ, R_DrawTranslatedColumn15_LinearUV_PointZ, R_DrawFuzzColumn15_LinearUV_PointZ,},
            {R_DrawColumn15_RoundedUV_PointZ, R_DrawTLColumn15_RoundedUV_PointZ, R_DrawTranslatedColumn15_RoundedUV_PointZ, R_DrawFuzzColumn15_RoundedUV_PointZ,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn15_PointUV_LinearZ, R_DrawTLColumn15_PointUV_LinearZ, R_DrawTranslatedColumn15_PointUV_LinearZ, R_DrawFuzzColumn15_PointUV_LinearZ,},
            {R_DrawColumn15_LinearUV_LinearZ, R_DrawTLColumn15_LinearUV_LinearZ, R_DrawTranslatedColumn15_LinearUV_LinearZ, R_DrawFuzzColumn15_LinearUV_LinearZ,},
            {R_DrawColumn15_RoundedUV_LinearZ, R_DrawTLColumn15_RoundedUV_LinearZ, R_DrawTranslatedColumn15_RoundedUV_LinearZ, R_DrawFuzzColumn15_RoundedUV_LinearZ,},
        },
    },
    {
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn16_PointUV, R_DrawTLColumn16_PointUV, R_DrawTranslatedColumn16_PointUV, R_DrawFuzzColumn16_PointUV,},
            {R_DrawColumn16_LinearUV, R_DrawTLColumn16_LinearUV, R_DrawTranslatedColumn16_LinearUV, R_DrawFuzzColumn16_LinearUV,},
            {R_DrawColumn16_RoundedUV, R_DrawTLColumn16_RoundedUV, R_DrawTranslatedColumn16_RoundedUV, R_DrawFuzzColumn16_RoundedUV,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn16_PointUV_PointZ, R_DrawTLColumn16_PointUV_PointZ, R_DrawTranslatedColumn16_PointUV_PointZ, R_DrawFuzzColumn16_PointUV_PointZ,},
            {R_DrawColumn16_LinearUV_PointZ, R_DrawTLColumn16_LinearUV_PointZ, R_DrawTranslatedColumn16_LinearUV_PointZ, R_DrawFuzzColumn16_LinearUV_PointZ,},
            {R_DrawColumn16_RoundedUV_PointZ, R_DrawTLColumn16_RoundedUV_PointZ, R_DrawTranslatedColumn16_RoundedUV_PointZ, R_DrawFuzzColumn16_RoundedUV_PointZ,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn16_PointUV_LinearZ, R_DrawTLColumn16_PointUV_LinearZ, R_DrawTranslatedColumn16_PointUV_LinearZ, R_DrawFuzzColumn16_PointUV_LinearZ,},
            {R_DrawColumn16_LinearUV_LinearZ, R_DrawTLColumn16_LinearUV_LinearZ, R_DrawTranslatedColumn16_LinearUV_LinearZ, R_DrawFuzzColumn16_LinearUV_LinearZ,},
            {R_DrawColumn16_RoundedUV_LinearZ, R_DrawTLColumn16_RoundedUV_LinearZ, R_DrawTranslatedColumn16_RoundedUV_LinearZ, R_DrawFuzzColumn16_RoundedUV_LinearZ,},
        },
    },
    {
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn32_PointUV, R_DrawTLColumn32_PointUV, R_DrawTranslatedColumn32_PointUV, R_DrawFuzzColumn32_PointUV,},
            {R_DrawColumn32_LinearUV, R_DrawTLColumn32_LinearUV, R_DrawTranslatedColumn32_LinearUV, R_DrawFuzzColumn32_LinearUV,},
            {R_DrawColumn32_RoundedUV, R_DrawTLColumn32_RoundedUV, R_DrawTranslatedColumn32_RoundedUV, R_DrawFuzzColumn32_RoundedUV,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn32_PointUV_PointZ, R_DrawTLColumn32_PointUV_PointZ, R_DrawTranslatedColumn32_PointUV_PointZ, R_DrawFuzzColumn32_PointUV_PointZ,},
            {R_DrawColumn32_LinearUV_PointZ, R_DrawTLColumn32_LinearUV_PointZ, R_DrawTranslatedColumn32_LinearUV_PointZ, R_DrawFuzzColumn32_LinearUV_PointZ,},
            {R_DrawColumn32_RoundedUV_PointZ, R_DrawTLColumn32_RoundedUV_PointZ, R_DrawTranslatedColumn32_RoundedUV_PointZ, R_DrawFuzzColumn32_RoundedUV_PointZ,},
        },
        {
            {NULL, NULL, NULL, NULL,},
            {R_DrawColumn32_PointUV_LinearZ, R_DrawTLColumn32_PointUV_LinearZ, R_DrawTranslatedColumn32_PointUV_LinearZ, R_DrawFuzzColumn32_PointUV_LinearZ,},
            {R_DrawColumn32_LinearUV_LinearZ, R_DrawTLColumn32_LinearUV_LinearZ, R_DrawTranslatedColumn32_LinearUV_LinearZ, R_DrawFuzzColumn32_LinearUV_LinearZ,},
            {R_DrawColumn32_RoundedUV_LinearZ, R_DrawTLColumn32_RoundedUV_LinearZ, R_DrawTranslatedColumn32_RoundedUV_LinearZ, R_DrawFuzzColumn32_RoundedUV_LinearZ,},
        },
    },
};

R_DrawColumn_f R_GetDrawColumnFunc(enum column_pipeline_e type, enum draw_filter_type_e filter, enum draw_filter_type_e filterz)
{

    R_DrawColumn_f result = drawcolumnfuncs[V_GetMode()][filterz][filter][type];

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

byte playernumtotrans[MAXPLAYERS];
extern lighttable_t *(*c_zlight)[LIGHTLEVELS][MAXLIGHTZ];

void R_InitTranslationTables (void)
{
    int i, j;
#define MAXTRANS 3
    byte transtocolour[MAXTRANS];

    if (translationtables == NULL)
        translationtables = Z_Malloc(256 * MAXTRANS, PU_STATIC, 0);

    for (i = 0; i < MAXTRANS; i++)
        transtocolour[i] = 255;

    for (i = 0; i < MAXPLAYERS; i++)
    {

        byte wantcolour = mapcolor_plyr[i];
        playernumtotrans[i] = 0;

        if (wantcolour != 0x70)
        {

            for (j = 0; j < MAXTRANS; j++)
            {

                if (transtocolour[j] == 255)
                {

                    transtocolour[j] = wantcolour;
                    playernumtotrans[i] = j + 1;

                    break;
                }

            }

        }

    }

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
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan8_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 8
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan15_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 15
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan16_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 16
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_PointUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_PointUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_LinearUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_LinearUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_BILINEAR | RDC_DITHERZ)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_RoundedUV_PointZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED)
#include "r_drawspan.inl"

#define R_DRAWSPAN_FUNCNAME R_DrawSpan32_RoundedUV_LinearZ
#define R_DRAWSPAN_PIPELINE_BITS 32
#define R_DRAWSPAN_PIPELINE (RDC_STANDARD | RDC_ROUNDED | RDC_DITHERZ)
#include "r_drawspan.inl"

static R_DrawSpan_f drawspanfuncs[VID_MODEMAX][RDRAW_FILTER_MAXFILTERS][RDRAW_FILTER_MAXFILTERS] = {
    {
        {NULL, NULL, NULL, NULL,},
        {NULL, R_DrawSpan8_PointUV_PointZ, R_DrawSpan8_LinearUV_PointZ, R_DrawSpan8_RoundedUV_PointZ,},
        {NULL, R_DrawSpan8_PointUV_LinearZ, R_DrawSpan8_LinearUV_LinearZ, R_DrawSpan8_RoundedUV_LinearZ,},
        {NULL, NULL, NULL, NULL,},
    },
    {
        {NULL, NULL, NULL, NULL,},
        {NULL, R_DrawSpan15_PointUV_PointZ, R_DrawSpan15_LinearUV_PointZ, R_DrawSpan15_RoundedUV_PointZ,},
        {NULL, R_DrawSpan15_PointUV_LinearZ, R_DrawSpan15_LinearUV_LinearZ, R_DrawSpan15_RoundedUV_LinearZ,},
        {NULL, NULL, NULL, NULL,},
    },
    {
        {NULL, NULL, NULL, NULL,},
        {NULL, R_DrawSpan16_PointUV_PointZ, R_DrawSpan16_LinearUV_PointZ, R_DrawSpan16_RoundedUV_PointZ,},
        {NULL, R_DrawSpan16_PointUV_LinearZ, R_DrawSpan16_LinearUV_LinearZ, R_DrawSpan16_RoundedUV_LinearZ,},
        {NULL, NULL, NULL, NULL,},
    },
    {
        {NULL, NULL, NULL, NULL,},
        {NULL, R_DrawSpan32_PointUV_PointZ, R_DrawSpan32_LinearUV_PointZ, R_DrawSpan32_RoundedUV_PointZ,},
        {NULL, R_DrawSpan32_PointUV_LinearZ, R_DrawSpan32_LinearUV_LinearZ, R_DrawSpan32_RoundedUV_LinearZ,},
        {NULL, NULL, NULL, NULL,},
    },
};

R_DrawSpan_f R_GetDrawSpanFunc(enum draw_filter_type_e filter, enum draw_filter_type_e filterz)
{

    R_DrawSpan_f result = drawspanfuncs[V_GetMode()][filterz][filter];

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
    drawvars.short_topleft = (unsigned short *)(screens[0].data) + viewwindowy*screens[0].short_pitch + viewwindowx;
    drawvars.int_topleft = (unsigned int *)(screens[0].data) + viewwindowy*screens[0].int_pitch + viewwindowx;
    drawvars.byte_pitch = screens[0].byte_pitch;
    drawvars.short_pitch = screens[0].short_pitch;
    drawvars.int_pitch = screens[0].int_pitch;

    if (V_GetMode() == VID_MODE8)
    {

        for (i = 0; i < FUZZTABLE; i++)
            fuzzoffset[i] = fuzzoffset_org[i] * screens[0].byte_pitch;

    }
    
    else if ((V_GetMode() == VID_MODE15) || (V_GetMode() == VID_MODE16))
    {

        for (i = 0; i < FUZZTABLE; i++)
            fuzzoffset[i] = fuzzoffset_org[i] * screens[0].short_pitch;

    }
    
    else if (V_GetMode() == VID_MODE32)
    {

        for (i = 0; i < FUZZTABLE; i++)
            fuzzoffset[i] = fuzzoffset_org[i] * screens[0].int_pitch;

    }
}

void R_FillBackScreen(void)
{

    int x, y;

    if (scaledviewwidth == SCREENWIDTH)
        return;

    V_DrawBackground(gamemode == commercial ? "GRNROCK" : "FLOOR7_2", 1);

    for (x = 0; x < scaledviewwidth; x += 8)
        V_DrawNamePatch(viewwindowx + x, viewwindowy - 8, 1, "brdr_t", CR_DEFAULT, VPT_NONE);

    for (x = 0; x < scaledviewwidth; x += 8)
        V_DrawNamePatch(viewwindowx + x, viewwindowy + viewheight, 1, "brdr_b", CR_DEFAULT, VPT_NONE);

    for (y = 0; y < viewheight; y += 8)
        V_DrawNamePatch(viewwindowx - 8, viewwindowy + y, 1, "brdr_l", CR_DEFAULT, VPT_NONE);

    for (y = 0; y < viewheight; y += 8)
        V_DrawNamePatch(viewwindowx+scaledviewwidth,viewwindowy+y,1,"brdr_r", CR_DEFAULT, VPT_NONE);

    V_DrawNamePatch(viewwindowx - 8,viewwindowy - 8, 1, "brdr_tl", CR_DEFAULT, VPT_NONE);
    V_DrawNamePatch(viewwindowx + scaledviewwidth, viewwindowy - 8, 1, "brdr_tr", CR_DEFAULT, VPT_NONE);
    V_DrawNamePatch(viewwindowx - 8,viewwindowy + viewheight, 1, "brdr_bl", CR_DEFAULT, VPT_NONE);
    V_DrawNamePatch(viewwindowx + scaledviewwidth, viewwindowy + viewheight, 1, "brdr_br", CR_DEFAULT, VPT_NONE);

}

void R_VideoErase(int x, int y, int count)
{

    memcpy(screens[0].data + y * screens[0].byte_pitch + x * V_GetPixelDepth(), screens[1].data+y*screens[1].byte_pitch+x*V_GetPixelDepth(), count * V_GetPixelDepth());

}

void R_DrawViewBorder(void)
{

    int top, side, i;

    if ((SCREENHEIGHT != viewheight) || ((automapmode & am_active) && ! (automapmode & am_overlay)))
    {

        side = (SCREENWIDTH - ST_SCALED_WIDTH) / 2;

        if (side > 0)
        {

            for (i = (SCREENHEIGHT - ST_SCALED_HEIGHT); i < SCREENHEIGHT; i++)
            {

                R_VideoErase (0, i, side);
                R_VideoErase (ST_SCALED_WIDTH+side, i, side);

            }
        }
    }

    if (viewheight >= (SCREENHEIGHT - ST_SCALED_HEIGHT))
        return;

    top = ((SCREENHEIGHT - ST_SCALED_HEIGHT) - viewheight) / 2;
    side = (SCREENWIDTH - scaledviewwidth) / 2;

    for (i = 0; i < top; i++)
        R_VideoErase (0, i, SCREENWIDTH);

    for (i = top; i < (top+viewheight); i++)
    {

        R_VideoErase (0, i, side);
        R_VideoErase (viewwidth+side, i, side);

    }

    for (i = top + viewheight; i < (SCREENHEIGHT - ST_SCALED_HEIGHT); i++)
        R_VideoErase (0, i, SCREENWIDTH);

}

