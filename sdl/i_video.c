#include <stdlib.h>
#include <SDL.h>
#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "r_draw.h"
#include "d_main.h"
#include "d_event.h"
#include "i_video.h"
#include "z_zone.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "lprintf.h"

#define NO_PALETTE_CHANGE 1000

extern void M_QuitDOOM(int choice);

static SDL_Surface *screen;
int leds_always_off = 0;
static int newpal = 0;
static boolean mouse_enabled;
static boolean mouse_currently_grabbed;

static int I_TranslateKey(SDL_keysym *key)
{

    int rc = 0;

    switch (key->sym)
    {

    case SDLK_LEFT: rc = KEYD_LEFTARROW;  break;
    case SDLK_RIGHT:  rc = KEYD_RIGHTARROW; break;
    case SDLK_DOWN: rc = KEYD_DOWNARROW;  break;
    case SDLK_UP:   rc = KEYD_UPARROW;  break;
    case SDLK_ESCAPE: rc = KEYD_ESCAPE; break;
    case SDLK_RETURN: rc = KEYD_ENTER;  break;
    case SDLK_TAB:  rc = KEYD_TAB;    break;
    case SDLK_F1:   rc = KEYD_F1;   break;
    case SDLK_F2:   rc = KEYD_F2;   break;
    case SDLK_F3:   rc = KEYD_F3;   break;
    case SDLK_F4:   rc = KEYD_F4;   break;
    case SDLK_F5:   rc = KEYD_F5;   break;
    case SDLK_F6:   rc = KEYD_F6;   break;
    case SDLK_F7:   rc = KEYD_F7;   break;
    case SDLK_F8:   rc = KEYD_F8;   break;
    case SDLK_F9:   rc = KEYD_F9;   break;
    case SDLK_F10:  rc = KEYD_F10;    break;
    case SDLK_F11:  rc = KEYD_F11;    break;
    case SDLK_F12:  rc = KEYD_F12;    break;
    case SDLK_BACKSPACE:  rc = KEYD_BACKSPACE;  break;
    case SDLK_DELETE: rc = KEYD_DEL;  break;
    case SDLK_INSERT: rc = KEYD_INSERT; break;
    case SDLK_PAGEUP: rc = KEYD_PAGEUP; break;
    case SDLK_PAGEDOWN: rc = KEYD_PAGEDOWN; break;
    case SDLK_HOME: rc = KEYD_HOME; break;
    case SDLK_END:  rc = KEYD_END;  break;
    case SDLK_PAUSE:  rc = KEYD_PAUSE;  break;
    case SDLK_EQUALS: rc = KEYD_EQUALS; break;
    case SDLK_MINUS:  rc = KEYD_MINUS;  break;
    case SDLK_KP0:  rc = KEYD_KEYPAD0;  break;
    case SDLK_KP1:  rc = KEYD_KEYPAD1;  break;
    case SDLK_KP2:  rc = KEYD_KEYPAD2;  break;
    case SDLK_KP3:  rc = KEYD_KEYPAD3;  break;
    case SDLK_KP4:  rc = KEYD_KEYPAD4;  break;
    case SDLK_KP5:  rc = KEYD_KEYPAD5;  break;
    case SDLK_KP6:  rc = KEYD_KEYPAD6;  break;
    case SDLK_KP7:  rc = KEYD_KEYPAD7;  break;
    case SDLK_KP8:  rc = KEYD_KEYPAD8;  break;
    case SDLK_KP9:  rc = KEYD_KEYPAD9;  break;
    case SDLK_KP_PLUS:  rc = KEYD_KEYPADPLUS; break;
    case SDLK_KP_MINUS: rc = KEYD_KEYPADMINUS;  break;
    case SDLK_KP_DIVIDE:  rc = KEYD_KEYPADDIVIDE; break;
    case SDLK_KP_MULTIPLY: rc = KEYD_KEYPADMULTIPLY; break;
    case SDLK_KP_ENTER: rc = KEYD_KEYPADENTER;  break;
    case SDLK_KP_PERIOD:  rc = KEYD_KEYPADPERIOD; break;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: rc = KEYD_RSHIFT; break;
    case SDLK_LCTRL:
    case SDLK_RCTRL:  rc = KEYD_RCTRL;  break;
    case SDLK_LALT:
    case SDLK_LMETA:
    case SDLK_RALT:
    case SDLK_RMETA:  rc = KEYD_RALT;   break;
    case SDLK_CAPSLOCK: rc = KEYD_CAPSLOCK; break;
    default:    rc = key->sym;    break;

    }

    return rc;

}

