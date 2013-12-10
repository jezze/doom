#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include "doomdef.h"
#include "doomtype.h"
#include "d_main.h"
#include "m_fixed.h"
#include "i_system.h"
#include "i_video.h"
#include "lprintf.h"
#include "m_random.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_misc.h"
#include "i_sound.h"
#include "i_main.h"
#include "r_fps.h"
#include "lprintf.h"

int realtic_clock_rate = 100;
static int_64_t I_GetTime_Scale = 1 << 24;
static int has_exited;

static int gettime_scaled(void)
{

    return (int)((int_64_t)I_GetTime_RealTime() * I_GetTime_Scale >> 24);

}

static int gettime_fastdemo(void)
{

    static int fasttic;

    return fasttic++;

}

static int gettime_error(void)
{

    I_Error("GetTime() used before initialization");

    return 0;

}

static void handle_signal(int s)
{

    char buf[2048];

    signal(s, SIG_IGN);
    strcpy(buf,"Exiting on signal: ");
    I_SigString(buf + strlen(buf), 2000 - strlen(buf), s);
    I_Error("%s", buf);

}

static void quit(void)
{

    if (!has_exited)
        has_exited = 1;

    if (has_exited == 1)
    {

        if (demorecording)
            G_CheckDemoStatus();

        M_SaveDefaults();

    }

}

int (*I_GetTime)(void) = gettime_error;

void I_SafeExit(int rc)
{

    if (!has_exited)
    {

        has_exited = rc ? 2 : 1;

        exit(rc);

    }

}

void I_Init(void)
{

    if (fastdemo)
    {

        I_GetTime = gettime_fastdemo;

    }

    else
    {

        if (realtic_clock_rate != 100)
        {

            I_GetTime_Scale = ((int_64_t)realtic_clock_rate << 24) / 100;
            I_GetTime = gettime_scaled;

        }

        else
        {

            I_GetTime = I_GetTime_RealTime;

        }

    }

    if (!(nomusicparm && nosfxparm))
        I_InitSound();

    R_InitInterpolation();

}

int main(int argc, char **argv)
{

    atexit(quit);
    signal(SIGSEGV, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGFPE, handle_signal);
    signal(SIGILL, handle_signal);
    signal(SIGINT, handle_signal);
    signal(SIGABRT, handle_signal);
    I_PreInitGraphics();
    D_DoomMain();

    return 0;

}

