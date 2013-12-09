#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "doomdef.h"
#include "d_main.h"
#include "m_fixed.h"
#include "i_system.h"
#include "i_video.h"
#include "z_zone.h"
#include "m_random.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_main.h"

int realtic_clock_rate = 100;

static int gettime(void)
{

    return 0;

}

void I_SafeExit(int rc)
{

}

void I_Init(void)
{

}

int (*I_GetTime)(void) = gettime;

int main(int argc, char **argv)
{

    I_PreInitGraphics();
    D_DoomMain();

    return 0;

}

