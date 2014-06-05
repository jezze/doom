#if (R_DRAWCOLUMN_PIPELINE & RDC_TRANSLATED)
#define GETCOL8_MAPPED(col) (translation[(col)])
#else
#define GETCOL8_MAPPED(col) (col)
#endif

#if (R_DRAWCOLUMN_PIPELINE & RDC_NOCOLMAP)
#define GETCOL8_DEPTH(col) GETCOL8_MAPPED(col)
#else
#if (R_DRAWCOLUMN_PIPELINE & RDC_DITHERZ)  
#define GETCOL8_DEPTH(col) (dither_colormaps[filter_getDitheredPixelLevel(x, y, fracz)][GETCOL8_MAPPED(col)])
#else
#define GETCOL8_DEPTH(col) colormap[GETCOL8_MAPPED(col)]
#endif
#endif

#if (R_DRAWCOLUMN_PIPELINE & RDC_BILINEAR)
#define GETCOL8(frac, nextfrac) GETCOL8_DEPTH(filter_getDitheredForColumn(x,y,frac,nextfrac))
#elif (R_DRAWCOLUMN_PIPELINE & RDC_ROUNDED)
#define GETCOL8(frac, nextfrac) GETCOL8_DEPTH(filter_getRoundedForColumn(frac,nextfrac))
#else
#define GETCOL8(frac, nextfrac) GETCOL8_DEPTH(source[(frac) >> FRACBITS])
#endif

#if (R_DRAWCOLUMN_PIPELINE & (RDC_BILINEAR | RDC_ROUNDED | RDC_DITHERZ))
#define INCY(y) (y++)
#else
#define INCY(y)
#endif

#define COLTYPE (COL_NONE)

#define GETCOL(frac, nextfrac) GETCOL8(frac, nextfrac)

