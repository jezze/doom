#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"

#define S_HILITE                        0x1
#define S_SELECT                        0x2
#define S_END                           0x8000

void M_StartControlPanel(void);
boolean M_Responder(event_t *ev);
void M_Ticker(void);
void M_Drawer(void);
void M_Init(void);

#endif
