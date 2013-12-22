#ifndef __D_EVENT__
#define __D_EVENT__

typedef enum
{

    ev_keydown,
    ev_keyup,
    ev_mouse

} evtype_t;

typedef struct
{

    evtype_t type;
    int data1;
    int data2;
    int data3;

} event_t;

typedef enum
{

    ga_nothing,
    ga_loadlevel,
    ga_newgame,
    ga_completed,
    ga_victory,
    ga_worlddone

} gameaction_t;

typedef enum
{

    BT_ATTACK = 1,
    BT_USE = 2,
    BT_SPECIAL = 128,
    BT_SPECIALMASK = 3,
    BT_CHANGE = 4,
    BT_WEAPONMASK = (8 + 16 + 32 + 64),
    BT_WEAPONSHIFT = 3,
    BTS_PAUSE = 1,
    BTS_RESTARTLEVEL = 3

} buttoncode_t;

extern gameaction_t gameaction;

#endif
