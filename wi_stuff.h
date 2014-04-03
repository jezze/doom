#ifndef __WI_STUFF__
#define __WI_STUFF__

typedef enum
{

    NoState = -1,
    StatCount,
    ShowNextLoc

} stateenum_t;

void WI_Ticker(void);
void WI_Drawer(void);
void WI_checkForAccelerate(void);
void WI_Start(wbstartstruct_t *wbstartstruct);
void WI_End(void);

#endif
