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

int I_GetTime_RealTime(void)
{

    return 0;

}

fixed_t I_GetTimeFrac (void)
{

    return 0;

}

void I_GetTime_SaveMS(void)
{

}

unsigned long I_GetRandomTimeSeed(void)
{

    return 0;

}

const char *I_SigString(char *buf, size_t sz, int signum)
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

const char *I_DoomExeDir(void)
{

    return 0;

}

boolean HasTrailingSlash(const char *dn)
{

    return ((dn[strlen(dn) - 1] == '/'));

}

char *I_FindFile(const char *wfname, const char *ext)
{

    return 0;

}

