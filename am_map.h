#ifndef __AMMAP_H__
#define __AMMAP_H__

#define MAPBITS                         12
#define FRACTOMAPBITS                   (FRACBITS-MAPBITS)
#define AM_MSGHEADER                    (('a' << 24) + ('m' << 16))
#define AM_MSGENTERED                   (AM_MSGHEADER | ('e' << 8))
#define AM_MSGEXITED                    (AM_MSGHEADER | ('x' << 8))

boolean AM_Responder (event_t* ev);

void AM_Ticker(void);
void AM_Drawer(void);
void AM_Stop(void);
void AM_Start(void);
void AM_ClearMarks(void);

typedef struct
{

    fixed_t x,y;

} mpoint_t;

extern mpoint_t *markpoints;
extern int markpointnum, markpointnum_max;
extern int mapcolor_back;
extern int mapcolor_grid;
extern int mapcolor_wall;
extern int mapcolor_fchg;
extern int mapcolor_cchg;
extern int mapcolor_clsd;
extern int mapcolor_rkey;
extern int mapcolor_bkey;
extern int mapcolor_ykey;
extern int mapcolor_rdor;
extern int mapcolor_bdor;
extern int mapcolor_ydor;
extern int mapcolor_tele;
extern int mapcolor_secr;
extern int mapcolor_exit;
extern int mapcolor_unsn;
extern int mapcolor_flat;
extern int mapcolor_sprt;
extern int mapcolor_item;
extern int mapcolor_enemy;
extern int mapcolor_frnd;
extern int mapcolor_hair;
extern int mapcolor_sngl;
extern int mapcolor_plyr[4];
extern int mapcolor_me;
extern int map_secret_after;

#endif
