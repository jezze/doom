#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "m_swap.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_main.h"
#include "r_draw.h"

#define noterased viewwindowx

extern int key_backspace;
extern int key_enter;

void HUlib_clearTextLine(hu_textline_t* t)
{

    t->linelen = t->len = 0;
    t->l[0] = 0;
    t->needsupdate = true;

}

void HUlib_initTextLine(hu_textline_t* t, int x, int y, const patchnum_t* f, int sc, int cm)
{

    t->x = x;
    t->y = y;
    t->f = f;
    t->sc = sc;
    t->cm = cm;

    HUlib_clearTextLine(t);

}

boolean HUlib_addCharToTextLine(hu_textline_t *t, char ch)
{

    if (t->linelen == HU_MAXLINELENGTH)
    {

        return false;

    }

    else
    {

        t->linelen++;

        if (ch == '\n')
            t->linelen = 0;

        t->l[t->len++] = ch;
        t->l[t->len] = 0;
        t->needsupdate = 4;

        return true;

    }

}

static boolean HUlib_delCharFromTextLine(hu_textline_t *t)
{

    if (!t->len)
        return false;
    else
    {

        t->l[--t->len] = 0;
        t->needsupdate = 4;

        return true;

    }

}

void HUlib_drawTextLine(hu_textline_t *l, boolean drawcursor)
{

    int i;
    int w;
    int x;
    unsigned char c;
    int oc = l->cm;
    int y = l->y;

    x = l->x;

    for (i = 0; i < l->len; i++)
    {

        c = toupper(l->l[i]);

        if (c == '\n')
            x = 0, y += 8;

        else if (c == '\t')
            x = x - x % 80 + 80;

        else if (c=='\x1b')
        {

            if (++i < l->len)
                if (l->l[i] >= '0' && l->l[i] <= '9')
                    l->cm = l->l[i] - '0';

        }

        else if (c != ' ' && c >= l->sc && c <= 127)
        {

            w = l->f[c - l->sc].width;

            if (x+w > BASE_WIDTH)
                break;

            V_DrawNumPatch(x, y, FG, l->f[c - l->sc].lumpnum, l->cm, VPT_TRANS | VPT_STRETCH);

            x += w;

        }

        else
        {
        
            x += 4;

            if (x >= BASE_WIDTH)
                break;

        }

    }

    l->cm = oc;

    if (drawcursor && x + l->f['_' - l->sc].width <= BASE_WIDTH)
        V_DrawNumPatch(x, y, FG, l->f['_' - l->sc].lumpnum, CR_DEFAULT, VPT_NONE | VPT_STRETCH);

}

void HUlib_eraseTextLine(hu_textline_t* l)
{

    int lh;
    int y;

    if (!(automapmode & am_active) && viewwindowx && l->needsupdate)
    {

        lh = l->f[0].height + 1;

        for (y = l->y; y < l->y + lh; y++)
        {

            if (y < viewwindowy || y >= viewwindowy + viewheight)
            {

                R_VideoErase(0, y, SCREENWIDTH);

            }

            else
            {

                R_VideoErase(0, y, viewwindowx);
                R_VideoErase(viewwindowx + viewwidth, y, viewwindowx);

            }

        }

    }

    if (l->needsupdate)
        l->needsupdate--;

}

void HUlib_initSText(hu_stext_t *s, int x, int y, int h, const patchnum_t *font, int startchar, int cm, boolean *on)
{

    int i;

    s->h = h;
    s->on = on;
    s->laston = true;
    s->cl = 0;

    for (i = 0; i < h; i++)
        HUlib_initTextLine(&s->l[i], x, y - i * (font[0].height + 1), font, startchar, cm);

}

static void HUlib_addLineToSText(hu_stext_t *s)
{

    int i;

    if (++s->cl == s->h)
        s->cl = 0;

    HUlib_clearTextLine(&s->l[s->cl]);

    for (i = 0; i < s->h; i++)
        s->l[i].needsupdate = 4;

}

void HUlib_addMessageToSText(hu_stext_t *s, const char *prefix, const char *msg)
{

    HUlib_addLineToSText(s);

    if (prefix)
    {

        while (*prefix)
            HUlib_addCharToTextLine(&s->l[s->cl], *(prefix++));

    }

    while (*msg)
        HUlib_addCharToTextLine(&s->l[s->cl], *(msg++));

}

void HUlib_drawSText(hu_stext_t *s)
{

    int i, idx;
    hu_textline_t *l;

    if (!*s->on)
        return;

    for (i = 0; i < s->h; i++)
    {

        idx = s->cl - i;

        if (idx < 0)
            idx += s->h;

        l = &s->l[idx];

        HUlib_drawTextLine(l, false);

    }

}

void HUlib_eraseSText(hu_stext_t* s)
{

    int i;

    for (i = 0; i < s->h; i++)
    {

        if (s->laston && !*s->on)
            s->l[i].needsupdate = 4;

        HUlib_eraseTextLine(&s->l[i]);

    }

    s->laston = *s->on;

}

void HUlib_initMText(hu_mtext_t *m, int x, int y, int w, int h, const patchnum_t* font, int startchar, int cm, const patchnum_t* bgfont, boolean *on)
{

    int i;

    m->nl = 0;
    m->nr = 0;
    m->cl = -1;
    m->x = x;
    m->y = y;
    m->w = w;
    m->h = h;
    m->bg = bgfont;
    m->on = on;

    for (i = 0; i < HU_MAXMESSAGES; i++)
        HUlib_initTextLine(&m->l[i], x, y + (hud_list_bgon ? i + 1 : i) * HU_REFRESHSPACING, font, startchar, cm);

}

