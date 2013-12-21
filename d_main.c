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

boolean clrespawnparm;
boolean clfastparm;
boolean respawnparm;
boolean fastparm;
boolean nosfxparm;
boolean nomusicparm;

skill_t startskill;
int startepisode;
int startmap;

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

void D_Display(void)
{

    if (!I_StartDisplay())
        return;

    if (gamestate != GS_LEVEL)
    {

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

        HU_Erase();
        R_RenderPlayerView(&players[displayplayer]);
        ST_Drawer();
        HU_Drawer();

    }

    M_Drawer();
    D_BuildNewTiccmds();
    I_FinishUpdate();
    I_EndDisplay();

}

static void D_DoomLoop(void)
{

    for (;;)
    {

        I_StartFrame();
        TryRunTics();

        if (players[displayplayer].mo)
            S_UpdateSounds(players[displayplayer].mo);

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

static void CheckIWAD(const char *iwadname, GameMode_t *gmode, boolean *hassec)
{

    int ud = 0, rg = 0, sw = 0, cm = 0, sc = 0;
    FILE* fp;
    wadinfo_t header;
    size_t length;
    filelump_t *fileinfo;

    if (!(fp = fopen(iwadname, "rb")))
        I_Error("CheckIWAD: could not open %s", iwadname);

    if (fread(&header, sizeof (header), 1, fp) != 1)
        I_Error("CheckIWAD: could not read %s", iwadname);

    if (strncmp(header.identification, "IWAD", 4))
        I_Error("CheckIWAD: identification failed");

    header.numlumps = header.numlumps;
    header.infotableofs = header.infotableofs;
    length = header.numlumps;
    fileinfo = malloc(length * sizeof (filelump_t));

    if (fseek(fp, header.infotableofs, SEEK_SET))
        I_Error("CheckIWAD: failed to seek");

    if (fread(fileinfo, sizeof (filelump_t), length, fp) != length)
        I_Error("CheckIWAD: failed to read");

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
            {

                if (fileinfo[length].name[4] == '1' || fileinfo[length].name[4] == '2')
                    ++sc;

            }

        }

    }

    free(fileinfo);

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

static char *FindIWADFile(void)
{

    int i;
    char *iwad  = NULL;

    for (i = 0; !iwad && i < nstandard_iwads; i++)
        iwad = I_FindFile(standard_iwads[i], ".wad");

    return iwad;

}

static void IdentifyVersion(void)
{

    int i;
    char *iwad = FindIWADFile();

    if (iwad && *iwad)
    {

        I_Print("IWAD found: %s\n", iwad);
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
            I_Print("Unknown Game Version, may not work\n");

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

    I_Print("M_LoadDefaults: Load system defaults.\n");
    M_LoadDefaults();

    IdentifyVersion();

    respawnparm = clrespawnparm = 0;
    fastparm = clfastparm = 0;

    startskill = sk_none;
    startepisode = 1;
    startmap = 1;
    nomusicparm = 0;
    nosfxparm = 0;

    G_ReloadDefaults();
    I_CalculateRes(desired_screenwidth, desired_screenheight);
    I_Print("V_Init: allocate screens.\n");
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

                I_Print("Failed to autoload %s\n", fname);

            }

            else
            {

                D_AddFile(fpath, source_auto_load);
                free(fpath);

            }

        }

    }

    I_Print("D_InitNetGame: Checking for network game.\n");
    D_InitNetGame();
    I_Print("W_Init: Init WADfiles.\n");
    W_Init();
    I_Print("M_Init: Init miscellaneous info.\n");
    M_Init();
    I_Print("R_Init: Init DOOM refresh daemon - ");
    R_Init();
    I_Print("\nP_Init: Init Playloop state.\n");
    P_Init();
    I_Print("I_Init: Setting up machine state.\n");
    I_Init();
    I_Print("S_Init: Setting up sound.\n");
    S_Init(snd_SfxVolume, snd_MusicVolume);
    I_Print("HU_Init: Setting up heads up display.\n");
    HU_Init();
    I_InitGraphics();
    I_Print("ST_Init: Init status bar.\n");
    ST_Init();

    idmusnum = -1;

    G_InitNew(startskill, startepisode, startmap);

}

void D_DoomMain(void)
{

    D_DoomMainSetup();
    D_DoomLoop();

}

