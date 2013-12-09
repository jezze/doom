#ifndef __STSTUFF_H__
#define __STSTUFF_H__

#include "doomtype.h"
#include "d_event.h"

#define ST_HEIGHT                       32
#define ST_WIDTH                        320
#define ST_Y                            (200 - ST_HEIGHT)
#define ST_SCALED_HEIGHT                (ST_HEIGHT*SCREENHEIGHT/200)
#define ST_SCALED_WIDTH                 SCREENWIDTH
#define ST_SCALED_Y                     (SCREENHEIGHT - ST_SCALED_HEIGHT)

boolean ST_Responder(event_t *ev);

void ST_Ticker(void);
void ST_Drawer(boolean st_statusbaron, boolean refresh);
void ST_Start(void);
void ST_Init(void);

typedef enum
{

    AutomapState,
    FirstPersonState

} st_stateenum_t;

typedef enum
{

    StartChatState,
    WaitDestState,
    GetChatState

} st_chatstateenum_t;

extern int health_red;
extern int health_yellow;
extern int health_green;
extern int armor_red;
extern int armor_yellow;
extern int armor_green;
extern int ammo_red;
extern int ammo_yellow;
extern int sts_always_red;
extern int sts_pct_always_gray;
extern int sts_traditional_keys;
extern int st_palette;

#endif
