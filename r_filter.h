#ifndef R_FILTER_H
#define R_FILTER_H

#define DITHER_DIM                      4
#define FILTER_UVBITS                   6
#define FILTER_UVDIM                    (1 << FILTER_UVBITS)

extern byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM];
extern byte filter_roundedUVMap[FILTER_UVDIM*FILTER_UVDIM];
extern byte filter_roundedRowMap[4 * 16];

void R_FilterInit(void);

#define filter_getDitheredPixelLevel(x, y, intensity)                           ((filter_ditherMatrix[(y) & (DITHER_DIM - 1)][(x) & (DITHER_DIM - 1)] < (intensity)) ? 1 : 0)
#define FILTER_GETV(x, y, texV, nextRowTexV)                                    (filter_getDitheredPixelLevel(x, y, (((texV) - yl) >> 8)&0xff) ? ((nextRowTexV) >> FRACBITS) : ((texV) >> FRACBITS))
#define filter_getDitheredForColumn(x, y, texV, nextRowTexV)                    dither_sources[(filter_getDitheredPixelLevel(x, y, filter_fracu))][FILTER_GETV(x, y, texV, nextRowTexV)]
#define filter_getRoundedForColumn(texV, nextRowTexV)                           filter_getScale2xQuadColors(source[((texV) >> FRACBITS)], source[(MAX(0, ((texV) >> FRACBITS) - 1))], nextsource[((texV) >> FRACBITS)], source[((nextRowTexV) >> FRACBITS)], prevsource[((texV) >> FRACBITS)]) [filter_roundedUVMap[((filter_fracu >> (8 - FILTER_UVBITS)) << FILTER_UVBITS) + ((((texV) >> 8) & 0xff) >> (8 - FILTER_UVBITS))]]
#define filter_getRoundedForSpan(texU, texV)                                    filter_getScale2xQuadColors(source[(((texU) >> 16) & 0x3f) | (((texV) >> 10) & 0xfc0)], source[(((texU) >> 16) & 0x3f) | ((((texV) - FRACUNIT) >> 10) & 0xfc0)], source[((((texU) + FRACUNIT) >> 16) & 0x3f) | (((texV) >> 10) & 0xfc0)], source[(((texU) >> 16) & 0x3f) | ((((texV) + FRACUNIT) >> 10) & 0xfc0)], source[((((texU) - FRACUNIT) >> 16) & 0x3f) | (((texV) >> 10) & 0xfc0)]) [filter_roundedUVMap[(((((texU) >> 8) & 0xff) >> (8 - FILTER_UVBITS)) << FILTER_UVBITS) + ((((texV) >> 8) & 0xff) >> (8 - FILTER_UVBITS))]]

byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d);

#endif
