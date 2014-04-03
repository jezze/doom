#ifndef __HULIB__
#define __HULIB__

#include "r_defs.h"
#include "v_video.h"

#define HU_MAXLINES                     4
#define HU_MAXLINELENGTH                80
#define HU_REFRESHSPACING               8
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

void HUlib_clearTextLine(hu_textline_t *t);
void HUlib_initTextLine(hu_textline_t *t, int x, int y, const patchnum_t *f, int sc, int cm);
boolean HUlib_addCharToTextLine(hu_textline_t *t, char ch);
void HUlib_drawTextLine(hu_textline_t *l, boolean drawcursor);
void HUlib_eraseTextLine(hu_textline_t *l);
void HUlib_initSText(hu_stext_t *s, int x, int y, int h, const patchnum_t* font, int startchar, int cm, boolean *on);
void HUlib_addMessageToSText(hu_stext_t *s, const char* prefix, const char *msg);
void HUlib_drawSText(hu_stext_t *s);
void HUlib_eraseSText(hu_stext_t *s);
void HUlib_initMText(hu_mtext_t *m, int x, int y, int w, int h, const patchnum_t *font, int startchar, int cm, const patchnum_t* bgfont, boolean *on);
void HUlib_addMessageToMText(hu_mtext_t *m, const char *prefix, const char *msg);
void HUlib_drawMText(hu_mtext_t *m);
void HUlib_eraseMText(hu_mtext_t *m);
void HUlib_initIText(hu_itext_t *it, int x, int y, const patchnum_t* font, int startchar, int cm, boolean *on);
boolean HUlib_keyInIText(hu_itext_t *it, unsigned char ch);
void HUlib_drawIText(hu_itext_t *it);
void HUlib_eraseIText(hu_itext_t *it);

#endif
