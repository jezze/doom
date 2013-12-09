#ifndef __DOOMTYPE__
#define __DOOMTYPE__

typedef enum {false, true} boolean;
typedef unsigned char byte;

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

typedef signed long long int_64_t;
typedef unsigned long long uint_64_t;

#define LONGLONG(num)   (uint_64_t)num ## ll

#include <limits.h>

typedef enum
{

    doom_12_compatibility,
    doom_1666_compatibility,
    doom2_19_compatibility,
    ultdoom_compatibility,
    finaldoom_compatibility,
    dosdoom_compatibility,
    tasdoom_compatibility,
    boom_compatibility_compatibility,
    boom_201_compatibility,
    boom_202_compatibility,
    lxdoom_1_compatibility,
    mbf_compatibility,
    prboom_1_compatibility,
    prboom_2_compatibility,
    prboom_3_compatibility,
    prboom_4_compatibility,
    prboom_5_compatibility,
    prboom_6_compatibility,
    MAX_COMPATIBILITY_LEVEL,
    boom_compatibility = boom_201_compatibility,
    best_compatibility = prboom_6_compatibility

} complevel_t;

enum patch_translation_e
{

    VPT_NONE = 0,
    VPT_FLIP = 1,
    VPT_TRANS = 2,
    VPT_STRETCH = 4

};

#endif
