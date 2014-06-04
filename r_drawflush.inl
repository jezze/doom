#if (R_DRAWCOLUMN_PIPELINE & RDC_FUZZ)
#define GETDESTCOLOR(col) (tempfuzzmap[6 * 256 + (col)])
#else
#define GETDESTCOLOR(col) (col)
#endif

static void R_FLUSHWHOLE_FUNCNAME(void)
{

    byte *source;
    byte *dest;
    int count, yl;

    while (--temp_x >= 0)
    {

        yl = tempyl[temp_x];
        source = &byte_tempbuf[temp_x + (yl << 2)];
        dest = drawvars.byte_topleft + yl * drawvars.byte_pitch + startx + temp_x;
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
            dest += drawvars.byte_pitch;

      }

   }

}

static void R_FLUSHHEADTAIL_FUNCNAME(void)
{

    byte *source;
    byte *dest;
    int count, colnum = 0;
    int yl, yh;

    while (colnum < 4)
    {

        yl = tempyl[colnum];
        yh = tempyh[colnum];
      
        if (yl < commontop)
        {

            source = &byte_tempbuf[colnum + (yl << 2)];
            dest = drawvars.byte_topleft + yl*drawvars.byte_pitch + startx + colnum;
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
                dest += drawvars.byte_pitch;

            }

        }
      
        if (yh > commonbot)
        {

            source = &byte_tempbuf[colnum + ((commonbot + 1) << 2)];
            dest = drawvars.byte_topleft + (commonbot + 1) * drawvars.byte_pitch + startx + colnum;
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
                dest += drawvars.byte_pitch;

            }

        }

        ++colnum;

    }

}

static void R_FLUSHQUAD_FUNCNAME(void)
{

    byte *source = &byte_tempbuf[commontop << 2];
    byte *dest = drawvars.byte_topleft + commontop*drawvars.byte_pitch + startx;
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
        dest += drawvars.byte_pitch * sizeof(byte);

    }
#else
    if ((sizeof(int) == 4) && (((long)source % 4) == 0) && (((long)dest % 4) == 0))
    {

        while (--count >= 0)
        {

            *(int *)dest = *(int *)source;
            source += 4 * sizeof(byte);
            dest += drawvars.byte_pitch * sizeof(byte);

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
            dest += drawvars.byte_pitch * sizeof(byte);

        }

    }
#endif

}
#undef GETDESTCOLOR

