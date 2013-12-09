#ifndef R_FILTER_H
#define R_FILTER_H

#define DITHER_DIM 4
#define FILTER_UVBITS 6
#define FILTER_UVDIM (1 << FILTER_UVBITS)

extern byte filter_ditherMatrix[DITHER_DIM][DITHER_DIM];
extern byte filter_roundedUVMap[FILTER_UVDIM*FILTER_UVDIM];
extern byte filter_roundedRowMap[4*16];

void R_FilterInit(void);

#define filter_getDitheredPixelLevel(x, y, intensity) \
  ((filter_ditherMatrix[(y)&(DITHER_DIM-1)][(x)&(DITHER_DIM-1)] < (intensity)) ? 1 : 0)

#define FILTER_GETV(x,y,texV,nextRowTexV) \
  (filter_getDitheredPixelLevel(x, y, (((texV) - yl) >> 8)&0xff) ? ((nextRowTexV)>>FRACBITS) : ((texV)>>FRACBITS))

#define filter_getDitheredForColumn(x, y, texV, nextRowTexV) \
  dither_sources[(filter_getDitheredPixelLevel(x, y, filter_fracu))][FILTER_GETV(x,y,texV,nextRowTexV)]

#define filter_getRoundedForColumn(texV, nextRowTexV) \
  filter_getScale2xQuadColors( \
    source[      ((texV)>>FRACBITS)              ], \
    source[      (MAX(0, ((texV)>>FRACBITS)-1))  ], \
    nextsource[  ((texV)>>FRACBITS)              ], \
    source[      ((nextRowTexV)>>FRACBITS)       ], \
    prevsource[  ((texV)>>FRACBITS)              ] \
  ) \
    [ filter_roundedUVMap[ \
      ((filter_fracu>>(8-FILTER_UVBITS))<<FILTER_UVBITS) + \
      ((((texV)>>8) & 0xff)>>(8-FILTER_UVBITS)) \
    ] ]

#define filter_getRoundedForSpan(texU, texV) \
  filter_getScale2xQuadColors( \
    source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)            ], \
    source[ (((texU)>>16)&0x3f) | ((((texV)-FRACUNIT)>>10)&0xfc0) ], \
    source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0) ], \
    source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0) ], \
    source[ ((((texU)-FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0) ] \
  ) \
    [ filter_roundedUVMap[ \
      (((((texU)>>8) & 0xff)>>(8-FILTER_UVBITS))<<FILTER_UVBITS) + \
      ((((texV)>>8) & 0xff)>>(8-FILTER_UVBITS)) \
    ] ]

byte *filter_getScale2xQuadColors(byte e, byte b, byte f, byte h, byte d);

#define filter_getFilteredForColumn32(depthmap, texV, nextRowTexV) ( \
  VID_PAL32( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL32( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

#define filter_getFilteredForColumn16(depthmap, texV, nextRowTexV) ( \
  VID_PAL16( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL16( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

#define filter_getFilteredForColumn15(depthmap, texV, nextRowTexV) ( \
  VID_PAL15( depthmap(nextsource[(nextRowTexV)>>FRACBITS]),   (filter_fracu*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(source[(nextRowTexV)>>FRACBITS]),       ((0xffff-filter_fracu)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(source[(texV)>>FRACBITS]),              ((0xffff-filter_fracu)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ) + \
  VID_PAL15( depthmap(nextsource[(texV)>>FRACBITS]),          (filter_fracu*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS) ))

#define filter_getFilteredForSpan32(depthmap, texU, texV) ( \
  VID_PAL32( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL32( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

#define filter_getFilteredForSpan16(depthmap, texU, texV) ( \
  VID_PAL16( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL16( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

#define filter_getFilteredForSpan15(depthmap, texU, texV) ( \
  VID_PAL15( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),  (unsigned int)(((texU)&0xffff)*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ (((texU)>>16)&0x3f) | ((((texV)+FRACUNIT)>>10)&0xfc0)]),             (unsigned int)((0xffff-((texU)&0xffff))*((texV)&0xffff))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ (((texU)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),                        (unsigned int)((0xffff-((texU)&0xffff))*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)) + \
  VID_PAL15( depthmap(source[ ((((texU)+FRACUNIT)>>16)&0x3f) | (((texV)>>10)&0xfc0)]),             (unsigned int)(((texU)&0xffff)*(0xffff-((texV)&0xffff)))>>(32-VID_COLORWEIGHTBITS)))

#define GETBLENDED15_5050(col1, col2) ((((col1&0x7c1f)+(col2&0x7c1f))>>1)&0x7c1f) | ((((col1&0x03e0)+(col2&0x03e0))>>1)&0x03e0)
#define GETBLENDED16_5050(col1, col2) ((((col1&0xf81f)+(col2&0xf81f))>>1)&0xf81f) | ((((col1&0x07e0)+(col2&0x07e0))>>1)&0x07e0)
#define GETBLENDED32_5050(col1, col2) ((((col1&0xff00ff)+(col2&0xff00ff))>>1)&0xff00ff) | ((((col1&0x00ff00)+(col2&0x00ff00))>>1)&0x00ff00)
#define GETBLENDED15_3268(col1, col2) ((((col1&0x7c1f)*5+(col2&0x7c1f)*11)>>4)&0x7c1f) | ((((col1&0x03e0)*5+(col2&0x03e0)*11)>>4)&0x03e0)
#define GETBLENDED16_3268(col1, col2) ((((col1&0xf81f)*5+(col2&0xf81f)*11)>>4)&0xf81f) | ((((col1&0x07e0)*5+(col2&0x07e0)*11)>>4)&0x07e0)
#define GETBLENDED32_3268(col1, col2) ((((col1&0xff00ff)*5+(col2&0xff00ff)*11)>>4)&0xff00ff) | ((((col1&0x00ff00)*5+(col2&0x00ff00)*11)>>4)&0x00ff00)
#define GETBLENDED15_9406(col1, col2) ((((col1&0x7c1f)*15+(col2&0x7c1f))>>4)&0x7c1f) | ((((col1&0x03e0)*15+(col2&0x03e0))>>4)&0x03e0)
#define GETBLENDED16_9406(col1, col2) ((((col1&0xf81f)*15+(col2&0xf81f))>>4)&0xf81f) | ((((col1&0x07e0)*15+(col2&0x07e0))>>4)&0x07e0)
#define GETBLENDED32_9406(col1, col2) ((((col1&0xff00ff)*15+(col2&0xff00ff))>>4)&0xff00ff) | ((((col1&0x00ff00)*15+(col2&0x00ff00))>>4)&0x00ff00)

#endif
