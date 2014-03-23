#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "w_wad.h"

#define MAXLOADFILES 3

typedef struct
{

    char forwardmove;
    char sidemove;
    short angleturn;
    byte buttons;

} ticcmd_t;

extern const char *wad_files[MAXLOADFILES];

void D_PostEvent(event_t* ev);
void D_DoomMain(void);

#endif
