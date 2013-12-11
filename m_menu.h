#ifndef __M_MENU__
#define __M_MENU__

#include "d_event.h"

#define S_HILITE                        0x1
#define S_SELECT                        0x2
#define S_END                           0x8000

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

void M_Ticker(void);
void M_Drawer(void);
void M_Init(void);
void M_StartControlPanel(void);
void M_ResetMenu(void);
boolean M_Responder(event_t *ev);

#endif