static int I_SDLtoDoomMouseState(Uint8 buttonstate)
{

    return 0 | (buttonstate & SDL_BUTTON(1) ? 1 : 0) | (buttonstate & SDL_BUTTON(2) ? 2 : 0) | (buttonstate & SDL_BUTTON(3) ? 4 : 0);

}

static void I_GetEvent(SDL_Event *Event)
{

    event_t event;

    switch (Event->type)
    {

    case SDL_KEYDOWN:
        event.type = ev_keydown;
        event.data1 = I_TranslateKey(&Event->key.keysym);

        D_PostEvent(&event);

        break;

    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = I_TranslateKey(&Event->key.keysym);

        D_PostEvent(&event);

        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        if (mouse_enabled)
        {

            event.type = ev_mouse;
            event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
            event.data2 = event.data3 = 0;

            D_PostEvent(&event);
        }

        break;

    case SDL_MOUSEMOTION:
        if (mouse_currently_grabbed)
        {

            event.type = ev_mouse;
            event.data1 = I_SDLtoDoomMouseState(Event->motion.state);
            event.data2 = Event->motion.xrel << 5;
            event.data3 = -Event->motion.yrel << 5;

            D_PostEvent(&event);

        }

        break;

    case SDL_QUIT:
        S_StartSound(NULL, sfx_swtchn);
        M_QuitDOOM(0);

    default:
        break;

    }

}

void I_StartTic(void)
{

    SDL_Event Event;

    boolean should_be_grabbed = mouse_enabled && !(paused || (gamestate != GS_LEVEL) || demoplayback);

    if (mouse_currently_grabbed != should_be_grabbed)
        SDL_WM_GrabInput((mouse_currently_grabbed = should_be_grabbed) ? SDL_GRAB_ON : SDL_GRAB_OFF);

    while (SDL_PollEvent(&Event))
        I_GetEvent(&Event);

}

void I_StartFrame(void)
{

}

static void I_InitInputs(void)
{

    mouse_enabled = 1;

    SDL_WarpMouse((unsigned short)(SCREENWIDTH / 2), (unsigned short)(SCREENHEIGHT / 2));

}

inline static boolean I_SkipFrame(void)
{

    static int frameno;

    frameno++;

    switch (gamestate)
    {

    case GS_LEVEL:
        if (!paused)
            return false;

    default:
        return (frameno & 1) ? true : false;

    }

}

static void I_UploadNewPalette(int pal)
{

    static SDL_Color* colours;
    static int cachedgamma;
    static size_t num_pals;

    if ((colours == NULL) || (cachedgamma != usegamma))
    {

        int pplump = W_GetNumForName("PLAYPAL");
        int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
        register const byte * palette = W_CacheLumpNum(pplump);
        register const byte * const gtable = (const byte *)W_CacheLumpNum(gtlump) + 256*(cachedgamma = usegamma);
        register int i;

        num_pals = W_LumpLength(pplump) / (3*256);
        num_pals *= 256;

        if (!colours)
            colours = malloc(sizeof(*colours)*num_pals);

        for (i = 0; (size_t)i < num_pals; i++)
        {

            colours[i].r = gtable[palette[0]];
            colours[i].g = gtable[palette[1]];
            colours[i].b = gtable[palette[2]];
            palette += 3;

        }

        W_UnlockLumpNum(pplump);
        W_UnlockLumpNum(gtlump);

        num_pals /= 256;

    }

    SDL_SetPalette(SDL_GetVideoSurface(), SDL_LOGPAL | SDL_PHYSPAL, colours + 256 * pal, 0, 256);

}

