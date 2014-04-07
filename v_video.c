#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "r_main.h"
#include "r_draw.h"
#include "r_filter.h"
#include "v_video.h"
#include "i_system.h"
#include "i_video.h"

screeninfo_t screens[NUM_SCREENS];

const byte *colrngs[CR_LIMIT];
static int currentPaletteIndex = 0;

static void FUNC_V_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, enum patch_translation_e flags)
{

    byte *src;
    byte *dest;

    if (flags & VPT_STRETCH)
    {

        srcx = srcx * SCREENWIDTH / 320;
        srcy = srcy * SCREENHEIGHT / 200;
        width = width * SCREENWIDTH / 320;
        height = height * SCREENHEIGHT / 200;
        destx = destx * SCREENWIDTH / 320;
        desty = desty * SCREENHEIGHT / 200;

    }

    src = screens[srcscrn].data + screens[srcscrn].byte_pitch * srcy + srcx;
    dest = screens[destscrn].data + screens[destscrn].byte_pitch * desty + destx;

    for (; height > 0; height--)
    {

        memcpy(dest, src, width);

        src += screens[srcscrn].byte_pitch;
        dest += screens[destscrn].byte_pitch;

    }

}

static void FUNC_V_DrawBackground(const char* flatname, int scrn)
{

    int x, y;
    int width = 64;
    int height = 64;
    int lump;
    const byte *src = W_CacheLumpNum(lump = firstflat + R_FlatNumForName(flatname));
    byte *dest = screens[scrn].data;

    while (height--)
    {

        memcpy(dest, src, width);

        src += width;
        dest += screens[scrn].byte_pitch;

    }

    for (y = 0; y < SCREENHEIGHT; y += 64)
    {

        for (x = y ? 0 : 64; x < SCREENWIDTH; x += 64)
            V_CopyRect(0, 0, scrn, ((SCREENWIDTH - x) < 64) ? (SCREENWIDTH - x) : 64, ((SCREENHEIGHT - y) < 64) ? (SCREENHEIGHT - y) : 64, x, y, scrn, VPT_NONE);

    }

    W_UnlockLumpNum(lump);

}

void V_Init(void)
{

    int i;

    for (i = 0; i<NUM_SCREENS; i++)
    {

        screens[i].data = NULL;
        screens[i].not_on_heap = false;
        screens[i].width = 0;
        screens[i].height = 0;
        screens[i].byte_pitch = 0;
        screens[i].short_pitch = 0;
        screens[i].int_pitch = 0;

    }

}

