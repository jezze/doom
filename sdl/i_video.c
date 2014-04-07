#include <stdlib.h>
#include <SDL.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "r_data.h"
#include "r_draw.h"
#include "v_video.h"
#include "st_stuff.h"
#include "i_system.h"
#include "i_video.h"

#define NO_PALETTE_CHANGE 1000

int SCREENWIDTH = 320;
int SCREENHEIGHT = 200;
int SCREENPITCH = 320;
static SDL_Surface *surface;
static SDL_Color* colours;
static size_t num_pals;
static int newpal = 0;
static int frameno;

static int getkey(SDL_keysym *key)
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

static int getmouse(Uint8 buttonstate)
{

    return 0 | (buttonstate & SDL_BUTTON(1) ? 1 : 0) | (buttonstate & SDL_BUTTON(2) ? 2 : 0) | (buttonstate & SDL_BUTTON(3) ? 4 : 0);

}

static void postevent(SDL_Event *Event)
{

    event_t event;

    switch (Event->type)
    {

    case SDL_KEYDOWN:
        event.type = ev_keydown;
        event.data1 = getkey(&Event->key.keysym);

        D_PostEvent(&event);

        break;

    case SDL_KEYUP:
        event.type = ev_keyup;
        event.data1 = getkey(&Event->key.keysym);

        D_PostEvent(&event);

        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        event.type = ev_mouse;
        event.data1 = getmouse(SDL_GetMouseState(NULL, NULL));
        event.data2 = 0;
        event.data3 = 0;

        D_PostEvent(&event);

        break;

    case SDL_MOUSEMOTION:
        event.type = ev_mouse;
        event.data1 = getmouse(Event->motion.state);
        event.data2 = Event->motion.xrel << 5;
        event.data3 = -Event->motion.yrel << 5;

        D_PostEvent(&event);

        break;

    case SDL_QUIT:
        I_Exit(0);

    default:
        break;

    }

}

static boolean skipframe(void)
{

    frameno++;

    switch (gamestate)
    {

    case GS_LEVEL:
        return false;

    default:
        return (frameno & 1) ? true : false;

    }

}

static void setpalette(int pal)
{

    if (colours == NULL)
    {

        int pplump = W_GetNumForName("PLAYPAL");
        register const byte * palette = W_CacheLumpNum(pplump);
        register int i;

        num_pals = W_LumpLength(pplump) / (3*256);
        num_pals *= 256;

        if (!colours)
            colours = malloc(sizeof(*colours) * num_pals);

        for (i = 0; (size_t)i < num_pals; i++)
        {

            colours[i].r = palette[0];
            colours[i].g = palette[1];
            colours[i].b = palette[2];
            palette += 3;

        }

        W_UnlockLumpNum(pplump);

        num_pals /= 256;

    }

    SDL_SetPalette(surface, SDL_LOGPAL | SDL_PHYSPAL, colours + 256 * pal, 0, 256);

}

void I_StartTic(void)
{

    SDL_Event Event;

    while (SDL_PollEvent(&Event))
        postevent(&Event);

}

void I_FinishUpdate(void)
{

    if (skipframe())
        return;

    if (SDL_MUSTLOCK(surface))
    {

        int h;
        byte *src;
        byte *dest;

        if (SDL_LockSurface(surface) < 0)
        {

            I_Error("I_FinishUpdate: %s", SDL_GetError());

            return;

        }

        dest = surface->pixels;
        src = screens[0].data;
        h = surface->h;

        for (; h > 0; h--)
        {

            memcpy(dest, src, SCREENWIDTH);

            dest += surface->pitch;
            src += screens[0].byte_pitch;

        }

        SDL_UnlockSurface(surface);

    }

    if (newpal != NO_PALETTE_CHANGE)
    {

        setpalette(newpal);

        newpal = NO_PALETTE_CHANGE;

    }

    SDL_Flip(surface);

}

void I_SetPalette(int pal)
{

    newpal = pal;

}

void I_PreInitGraphics(void)
{

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        I_Error("I_PreInitGraphics: Could not initialize SDL [%s]", SDL_GetError());

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

void I_InitGraphics(void)
{

    int i;

    V_InitMode();
    V_FreeScreens();
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
    surface = SDL_SetVideoMode(SCREENWIDTH, SCREENHEIGHT, 8, SDL_DOUBLEBUF | SDL_HWPALETTE | SDL_FULLSCREEN);

    if (surface == NULL)
        I_Error("I_InitGraphics: Couldn't set %dx%d video mode [%s]", SCREENWIDTH, SCREENHEIGHT, SDL_GetError());

    if (!SDL_MUSTLOCK(surface))
    {

        screens[0].not_on_heap = true;
        screens[0].data = (unsigned char *)(surface->pixels);
        screens[0].byte_pitch = surface->pitch;
    }

    else
    {

        screens[0].not_on_heap = false;

    }

    V_AllocScreens();
    R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);
    SDL_ShowCursor(0);
    SDL_WM_GrabInput(SDL_GRAB_ON);
    SDL_WarpMouse((unsigned short)(SCREENWIDTH / 2), (unsigned short)(SCREENHEIGHT / 2));

}

