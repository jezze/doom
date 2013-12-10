#ifndef __D_NET__
#define __D_NET__

typedef struct
{

    signed char forwardmove;
    signed char sidemove;
    signed short  angleturn;
    short consistancy;
    byte chatchar;
    byte buttons;

} ticcmd_t;

void D_BuildNewTiccmds(void);
void TryRunTics (void);
void D_InitNetGame(void);

#endif
