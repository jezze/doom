#ifndef __M_FIXED__
#define __M_FIXED__

#include "doomtype.h"

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

#define D_abs(x) ({int _t = (x), _s = _t >> (8 * sizeof _t - 1); (_t^_s) - _s;})

inline static int FixedMul(int a, int b)
{

    return (int)((int_64_t) a * b >> FRACBITS);

}

inline static int FixedDiv(int a, int b)
{

    return ((unsigned)D_abs(a) >> 14) >= (unsigned)D_abs(b) ? ((a ^ b) >> 31) ^ INT_MAX : (int)(((int_64_t) a << FRACBITS) / b);

}

inline static int FixedMod(int a, int b)
{

    if (b & (b - 1))
    {

        int r = a % b;

        return ((r < 0) ? r + b : r);

    }

    return (a & (b - 1));

}

#endif
