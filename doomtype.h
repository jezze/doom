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

typedef long long int_64_t;
typedef unsigned long long uint_64_t;

#define LONGLONG(num)   (uint_64_t)num ## ll

#include <limits.h>

enum patch_translation_e
{

    VPT_NONE = 0,
    VPT_FLIP = 1,
    VPT_TRANS = 2,
    VPT_STRETCH = 4

};

#endif