static void V_DrawMemPatch(int x, int y, int scrn, const rpatch_t *patch, int cm, enum patch_translation_e flags)
{

    const byte *trans;

    if (cm < CR_LIMIT)
        trans = colrngs[cm];
    else
        trans = translationtables + 256 * ((cm - CR_LIMIT) - 1);

    y -= patch->topoffset;
    x -= patch->leftoffset;

    if (flags & VPT_STRETCH)
        if ((SCREENWIDTH == 320) && (SCREENHEIGHT == 200))
            flags &= ~VPT_STRETCH;

    if (!trans)
        flags &= ~VPT_TRANS;

    if (!(flags & VPT_STRETCH))
    {

        int col;
        byte *desttop = screens[scrn].data+y*screens[scrn].byte_pitch+x;
        unsigned int w = patch->width;

        if (y < 0 || y + patch->height > ((flags & VPT_STRETCH) ? 200 : SCREENHEIGHT))
        {

            I_Error("V_DrawMemPatch8: Patch (%d,%d)-(%d,%d) exceeds LFB in vertical direction (horizontal is clipped)\nBad V_DrawMemPatch8 (flags=%u)", x, y, x + patch->width, y+patch->height, flags);

            return;

        }

        w--;

        for (col = 0; (unsigned int)col<=w; desttop++, col++, x++)
        {

            int i;
            const int colindex = (flags & VPT_FLIP) ? (w - col) : (col);
            const rcolumn_t *column = R_GetPatchColumn(patch, colindex);

            if (x < 0)
                continue;

            if (x >= SCREENWIDTH)
                break;

            for (i = 0; i < column->numPosts; i++)
            {

                const rpost_t *post = &column->posts[i];
                const byte *source = column->pixels + post->topdelta;
                byte *dest = desttop + post->topdelta*screens[scrn].byte_pitch;
                int count = post->length;

                if (!(flags & VPT_TRANS))
                {

                    if ((count -= 4) >= 0)
                    {

                        do
                        {

                            register byte s0 = source[0];
                            register byte s1 = source[1];

                            dest[0] = s0;
                            dest[screens[scrn].byte_pitch] = s1;
                            dest += screens[scrn].byte_pitch * 2;
                            s0 = source[2];
                            s1 = source[3];
                            source += 4;
                            dest[0] = s0;
                            dest[screens[scrn].byte_pitch] = s1;
                            dest += screens[scrn].byte_pitch * 2;

                        } while ((count -= 4) >= 0);

                    }

                    if (count += 4)
                    {

                        do
                        {

                            *dest = *source++;
                            dest += screens[scrn].byte_pitch;

                        } while (--count);

                    }

                }
                
                else
                {

                    if ((count -= 4) >= 0)
                    {

                        do
                        {

                            register byte s0 = trans[source[0]];
                            register byte s1 = trans[source[1]];

                            dest[0] = s0;
                            dest[screens[scrn].byte_pitch] = s1;
                            dest += screens[scrn].byte_pitch * 2;
                            s0 = trans[source[2]];
                            s1 = trans[source[3]];
                            source += 4;
                            dest[0] = s0;
                            dest[screens[scrn].byte_pitch] = s1;
                            dest += screens[scrn].byte_pitch * 2;

                        } while ((count -= 4) >= 0);

                    }

                    if (count+=4)
                    {

                        do
                        {

                            *dest = trans[*source++];
                            dest += screens[scrn].byte_pitch;

                        } while (--count);

                    }

                }

            }

        }

    }

    else
    {

        int col;
        int w = (patch->width << 16) - 1;
        int left, right, top, bottom;
        int DX = (SCREENWIDTH << 16) / 320;
        int DXI = (320 << 16) / SCREENWIDTH;
        int DY = (SCREENHEIGHT << 16) / 200;
        int DYI = (200 << 16) / SCREENHEIGHT;

        R_DrawColumn_f colfunc;
        draw_column_vars_t dcvars;
        draw_vars_t olddrawvars = drawvars;

        R_SetDefaultDrawColumnVars(&dcvars);

        drawvars.byte_topleft = screens[scrn].data;
        drawvars.byte_pitch = screens[scrn].byte_pitch;

        if (!(flags & VPT_STRETCH))
        {

            DX = 1 << 16;
            DXI = 1 << 16;
            DY = 1 << 16;
            DYI = 1 << 16;

        }

        if (flags & VPT_TRANS)
        {

            colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_TRANSLATED, drawvars.filterpatch, RDRAW_FILTER_NONE);
            dcvars.translation = trans;

        }
        
        else
        {

            colfunc = R_GetDrawColumnFunc(RDC_PIPELINE_STANDARD, drawvars.filterpatch, RDRAW_FILTER_NONE);

        }

        left = (x * DX) >> FRACBITS;
        top = (y * DY) >> FRACBITS;
        right = ((x + patch->width) * DX) >> FRACBITS;
        bottom = ((y + patch->height) * DY) >> FRACBITS;
        dcvars.texheight = patch->height;
        dcvars.iscale = DYI;
        dcvars.drawingmasked = MAX(patch->width, patch->height) > 8;
        dcvars.edgetype = drawvars.patch_edges;

        if (drawvars.filterpatch == RDRAW_FILTER_LINEAR)
        {

            if (patch->isNotTileable)
                col = -(FRACUNIT >> 1);
            else
                col = (patch->width << FRACBITS) - (FRACUNIT >> 1);

        }

        else
        {

            col = 0;

        }

        for (dcvars.x = left; dcvars.x < right; dcvars.x++, col += DXI)
        {

            int i;
            const int colindex = (flags & VPT_FLIP) ? ((w - col) >> 16) : (col >> 16);
            const rcolumn_t *column = R_GetPatchColumn(patch, colindex);
            const rcolumn_t *prevcolumn = R_GetPatchColumn(patch, colindex - 1);
            const rcolumn_t *nextcolumn = R_GetPatchColumn(patch, colindex + 1);

            if (dcvars.x < 0)
                continue;

            if (dcvars.x >= SCREENWIDTH)
                break;

            dcvars.texu = ((flags & VPT_FLIP) ? ((patch->width << FRACBITS) - col) : col) % (patch->width << FRACBITS);

            for (i = 0; i < column->numPosts; i++)
            {

                const rpost_t *post = &column->posts[i];
                int yoffset = 0;

                dcvars.yl = (((y + post->topdelta) * DY) >> FRACBITS);
                dcvars.yh = (((y + post->topdelta + post->length) * DY - (FRACUNIT >> 1)) >> FRACBITS);
                dcvars.edgeslope = post->slope;

                if ((dcvars.yh < 0) || (dcvars.yh < top))
                    continue;

                if ((dcvars.yl >= SCREENHEIGHT) || (dcvars.yl >= bottom))
                    continue;

                if (dcvars.yh >= bottom)
                {

                    dcvars.yh = bottom - 1;
                    dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;

                }

                if (dcvars.yh >= SCREENHEIGHT)
                {

                    dcvars.yh = SCREENHEIGHT - 1;
                    dcvars.edgeslope &= ~RDRAW_EDGESLOPE_BOT_MASK;

                }

                if (dcvars.yl < 0)
                {

                    yoffset = 0 - dcvars.yl;
                    dcvars.yl = 0;
                    dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;
                }

                if (dcvars.yl < top)
                {

                    yoffset = top - dcvars.yl;
                    dcvars.yl = top;
                    dcvars.edgeslope &= ~RDRAW_EDGESLOPE_TOP_MASK;

                }

                dcvars.source = column->pixels + post->topdelta + yoffset;
                dcvars.prevsource = prevcolumn ? prevcolumn->pixels + post->topdelta + yoffset : dcvars.source;
                dcvars.nextsource = nextcolumn ? nextcolumn->pixels + post->topdelta + yoffset : dcvars.source;
                dcvars.texturemid = -((dcvars.yl - centery) * dcvars.iscale);

                colfunc(&dcvars);

            }

        }

        R_ResetColumnBuffer();

        drawvars = olddrawvars;

    }

}

