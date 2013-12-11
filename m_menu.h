#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"

#define warn_about_changes(x)           (warning_about_changes=(x), print_warning_about_changes = 2)
#define S_HILITE                        0x1
#define S_SELECT                        0x2
#define S_TITLE                         0x4
#define S_YESNO                         0x8
#define S_CRITEM                        0x10
#define S_COLOR                         0x20
#define S_CHAT                          0x40
#define S_RESET                         0x80
#define S_PREV                          0x100
#define S_NEXT                          0x200
#define S_KEY                           0x400
#define S_WEAP                          0x800
#define S_NUM                           0x1000
#define S_SKIP                          0x2000
#define S_KEEP                          0x4000
#define S_END                           0x8000
#define S_LEVWARN                       0x10000
#define S_PRGWARN                       0x20000
#define S_BADVAL                        0x40000
#define S_FILE                          0x80000
#define S_LEFTJUST                      0x100000
#define S_CREDIT                        0x200000
#define S_BADVID                        0x400000
#define S_CHOICE                        0x800000
#define S_SHOWDESC                      (S_TITLE | S_YESNO | S_CRITEM | S_COLOR | S_CHAT | S_RESET | S_PREV | S_NEXT | S_KEY | S_WEAP | S_NUM | S_FILE | S_CREDIT | S_CHOICE)
#define S_SHOWSET                       (S_YESNO | S_CRITEM | S_COLOR | S_CHAT | S_KEY | S_WEAP | S_NUM | S_FILE | S_CHOICE)
#define S_STRING                        (S_CHAT | S_FILE)
#define S_HASDEFPTR                     (S_STRING | S_YESNO|S_NUM | S_WEAP | S_COLOR | S_CRITEM | S_CHOICE)

typedef enum
{

    m_null,
    m_scrn,
    m_map,
    m_menu,

} setup_group;

typedef struct setup_menu_s
{

    const char *m_text;
    int m_flags;
    setup_group m_group;
    short m_x;
    short m_y;
    union
    {

        const void *var;
        int *m_key;
        const char *name;
        struct default_s *def;
        struct setup_menu_s *menu;

    } var;
    int *m_mouse;
    int *m_joy;
    void (*action)(void);
    const char **selectstrings;

} setup_menu_t;

extern int warning_about_changes, print_warning_about_changes;

void M_Ticker(void);
void M_Drawer(void);
void M_Init(void);
void M_StartControlPanel(void);
void M_ResetMenu(void);
void M_DrawCredits(void);
boolean M_Responder(event_t *ev);

#endif
