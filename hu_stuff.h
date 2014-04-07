#ifndef __HU_STUFF_H__
#define __HU_STUFF_H__

#include "r_defs.h"

#define HU_FONTSTART                    '!'
#define HU_FONTEND                      0x7f
#define HU_FONTSIZE                     (HU_FONTEND - HU_FONTSTART + 1)
#define HU_MAXLINES                     4
#define HU_MAXLINELENGTH                80
#define HU_MAXMESSAGES                  16
#define MAXLINES                        25

typedef struct
{

    int x;
    int y;
    const patchnum_t *f;
    int sc;
    int cm;
    int linelen;
    char l[HU_MAXLINELENGTH * MAXLINES + 1];
    int len;
    int needsupdate;

} hu_textline_t;

typedef struct
{

    hu_textline_t l[HU_MAXLINES];
    int h;
    int cl;
    boolean *on;
    boolean laston;

} hu_stext_t;

typedef struct
{

    hu_textline_t l[HU_MAXMESSAGES];
    int nl;
    int nr;
    int cl;
    int x,y,w,h;
    const patchnum_t *bg;
    boolean *on;
    boolean laston;

} hu_mtext_t;

typedef struct
{

    hu_textline_t l;
    int lm;
    boolean *on;
    boolean laston;

} hu_itext_t;

void HU_Init(void);
void HU_Start(void);
boolean HU_Responder(event_t *ev);
void HU_Ticker(void);
void HU_Drawer(void);
void HU_Erase(void);

#endif
