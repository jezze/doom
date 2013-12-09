#ifndef __D_MAIN__
#define __D_MAIN__

#include "d_event.h"
#include "w_wad.h"

#define MAXLOADFILES 3

extern char basesavegame[];

extern boolean clnomonsters;
extern boolean clrespawnparm;
extern boolean clfastparm;
extern boolean nosfxparm;
extern boolean nomusicparm;
extern int ffmap;

void D_PostEvent(event_t* ev);

extern boolean advancedemo;
void D_AdvanceDemo(void);
void D_DoAdvanceDemo (void);

void D_Display(void);
void D_PageTicker(void);
void D_StartTitle(void);
void D_DoomMain(void);
void D_AddFile (const char *file, wad_source_t source);

extern const char *wad_files[MAXLOADFILES];

#endif