static void HUlib_addLineToMText(hu_mtext_t* m)
{

    if (++m->cl == hud_msg_lines)
        m->cl = 0;

    HUlib_clearTextLine(&m->l[m->cl]);

    if (m->nl<hud_msg_lines)
        m->nl++;

    m->l[m->cl].needsupdate = 4;

}

void HUlib_addMessageToMText(hu_mtext_t* m, const char* prefix, const char* msg)
{

    HUlib_addLineToMText(m);

    if (prefix)
    {

        while (*prefix)
            HUlib_addCharToTextLine(&m->l[m->cl], *(prefix++));

    }

    while (*msg)
        HUlib_addCharToTextLine(&m->l[m->cl], *(msg++));

}

void HUlib_drawMBg(int x, int y, int w, int h, const patchnum_t* bgp)
{

    int xs = bgp[0].width;
    int ys = bgp[0].height;
    int i, j;

    V_DrawNumPatch(x, y, FG, bgp[0].lumpnum, CR_DEFAULT, VPT_STRETCH);

    for (j = x + xs; j < x + w - xs; j += xs)
        V_DrawNumPatch(j, y, FG, bgp[1].lumpnum, CR_DEFAULT, VPT_STRETCH);

    V_DrawNumPatch(j, y, FG, bgp[2].lumpnum, CR_DEFAULT, VPT_STRETCH);

    for (i=y+ys;i<y+h-ys;i+=ys)
    {

        V_DrawNumPatch(x, i, FG, bgp[3].lumpnum, CR_DEFAULT, VPT_STRETCH);

        for (j = x + xs; j < x + w - xs; j += xs)
            V_DrawNumPatch(j, i, FG, bgp[4].lumpnum, CR_DEFAULT, VPT_STRETCH);

        V_DrawNumPatch(j, i, FG, bgp[5].lumpnum, CR_DEFAULT, VPT_STRETCH);

    }

    V_DrawNumPatch(x, i, FG, bgp[6].lumpnum, CR_DEFAULT, VPT_STRETCH);

    for (j = x + xs; j < x + w - xs; j += xs)
        V_DrawNumPatch(j, i, FG, bgp[7].lumpnum, CR_DEFAULT, VPT_STRETCH);

    V_DrawNumPatch(j, i, FG, bgp[8].lumpnum, CR_DEFAULT, VPT_STRETCH);

}

void HUlib_drawMText(hu_mtext_t* m)
{

    int i, idx;
    hu_textline_t *l;

    if (!*m->on)
        return;

    if (hud_list_bgon)
        HUlib_drawMBg(m->x,m->y,m->w,m->h,m->bg);

    for (i = 0; i < m->nl; i++)
    {

        idx = m->cl - i;

        if (idx < 0)
            idx += m->nl;

        l = &m->l[idx];

        if (hud_list_bgon)
        {

            l->x = m->x + 4;
            l->y = m->y + (i + 1) * HU_REFRESHSPACING;

        }

        else
        {

            l->x = m->x;
            l->y = m->y + i * HU_REFRESHSPACING;

        }

        HUlib_drawTextLine(l, false);

    }

}

static void HUlib_eraseMBg(hu_mtext_t* m)
{

    int lh;
    int y;

    if (!(automapmode & am_active) && viewwindowx)
    {

        lh = m->l[0].f[0].height + 1;

        for (y=m->y; y<m->y+lh*(hud_msg_lines+2) ; y++)
        {

            if (y < viewwindowy || y >= viewwindowy + viewheight)
            {

                R_VideoErase(0, y, SCREENWIDTH);

            }

            else
            {

                R_VideoErase(0, y, viewwindowx);
                R_VideoErase(viewwindowx + viewwidth, y, viewwindowx);

            }

        }

    }
}

void HUlib_eraseMText(hu_mtext_t* m)
{

    int i;

    if (hud_list_bgon)
        HUlib_eraseMBg(m);

    for (i = 0; i < m->nl; i++)
    {

        m->l[i].needsupdate = 4;

        HUlib_eraseTextLine(&m->l[i]);

    }

}

void HUlib_initIText(hu_itext_t *it, int x, int y, const patchnum_t *font, int startchar, int cm, boolean *on)
{

    it->lm = 0;
    it->on = on;
    it->laston = true;

    HUlib_initTextLine(&it->l, x, y, font, startchar, cm);

}

static void HUlib_delCharFromIText(hu_itext_t* it)
{

    if (it->l.len != it->lm)
        HUlib_delCharFromTextLine(&it->l);

}

void HUlib_resetIText(hu_itext_t* it)
{

    it->lm = 0;

    HUlib_clearTextLine(&it->l);

}

void HUlib_addPrefixToIText(hu_itext_t *it, char *str)
{

    while (*str)
        HUlib_addCharToTextLine(&it->l, *(str++));

    it->lm = it->l.len;

}

boolean HUlib_keyInIText(hu_itext_t* it, unsigned char ch)
{

    if (ch >= ' ' && ch <= '_')
        HUlib_addCharToTextLine(&it->l, (char)ch);
    else if (ch == key_backspace)
        HUlib_delCharFromIText(it);
    else if (ch != key_enter)
        return false;

    return true;

}

void HUlib_drawIText(hu_itext_t* it)
{

    hu_textline_t *l = &it->l;

    if (!*it->on)
        return;

    HUlib_drawTextLine(l, true);

}

void HUlib_eraseIText(hu_itext_t* it)
{

    if (it->laston && !*it->on)
        it->l.needsupdate = 4;

    HUlib_eraseTextLine(&it->l);

    it->laston = *it->on;

}

