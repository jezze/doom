#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"
#include "r_defs.h"
#include "v_video.h"

#define ST_HEIGHT                       32
#define ST_WIDTH                        320
#define ST_Y                            (200 - ST_HEIGHT)
#define ST_SCALED_HEIGHT                (ST_HEIGHT*SCREENHEIGHT/200)
#define ST_SCALED_WIDTH                 SCREENWIDTH
#define ST_SCALED_Y                     (SCREENHEIGHT - ST_SCALED_HEIGHT)
#define BG 4
#define FG 0

typedef struct
{

    int x;
    int y;
    int width;
    int oldnum;
    int *num;
    boolean *on;
    const patchnum_t *p;
    int data;

} st_number_t;

typedef struct
{

    st_number_t n;
    const patchnum_t *p;

} st_percent_t;

typedef struct
{

    int x;
    int y;
    int oldinum;
    int *inum;
    boolean *on;
    const patchnum_t *p;
    int data;

} st_multicon_t;

typedef struct
{

    int x;
    int y;
    boolean oldval;
    boolean *val;
    boolean *on;
    const patchnum_t *p;
    int data;

} st_binicon_t;

boolean ST_Responder(event_t *ev);

void ST_Ticker(void);
void ST_Drawer(void);
void ST_Start(void);
void ST_Init(void);

extern int health_red;
extern int health_yellow;
extern int health_green;
extern int armor_red;
extern int armor_yellow;
extern int armor_green;
extern int ammo_red;
extern int ammo_yellow;

#endif
