#include "doomdef.h"
#include "doomstat.h"
#include "v_video.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "st_lib.h"
#include "r_main.h"

void STlib_initNum(st_number_t *n, int x, int y, const patchnum_t *pl, int *num, boolean *on, int width)
{

    n->x = x;
    n->y = y;
    n->oldnum = 0;
    n->width = width;
    n->num = num;
    n->on = on;
    n->p = pl;

}

static void STlib_drawNum(st_number_t *n, int cm, boolean refresh)
{

    int numdigits = n->width;
    int num = *n->num;
    int w = n->p[0].width;
    int h = n->p[0].height;
    int x = n->x;
    int neg;

    if (n->oldnum == num && !refresh)
        return;

    if ((neg = (n->oldnum = num) < 0))
    {

        if (numdigits == 2 && num < -9)
            num = -9;
        else if (numdigits == 3 && num < -99)
            num = -99;

        num = -num;

    }

    x = n->x - numdigits * w;

    V_CopyRect(x, n->y - ST_Y, BG, w * numdigits, h, x, n->y, FG, VPT_STRETCH);

    if (num == 1994)
        return;

    x = n->x;

    if (!num)
        V_DrawNumPatch(x - w, n->y, FG, n->p[0].lumpnum, cm, ((cm != CR_DEFAULT) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);

    while (num && numdigits--)
    {

        x -= w;

        V_DrawNumPatch(x, n->y, FG, n->p[num % 10].lumpnum, cm, ((cm != CR_DEFAULT) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);

        num /= 10;

    }

    if (neg)
        V_DrawNamePatch(x - w, n->y, FG, "STTMINUS", cm, ((cm != CR_DEFAULT) ? VPT_TRANS : VPT_NONE) | VPT_STRETCH);

}

void STlib_updateNum(st_number_t *n, int cm, boolean refresh)
{

    if (*n->on)
        STlib_drawNum(n, cm, refresh);

}

void STlib_initPercent(st_percent_t *p, int x, int y, const patchnum_t *pl, int *num, boolean *on, const patchnum_t *percent)
{

    STlib_initNum(&p->n, x, y, pl, num, on, 3);

    p->p = percent;

}

void STlib_updatePercent(st_percent_t *per, int cm, int refresh)
{

    if (*per->n.on && (refresh || (per->n.oldnum != *per->n.num)))
        V_DrawNumPatch(per->n.x, per->n.y, FG, per->p->lumpnum, cm, VPT_NONE | VPT_STRETCH);

    STlib_updateNum(&per->n, cm, refresh);

}

void STlib_initMultIcon(st_multicon_t *i, int x, int y, const patchnum_t *il, int *inum, boolean *on)
{

    i->x = x;
    i->y = y;
    i->oldinum = -1;
    i->inum = inum;
    i->on = on;
    i->p = il;

}

void STlib_updateMultIcon(st_multicon_t *mi, boolean refresh)
{

    int w;
    int h;
    int x;
    int y;

    if (*mi->on && (mi->oldinum != *mi->inum || refresh))
    {

        if (mi->oldinum != -1)
        {

            x = mi->x - mi->p[mi->oldinum].leftoffset;
            y = mi->y - mi->p[mi->oldinum].topoffset;
            w = mi->p[mi->oldinum].width;
            h = mi->p[mi->oldinum].height;

            V_CopyRect(x, y-ST_Y, BG, w, h, x, y, FG, VPT_STRETCH);

        }

        if (*mi->inum != -1)
            V_DrawNumPatch(mi->x, mi->y, FG, mi->p[*mi->inum].lumpnum, CR_DEFAULT, VPT_STRETCH);

        mi->oldinum = *mi->inum;

    }

}

void STlib_initBinIcon(st_binicon_t *b, int x, int y, const patchnum_t *i, boolean *val, boolean *on)
{

    b->x = x;
    b->y = y;
    b->oldval = 0;
    b->val = val;
    b->on = on;
    b->p = i;

}

void STlib_updateBinIcon(st_binicon_t *bi, boolean refresh)
{

    int x;
    int y;
    int w;
    int h;

    if (*bi->on && (bi->oldval != *bi->val || refresh))
    {

        x = bi->x - bi->p->leftoffset;
        y = bi->y - bi->p->topoffset;
        w = bi->p->width;
        h = bi->p->height;

        if (*bi->val)
            V_DrawNumPatch(bi->x, bi->y, FG, bi->p->lumpnum, CR_DEFAULT, VPT_STRETCH);
        else
            V_CopyRect(x, y-ST_Y, BG, w, h, x, y, FG, VPT_STRETCH);

        bi->oldval = *bi->val;

    }

}

