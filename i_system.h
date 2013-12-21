#ifndef __I_SYSTEM__
#define __I_SYSTEM__

void I_Print(const char *s, ...) __attribute__((format(printf, 1, 2)));
void I_Error(const char *s, ...) __attribute__((format(printf, 1, 2)));
boolean I_StartDisplay(void);
void I_EndDisplay(void);
int I_GetTime_RealTime(void);
fixed_t I_GetTimeFrac(void);
void I_GetTime_SaveMS(void);
unsigned long I_GetRandomTimeSeed(void);
void I_uSleep(unsigned long usecs);
const char *I_SigString(char *buf, size_t sz, int signum);
boolean HasTrailingSlash(const char *dn);
char *I_FindFile(const char *wfname, const char *ext);
void I_Read(int fd, void *buf, size_t sz);
int I_Filelength(int handle);

extern int ms_to_next_tick;

#endif
