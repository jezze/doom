#include <sys/types.h>
#include "doomtype.h"
#include "doomstat.h"
#include "d_client.h"
#include "z_zone.h"
#include "d_main.h"
#include "g_game.h"
#include "m_menu.h"
#include "i_system.h"
#include "i_video.h"
#include "r_main.h"

ticcmd_t netcmds[MAXPLAYERS][BACKUPTICS];
static ticcmd_t* localcmds;
int maketic;

void D_InitNetGame(void)
{

    int i;

    localcmds = netcmds[0];

    for (i = 0; i < MAXPLAYERS; i++)
        playeringame[i] = false;

    playeringame[0] = true;
    consoleplayer = displayplayer = 0;

}

void D_BuildNewTiccmds(void)
{

    static int lastmadetic;
    int newtics = I_GetTime() - lastmadetic;

    lastmadetic += newtics;

    while (newtics--)
    {

        I_StartTic();

        if (maketic - gametic > BACKUPTICS / 2)
            break;

        G_BuildTiccmd(&localcmds[maketic % BACKUPTICS]);

        maketic++;

    }
}

void TryRunTics(void)
{

    int runtics;
    int entertime = I_GetTime();

    while (1)
    {

        D_BuildNewTiccmds();

        runtics = maketic - gametic;

        if (!runtics)
        {

            I_uSleep(ms_to_next_tick * 1000);

            if (I_GetTime() - entertime > 10)
            {

                M_Ticker();
                
                return;

            }

        }
        
        else
        {
        
            break;

        }

    }

    while (runtics--)
    {

        M_Ticker();
        G_Ticker ();

        gametic++;

    }

}

