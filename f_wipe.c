#include "z_zone.h"
#include "doomdef.h"
#include "i_video.h"
#include "v_video.h"
#include "m_random.h"
#include "f_wipe.h"

#define SRC_SCR                         2
#define DEST_SCR                        3

static screeninfo_t wipe_scr_start;
static screeninfo_t wipe_scr_end;
static screeninfo_t wipe_scr;
static int y_lookup[MAX_SCREENWIDTH];

static int init_melt(int ticks)
{

    int i;

    for (i = 0; i < SCREENHEIGHT; i++)
        memcpy(wipe_scr.data + i * wipe_scr.byte_pitch, wipe_scr_start.data + i * wipe_scr.byte_pitch, SCREENWIDTH * V_GetPixelDepth());

    y_lookup[0] = -(M_Random() % 16);

    for (i = 1; i < SCREENWIDTH; i++)
    {

        int r = (M_Random() % 3) - 1;

        y_lookup[i] = y_lookup[i - 1] + r;

        if (y_lookup[i] > 0)
            y_lookup[i] = 0;
        else if (y_lookup[i] == -16)
            y_lookup[i] = -15;

    }

    return 0;

}

static int do_melt(int ticks)
{

    boolean done = true;
    int i;
    const int depth = V_GetPixelDepth();

    while (ticks--)
    {

        for (i = 0; i < (SCREENWIDTH); i++)
        {

            if (y_lookup[i] < 0)
            {

                y_lookup[i]++;
                done = false;

                continue;
            }

            if (y_lookup[i] < SCREENHEIGHT)
            {

                byte *s, *d;
                int j, k, dy;

                dy = (y_lookup[i] < 16) ? y_lookup[i] + 1 : SCREENHEIGHT / 25;

                if (y_lookup[i]+dy >= SCREENHEIGHT)
                    dy = SCREENHEIGHT - y_lookup[i];

                s = wipe_scr_end.data + (y_lookup[i] * wipe_scr_end.byte_pitch + (i * depth));
                d = wipe_scr.data + (y_lookup[i] * wipe_scr.byte_pitch + (i * depth));

                for (j = dy; j; j--)
                {

                    for (k = 0; k < depth; k++)
                        d[k] = s[k];

                    d += wipe_scr.byte_pitch;
                    s += wipe_scr_end.byte_pitch;
                }

                y_lookup[i] += dy;
                s = wipe_scr_start.data + (i * depth);
                d = wipe_scr.data + (y_lookup[i] * wipe_scr.byte_pitch + (i * depth));

                for (j = SCREENHEIGHT - y_lookup[i]; j; j--)
                {

                    for (k = 0; k < depth; k++)
                        d[k] = s[k];

                    d += wipe_scr.byte_pitch;
                    s += wipe_scr_end.byte_pitch;

                }

                done = false;

            }
        }
    }

    return done;

}

static int exit_melt(int ticks)
{

    V_FreeScreen(&wipe_scr_start);

    wipe_scr_start.width = 0;
    wipe_scr_start.height = 0;

    V_FreeScreen(&wipe_scr_end);

    wipe_scr_end.width = 0;
    wipe_scr_end.height = 0;

    screens[SRC_SCR] = wipe_scr_start;
    screens[DEST_SCR] = wipe_scr_end;

    return 0;

}

int wipe_StartScreen(void)
{

    wipe_scr_start.width = SCREENWIDTH;
    wipe_scr_start.height = SCREENHEIGHT;
    wipe_scr_start.byte_pitch = screens[0].byte_pitch;
    wipe_scr_start.short_pitch = screens[0].short_pitch;
    wipe_scr_start.int_pitch = screens[0].int_pitch;
    wipe_scr_start.not_on_heap = false;

    V_AllocScreen(&wipe_scr_start);

    screens[SRC_SCR] = wipe_scr_start;

    V_CopyRect(0, 0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, SRC_SCR, VPT_NONE);

    return 0;

}

int wipe_EndScreen(void)
{

    wipe_scr_end.width = SCREENWIDTH;
    wipe_scr_end.height = SCREENHEIGHT;
    wipe_scr_end.byte_pitch = screens[0].byte_pitch;
    wipe_scr_end.short_pitch = screens[0].short_pitch;
    wipe_scr_end.int_pitch = screens[0].int_pitch;
    wipe_scr_end.not_on_heap = false;

    V_AllocScreen(&wipe_scr_end);

    screens[DEST_SCR] = wipe_scr_end;

    V_CopyRect(0, 0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, DEST_SCR, VPT_NONE);
    V_CopyRect(0, 0, SRC_SCR, SCREENWIDTH, SCREENHEIGHT, 0, 0, 0, VPT_NONE);

    return 0;

}

int wipe_ScreenWipe(int ticks)
{

    static boolean go;

    if (!go)
    {

        go = 1;
        wipe_scr = screens[0];

        init_melt(ticks);

    }

    if (do_melt(ticks))
    {

        exit_melt(ticks);
        go = 0;

    }

    return !go;

}

