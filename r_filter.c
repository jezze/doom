#include "doomtype.h"
#include "r_filter.h"

#define DMR                             16

byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM] = {
    0 * DMR,
    14 * DMR,
    3 * DMR,
    13 * DMR,
    11 * DMR,
    5 * DMR,
    8 * DMR,
    6 * DMR,
    12 * DMR,
    2 * DMR,
    15 * DMR,
    1 * DMR,
    7 * DMR,
    9 * DMR,
    4 * DMR,
    10 * DMR
};

byte filter_roundedUVMap[FILTER_UVDIM * FILTER_UVDIM];
byte filter_roundedRowMap[4 * 16];

void R_FilterInit(void)
{

    int i, j, s, t;

    for (i = 0; i < 16; i++)
    {

        filter_roundedRowMap[0 * 16 + i] = (i == 0x8 || i == 0xA) ? 0 : 1;
        filter_roundedRowMap[1 * 16 + i] = (i == 0x5 || i == 0x1) ? 2 : 1;
        filter_roundedRowMap[2 * 16 + i] = (i == 0x4 || i == 0x5) ? 0 : 1;
        filter_roundedRowMap[3 * 16 + i] = (i == 0xA || i == 0x2) ? 2 : 1;

    }

    for (i = 0; i < FILTER_UVDIM; i++)
    {

        for (j = 0; j < FILTER_UVDIM; j++)
        {

            s = (FILTER_UVDIM / 2) - i;
            t = (FILTER_UVDIM / 2) - j;

            if (s >= 0 && t >= 0)
                filter_roundedUVMap[i * FILTER_UVDIM + j] = (s + t > FILTER_UVDIM / 2) ? 0 : 4;
            else if (s >= 0 && t <= 0)
                filter_roundedUVMap[i * FILTER_UVDIM + j] = (s - t > FILTER_UVDIM / 2) ? 2 : 4;
            else if (s <= 0 && t >= 0)
                filter_roundedUVMap[i * FILTER_UVDIM + j] = (-s + t > FILTER_UVDIM / 2) ? 1 : 4;
            else if (s <= 0 && t <= 0)
                filter_roundedUVMap[i * FILTER_UVDIM + j] = (-s - t > FILTER_UVDIM / 2) ? 3 : 4;
            else
                filter_roundedUVMap[i * FILTER_UVDIM + j] = 4;

        }

    }

}

byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d)
{

    static byte quad[5];
    static byte rowColors[3];
    int code;
  
    rowColors[0] = d;
    rowColors[1] = e;
    rowColors[2] = f;
  
#define getCode(b, f, h, d) ((b == f) << 0 | (f == h) << 1 | (h == d) << 2 | (d == b) << 3)

    code = getCode(b,f,h,d);
    quad[0] = rowColors[filter_roundedRowMap[0 * 16 + code]];
    quad[1] = rowColors[filter_roundedRowMap[1 * 16 + code]];
    quad[2] = rowColors[filter_roundedRowMap[2 * 16 + code]];
    quad[3] = rowColors[filter_roundedRowMap[3 * 16 + code]];
    quad[4] = e;

    return quad;

}