static void FUNC_V_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags)
{

    V_DrawMemPatch(x, y, scrn, R_CachePatchNum(lump), cm, flags);
    R_UnlockPatchNum(lump);

}

void V_SetPalette(int pal)
{

    currentPaletteIndex = pal;

    I_SetPalette(pal);

}

static void V_FillRect8(int scrn, int x, int y, int width, int height, byte colour)
{

    byte* dest = screens[scrn].data + x + y*screens[scrn].byte_pitch;

    while (height--)
    {

        memset(dest, colour, width);

        dest += screens[scrn].byte_pitch;

    }

}

static void WRAP_V_DrawLine(fline_t* fl, int color);
static void V_PlotPixel8(int scrn, int x, int y, byte color);

static void NULL_FillRect(int scrn, int x, int y, int width, int height, byte colour) {}
static void NULL_CopyRect(int srcx, int srcy, int srcscrn, int width, int height, int destx, int desty, int destscrn, enum patch_translation_e flags) {}
static void NULL_DrawBackground(const char *flatname, int n) {}
static void NULL_DrawNumPatch(int x, int y, int scrn, int lump, int cm, enum patch_translation_e flags) {}
static void NULL_PlotPixel(int scrn, int x, int y, byte color) {}
static void NULL_DrawLine(fline_t* fl, int color) {}

V_CopyRect_f V_CopyRect = NULL_CopyRect;
V_FillRect_f V_FillRect = NULL_FillRect;
V_DrawNumPatch_f V_DrawNumPatch = NULL_DrawNumPatch;
V_DrawBackground_f V_DrawBackground = NULL_DrawBackground;
V_PlotPixel_f V_PlotPixel = NULL_PlotPixel;
V_DrawLine_f V_DrawLine = NULL_DrawLine;

void V_InitMode()
{

    V_CopyRect = FUNC_V_CopyRect;
    V_FillRect = V_FillRect8;
    V_DrawNumPatch = FUNC_V_DrawNumPatch;
    V_DrawBackground = FUNC_V_DrawBackground;
    V_PlotPixel = V_PlotPixel8;
    V_DrawLine = WRAP_V_DrawLine;
    R_FilterInit();

}

void V_AllocScreen(screeninfo_t *scrn)
{

    if (!scrn->not_on_heap)
        if ((scrn->byte_pitch * scrn->height) > 0)
            scrn->data = malloc(scrn->byte_pitch * scrn->height);

}

void V_AllocScreens(void)
{

    int i;

    for (i = 0; i < NUM_SCREENS; i++)
        V_AllocScreen(&screens[i]);

}

void V_FreeScreen(screeninfo_t *scrn)
{

    if (!scrn->not_on_heap)
    {

        free(scrn->data);

        scrn->data = NULL;

    }

}

void V_FreeScreens(void)
{

    int i;

    for (i = 0; i < NUM_SCREENS; i++)
        V_FreeScreen(&screens[i]);

}

static void V_PlotPixel8(int scrn, int x, int y, byte color)
{

    screens[scrn].data[x + screens[scrn].byte_pitch * y] = color;

}

static void WRAP_V_DrawLine(fline_t* fl, int color)
{

    register int x;
    register int y;
    register int dx;
    register int dy;
    register int sx;
    register int sy;
    register int ax;
    register int ay;
    register int d;

    #define PUTDOT(xx,yy,cc) V_PlotPixel(0,xx,yy,(byte)cc)

    dx = fl->b.x - fl->a.x;
    ax = 2 * (dx < 0 ? -dx : dx);
    sx = dx < 0 ? -1 : 1;
    dy = fl->b.y - fl->a.y;
    ay = 2 * (dy < 0 ? -dy : dy);
    sy = dy < 0 ? -1 : 1;
    x = fl->a.x;
    y = fl->a.y;

    if (ax > ay)
    {

        d = ay - ax / 2;

        while (1)
        {

            PUTDOT(x, y, color);

            if (x == fl->b.x)
                return;

            if (d >= 0)
            {

                y += sy;
                d -= ax;

            }

            x += sx;
            d += ay;

        }
    }

    else
    {

        d = ax - ay / 2;

        while (1)
        {

            PUTDOT(x, y, color);

            if (y == fl->b.y)
                return;

            if (d >= 0)
            {

                x += sx;
                d -= ay;

            }

            y += sy;
            d += ax;

        }

    }

}

