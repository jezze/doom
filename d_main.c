#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "doomdef.h"
#include "doomtype.h"
#include "doomstat.h"
#include "d_englsh.h"
#include "d_client.h"
#include "sounds.h"
#include "z_zone.h"
#include "w_wad.h"
#include "s_sound.h"
#include "v_video.h"
#include "f_finale.h"
#include "m_misc.h"
#include "m_menu.h"
#include "i_main.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "g_game.h"
#include "hu_stuff.h"
#include "wi_stuff.h"
#include "st_stuff.h"
#include "p_setup.h"
#include "r_draw.h"
#include "r_main.h"
#include "r_fps.h"
#include "d_main.h"
#include "lprintf.h"

boolean clnomonsters;
boolean clrespawnparm;
boolean clfastparm;
boolean nomonsters;
boolean respawnparm;
boolean fastparm;
boolean singletics = false;
boolean nosfxparm;
boolean nomusicparm;

skill_t startskill;
int startepisode;
int startmap;

char wadfile[PATH_MAX + 1];

const char *const standard_iwads[] = {
    "doom2.wad",
    "plutonia.wad",
    "tnt.wad",
    "doom.wad",
    "doom1.wad",
    "doomu.wad",
    "freedoom.wad",
};

static const int nstandard_iwads = sizeof standard_iwads / sizeof * standard_iwads;

void D_PostEvent(event_t *ev)
{

    if (gametic < 3)
        return;

    M_Responder(ev) || (gamestate == GS_LEVEL && (HU_Responder(ev) || ST_Responder(ev))) || G_Responder(ev);

}

extern boolean setsizeneeded;
extern int showMessages;

void D_Display (void)
{

    static boolean isborderstate = false;
    static boolean borderwillneedredraw = false;
    static gamestate_t oldgamestate = -1;
    boolean viewactive = false, isborder = false;

    if (!I_StartDisplay())
        return;

    if (gamestate != GS_LEVEL)
    {

        switch (oldgamestate)
        {

        case -1:
        case GS_LEVEL:
            V_SetPalette(0);

        default:
            break;

        }

        switch (gamestate)
        {

        case GS_INTERMISSION:
            WI_Drawer();

            break;

        case GS_FINALE:
            F_Drawer();

            break;

        default:
            break;

        }

    }
    
    else if (gametic != basetic)
    {

        boolean redrawborderstuff;

        HU_Erase();

        if (setsizeneeded)
        {

            R_ExecuteSetViewSize();
            oldgamestate = -1;

        }

        viewactive = 1;
        isborder = viewheight != SCREENHEIGHT;

        if (oldgamestate != GS_LEVEL)
        {

            R_FillBackScreen();

            redrawborderstuff = isborder;

        }
        
        else
        {

            redrawborderstuff = isborder && (!isborderstate || borderwillneedredraw);
            borderwillneedredraw = menuactive && isborder && viewactive && (viewwidth != SCREENWIDTH);

        }

        if (redrawborderstuff)
            R_DrawViewBorder();

        if (viewactive)
            R_RenderPlayerView(&players[displayplayer]);

        ST_Drawer(isborder, redrawborderstuff);
        R_DrawViewBorder();
        HU_Drawer();

    }

    isborderstate = isborder;
    oldgamestate = gamestate;

    if (paused)
    {

        V_DrawNamePatch((320 - V_NamePatchWidth("M_PAUSE")) / 2, 4, 0, "M_PAUSE", CR_DEFAULT, VPT_STRETCH);

    }

    M_Drawer();
    D_BuildNewTiccmds();
    I_FinishUpdate();
    I_EndDisplay();

    if (paused)
        I_uSleep(1000);

}

