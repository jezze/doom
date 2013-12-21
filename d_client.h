#ifndef __D_NET__
#define __D_NET__

typedef struct
{

    char forwardmove;
    char sidemove;
    short angleturn;
    byte buttons;

} ticcmd_t;

void D_BuildNewTiccmds(void);
void TryRunTics (void);
void D_InitNetGame(void);

#endif
