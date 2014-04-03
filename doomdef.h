#ifndef __DOOMDEF__
#define __DOOMDEF__

#define MAX_SCREENWIDTH                 2048
#define MAX_SCREENHEIGHT                1536
#define MAXPLAYERS                      4
#define DEN_PLAYER5                     4001
#define DEN_PLAYER6                     4002
#define DEN_PLAYER7                     4003
#define DEN_PLAYER8                     4004
#define TICRATE                         35
#define MTF_EASY                        1
#define MTF_NORMAL                      2
#define MTF_HARD                        4
#define MTF_AMBUSH                      8
#define MTF_NOTSINGLE                   16
#define MTF_NOTDM                       32
#define MTF_NOTCOOP                     64
#define MTF_FRIEND                      128
#define MTF_RESERVED                    256
#define MORE_FRICTION_MOMENTUM          15000
#define ORIG_FRICTION                   0xE800
#define ORIG_FRICTION_FACTOR            2048
#define KEYD_RIGHTARROW                 0xae
#define KEYD_LEFTARROW                  0xac
#define KEYD_UPARROW                    0xad
#define KEYD_DOWNARROW                  0xaf
#define KEYD_ESCAPE                     27
#define KEYD_ENTER                      13
#define KEYD_TAB                        9
#define KEYD_F1                         (0x80 + 0x3b)
#define KEYD_F2                         (0x80 + 0x3c)
#define KEYD_F3                         (0x80 + 0x3d)
#define KEYD_F4                         (0x80 + 0x3e)
#define KEYD_F5                         (0x80 + 0x3f)
#define KEYD_F6                         (0x80 + 0x40)
#define KEYD_F7                         (0x80 + 0x41)
#define KEYD_F8                         (0x80 + 0x42)
#define KEYD_F9                         (0x80 + 0x43)
#define KEYD_F10                        (0x80 + 0x44)
#define KEYD_F11                        (0x80 + 0x57)
#define KEYD_F12                        (0x80 + 0x58)
#define KEYD_BACKSPACE                  127
#define KEYD_PAUSE                      0xff
#define KEYD_EQUALS                     0x3d
#define KEYD_MINUS                      0x2d
#define KEYD_RSHIFT                     (0x80 + 0x36)
#define KEYD_RCTRL                      (0x80 + 0x1d)
#define KEYD_RALT                       (0x80 + 0x38)
#define KEYD_LALT                       KEYD_RALT
#define KEYD_CAPSLOCK                   0xba
#define KEYD_INSERT                     0xd2
#define KEYD_HOME                       0xc7
#define KEYD_PAGEUP                     0xc9
#define KEYD_PAGEDOWN                   0xd1
#define KEYD_DEL                        0xc8
#define KEYD_END                        0xcf
#define KEYD_SCROLLLOCK                 0xc6
#define KEYD_SPACEBAR                   0x20
#define KEYD_NUMLOCK                    0xC5
#define KEYD_KEYPAD0                    (0x100 + '0')
#define KEYD_KEYPAD1                    (0x100 + '1')
#define KEYD_KEYPAD2                    (0x100 + '2')
#define KEYD_KEYPAD3                    (0x100 + '3')
#define KEYD_KEYPAD4                    (0x100 + '4')
#define KEYD_KEYPAD5                    (0x100 + '5')
#define KEYD_KEYPAD6                    (0x100 + '6')
#define KEYD_KEYPAD7                    (0x100 + '7')
#define KEYD_KEYPAD8                    (0x100 + '8')
#define KEYD_KEYPAD9                    (0x100 + '9')
#define KEYD_KEYPADENTER                (0x100 + KEYD_ENTER)
#define KEYD_KEYPADDIVIDE               (0x100 + '/')
#define KEYD_KEYPADMULTIPLY             (0x100 + '*')
#define KEYD_KEYPADMINUS                (0x100 + '-')
#define KEYD_KEYPADPLUS                 (0x100 + '+')
#define KEYD_KEYPADPERIOD               (0x100 + '.')

typedef enum
{

    shareware,
    registered,
    commercial,
    retail,
    indetermined

} GameMode_t;

typedef enum
{

    GS_LEVEL,
    GS_INTERMISSION,
    GS_FINALE,

} gamestate_t;

typedef enum
{

    sk_none = -1,
    sk_baby = 0,
    sk_easy,
    sk_medium,
    sk_hard,
    sk_nightmare

} skill_t;

typedef enum
{

    it_bluecard,
    it_yellowcard,
    it_redcard,
    it_blueskull,
    it_yellowskull,
    it_redskull,
    NUMCARDS

} card_t;

typedef enum
{

    wp_fist,
    wp_pistol,
    wp_shotgun,
    wp_chaingun,
    wp_missile,
    wp_plasma,
    wp_bfg,
    wp_chainsaw,
    wp_supershotgun,
    NUMWEAPONS,
    wp_nochange

} weapontype_t;

typedef enum
{

    am_clip,
    am_shell,
    am_cell,
    am_misl,
    NUMAMMO,
    am_noammo

} ammotype_t;

typedef enum
{

  pw_invulnerability,
  pw_strength,
  pw_invisibility,
  pw_ironfeet,
  pw_allmap,
  pw_infrared,
  NUMPOWERS

} powertype_t;

typedef enum
{

  INVULNTICS = (30 * TICRATE),
  INVISTICS = (60 * TICRATE),
  INFRATICS = (120 * TICRATE),
  IRONTICS = (60 * TICRATE)

} powerduration_t;

#endif