static void D_DoomLoop(void)
{

    for (;;)
    {

        WasRenderedInTryRunTics = false;
        I_StartFrame ();

        if (singletics)
        {

            I_StartTic ();
            G_BuildTiccmd(&netcmds[consoleplayer][maketic % BACKUPTICS]);

            M_Ticker();
            G_Ticker();

            gametic++;
            maketic++;

        }

        else
        {

            TryRunTics ();

        }

        if (players[displayplayer].mo)
            S_UpdateSounds(players[displayplayer].mo);

        if (!movement_smooth || !WasRenderedInTryRunTics)
            D_Display();

    }

}

static void D_AddFile(const char *file, wad_source_t source)
{

    char *gwa_filename = NULL;

    wadfiles = realloc(wadfiles, sizeof(*wadfiles) * (numwadfiles + 1));
    wadfiles[numwadfiles].name = AddDefaultExtension(strcpy(malloc(strlen(file) + 5), file), ".wad");
    wadfiles[numwadfiles].src = source;
    numwadfiles++;
    gwa_filename = AddDefaultExtension(strcpy(malloc(strlen(file) + 5), file), ".wad");

    if (strlen(gwa_filename) > 4)
    {

        if (!strcasecmp(gwa_filename + (strlen(gwa_filename) - 4), ".wad"))
        {

            char *ext = &gwa_filename[strlen(gwa_filename) - 4];

            ext[1] = 'g';
            ext[2] = 'w';
            ext[3] = 'a';
            wadfiles = realloc(wadfiles, sizeof(*wadfiles) * (numwadfiles + 1));
            wadfiles[numwadfiles].name = gwa_filename;
            wadfiles[numwadfiles].src = source;
            numwadfiles++;

        }

    }

}

static void CheckIWAD(const char *iwadname,GameMode_t *gmode,boolean *hassec)
{

    if (!access(iwadname, R_OK))
    {

        int ud = 0, rg = 0, sw = 0, cm = 0, sc = 0;
        FILE* fp;

        if ((fp = fopen(iwadname, "rb")))
        {

            wadinfo_t header;

            if (fread(&header, sizeof(header), 1, fp) == 1 && !strncmp(header.identification, "IWAD", 4))
            {

                size_t length;
                filelump_t *fileinfo;

                header.numlumps = LONG(header.numlumps);
                header.infotableofs = LONG(header.infotableofs);
                length = header.numlumps;
                fileinfo = malloc(length * sizeof(filelump_t));

                if (fseek (fp, header.infotableofs, SEEK_SET) || fread(fileinfo, sizeof(filelump_t), length, fp) != length || fclose(fp))
                    I_Error("CheckIWAD: failed to read directory %s", iwadname);

                while (length--)
                {

                    if (fileinfo[length].name[0] == 'E' && fileinfo[length].name[2] == 'M' && fileinfo[length].name[4] == 0)
                    {

                        if (fileinfo[length].name[1] == '4')
                            ++ud;
                        else if (fileinfo[length].name[1] == '3')
                            ++rg;
                        else if (fileinfo[length].name[1] == '2')
                            ++rg;
                        else if (fileinfo[length].name[1] == '1')
                            ++sw;
                    }

                    else if (fileinfo[length].name[0] == 'M' && fileinfo[length].name[1] == 'A' && fileinfo[length].name[2] == 'P' && fileinfo[length].name[5] == 0)
                    {

                        ++cm;

                        if (fileinfo[length].name[3] == '3')
                            if (fileinfo[length].name[4] == '1' || fileinfo[length].name[4] == '2')
                                ++sc;

                    }

                }

                free(fileinfo);

            }

            else
            {

                I_Error("CheckIWAD: IWAD tag %s not present", iwadname);

            }

        }

        else
        {

            I_Error("CheckIWAD: Can't open IWAD %s", iwadname);

        }

        *gmode = indetermined;
        *hassec = false;

        if (cm >= 30)
        {

            *gmode = commercial;
            *hassec = sc >= 2;

        }

        else if (ud >= 9)
            *gmode = retail;
        else if (rg >= 18)
            *gmode = registered;
        else if (sw >= 9)
            *gmode = shareware;

    }

    else
    {

        I_Error("CheckIWAD: IWAD %s not readable", iwadname);

    }

}

