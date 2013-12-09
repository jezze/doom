#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"

#define FINEANGLES              8192
#define FINEMASK                (FINEANGLES-1)

#define ANGLETOFINESHIFT        19

#define ANG45   0x20000000
#define ANG90   0x40000000
#define ANG180  0x80000000
#define ANG270  0xc0000000
#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

#define SLOPERANGE 2048
#define SLOPEBITS    11
#define DBITS      (FRACBITS-SLOPEBITS)

typedef unsigned angle_t;

void R_LoadTrigTables(void);
extern fixed_t finesine[5*FINEANGLES/4];
static fixed_t *const finecosine = finesine + (FINEANGLES/4);
extern fixed_t finetangent[FINEANGLES/2];
extern angle_t tantoangle[SLOPERANGE+1];
int SlopeDiv(unsigned num, unsigned den);

#endif
