#if (R_DRAWCOLUMN_PIPELINE_BITS == 8)
#define SCREENTYPE byte
#define TOPLEFT byte_topleft
#define PITCH byte_pitch
#define TEMPBUF byte_tempbuf
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 15)
#define SCREENTYPE unsigned short
#define TOPLEFT short_topleft
#define PITCH short_pitch
#define TEMPBUF short_tempbuf
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 16)
#define SCREENTYPE unsigned short
#define TOPLEFT short_topleft
#define PITCH short_pitch
#define TEMPBUF short_tempbuf
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 32)
#define SCREENTYPE unsigned int
#define TOPLEFT int_topleft
#define PITCH int_pitch
#define TEMPBUF int_tempbuf
#endif

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
#define GETDESTCOLOR8(col) (tempfuzzmap[6*256+(col)])
#define GETDESTCOLOR15(col) GETBLENDED15_9406(col, 0)
#define GETDESTCOLOR16(col) GETBLENDED16_9406(col, 0)
#define GETDESTCOLOR32(col) GETBLENDED32_9406(col, 0)
#else
#define GETDESTCOLOR8(col) (col)
#define GETDESTCOLOR15(col) (col)
#define GETDESTCOLOR16(col) (col)
#define GETDESTCOLOR32(col) (col)
#endif

#if (R_DRAWCOLUMN_PIPELINE_BITS == 8)
#define GETDESTCOLOR(col) GETDESTCOLOR8(col)
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 15)
#define GETDESTCOLOR(col) GETDESTCOLOR15(col)
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 16)
#define GETDESTCOLOR(col) GETDESTCOLOR16(col)
#elif (R_DRAWCOLUMN_PIPELINE_BITS == 32)
#define GETDESTCOLOR(col) GETDESTCOLOR32(col)
#endif

static void R_FLUSHWHOLE_FUNCNAME(void)
{

    SCREENTYPE *source;
    SCREENTYPE *dest;
    int count, yl;

    while (--temp_x >= 0)
    {

        yl = tempyl[temp_x];
        source = &TEMPBUF[temp_x + (yl << 2)];
        dest = drawvars.TOPLEFT + yl * drawvars.PITCH + startx + temp_x;
        count = tempyh[temp_x] - yl + 1;
      
        while (--count >= 0)
        {

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
            *dest = GETDESTCOLOR(dest[fuzzoffset[fuzzpos]]);
         
            if (++fuzzpos == FUZZTABLE) 
                fuzzpos = 0;

#else
            *dest = *source;
#endif
            source += 4;
            dest += drawvars.PITCH;

      }

   }

}

static void R_FLUSHHEADTAIL_FUNCNAME(void)
{

    SCREENTYPE *source;
    SCREENTYPE *dest;
    int count, colnum = 0;
    int yl, yh;

    while (colnum < 4)
    {

        yl = tempyl[colnum];
        yh = tempyh[colnum];
      
        if (yl < commontop)
        {

            source = &TEMPBUF[colnum + (yl << 2)];
            dest = drawvars.TOPLEFT + yl*drawvars.PITCH + startx + colnum;
            count = commontop - yl;
         
            while (--count >= 0)
            {

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
                *dest = GETDESTCOLOR(dest[fuzzoffset[fuzzpos]]);
            
                if (++fuzzpos == FUZZTABLE) 
                    fuzzpos = 0;

#else
                *dest = *source;
#endif

                source += 4;
                dest += drawvars.PITCH;

            }

        }
      
        if (yh > commonbot)
        {

            source = &TEMPBUF[colnum + ((commonbot + 1) << 2)];
            dest = drawvars.TOPLEFT + (commonbot + 1) * drawvars.PITCH + startx + colnum;
            count = yh - commonbot;
         
            while (--count >= 0)
            {

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
                *dest = GETDESTCOLOR(dest[fuzzoffset[fuzzpos]]);
            
                if (++fuzzpos == FUZZTABLE) 
                    fuzzpos = 0;

#else
                *dest = *source;
#endif
                source += 4;
                dest += drawvars.PITCH;

            }

        }

        ++colnum;

    }

}

static void R_FLUSHQUAD_FUNCNAME(void)
{

    SCREENTYPE *source = &TEMPBUF[commontop << 2];
    SCREENTYPE *dest = drawvars.TOPLEFT + commontop*drawvars.PITCH + startx;
    int count;
#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
    int fuzz1 = fuzzpos;
    int fuzz2 = (fuzz1 + tempyl[1]) % FUZZTABLE;
    int fuzz3 = (fuzz2 + tempyl[2]) % FUZZTABLE;
    int fuzz4 = (fuzz3 + tempyl[3]) % FUZZTABLE;
#endif

    count = commonbot - commontop + 1;

#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
    while (--count >= 0)
    {

        dest[0] = GETDESTCOLOR(dest[0 + fuzzoffset[fuzz1]]);
        dest[1] = GETDESTCOLOR(dest[1 + fuzzoffset[fuzz2]]);
        dest[2] = GETDESTCOLOR(dest[2 + fuzzoffset[fuzz3]]);
        dest[3] = GETDESTCOLOR(dest[3 + fuzzoffset[fuzz4]]);
        fuzz1 = (fuzz1 + 1) % FUZZTABLE;
        fuzz2 = (fuzz2 + 1) % FUZZTABLE;
        fuzz3 = (fuzz3 + 1) % FUZZTABLE;
        fuzz4 = (fuzz4 + 1) % FUZZTABLE;
        source += 4 * sizeof(byte);
        dest += drawvars.PITCH * sizeof(byte);

    }
#else

#if (R_DRAWCOLUMN_PIPELINE_BITS == 8)
    if ((sizeof(int) == 4) && (((long)source % 4) == 0) && (((long)dest % 4) == 0))
    {

        while (--count >= 0)
        {

            *(int *)dest = *(int *)source;
            source += 4 * sizeof(byte);
            dest += drawvars.PITCH * sizeof(byte);

        }

    }
   
    else
    {

        while (--count >= 0)
        {

            dest[0] = source[0];
            dest[1] = source[1];
            dest[2] = source[2];
            dest[3] = source[3];
            source += 4 * sizeof(byte);
            dest += drawvars.PITCH * sizeof(byte);

        }

    }
#else
    while (--count >= 0)
    {

        dest[0] = source[0];
        dest[1] = source[1];
        dest[2] = source[2];
        dest[3] = source[3];
        source += 4;
        dest += drawvars.PITCH;
    }
#endif

#endif
}

#undef GETDESTCOLOR32
#undef GETDESTCOLOR16
#undef GETDESTCOLOR15
#undef GETDESTCOLOR8
#undef GETDESTCOLOR
#undef TEMPBUF
#undef PITCH
#undef TOPLEFT
#undef SCREENTYPE
#undef R_DRAWCOLUMN_PIPELINE_BITS
#undef R_DRAWCOLUMN_PIPELINE
#undef R_FLUSHWHOLE_FUNCNAME
#undef R_FLUSHHEADTAIL_FUNCNAME
#undef R_FLUSHQUAD_FUNCNAME
