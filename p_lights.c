#include <stdlib.h>
#include <string.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "m_random.h"
#include "r_main.h"
#include "p_spec.h"
#include "p_tick.h"
#include "z_zone.h"

void T_FireFlicker(fireflicker_t *flick)
{

    int amount;

    if (--flick->count)
        return;

    amount = (P_Random(pr_lights) & 3) * 16;

    if (flick->sector->lightlevel - amount < flick->minlight)
        flick->sector->lightlevel = flick->minlight;
    else
        flick->sector->lightlevel = flick->maxlight - amount;

    flick->count = 4;

}

void T_LightFlash(lightflash_t *flash)
{

    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->maxlight)
    {

        flash->sector->lightlevel = flash->minlight;
        flash->count = (P_Random(pr_lights)&flash->mintime) + 1;

    }

    else
    {

        flash->sector->lightlevel = flash->maxlight;
        flash->count = (P_Random(pr_lights)&flash->maxtime) + 1;

    }

}

void T_StrobeFlash (strobe_t *flash)
{

    if (--flash->count)
        return;

    if (flash->sector->lightlevel == flash->minlight)
    {

        flash->sector->lightlevel = flash->maxlight;
        flash->count = flash->brighttime;

    }

    else
    {

        flash->sector->lightlevel = flash->minlight;
        flash->count =flash->darktime;

    }

}

void T_Glow(glow_t *g)
{

    switch (g->direction)
    {

    case -1:

        g->sector->lightlevel -= GLOWSPEED;

        if (g->sector->lightlevel <= g->minlight)
        {

            g->sector->lightlevel += GLOWSPEED;
            g->direction = 1;

        }

        break;

    case 1:

        g->sector->lightlevel += GLOWSPEED;

        if (g->sector->lightlevel >= g->maxlight)
        {

            g->sector->lightlevel -= GLOWSPEED;
            g->direction = -1;

        }

        break;

    }

}

void P_SpawnFireFlicker(sector_t *sector)
{

    fireflicker_t *flick;
    sector->special &= ~31;

    flick = Z_Malloc(sizeof(*flick), PU_LEVSPEC, 0);

    memset(flick, 0, sizeof(*flick));
    P_AddThinker (&flick->thinker);

    flick->thinker.function = T_FireFlicker;
    flick->sector = sector;
    flick->maxlight = sector->lightlevel;
    flick->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel) + 16;
    flick->count = 4;

}

void P_SpawnLightFlash(sector_t *sector)
{

    lightflash_t *flash;
    sector->special &= ~31;

    flash = Z_Malloc(sizeof(*flash), PU_LEVSPEC, 0);

    memset(flash, 0, sizeof(*flash));
    P_AddThinker(&flash->thinker);

    flash->thinker.function = T_LightFlash;
    flash->sector = sector;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    flash->maxtime = 64;
    flash->mintime = 7;
    flash->count = (P_Random(pr_lights)&flash->maxtime) + 1;

}

void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync)
{

    strobe_t *flash = Z_Malloc (sizeof(*flash), PU_LEVSPEC, 0);

    memset(flash, 0, sizeof(*flash));
    P_AddThinker (&flash->thinker);

    flash->sector = sector;
    flash->darktime = fastOrSlow;
    flash->brighttime = STROBEBRIGHT;
    flash->thinker.function = T_StrobeFlash;
    flash->maxlight = sector->lightlevel;
    flash->minlight = P_FindMinSurroundingLight(sector, sector->lightlevel);

    if (flash->minlight == flash->maxlight)
        flash->minlight = 0;

    sector->special &= ~31;

    if (!inSync)
        flash->count = (P_Random(pr_lights) & 7) + 1;
    else
        flash->count = 1;

}

void P_SpawnGlowingLight(sector_t *sector)
{

    glow_t *g = Z_Malloc(sizeof(*g), PU_LEVSPEC, 0);

    memset(g, 0, sizeof(*g));
    P_AddThinker(&g->thinker);

    g->sector = sector;
    g->minlight = P_FindMinSurroundingLight(sector,sector->lightlevel);
    g->maxlight = sector->lightlevel;
    g->thinker.function = T_Glow;
    g->direction = -1;

    sector->special &= ~31;

}

int EV_StartLightStrobing(line_t *line)
{

    int secnum = -1;
    sector_t* sec;

    while ((secnum = P_FindSectorFromLineTag(line,secnum)) >= 0)
    {

        sec = &sectors[secnum];

        if (P_SectorActive(lighting_special,sec))
            continue;

        P_SpawnStrobeFlash (sec,SLOWDARK, 0);

    }

    return 1;

}

int EV_TurnTagLightsOff(line_t *line)
{

    int j;

    for (j = -1; (j = P_FindSectorFromLineTag(line,j)) >= 0;)
    {

        sector_t *sector = sectors + j, *tsec;
        int i, min = sector->lightlevel;

        for (i = 0;i < sector->linecount; i++)
        {

            if ((tsec = getNextSector(sector->lines[i], sector)) && tsec->lightlevel < min)
                min = tsec->lightlevel;

        }

        sector->lightlevel = min;

    }

    return 1;

}

int EV_LightTurnOn(line_t *line, int bright)
{

    int i;

    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {

        sector_t *temp, *sector = sectors + i;
        int j, tbright = bright;

        if (!bright)
        {

            for (j = 0;j < sector->linecount; j++)
            {

                if ((temp = getNextSector(sector->lines[j],sector)) && temp->lightlevel > tbright)
                    tbright = temp->lightlevel;

            }

            sector->lightlevel = tbright;

        }

    }

    return 1;

}

int EV_LightTurnOnPartway(line_t *line, fixed_t level)
{

    int i;

    if (level < 0)
        level = 0;

    if (level > FRACUNIT)
        level = FRACUNIT;

    for (i = -1; (i = P_FindSectorFromLineTag(line, i)) >= 0;)
    {

        sector_t *temp, *sector = sectors + i;
        int j, bright = 0, min = sector->lightlevel;

        for (j = 0; j < sector->linecount; j++)
        {

            if ((temp = getNextSector(sector->lines[j], sector)))
            {

                if (temp->lightlevel > bright)
                    bright = temp->lightlevel;

                if (temp->lightlevel < min)
                    min = temp->lightlevel;

            }

        }

        sector->lightlevel = (level * bright + (FRACUNIT-level) * min) >> FRACBITS;

    }

    return 1;

}


