#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "doomdef.h"
#include "d_think.h"
#include "p_pspr.h"
#include "doomstat.h"
#include "g_game.h"
#include "m_menu.h"
#include "w_wad.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "d_englsh.h"
#include "m_misc.h"
#include "s_sound.h"
#include "d_main.h"
#include "r_draw.h"
#include "r_main.h"
#include "z_zone.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"

#define MAX_KEY                         65536
#define MAX_MOUSEB                      2
#define UL                              (-123456789)

struct setting
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
    int *current;

};

static struct setting defaults[] = {
    {"default_skill", {&defaultskill}, {3}, 1, 5, def_int},
    {"weapon_recoil", {&default_weapon_recoil}, {1}, 0, 1, def_bool, &weapon_recoil},
    {"player_bobbing", {&default_player_bobbing}, {1}, 0, 1, def_bool, &player_bobbing},
    {"monsters_remember", {&default_monsters_remember}, {1}, 0, 1, def_bool, &monsters_remember},
    {"monster_infighting", {&default_monster_infighting}, {1}, 0, 1, def_bool, &monster_infighting},
    {"monster_backing", {&default_monster_backing}, {0}, 0, 1, def_bool, &monster_backing},
    {"monster_avoid_hazards", {&default_monster_avoid_hazards}, {1}, 0, 1, def_bool, &monster_avoid_hazards},
    {"monster_friction", {&default_monster_friction}, {1}, 0, 1, def_bool, &monster_friction},
    {"autorun", {&autorun}, {0}, 0, 1, def_bool},
    {"samplerate", {&snd_samplerate}, {22050}, 11025, 48000, def_int},
    {"sfx_volume", {&snd_SfxVolume}, {8}, 0, 15, def_int},
    {"music_volume", {&snd_MusicVolume}, {8}, 0, 15, def_int},
    {"filter_wall", {(int*)&drawvars.filterwall}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int},
    {"filter_floor", {(int*)&drawvars.filterfloor}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int},
    {"filter_sprite", {(int*)&drawvars.filtersprite}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int},
    {"filter_z", {(int*)&drawvars.filterz}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_LINEAR, def_int},
    {"filter_patch", {(int*)&drawvars.filterpatch},{RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int},
    {"filter_threshold", {(int*)&drawvars.mag_threshold}, {49152}, 0, UL, def_int},
    {"sprite_edges", {(int*)&drawvars.sprite_edges}, {RDRAW_MASKEDCOLUMNEDGE_SQUARE}, RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int},
    {"patch_edges", {(int*)&drawvars.patch_edges}, {RDRAW_MASKEDCOLUMNEDGE_SQUARE}, RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int},
    {"mouse_sensitivity_horiz", {&mouseSensitivity_horiz}, {10}, 0, UL, def_int},
    {"mouse_sensitivity_vert", {&mouseSensitivity_vert}, {10}, 0, UL, def_int},
    {"mouseb_fire", {&mousebfire}, {0}, -1, MAX_MOUSEB, def_int},
    {"mouseb_strafe", {&mousebstrafe}, {1}, -1, MAX_MOUSEB, def_int},
    {"mouseb_forward", {&mousebforward}, {2}, -1, MAX_MOUSEB, def_int},
    {"key_right", {&key_right}, {KEYD_RIGHTARROW}, 0, MAX_KEY, def_key},
    {"key_left", {&key_left}, {KEYD_LEFTARROW}, 0, MAX_KEY,def_key},
    {"key_up", {&key_up}, {KEYD_UPARROW}, 0, MAX_KEY, def_key},
    {"key_down", {&key_down}, {KEYD_DOWNARROW}, 0, MAX_KEY, def_key},
    {"key_menu_right", {&key_menu_right}, {KEYD_RIGHTARROW}, 0, MAX_KEY, def_key},
    {"key_menu_left", {&key_menu_left}, {KEYD_LEFTARROW}, 0, MAX_KEY, def_key},
    {"key_menu_up", {&key_menu_up}, {KEYD_UPARROW}, 0, MAX_KEY, def_key},
    {"key_menu_down", {&key_menu_down}, {KEYD_DOWNARROW}, 0, MAX_KEY, def_key},
    {"key_menu_backspace", {&key_menu_backspace}, {KEYD_BACKSPACE}, 0, MAX_KEY, def_key},
    {"key_menu_escape", {&key_menu_escape}, {KEYD_ESCAPE}, 0, MAX_KEY, def_key},
    {"key_menu_enter", {&key_menu_enter}, {KEYD_ENTER}, 0, MAX_KEY, def_key},
    {"key_strafeleft", {&key_strafeleft}, {','}, 0, MAX_KEY, def_key},
    {"key_straferight", {&key_straferight}, {'.'}, 0, MAX_KEY, def_key},
    {"key_fire", {&key_fire}, {KEYD_RCTRL}, 0, MAX_KEY, def_key},
    {"key_use", {&key_use}, {' '}, 0, MAX_KEY, def_key},
    {"key_strafe", {&key_strafe}, {KEYD_RALT}, 0, MAX_KEY, def_key},
    {"key_speed", {&key_speed}, {KEYD_RSHIFT}, 0, MAX_KEY, def_key},
    {"key_autorun", {&key_autorun}, {KEYD_CAPSLOCK}, 0, MAX_KEY, def_key},
    {"key_backspace", {&key_backspace}, {KEYD_BACKSPACE}, 0, MAX_KEY, def_key},
    {"key_enter", {&key_enter}, {KEYD_ENTER}, 0, MAX_KEY, def_key},
    {"key_reverse", {&key_reverse}, {'/'}, 0, MAX_KEY, def_key},
    {"key_weapontoggle",{&key_weapontoggle}, {'0'}, 0, MAX_KEY, def_key},
    {"key_weapon1", {&key_weapon1}, {'1'}, 0, MAX_KEY, def_key},
    {"key_weapon2", {&key_weapon2}, {'2'}, 0, MAX_KEY, def_key},
    {"key_weapon3", {&key_weapon3}, {'3'}, 0, MAX_KEY, def_key},
    {"key_weapon4", {&key_weapon4}, {'4'}, 0, MAX_KEY, def_key},
    {"key_weapon5", {&key_weapon5}, {'5'}, 0, MAX_KEY, def_key},
    {"key_weapon6", {&key_weapon6}, {'6'}, 0, MAX_KEY, def_key},
    {"key_weapon7", {&key_weapon7}, {'7'}, 0, MAX_KEY, def_key},
    {"key_weapon8", {&key_weapon8}, {'8'}, 0, MAX_KEY, def_key},
    {"key_weapon9", {&key_weapon9}, {'9'}, 0, MAX_KEY, def_key},
    {"health_red", {&health_red}, {25}, 0, 200, def_int},
    {"health_yellow", {&health_yellow}, {50}, 0, 200, def_int},
    {"health_green", {&health_green}, {100}, 0, 200, def_int},
    {"armor_red", {&armor_red}, {25}, 0, 200, def_int},
    {"armor_yellow", {&armor_yellow}, {50}, 0, 200, def_int},
    {"armor_green", {&armor_green}, {100}, 0, 200, def_int},
    {"ammo_red", {&ammo_red}, {25}, 0, 100, def_int},
    {"ammo_yellow", {&ammo_yellow}, {50}, 0, 100, def_int},
    {"weapon_choice_1", {&weapon_preferences[0][0]}, {6}, 0, 9, def_int},
    {"weapon_choice_2", {&weapon_preferences[0][1]}, {9}, 0, 9, def_int},
    {"weapon_choice_3", {&weapon_preferences[0][2]}, {4}, 0, 9, def_int},
    {"weapon_choice_4", {&weapon_preferences[0][3]}, {3}, 0, 9, def_int},
    {"weapon_choice_5", {&weapon_preferences[0][4]}, {2}, 0, 9, def_int},
    {"weapon_choice_6", {&weapon_preferences[0][5]}, {8}, 0, 9, def_int},
    {"weapon_choice_7", {&weapon_preferences[0][6]}, {5}, 0, 9, def_int},
    {"weapon_choice_8", {&weapon_preferences[0][7]}, {7}, 0, 9, def_int},
    {"weapon_choice_9", {&weapon_preferences[0][8]}, {1}, 0, 9, def_int},
    {"mus_e1m1", {0, &S_music_files[mus_e1m1]}, {0,"e1m1.mp3"}, UL, UL, def_str},
    {"mus_e1m2", {0, &S_music_files[mus_e1m2]}, {0,"e1m2.mp3"}, UL, UL, def_str},
    {"mus_e1m3", {0, &S_music_files[mus_e1m3]}, {0,"e1m3.mp3"}, UL, UL, def_str},
    {"mus_e1m4", {0, &S_music_files[mus_e1m4]}, {0,"e1m4.mp3"}, UL, UL, def_str},
    {"mus_e1m5", {0, &S_music_files[mus_e1m5]}, {0,"e1m5.mp3"}, UL, UL, def_str},
    {"mus_e1m6", {0, &S_music_files[mus_e1m6]}, {0,"e1m6.mp3"}, UL, UL, def_str},
    {"mus_e1m7", {0, &S_music_files[mus_e1m7]}, {0,"e1m7.mp3"}, UL, UL, def_str},
    {"mus_e1m8", {0, &S_music_files[mus_e1m8]}, {0,"e1m8.mp3"}, UL, UL, def_str},
    {"mus_e1m9", {0, &S_music_files[mus_e1m9]}, {0,"e1m9.mp3"}, UL, UL, def_str},
    {"mus_e2m1", {0, &S_music_files[mus_e2m1]}, {0,"e2m1.mp3"}, UL, UL, def_str},
    {"mus_e2m2", {0, &S_music_files[mus_e2m2]}, {0,"e2m2.mp3"}, UL, UL, def_str},
    {"mus_e2m3", {0, &S_music_files[mus_e2m3]}, {0,"e2m3.mp3"}, UL, UL, def_str},
    {"mus_e2m4", {0, &S_music_files[mus_e2m4]}, {0,"e2m4.mp3"}, UL, UL, def_str},
    {"mus_e2m5", {0, &S_music_files[mus_e2m5]}, {0,"e1m7.mp3"}, UL, UL, def_str},
    {"mus_e2m6", {0, &S_music_files[mus_e2m6]}, {0,"e2m6.mp3"}, UL, UL, def_str},
    {"mus_e2m7", {0, &S_music_files[mus_e2m7]}, {0,"e2m7.mp3"}, UL, UL, def_str},
    {"mus_e2m8", {0, &S_music_files[mus_e2m8]}, {0,"e2m8.mp3"}, UL, UL, def_str},
    {"mus_e2m9", {0, &S_music_files[mus_e2m9]}, {0,"e3m1.mp3"}, UL, UL, def_str},
    {"mus_e3m1", {0, &S_music_files[mus_e3m1]}, {0,"e3m1.mp3"}, UL, UL, def_str},
    {"mus_e3m2", {0, &S_music_files[mus_e3m2]}, {0,"e3m2.mp3"}, UL, UL, def_str},
    {"mus_e3m3", {0, &S_music_files[mus_e3m3]}, {0,"e3m3.mp3"}, UL, UL, def_str},
    {"mus_e3m4", {0, &S_music_files[mus_e3m4]}, {0,"e1m8.mp3"}, UL, UL, def_str},
    {"mus_e3m5", {0, &S_music_files[mus_e3m5]}, {0,"e1m7.mp3"}, UL, UL, def_str},
    {"mus_e3m6", {0, &S_music_files[mus_e3m6]}, {0,"e1m6.mp3"}, UL, UL, def_str},
    {"mus_e3m7", {0, &S_music_files[mus_e3m7]}, {0,"e2m7.mp3"}, UL, UL, def_str},
    {"mus_e3m8", {0, &S_music_files[mus_e3m8]}, {0,"e3m8.mp3"}, UL, UL, def_str},
    {"mus_e3m9", {0, &S_music_files[mus_e3m9]}, {0,"e1m9.mp3"}, UL, UL, def_str},
    {"mus_inter", {0, &S_music_files[mus_inter]}, {0,"e2m3.mp3"}, UL, UL, def_str},
    {"mus_intro", {0, &S_music_files[mus_intro]}, {0,"intro.mp3"}, UL, UL, def_str},
    {"mus_bunny", {0, &S_music_files[mus_bunny]}, {0,"bunny.mp3"}, UL, UL, def_str},
    {"mus_victor", {0, &S_music_files[mus_victor]}, {0,"victor.mp3"}, UL, UL, def_str},
    {"mus_introa", {0, &S_music_files[mus_introa]}, {0,"intro.mp3"}, UL, UL, def_str},
    {"mus_runnin", {0, &S_music_files[mus_runnin]}, {0,"runnin.mp3"}, UL, UL, def_str},
    {"mus_stalks", {0, &S_music_files[mus_stalks]}, {0,"stalks.mp3"}, UL, UL, def_str},
    {"mus_countd", {0, &S_music_files[mus_countd]}, {0,"countd.mp3"}, UL, UL, def_str},
    {"mus_betwee", {0, &S_music_files[mus_betwee]}, {0,"betwee.mp3"}, UL, UL, def_str},
    {"mus_doom", {0, &S_music_files[mus_doom]}, {0,"doom.mp3"}, UL, UL, def_str},
    {"mus_the_da", {0, &S_music_files[mus_the_da]}, {0,"the_da.mp3"}, UL, UL, def_str},
    {"mus_shawn", {0, &S_music_files[mus_shawn]}, {0,"shawn.mp3"}, UL, UL, def_str},
    {"mus_ddtblu", {0, &S_music_files[mus_ddtblu]}, {0,"ddtblu.mp3"}, UL, UL, def_str},
    {"mus_in_cit", {0, &S_music_files[mus_in_cit]}, {0,"in_cit.mp3"}, UL, UL, def_str},
    {"mus_dead", {0, &S_music_files[mus_dead]}, {0,"dead.mp3"}, UL, UL, def_str},
    {"mus_stlks2", {0, &S_music_files[mus_stlks2]}, {0,"stalks.mp3"}, UL, UL, def_str},
    {"mus_theda2", {0, &S_music_files[mus_theda2]}, {0,"the_da.mp3"}, UL, UL, def_str},
    {"mus_doom2", {0, &S_music_files[mus_doom2]}, {0,"doom.mp3"}, UL, UL, def_str},
    {"mus_ddtbl2", {0, &S_music_files[mus_ddtbl2]}, {0,"ddtblu.mp3"}, UL, UL, def_str},
    {"mus_runni2", {0, &S_music_files[mus_runni2]}, {0,"runnin.mp3"}, UL, UL, def_str},
    {"mus_dead2", {0, &S_music_files[mus_dead2]}, {0,"dead.mp3"}, UL, UL, def_str},
    {"mus_stlks3", {0, &S_music_files[mus_stlks3]}, {0,"stalks.mp3"}, UL, UL, def_str},
    {"mus_romero", {0, &S_music_files[mus_romero]}, {0,"romero.mp3"}, UL, UL, def_str},
    {"mus_shawn2", {0, &S_music_files[mus_shawn2]}, {0,"shawn.mp3"}, UL, UL, def_str},
    {"mus_messag", {0, &S_music_files[mus_messag]}, {0,"messag.mp3"}, UL, UL, def_str},
    {"mus_count2", {0, &S_music_files[mus_count2]}, {0,"countd.mp3"}, UL, UL, def_str},
    {"mus_ddtbl3", {0, &S_music_files[mus_ddtbl3]}, {0,"ddtblu.mp3"}, UL, UL, def_str},
    {"mus_ampie", {0, &S_music_files[mus_ampie]}, {0,"ampie.mp3"}, UL, UL, def_str},
    {"mus_theda3", {0, &S_music_files[mus_theda3]}, {0,"the_da.mp3"}, UL, UL, def_str},
    {"mus_adrian", {0, &S_music_files[mus_adrian]}, {0,"adrian.mp3"}, UL, UL, def_str},
    {"mus_messg2", {0, &S_music_files[mus_messg2]}, {0,"messag.mp3"}, UL, UL, def_str},
    {"mus_romer2", {0, &S_music_files[mus_romer2]}, {0,"romero.mp3"}, UL, UL, def_str},
    {"mus_tense", {0, &S_music_files[mus_tense]}, {0,"tense.mp3"}, UL, UL, def_str},
    {"mus_shawn3", {0, &S_music_files[mus_shawn3]}, {0,"shawn.mp3"}, UL, UL, def_str},
    {"mus_openin", {0, &S_music_files[mus_openin]}, {0,"openin.mp3"}, UL, UL, def_str},
    {"mus_evil", {0, &S_music_files[mus_evil]}, {0,"evil.mp3"}, UL, UL, def_str},
    {"mus_ultima", {0, &S_music_files[mus_ultima]}, {0,"ultima.mp3"}, UL, UL, def_str},
    {"mus_read_m", {0, &S_music_files[mus_read_m]}, {0,"read_m.mp3"}, UL, UL, def_str},
    {"mus_dm2ttl", {0, &S_music_files[mus_dm2ttl]}, {0,"dm2ttl.mp3"}, UL, UL, def_str},
    {"mus_dm2int", {0, &S_music_files[mus_dm2int]}, {0,"dm2int.mp3"}, UL, UL, def_str},
};

void M_LoadDefaults(void)
{

    int i;

    for (i = 0; i < (sizeof(defaults) / sizeof(defaults[0])); i++)
    {

        if (defaults[i].location.ppsz)
            *defaults[i].location.ppsz = strdup(defaults[i].defaultvalue.psz);

        if (defaults[i].location.pi)
            *defaults[i].location.pi = defaults[i].defaultvalue.i;

    }

}

