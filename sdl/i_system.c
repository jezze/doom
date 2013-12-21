#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <SDL.h>
#include "doomtype.h"
#include "doomdef.h"
#include "m_fixed.h"
#include "i_main.h"
#include "i_system.h"

#define MAX_MESSAGE_SIZE                1024

static unsigned int start_displaytime;
static unsigned int displaytime;
static boolean InDisplay = false;
int ms_to_next_tick;

void I_Print(const char *s, ...)
{

    char msg[MAX_MESSAGE_SIZE];
    va_list v;
 
    va_start(v, s);
    vsnprintf(msg, sizeof (msg), s, v);
    va_end(v);
    fprintf(stdout, "%s\n", msg);

}

void I_Error(const char *s, ...)
{

    char msg[MAX_MESSAGE_SIZE];
    va_list v;

    va_start(v, s);
    vsnprintf(msg, sizeof (msg), s, v);
    va_end(v);
    fprintf(stderr, "%s\n", msg);
    I_SafeExit(-1);

}

boolean I_StartDisplay(void)
{

    if (InDisplay)
        return false;

    start_displaytime = SDL_GetTicks();
    InDisplay = true;

    return true;

}

void I_EndDisplay(void)
{

    displaytime = SDL_GetTicks() - start_displaytime;
    InDisplay = false;

}

void I_uSleep(unsigned long usecs)
{

    SDL_Delay(usecs / 1000);

}

int I_GetTime(void)
{

    int t = SDL_GetTicks();
    int i = t * (TICRATE / 5) / 200;

    ms_to_next_tick = (i + 1) * 200 / (TICRATE / 5) - t;

    if (ms_to_next_tick > 1000 / TICRATE || ms_to_next_tick < 1)
        ms_to_next_tick = 1;

    return i;

}

fixed_t I_GetTimeFrac(void)
{

    unsigned long now = SDL_GetTicks();
    fixed_t frac;

    if (tic_vars.step == 0)
        return FRACUNIT;

    frac = (fixed_t)((now - tic_vars.start + displaytime) * FRACUNIT / tic_vars.step);

    if (frac < 0)
        frac = 0;

    if (frac > FRACUNIT)
        frac = FRACUNIT;

    return frac;

}

unsigned long I_GetRandomTimeSeed(void)
{

    return SDL_GetTicks();

}

const char *I_SigString(char *buf, size_t sz, int signum)
{

    snprintf(buf, sz, "signal %d", signum);

    return buf;

}

void I_Read(int fd, void *vbuf, size_t sz)
{

    unsigned char *buf = vbuf;

    while (sz)
    {

        int rc = read(fd, buf, sz);

        if (rc <= 0)
            I_Error("I_Read: read failed: %s", rc ? strerror(errno) : "EOF");

        sz -= rc;
        buf += rc;

    }

}

int I_Filelength(int handle)
{

    struct stat fileinfo;

    if (fstat(handle, &fileinfo) == -1)
        I_Error("I_Filelength: %s", strerror(errno));

    return fileinfo.st_size;

}

boolean HasTrailingSlash(const char *dn)
{

    return ((dn[strlen(dn) - 1] == '/'));

}

char *I_FindFile(const char *wfname, const char *ext)
{

    static const struct
    {

        const char *dir;
        const char *sub;
        const char *env;
        const char *(*func)(void);

    } search[] = {
        {NULL},
        {NULL, "doom", "HOME"},
        {NULL, NULL, "HOME"},
        {"/usr/local/share/games/doom"},
        {"/usr/share/games/doom"},
        {"/usr/local/share/doom"},
        {"/usr/share/doom"},
    };

    int i;
    size_t pl = strlen(wfname) + strlen(ext) + 4;

    for (i = 0; i < sizeof(search) / sizeof(*search); i++)
    {

        char *p;
        const char *d = NULL;
        const char *s = NULL;

        if (search[i].env)
        {

            if (!(d = getenv(search[i].env)))
                continue;

        }
        
        else if (search[i].func)
        {

            d = search[i].func();

        }

        else
        {

            d = search[i].dir;

        }

        s = search[i].sub;
        p = malloc((d ? strlen(d) : 0) + (s ? strlen(s) : 0) + pl);

        sprintf(p, "%s%s%s%s%s", d ? d : "", (d && !HasTrailingSlash(d)) ? "/" : "", s ? s : "", (s && !HasTrailingSlash(s)) ? "/" : "", wfname);

        if (access(p, F_OK))
            strcat(p, ext);

        if (!access(p,F_OK))
        {

            I_Print(" found %s\n", p);

            return p;

        }

        free(p);

    }

    return NULL;

}

