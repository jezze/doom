#ifndef __TABLES__
#define __TABLES__

#include "m_fixed.h"

#define FINEANGLES                      8192
#define FINEMASK                        (FINEANGLES - 1)

#define ANGLETOFINESHIFT                19

#define ANG45                           0x20000000
#define ANG90                           0x40000000
#define ANG180                          0x80000000
#define ANG270                          0xc0000000
#ifndef M_PI
#define M_PI                            3.14159265358979323846
#endif

#define SLOPERANGE                      2048
#define SLOPEBITS                       11
#define DBITS                           (FRACBITS - SLOPEBITS)

typedef unsigned angle_t;
extern const fixed_t *finesine;
extern const fixed_t *finecosine;
extern const fixed_t *finetangent;
extern const angle_t *tantoangle;

void R_LoadTrigTables(void);
int SlopeDiv(unsigned num, unsigned den);

#endif
