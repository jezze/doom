#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "d_event.h"

#define HU_FONTSTART                    '!'
#define HU_FONTEND                      0x7f
#define HU_FONTSIZE                     (HU_FONTEND - HU_FONTSTART + 1)
#define HU_BROADCAST                    5
#define HU_MSGX                         0
#define HU_MSGY                         0
#define HU_MSGWIDTH                     64
#define HU_MSGHEIGHT                    1
#define HU_MSGTIMEOUT                   (4 * TICRATE)

extern int hudcolor_titl;
extern int hudcolor_xyco;
extern int hudcolor_mesg;
extern int hudcolor_chat;
extern int hudcolor_list;
extern int hud_msg_lines;

void HU_Init(void);
void HU_Start(void);
boolean HU_Responder(event_t *ev);
void HU_Ticker(void);
void HU_Drawer(void);
void HU_Erase(void);

#endif
