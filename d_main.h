#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "w_wad.h"

#define MAXLOADFILES 3

extern boolean clnomonsters;
extern boolean clrespawnparm;
extern boolean clfastparm;
extern boolean nosfxparm;
extern boolean nomusicparm;

extern const char *wad_files[MAXLOADFILES];

void D_PostEvent(event_t* ev);
void D_Display(void);
void D_DoomMain(void);

#endif
