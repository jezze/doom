static void R_DRAWSPAN_FUNCNAME(draw_span_vars_t *dsvars)
{

#if (R_DRAWSPAN_PIPELINE & (RDC_ROUNDED | RDC_BILINEAR))
    if ((D_abs(dsvars->xstep) > drawvars.mag_threshold) || (D_abs(dsvars->ystep) > drawvars.mag_threshold))
    {

        R_GetDrawSpanFunc(RDRAW_FILTER_POINT, drawvars.filterz)(dsvars);

        return;

    }
#endif

    {

        unsigned count = dsvars->x2 - dsvars->x1 + 1;
        fixed_t xfrac = dsvars->xfrac;
        fixed_t yfrac = dsvars->yfrac;
        const fixed_t xstep = dsvars->xstep;
        const fixed_t ystep = dsvars->ystep;
        const byte *source = dsvars->source;
        const byte *colormap = dsvars->colormap;
        byte *dest = drawvars.byte_topleft + dsvars->y*drawvars.byte_pitch + dsvars->x1;
#if (R_DRAWSPAN_PIPELINE & (RDC_DITHERZ | RDC_BILINEAR))
        const int y = dsvars->y;
        int x1 = dsvars->x1;
#endif
#if (R_DRAWSPAN_PIPELINE & RDC_DITHERZ)
        const int fracz = (dsvars->z >> 12) & 255;
        const byte *dither_colormaps[2] = { dsvars->colormap, dsvars->nextcolormap };
#endif

        while (count)
        {

#if (R_DRAWSPAN_PIPELINE & RDC_BILINEAR)
            const fixed_t xtemp = ((xfrac >> 16) + (filter_getDitheredPixelLevel(x1, y, ((xfrac >> 8) & 0xff)))) & 63;
            const fixed_t ytemp = ((yfrac >> 10) + 64 * (filter_getDitheredPixelLevel(x1, y, ((yfrac >> 8) & 0xff)))) & 4032;
#else
            const fixed_t xtemp = (xfrac >> 16) & 63;
            const fixed_t ytemp = (yfrac >> 10) & 4032;
#endif
            const fixed_t spot = xtemp | ytemp;

            xfrac += xstep;
            yfrac += ystep;
#if (R_DRAWSPAN_PIPELINE & RDC_DITHERZ)  
            *dest++ = dither_colormaps[filter_getDitheredPixelLevel(x1, y, fracz)][source[spot]];
#else
            *dest++ = colormap[source[spot]];
#endif
            count--;
#if (R_DRAWSPAN_PIPELINE & (RDC_DITHERZ | RDC_BILINEAR))
            x1--;
#endif

        }

    }

}

