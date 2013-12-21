#ifndef __I_MAIN__
#define __I_MAIN__

typedef struct
{

    unsigned int start;
    unsigned int next;
    unsigned int step;
    fixed_t frac;
    float msec;

} tic_vars_t;

extern tic_vars_t tic_vars;

extern int (*I_GetTime)(void);
void I_SafeExit(int rc);
void I_Init(void);

#endif
