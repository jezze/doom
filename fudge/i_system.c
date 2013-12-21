#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include "doomtype.h"
#include "doomdef.h"
#include "m_fixed.h"
#include "r_fps.h"
#include "i_system.h"

int ms_to_next_tick;
int realtic_clock_rate = 100;

void I_SafeExit(int rc)
{

}

void I_Init(void)
{

}

void I_Print(const char *s, ...)
{

}

void I_Error(const char *s, ...)
{

}

boolean I_StartDisplay(void)
{

    return true;

}

void I_EndDisplay(void)
{

}

void I_uSleep(unsigned long usecs)
{

}

fixed_t I_GetTimeFrac(void)
{

    return 0;

}

unsigned long I_GetRandomTimeSeed(void)
{

    return 0;

}

void I_Read(int fd, void *vbuf, size_t sz)
{

}

int I_Filelength(int handle)
{

    return 0;

}

char *I_FindFile(const char *wfname, const char *ext)
{

    return 0;

}

int main(int argc, char **argv)
{

    I_PreInitGraphics();
    D_DoomMain();

    return 0;

}

