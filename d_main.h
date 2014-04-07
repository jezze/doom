#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "w_wad.h"

typedef struct
{

    char forwardmove;
    char sidemove;
    short angleturn;
    byte buttons;

} ticcmd_t;

void D_PostEvent(event_t *ev);
void D_DoomMain(void);

#endif
