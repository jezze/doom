#include <stdlib.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "p_user.h"
#include "p_spec.h"
#include "p_tick.h"
#include "p_map.h"
#include "z_zone.h"

int leveltime;

static boolean newthinkerpresent;

thinker_t thinkerclasscap[th_all+1];

void P_InitThinkers(void)
{

    int i;

    for (i = 0; i < NUMTHCLASS; i++)
        thinkerclasscap[i].cprev = thinkerclasscap[i].cnext = &thinkerclasscap[i];

    thinkercap.prev = thinkercap.next = &thinkercap;

}

void P_UpdateThinker(thinker_t *thinker)
{

    register thinker_t *th;

    int class = thinker->function == P_RemoveThinkerDelayed ? th_delete : thinker->function == P_MobjThinker && ((mobj_t *) thinker)->health > 0 && (((mobj_t *) thinker)->flags & MF_COUNTKILL || ((mobj_t *) thinker)->type == MT_SKULL) ? ((mobj_t *) thinker)->flags & MF_FRIEND ? th_friends : th_enemies : th_misc;

    if ((th = thinker->cnext)!= NULL)
        (th->cprev = thinker->cprev)->cnext = th;

    th = &thinkerclasscap[class];
    th->cprev->cnext = thinker;
    thinker->cnext = th;
    thinker->cprev = th->cprev;
    th->cprev = thinker;

}

void P_AddThinker(thinker_t *thinker)
{

    thinkercap.prev->next = thinker;
    thinker->next = &thinkercap;
    thinker->prev = thinkercap.prev;
    thinkercap.prev = thinker;
    thinker->references = 0;
    thinker->cnext = thinker->cprev = NULL;

    P_UpdateThinker(thinker);

    newthinkerpresent = true;

}

static thinker_t *currentthinker;

void P_RemoveThinkerDelayed(thinker_t *thinker)
{

    if (!thinker->references)
    {

        thinker_t *next = thinker->next;
        (next->prev = currentthinker = thinker->prev)->next = next;

        {

            thinker_t *th = thinker->cnext;
            (th->cprev = thinker->cprev)->cnext = th;

        }

        Z_Free(thinker);

    }
}

void P_RemoveThinker(thinker_t *thinker)
{

    thinker->function = P_RemoveThinkerDelayed;

    P_UpdateThinker(thinker);

}

thinker_t* P_NextThinker(thinker_t* th, th_class cl)
{

    thinker_t* top = &thinkerclasscap[cl];

    if (!th)
        th = top;

    th = cl == th_all ? th->next : th->cnext;

    return th == top ? NULL : th;

}

void P_SetTarget(mobj_t **mop, mobj_t *targ)
{

    if (*mop)
        (*mop)->thinker.references--;

    if ((*mop = targ))
        targ->thinker.references++;

}

static void P_RunThinkers(void)
{

    for (currentthinker = thinkercap.next; currentthinker != &thinkercap; currentthinker = currentthinker->next)
    {

        if (currentthinker->function)
            currentthinker->function(currentthinker);

    }

    newthinkerpresent = false;

}

void P_Ticker(void)
{

    int i;

    if ((menuactive && players[consoleplayer].viewz != 1))
        return;

    P_MapStart();

    if (gamestate == GS_LEVEL)
    {

        for (i = 0; i < MAXPLAYERS; i++)
        {

            if (playeringame[i])
                P_PlayerThink(&players[i]);

        }

    }

    P_RunThinkers();
    P_UpdateSpecials();
    P_MapEnd();

    leveltime++;

}