static void R_DRAWCOLUMN_FUNCNAME(draw_column_vars_t *dcvars)
{

    int count;
    byte *dest;
    fixed_t frac;
    const fixed_t fracstep = dcvars->iscale;
    const fixed_t slope_texu = dcvars->texu;

#if (R_DRAWCOLUMN_PIPELINE & (RDC_BILINEAR | RDC_ROUNDED))
    if (dcvars->iscale > drawvars.mag_threshold)
    {

        R_GetDrawColumnFunc(R_DRAWCOLUMN_PIPELINE_TYPE, RDRAW_FILTER_POINT, drawvars.filterz)(dcvars);

        return;

    }
#endif

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
    if (!dcvars->yl)
        dcvars->yl = 1;

    if (dcvars->yh == viewheight-1)
        dcvars->yh = viewheight - 2;
#endif

    count = dcvars->yh - dcvars->yl;

    if (count < 0)
        return;

  #if (R_DRAWCOLUMN_PIPELINE & RDC_BILINEAR)
    frac = dcvars->texturemid - (FRACUNIT >> 1) + (dcvars->yl - centery) * fracstep;
  #else
    frac = dcvars->texturemid + (dcvars->yl - centery) * fracstep;
  #endif

    if (dcvars->drawingmasked && dcvars->edgetype == RDRAW_MASKEDCOLUMNEDGE_SLOPED)
    {

        if (dcvars->yl != 0)
        {

            if (dcvars->edgeslope & RDRAW_EDGESLOPE_TOP_UP)
            {

                int shift = ((0xffff - (slope_texu & 0xffff)) / dcvars->iscale);

                dcvars->yl += shift;
                count -= shift;
                frac += 0xffff - (slope_texu & 0xffff);

            }

            else if (dcvars->edgeslope & RDRAW_EDGESLOPE_TOP_DOWN)
            {

                int shift = ((slope_texu & 0xffff) / dcvars->iscale);

                dcvars->yl += shift;
                count -= shift;
                frac += slope_texu & 0xffff;

            }
        }

        if (dcvars->yh != viewheight - 1)
        {

            if (dcvars->edgeslope & RDRAW_EDGESLOPE_BOT_UP)
            {

                int shift = ((0xffff - (slope_texu & 0xffff)) / dcvars->iscale);

                dcvars->yh -= shift;
                count -= shift;

            }

            else if (dcvars->edgeslope & RDRAW_EDGESLOPE_BOT_DOWN)
            {

                int shift = ((slope_texu & 0xffff) / dcvars->iscale);

                dcvars->yh -= shift;
                count -= shift;

            }
        }

        if (count <= 0)
            return;

    }
  
    if (temp_x == 4 || (temp_x && (temptype != COLTYPE || temp_x + startx != dcvars->x)))
        R_FlushColumns();

    if (!temp_x)
    {

        startx = dcvars->x;
        tempyl[0] = commontop = dcvars->yl;
        tempyh[0] = commonbot = dcvars->yh;
        temptype = COLTYPE;
#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
        tempfuzzmap = fullcolormap;
#endif
        R_FlushWholeColumns = R_FLUSHWHOLE_FUNCNAME;
        R_FlushHTColumns = R_FLUSHHEADTAIL_FUNCNAME;
        R_FlushQuadColumn = R_FLUSHQUAD_FUNCNAME;
        dest = &byte_tempbuf[dcvars->yl << 2];

    }
    
    else
    {

        tempyl[temp_x] = dcvars->yl;
        tempyh[temp_x] = dcvars->yh;
   
        if (dcvars->yl > commontop)
            commontop = dcvars->yl;
        if (dcvars->yh < commonbot)
            commonbot = dcvars->yh;

        dest = &byte_tempbuf[(dcvars->yl << 2) + temp_x];

    }

    temp_x += 1;

#if (!(R_DRAWCOLUMN_PIPELINE & RDC_FUZZ))
    {

        const byte *source = dcvars->source;
        const lighttable_t *colormap = dcvars->colormap;
        const byte *translation = dcvars->translation;
#if (R_DRAWCOLUMN_PIPELINE & (RDC_BILINEAR | RDC_ROUNDED | RDC_DITHERZ))
        int y = dcvars->yl;
        const int x = dcvars->x;
#endif
#if (R_DRAWCOLUMN_PIPELINE & RDC_DITHERZ)
        const int fracz = (dcvars->z >> 6) & 255;
        const byte *dither_colormaps[2] = {dcvars->colormap, dcvars->nextcolormap};
#endif
#if (R_DRAWCOLUMN_PIPELINE & RDC_BILINEAR)
        const int yl = dcvars->yl;
        const byte *dither_sources[2] = {dcvars->source, dcvars->nextsource};
        const unsigned int filter_fracu = (dcvars->source == dcvars->nextsource) ? 0 : (dcvars->texu >> 8) & 0xff;
#endif
#if (R_DRAWCOLUMN_PIPELINE & RDC_ROUNDED)
        const byte *prevsource = dcvars->prevsource;
        const byte *nextsource = dcvars->nextsource;
        const unsigned int filter_fracu = (dcvars->source == dcvars->nextsource) ? 0 : (dcvars->texu >> 8) & 0xff;
#endif

        count++;

        if (dcvars->texheight == 128)
        {

#define FIXEDT_128MASK ((127 << FRACBITS) | 0xffff)
            while (count--)
            {

                *dest = (GETCOL(frac & FIXEDT_128MASK, (frac + FRACUNIT) & FIXEDT_128MASK));

                INCY(y);

                dest += 4;
                frac += fracstep;
            }

        }
        
        else if (dcvars->texheight == 0)
        {

            while (count--)
            {

                *dest = (GETCOL(frac, (frac + FRACUNIT)));

                INCY(y);

                dest += 4;
                frac += fracstep;

            }

        }
        
        else
        {

            unsigned heightmask = dcvars->texheight - 1;

            if (!(dcvars->texheight & heightmask))
            {

                fixed_t fixedt_heightmask = (heightmask << FRACBITS) | 0xffff;

                while ((count -= 2) >= 0)
                {

                    *dest = (GETCOL(frac & fixedt_heightmask, (frac + FRACUNIT) & fixedt_heightmask));

                    INCY(y);

                    dest += 4;
                    frac += fracstep;
                    *dest = (GETCOL(frac & fixedt_heightmask, (frac + FRACUNIT) & fixedt_heightmask));

                    INCY(y);

                    dest += 4;
                    frac += fracstep;

                }

                if (count & 1)
                    *dest = (GETCOL(frac & fixedt_heightmask, (frac + FRACUNIT) & fixedt_heightmask));

                INCY(y);

            }
            
            else
            {

                fixed_t nextfrac = 0;

                heightmask++;
                heightmask <<= FRACBITS;

                if (frac < 0)
                    while ((frac += heightmask) < 0);
                else
                    while (frac >= (int)heightmask)
                        frac -= heightmask;

#if (R_DRAWCOLUMN_PIPELINE & (RDC_BILINEAR | RDC_ROUNDED))
                nextfrac = frac + FRACUNIT;

                while (nextfrac >= (int)heightmask)
                    nextfrac -= heightmask;
#endif
#define INCFRAC(f) if ((f += fracstep) >= (int)heightmask) f -= heightmask;
                while (count--)
                {

                    *dest = (GETCOL(frac, nextfrac));

                    INCY(y);

                    dest += 4;

                    INCFRAC(frac);
#if (R_DRAWCOLUMN_PIPELINE & (RDC_BILINEAR | RDC_ROUNDED))
                    INCFRAC(nextfrac); 
#endif
                }

            }

        }

    }
#endif

}
#undef GETCOL8_MAPPED
#undef GETCOL8_DEPTH
#undef GETCOL8
#undef GETCOL
#undef INCY
#undef INCFRAC
#undef COLTYPE

