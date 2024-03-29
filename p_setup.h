#ifndef __P_SETUP__
#define __P_SETUP__

extern const byte *rejectmatrix;
extern long *blockmaplump;
extern long *blockmap;
extern int bmapwidth;
extern int bmapheight;
extern int bmaporgx;
extern int bmaporgy;
extern mobj_t **blocklinks;

void P_SetupLevel(int episode, int map, int playermask, skill_t skill);
void P_Init(void);

#endif