static char *FindIWADFile(void)
{

    int i;
    char *iwad  = NULL;

    for (i = 0; !iwad && i < nstandard_iwads; i++)
        iwad = I_FindFile(standard_iwads[i], ".wad");

    return iwad;

}

static void IdentifyVersion (void)
{

    int i;
    char *iwad = FindIWADFile();

    if (iwad && *iwad)
    {

        lprintf(LO_CONFIRM,"IWAD found: %s\n", iwad);
        CheckIWAD(iwad, &gamemode, &haswolflevels);

        switch (gamemode)
        {

        case retail:
        case registered:
        case shareware:
            gamemission = doom;

            break;

        case commercial:
            i = strlen(iwad);
            gamemission = doom2;

            if (i >= 7 && !strncasecmp(iwad + i - 7, "tnt.wad", 7))
                gamemission = pack_tnt;
            else if (i >= 12 && !strncasecmp(iwad + i - 12, "plutonia.wad", 12))
                gamemission = pack_plut;
                
            break;

        default:
            gamemission = none;

            break;

        }

        if (gamemode == indetermined)
            lprintf(LO_WARN,"Unknown Game Version, may not work\n");

        D_AddFile(iwad,source_iwad);
        free(iwad);

    }

    else
    {

        I_Error("IdentifyVersion: IWAD not found\n");

    }

}

const char *wad_files[MAXLOADFILES];
unsigned int desired_screenwidth, desired_screenheight;

static void D_DoomMainSetup(void)
{

    setbuf(stdout, NULL);

    lprintf(LO_INFO,"M_LoadDefaults: Load system defaults.\n");
    M_LoadDefaults();

    IdentifyVersion();

    nomonsters = clnomonsters = 0;
    respawnparm = clrespawnparm = 0;
    fastparm = clfastparm = 0;

    startskill = sk_none;
    startepisode = 1;
    startmap = 1;
    nomusicparm = 0;
    nosfxparm = 0;

    G_ReloadDefaults();

    I_CalculateRes(desired_screenwidth, desired_screenheight);

    lprintf(LO_INFO,"V_Init: allocate screens.\n");
    V_Init();

    {

        int i;

        for (i = 0; i < MAXLOADFILES; i++)
        {

            const char *fname = wad_files[i];
            char *fpath;

            if (!(fname && *fname))
                continue;

            fpath = I_FindFile(fname, ".wad");

            if (!fpath)
            {

                lprintf(LO_WARN, "Failed to autoload %s\n", fname);

            }

            else
            {

                D_AddFile(fpath, source_auto_load);
                free(fpath);

            }
        }
    }

    lprintf(LO_INFO,"D_InitNetGame: Checking for network game.\n");
    D_InitNetGame();

    lprintf(LO_INFO,"W_Init: Init WADfiles.\n");
    W_Init();
    lprintf(LO_INFO,"M_Init: Init miscellaneous info.\n");
    M_Init();
    lprintf(LO_INFO,"R_Init: Init DOOM refresh daemon - ");
    R_Init();
    lprintf(LO_INFO,"\nP_Init: Init Playloop state.\n");
    P_Init();
    lprintf(LO_INFO,"I_Init: Setting up machine state.\n");
    I_Init();
    lprintf(LO_INFO,"S_Init: Setting up sound.\n");
    S_Init(snd_SfxVolume, snd_MusicVolume);
    lprintf(LO_INFO,"HU_Init: Setting up heads up display.\n");
    HU_Init();

    I_InitGraphics();

    lprintf(LO_INFO,"ST_Init: Init status bar.\n");
    ST_Init();

    idmusnum = -1;

    G_InitNew(startskill, startepisode, startmap);

}

void D_DoomMain(void)
{

    D_DoomMainSetup();
    D_DoomLoop();

}