void I_ShutdownGraphics(void)
{

}

void I_UpdateNoBlit(void)
{

}

void I_FinishUpdate(void)
{

    if (I_SkipFrame())
        return;

    if (SDL_MUSTLOCK(screen))
    {

        int h;
        byte *src;
        byte *dest;

        if (SDL_LockSurface(screen) < 0)
        {

            lprintf(LO_INFO,"I_FinishUpdate: %s\n", SDL_GetError());

            return;

        }

        dest = screen->pixels;
        src = screens[0].data;
        h = screen->h;

        for (; h > 0; h--)
        {

            memcpy(dest, src, SCREENWIDTH);

            dest += screen->pitch;
            src += screens[0].byte_pitch;

        }

        SDL_UnlockSurface(screen);

    }

    if (newpal != NO_PALETTE_CHANGE)
    {

        I_UploadNewPalette(newpal);

        newpal = NO_PALETTE_CHANGE;

    }

    SDL_Flip(screen);

}

void I_SetPalette(int pal)
{

    newpal = pal;

}

static void I_ShutdownSDL(void)
{

    SDL_Quit();

    return;

}

void I_PreInitGraphics(void)
{

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        I_Error("Could not initialize SDL [%s]", SDL_GetError());

    atexit(I_ShutdownSDL);

}

void I_CalculateRes(unsigned int width, unsigned int height)
{

    SCREENWIDTH = (width + 15) & ~15;
    SCREENHEIGHT = height;

    if (!(SCREENWIDTH % 1024))
        SCREENPITCH = SCREENWIDTH + 32;
    else
        SCREENPITCH = SCREENWIDTH;

}

void I_SetRes(void)
{

    int i;

    I_CalculateRes(SCREENWIDTH, SCREENHEIGHT);

    for (i = 0; i < 3; i++)
    {

        screens[i].width = SCREENWIDTH;
        screens[i].height = SCREENHEIGHT;
        screens[i].byte_pitch = SCREENPITCH;

    }

    screens[4].width = SCREENWIDTH;
    screens[4].height = (ST_SCALED_HEIGHT + 1);
    screens[4].byte_pitch = SCREENPITCH;

    lprintf(LO_INFO,"I_SetRes: Using resolution %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

}

void I_InitGraphics(void)
{

    static int firsttime = 1;

    if (firsttime)
    {

        firsttime = 0;

        atexit(I_ShutdownGraphics);
        lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);
        I_UpdateVideoMode();
        I_InitInputs();

    }

}

void I_UpdateVideoMode(void)
{

    lprintf(LO_INFO, "I_UpdateVideoMode: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

    V_InitMode();
    V_FreeScreens();
    I_SetRes();

    screen = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, 8, SDL_DOUBLEBUF | SDL_HWPALETTE | SDL_FULLSCREEN);

    if (screen == NULL)
        I_Error("Couldn't set %dx%d video mode [%s]", SCREENWIDTH, SCREENHEIGHT, SDL_GetError());

    lprintf(LO_INFO, "I_UpdateVideoMode: %s, %s\n", screen->pixels ? "SDL buffer" : "own buffer", SDL_MUSTLOCK(screen) ? "lock-and-copy": "direct access");

    mouse_currently_grabbed = false;

    if (!SDL_MUSTLOCK(screen))
    {

        screens[0].not_on_heap = true;
        screens[0].data = (unsigned char *)(screen->pixels);
        screens[0].byte_pitch = screen->pitch;
    }

    else
    {

        screens[0].not_on_heap = false;

    }

    V_AllocScreens();
    SDL_ShowCursor(0);
    R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);

}

