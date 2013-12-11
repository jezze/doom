#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "doomtype.h"
#include "lprintf.h"
#include "i_main.h"

#define MAX_MESSAGE_SIZE                2048

static int cons_error_mask = -1 - LO_INFO;
static int cons_output_mask = -1;

int lprintf(OutputLevels pri, const char *s, ...)
{

    int r = 0;
    char msg[MAX_MESSAGE_SIZE];
    int lvl = pri;
    va_list v;
 
    va_start(v,s);
    vsnprintf(msg, sizeof(msg), s, v);
    va_end(v);

    if (lvl & cons_output_mask)
    {

        r = fprintf(stdout,"%s", msg);

    }

    if (!isatty(1) && lvl & cons_error_mask)
        r = fprintf(stderr,"%s", msg);

    return r;

}

void I_Error(const char *error, ...)
{

    char errmsg[MAX_MESSAGE_SIZE];
    va_list argptr;

    va_start(argptr, error);
    vsnprintf(errmsg, sizeof(errmsg), error, argptr);
    va_end(argptr);
    lprintf(LO_ERROR, "%s\n", errmsg);
    I_SafeExit(-1);

}

