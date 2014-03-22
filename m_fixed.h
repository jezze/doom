#ifndef __M_FIXED__
#define __M_FIXED__

#include "doomtype.h"

#define FRACBITS 16
#define FRACUNIT (1 << FRACBITS)

typedef int fixed_t;

#define D_abs(x) ({fixed_t _t = (x), _s = _t >> (8 * sizeof _t - 1); (_t^_s) - _s;})

inline static fixed_t FixedMul(fixed_t a, fixed_t b)
{

    return (fixed_t)((int_64_t) a * b >> FRACBITS);

}

inline static fixed_t FixedDiv(fixed_t a, fixed_t b)
{

    return ((unsigned)D_abs(a) >> 14) >= (unsigned)D_abs(b) ? ((a ^ b) >> 31) ^ INT_MAX : (fixed_t)(((int_64_t) a << FRACBITS) / b);

}

inline static fixed_t FixedMod(fixed_t a, fixed_t b)
{

    if (b & (b - 1))
    {

        fixed_t r = a % b;

        return ((r < 0) ? r + b : r);

    }

    return (a & (b - 1));

}

#endif
