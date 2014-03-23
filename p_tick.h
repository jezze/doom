#ifndef __P_TICK__
#define __P_TICK__

#define thinkercap thinkerclasscap[th_all]

typedef enum
{

    th_delete,
    th_misc,
    th_friends,
    th_enemies,
    NUMTHCLASS,
    th_all = NUMTHCLASS,

} th_class;

extern thinker_t thinkerclasscap[];

thinker_t* P_NextThinker(thinker_t *, th_class);
void P_Ticker(void);
void P_InitThinkers(void);
void P_AddThinker(thinker_t *thinker);
void P_RemoveThinker(thinker_t *thinker);
void P_RemoveThinkerDelayed(thinker_t *thinker);
void P_UpdateThinker(thinker_t *thinker);
void P_SetTarget(mobj_t **mo, mobj_t *target);

#endif
