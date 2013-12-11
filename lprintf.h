#ifndef __LPRINTF__
#define __LPRINTF__

typedef enum
{

    LO_INFO = 1,
    LO_CONFIRM = 2,
    LO_WARN = 4,
    LO_ERROR = 8,
    LO_FATAL = 16,
    LO_DEBUG = 32,
    LO_ALWAYS = 64,

} OutputLevels;

#ifndef __GNUC__
#define __attribute__(x)
#endif

extern int lprintf(OutputLevels pri, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void I_Error(const char *error, ...) __attribute__((format(printf,1,2)));

#endif
