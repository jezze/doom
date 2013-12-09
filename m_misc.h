#ifndef __M_MISC__
#define __M_MISC__

#include "doomtype.h"

boolean M_WriteFile(char const *name, void *source, int length);
int M_ReadFile(char const *name,byte **buffer);
void M_ScreenShot(void);
void M_DoScreenShot(const char *);
void M_LoadDefaults(void);
void M_SaveDefaults(void);
struct default_s *M_LookupDefault(const char *name);

typedef struct default_s
{

    const char *name;
    struct
    {

        int *pi;
        const char **ppsz;

    } location;
    struct
    {

        int i;
        const char* psz;

    } defaultvalue;
    int minvalue;
    int maxvalue;
    enum
    {

        def_none,
        def_str,
        def_int,
        def_hex,
        def_bool = def_int,
        def_key = def_hex,
        def_mouseb = def_int,
        def_colour = def_hex

    } type;
    int setupscreen;
    int *current;
    struct setup_menu_s *setup_menu;

} default_t;

#define IS_STRING(dv) ((dv).type == def_str)
#define MAX_KEY 65536
#define MAX_MOUSEB 2
#define UL (-123456789)

#endif
