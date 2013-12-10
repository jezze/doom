#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "doomstat.h"
#include "g_game.h"
#include "m_menu.h"
#include "am_map.h"
#include "w_wad.h"
#include "i_system.h"
#include "i_sound.h"
#include "i_video.h"
#include "v_video.h"
#include "hu_stuff.h"
#include "st_stuff.h"
#include "d_englsh.h"
#include "m_misc.h"
#include "s_sound.h"
#include "sounds.h"
#include "lprintf.h"
#include "d_main.h"
#include "r_draw.h"
#include "r_demo.h"
#include "r_fps.h"
#include "z_zone.h"

static inline void I_BeginRead(void) {}
static inline void I_EndRead(void) {}

boolean M_WriteFile(char const *name, void *source, int length)
{
  FILE *fp;

  errno = 0;

  if (!(fp = fopen(name, "wb")))
    return 0;

  I_BeginRead();
  length = fwrite(source, 1, length, fp) == (size_t)length;
  fclose(fp);
  I_EndRead();

  if (!length)
    remove(name);

  return length;
}

int M_ReadFile(char const *name, byte **buffer)
{
  FILE *fp;

  if ((fp = fopen(name, "rb")))
    {
      size_t length;

      I_BeginRead();
      fseek(fp, 0, SEEK_END);
      length = ftell(fp);
      fseek(fp, 0, SEEK_SET);
      *buffer = Z_Malloc(length, PU_STATIC, 0);
      if (fread(*buffer, 1, length, fp) == length)
        {
          fclose(fp);
          I_EndRead();
          return length;
        }
      fclose(fp);
    }

  return -1;
}

boolean    precache = true; /* if true, load all graphics at start */
extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;
extern int viewwidth;
extern int viewheight;
extern int realtic_clock_rate;
extern int screenblocks;
extern int showMessages;
int         mus_pause_opt;
extern const char* chat_macros[];
int endoom_mode;
extern const char* S_music_files[];
int map_point_coordinates;

default_t defaults[] =
{
  {"Misc settings", {NULL}, {0}, UL, UL, def_none,ss_none},
  {"default_compatibility_level", {(int*)&default_compatibility_level}, {-1}, -1, MAX_COMPATIBILITY_LEVEL - 1, def_int,ss_none},
  {"realtic_clock_rate", {&realtic_clock_rate}, {100}, 0, UL, def_int, ss_none},
  {"max_player_corpse", {&bodyquesize}, {32}, -1, UL, def_int, ss_none},
  {"flashing_hom", {&flashing_hom},{0}, 0, 1, def_bool,ss_none},
  {"demo_insurance", {&default_demo_insurance}, {2}, 0, 2, def_int, ss_none},
  {"endoom_mode", {&endoom_mode},{5},0,7, def_hex, ss_none},
  {"level_precache", {(int*)&precache}, {0}, 0, 1, def_bool, ss_none},
  {"demo_smoothturns", {&demo_smoothturns}, {0}, 0, 1, def_bool, ss_stat},
  {"demo_smoothturnsfactor", {&demo_smoothturnsfactor}, {6}, 1, SMOOTH_PLAYING_MAXFACTOR, def_int,ss_stat},
  {"Files", {NULL}, {0}, UL, UL, def_none,ss_none}, {"wadfile_1", {NULL, &wad_files[0]} , {0,""} , UL, UL, def_str, ss_none},
  {"wadfile_2", {NULL, &wad_files[1]}, {0,""}, UL, UL, def_str, ss_none}, {"Game settings", {NULL}, {0}, UL, UL, def_none, ss_none},
  {"default_skill", {&defaultskill}, {3}, 1, 5, def_int, ss_none},
  {"weapon_recoil", {&default_weapon_recoil}, {0}, 0, 1, def_bool, ss_weap, &weapon_recoil},
  {"doom_weapon_toggles", {&doom_weapon_toggles}, {1}, 0, 1, def_bool, ss_weap},
  {"player_bobbing", {&default_player_bobbing}, {1}, 0, 1, def_bool,ss_weap, &player_bobbing},
  {"monsters_remember", {&default_monsters_remember}, {1}, 0, 1, def_bool, ss_enem, &monsters_remember},
  {"monster_infighting", {&default_monster_infighting}, {1}, 0, 1, def_bool, ss_enem, &monster_infighting},
  {"monster_backing", {&default_monster_backing}, {0}, 0, 1, def_bool, ss_enem, &monster_backing},
  {"monster_avoid_hazards", {&default_monster_avoid_hazards}, {1}, 0, 1, def_bool, ss_enem, &monster_avoid_hazards},
  {"monster_friction", {&default_monster_friction}, {1}, 0, 1, def_bool, ss_enem, &monster_friction},
  {"help_friends", {&default_help_friends}, {1}, 0, 1, def_bool, ss_enem, &help_friends},
  {"allow_pushers", {&default_allow_pushers}, {1}, 0, 1, def_bool,ss_weap, &allow_pushers},
  {"variable_friction", {&default_variable_friction}, {1}, 0, 1, def_bool,ss_weap, &variable_friction},
  {"sts_always_red", {&sts_always_red}, {1}, 0, 1, def_bool,ss_stat},
  {"sts_pct_always_gray", {&sts_pct_always_gray}, {0}, 0, 1, def_bool,ss_stat},
  {"sts_traditional_keys", {&sts_traditional_keys}, {0}, 0, 1, def_bool,ss_stat},
  {"show_messages", {&showMessages}, {1}, 0, 1, def_bool,ss_none},
  {"autorun", {&autorun}, {0}, 0, 1, def_bool,ss_none},
  {"Compatibility settings", {NULL}, {0}, UL, UL, def_none, ss_none},
  {"comp_zombie", {&default_comp[comp_zombie]}, {0}, 0, 1, def_bool, ss_comp, &comp[comp_zombie]},
  {"comp_infcheat", {&default_comp[comp_infcheat]}, {0}, 0, 1, def_bool, ss_comp, &comp[comp_infcheat]},
  {"comp_stairs", {&default_comp[comp_stairs]}, {0}, 0, 1, def_bool, ss_comp, &comp[comp_stairs]},
  {"comp_telefrag", {&default_comp[comp_telefrag]}, {0}, 0, 1, def_bool, ss_comp, &comp[comp_telefrag]},
  {"comp_dropoff", {&default_comp[comp_dropoff]}, {0},0,1, def_bool, ss_comp, &comp[comp_dropoff]},
  {"comp_falloff", {&default_comp[comp_falloff]}, {0},0,1, def_bool, ss_comp, &comp[comp_falloff]},
  {"comp_staylift", {&default_comp[comp_staylift]}, {0},0,1, def_bool, ss_comp, &comp[comp_staylift]},
  {"comp_doorstuck", {&default_comp[comp_doorstuck]}, {0},0,1, def_bool, ss_comp, &comp[comp_doorstuck]},
  {"comp_pursuit", {&default_comp[comp_pursuit]}, {0},0,1, def_bool, ss_comp, &comp[comp_pursuit]},
  {"comp_vile", {&default_comp[comp_vile]}, {0},0,1, def_bool, ss_comp, &comp[comp_vile]},
  {"comp_pain", {&default_comp[comp_pain]}, {0},0,1, def_bool, ss_comp, &comp[comp_pain]},
  {"comp_skull", {&default_comp[comp_skull]}, {0},0,1, def_bool, ss_comp, &comp[comp_skull]},
  {"comp_blazing", {&default_comp[comp_blazing]}, {0},0,1, def_bool, ss_comp, &comp[comp_blazing]},
  {"comp_doorlight", {&default_comp[comp_doorlight]}, {0},0,1, def_bool, ss_comp, &comp[comp_doorlight]},
  {"comp_god", {&default_comp[comp_god]}, {0},0,1, def_bool, ss_comp, &comp[comp_god]},
  {"comp_skymap", {&default_comp[comp_skymap]}, {0},0,1, def_bool, ss_comp, &comp[comp_skymap]},
  {"comp_floors", {&default_comp[comp_floors]}, {0},0,1, def_bool, ss_comp, &comp[comp_floors]},
  {"comp_model", {&default_comp[comp_model]}, {0},0,1, def_bool, ss_comp, &comp[comp_model]},
  {"comp_zerotags", {&default_comp[comp_zerotags]}, {0},0,1, def_bool, ss_comp, &comp[comp_zerotags]},
  {"comp_moveblock", {&default_comp[comp_moveblock]}, {0},0,1, def_bool, ss_comp, &comp[comp_moveblock]},
  {"comp_sound", {&default_comp[comp_sound]}, {0},0,1, def_bool, ss_comp, &comp[comp_sound]},
  {"comp_666", {&default_comp[comp_666]}, {0},0,1, def_bool, ss_comp, &comp[comp_666]},
  {"comp_soul", {&default_comp[comp_soul]}, {0},0,1, def_bool, ss_comp, &comp[comp_soul]},
  {"comp_maskedanim", {&default_comp[comp_maskedanim]}, {0}, 0, 1, def_bool, ss_comp, &comp[comp_maskedanim]},
  {"Sound settings", {NULL}, {0}, UL, UL, def_none, ss_none},
  {"sound_card", {&snd_card}, {-1}, -1, 7, def_int,ss_none},
  {"music_card", {&mus_card}, {-1}, -1, 9, def_int, ss_none},
  {"samplerate", {&snd_samplerate}, {22050}, 11025, 48000, def_int, ss_none},
  {"sfx_volume", {&snd_SfxVolume}, {8}, 0, 15, def_int, ss_none},
  {"music_volume", {&snd_MusicVolume}, {8}, 0, 15, def_int,ss_none},
  {"mus_pause_opt", {&mus_pause_opt}, {2}, 0, 2, def_int, ss_none},
  {"Video settings", {NULL}, {0}, UL, UL, def_none, ss_none},
  {"screen_width", {&desired_screenwidth}, {640}, 320, MAX_SCREENWIDTH, def_int, ss_none},
  {"screen_height", {&desired_screenheight}, {480}, 200, MAX_SCREENHEIGHT, def_int, ss_none},
  {"screenblocks", {&screenblocks}, {10}, 3, 11, def_int, ss_none},
  {"usegamma", {&usegamma}, {3}, 0, 4, def_int, ss_none},
  {"filter_wall", {(int*)&drawvars.filterwall}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int, ss_none},
  {"filter_floor", {(int*)&drawvars.filterfloor}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int, ss_none},
  {"filter_sprite", {(int*)&drawvars.filtersprite}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int, ss_none},
  {"filter_z", {(int*)&drawvars.filterz}, {RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_LINEAR, def_int, ss_none},
  {"filter_patch", {(int*)&drawvars.filterpatch},{RDRAW_FILTER_POINT}, RDRAW_FILTER_POINT, RDRAW_FILTER_ROUNDED, def_int, ss_none},
  {"filter_threshold", {(int*)&drawvars.mag_threshold}, {49152}, 0, UL, def_int, ss_none},
  {"sprite_edges", {(int*)&drawvars.sprite_edges}, {RDRAW_MASKEDCOLUMNEDGE_SQUARE}, RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int, ss_none},
  {"patch_edges", {(int*)&drawvars.patch_edges}, {RDRAW_MASKEDCOLUMNEDGE_SQUARE}, RDRAW_MASKEDCOLUMNEDGE_SQUARE, RDRAW_MASKEDCOLUMNEDGE_SLOPED, def_int, ss_none},
  {"mouse_sensitivity_horiz", {&mouseSensitivity_horiz}, {10}, 0, UL, def_int, ss_none},
  {"mouse_sensitivity_vert", {&mouseSensitivity_vert}, {10}, 0, UL, def_int, ss_none},
  {"mouseb_fire", {&mousebfire}, {0}, -1, MAX_MOUSEB, def_int, ss_keys},
  {"mouseb_strafe", {&mousebstrafe}, {1}, -1, MAX_MOUSEB, def_int, ss_keys},
  {"mouseb_forward", {&mousebforward}, {2}, -1, MAX_MOUSEB, def_int, ss_keys},
  {"Key bindings", {NULL}, {0}, UL, UL, def_none, ss_none},
  {"key_right", {&key_right}, {KEYD_RIGHTARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_left", {&key_left}, {KEYD_LEFTARROW}, 0, MAX_KEY,def_key,ss_keys},
  {"key_up", {&key_up}, {KEYD_UPARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_down", {&key_down}, {KEYD_DOWNARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_right", {&key_menu_right}, {KEYD_RIGHTARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_left", {&key_menu_left}, {KEYD_LEFTARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_up", {&key_menu_up}, {KEYD_UPARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_down", {&key_menu_down}, {KEYD_DOWNARROW}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_backspace", {&key_menu_backspace}, {KEYD_BACKSPACE}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_escape", {&key_menu_escape}, {KEYD_ESCAPE}, 0, MAX_KEY, def_key, ss_keys},
  {"key_menu_enter", {&key_menu_enter}, {KEYD_ENTER}, 0, MAX_KEY, def_key, ss_keys},
  {"key_strafeleft", {&key_strafeleft}, {','}, 0, MAX_KEY, def_key, ss_keys},
  {"key_straferight", {&key_straferight}, {'.'}, 0, MAX_KEY, def_key, ss_keys},
  {"key_fire",        {&key_fire},           {KEYD_RCTRL}     ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_use",         {&key_use},            {' '}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_strafe",      {&key_strafe},         {KEYD_RALT}      ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_speed",       {&key_speed},          {KEYD_RSHIFT}    ,
   0,MAX_KEY,def_key,ss_keys},

  {"key_savegame",    {&key_savegame},       {KEYD_F2}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_loadgame",    {&key_loadgame},       {KEYD_F3}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_soundvolume", {&key_soundvolume},    {KEYD_F4}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_hud",         {&key_hud},            {KEYD_F5}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_quicksave",   {&key_quicksave},      {KEYD_F6}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_endgame",     {&key_endgame},        {KEYD_F7}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_messages",    {&key_messages},       {KEYD_F8}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_quickload",   {&key_quickload},      {KEYD_F9}        ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_quit",        {&key_quit},           {KEYD_F10}       ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_gamma",       {&key_gamma},          {KEYD_F11}       ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_spy",         {&key_spy},            {KEYD_F12}       ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_pause",       {&key_pause},          {KEYD_PAUSE}     ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_autorun",     {&key_autorun},        {KEYD_CAPSLOCK}  ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_chat",        {&key_chat},           {'t'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_backspace",   {&key_backspace},      {KEYD_BACKSPACE} ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_enter",       {&key_enter},          {KEYD_ENTER}     ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map",         {&key_map},            {KEYD_TAB}       ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_right",   {&key_map_right},      {KEYD_RIGHTARROW},
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_left",    {&key_map_left},       {KEYD_LEFTARROW} ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_up",      {&key_map_up},         {KEYD_UPARROW}   ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_down",    {&key_map_down},       {KEYD_DOWNARROW} ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_zoomin",  {&key_map_zoomin},      {'='}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_zoomout", {&key_map_zoomout},     {'-'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_gobig",   {&key_map_gobig},       {'0'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_follow",  {&key_map_follow},      {'f'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_mark",    {&key_map_mark},        {'m'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_clear",   {&key_map_clear},       {'c'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_grid",    {&key_map_grid},        {'g'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_rotate",  {&key_map_rotate},      {'r'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_map_overlay", {&key_map_overlay},     {'o'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_reverse",     {&key_reverse},         {'/'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_zoomin",      {&key_zoomin},          {'='}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_zoomout",     {&key_zoomout},         {'-'}           ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_chatplayer1", {&destination_keys[0]}, {'g'}            ,
   0,MAX_KEY,def_key,ss_keys},

  {"key_chatplayer2", {&destination_keys[1]}, {'i'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_chatplayer3", {&destination_keys[2]}, {'b'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_chatplayer4", {&destination_keys[3]}, {'r'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapontoggle",{&key_weapontoggle},    {'0'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon1",     {&key_weapon1},         {'1'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon2",     {&key_weapon2},         {'2'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon3",     {&key_weapon3},         {'3'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon4",     {&key_weapon4},         {'4'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon5",     {&key_weapon5},         {'5'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon6",     {&key_weapon6},         {'6'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon7",     {&key_weapon7},         {'7'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon8",     {&key_weapon8},         {'8'}            ,
   0,MAX_KEY,def_key,ss_keys},
  {"key_weapon9",     {&key_weapon9},         {'9'}            ,
   0,MAX_KEY,def_key,ss_keys},


  {"key_screenshot",  {&key_screenshot},      {'*'}            ,
   0,MAX_KEY,def_key,ss_keys},

  {"Chat macros",{NULL},{0},UL,UL,def_none,ss_none},
  {"chatmacro0", {0,&chat_macros[0]}, {0,HUSTR_CHATMACRO0},UL,UL,
   def_str,ss_chat},
  {"chatmacro1", {0,&chat_macros[1]}, {0,HUSTR_CHATMACRO1},UL,UL,
   def_str,ss_chat},
  {"chatmacro2", {0,&chat_macros[2]}, {0,HUSTR_CHATMACRO2},UL,UL,
   def_str,ss_chat},
  {"chatmacro3", {0,&chat_macros[3]}, {0,HUSTR_CHATMACRO3},UL,UL,
   def_str,ss_chat},
  {"chatmacro4", {0,&chat_macros[4]}, {0,HUSTR_CHATMACRO4},UL,UL,
   def_str,ss_chat},
  {"chatmacro5", {0,&chat_macros[5]}, {0,HUSTR_CHATMACRO5},UL,UL,
   def_str,ss_chat},
  {"chatmacro6", {0,&chat_macros[6]}, {0,HUSTR_CHATMACRO6},UL,UL,
   def_str,ss_chat},
  {"chatmacro7", {0,&chat_macros[7]}, {0,HUSTR_CHATMACRO7},UL,UL,
   def_str,ss_chat},
  {"chatmacro8", {0,&chat_macros[8]}, {0,HUSTR_CHATMACRO8},UL,UL,
   def_str,ss_chat},
  {"chatmacro9", {0,&chat_macros[9]}, {0,HUSTR_CHATMACRO9},UL,UL,
   def_str,ss_chat},

  {"Automap settings",{NULL},{0},UL,UL,def_none,ss_none},


  {"mapcolor_back", {&mapcolor_back}, {247},0,255,
   def_colour,ss_auto},
  {"mapcolor_grid", {&mapcolor_grid}, {104},0,255,
   def_colour,ss_auto},
  {"mapcolor_wall", {&mapcolor_wall}, {23},0,255,
   def_colour,ss_auto},
  {"mapcolor_fchg", {&mapcolor_fchg}, {55},0,255,
   def_colour,ss_auto},
  {"mapcolor_cchg", {&mapcolor_cchg}, {215},0,255,
   def_colour,ss_auto},
  {"mapcolor_clsd", {&mapcolor_clsd}, {208},0,255,
   def_colour,ss_auto},
  {"mapcolor_rkey", {&mapcolor_rkey}, {175},0,255,
   def_colour,ss_auto},
  {"mapcolor_bkey", {&mapcolor_bkey}, {204},0,255,
   def_colour,ss_auto},
  {"mapcolor_ykey", {&mapcolor_ykey}, {231},0,255,
   def_colour,ss_auto},
  {"mapcolor_rdor", {&mapcolor_rdor}, {175},0,255,
   def_colour,ss_auto},
  {"mapcolor_bdor", {&mapcolor_bdor}, {204},0,255,
   def_colour,ss_auto},
  {"mapcolor_ydor", {&mapcolor_ydor}, {231},0,255,
   def_colour,ss_auto},
  {"mapcolor_tele", {&mapcolor_tele}, {119},0,255,
   def_colour,ss_auto},
  {"mapcolor_secr", {&mapcolor_secr}, {252},0,255,
   def_colour,ss_auto},
  {"mapcolor_exit", {&mapcolor_exit}, {0},0,255,
   def_colour,ss_auto},
  {"mapcolor_unsn", {&mapcolor_unsn}, {104},0,255,
   def_colour,ss_auto},
  {"mapcolor_flat", {&mapcolor_flat}, {88},0,255,
   def_colour,ss_auto},
  {"mapcolor_sprt", {&mapcolor_sprt}, {112},0,255,
   def_colour,ss_auto},
  {"mapcolor_item", {&mapcolor_item}, {231},0,255,
   def_colour,ss_auto},
  {"mapcolor_hair", {&mapcolor_hair}, {208},0,255,
   def_colour,ss_auto},
  {"mapcolor_sngl", {&mapcolor_sngl}, {208},0,255,
   def_colour,ss_auto},
  {"mapcolor_me",   {&mapcolor_me}, {112},0,255,
   def_colour,ss_auto},
  {"mapcolor_enemy",   {&mapcolor_enemy}, {177},0,255,
   def_colour,ss_auto},
  {"mapcolor_frnd",   {&mapcolor_frnd}, {112},0,255,
   def_colour,ss_auto},

  {"map_secret_after", {&map_secret_after}, {0},0,1,
   def_bool,ss_auto},
  {"map_point_coord", {&map_point_coordinates}, {0},0,1,
   def_bool,ss_auto},

  {"automapmode", {(int*)&automapmode}, {0}, 0, 31,
   def_hex,ss_none},

  {"Heads-up display settings",{NULL},{0},UL,UL,def_none,ss_none},

  {"hudcolor_titl", {&hudcolor_titl}, {5},0,9,
   def_int,ss_auto},
  {"hudcolor_xyco", {&hudcolor_xyco}, {3},0,9,
   def_int,ss_auto},
  {"hudcolor_mesg", {&hudcolor_mesg}, {6},0,9,
   def_int,ss_mess},
  {"hudcolor_chat", {&hudcolor_chat}, {5},0,9,
   def_int,ss_mess},
  {"hudcolor_list", {&hudcolor_list}, {5},0,9,
   def_int,ss_mess},
  {"hud_msg_lines", {&hud_msg_lines}, {1},1,16,
   def_int,ss_mess},
  {"hud_list_bgon", {&hud_list_bgon}, {0},0,1,
   def_bool,ss_mess},
  {"hud_distributed",{&hud_distributed},{0},0,1,
   def_bool,ss_none},

  {"health_red",    {&health_red}   , {25},0,200,
   def_int,ss_stat},
  {"health_yellow", {&health_yellow}, {50},0,200,
   def_int,ss_stat},
  {"health_green",  {&health_green} , {100},0,200,
   def_int,ss_stat},
  {"armor_red",     {&armor_red}    , {25},0,200,
   def_int,ss_stat},
  {"armor_yellow",  {&armor_yellow} , {50},0,200,
   def_int,ss_stat},
  {"armor_green",   {&armor_green}  , {100},0,200,
   def_int,ss_stat},
  {"ammo_red",      {&ammo_red}     , {25},0,100,
   def_int,ss_stat},
  {"ammo_yellow",   {&ammo_yellow}  , {50},0,100,
   def_int,ss_stat},


  {"hud_active",    {&hud_active}, {2},0,2,
   def_int,ss_none},

  {"hud_displayed", {&hud_displayed},  {0},0,1,
   def_bool,ss_none},
  {"hud_nosecrets", {&hud_nosecrets},  {0},0,1,
   def_bool,ss_stat},

  {"Weapon preferences",{NULL},{0},UL,UL,def_none,ss_none},

  {"weapon_choice_1", {&weapon_preferences[0][0]}, {6}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_2", {&weapon_preferences[0][1]}, {9}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_3", {&weapon_preferences[0][2]}, {4}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_4", {&weapon_preferences[0][3]}, {3}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_5", {&weapon_preferences[0][4]}, {2}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_6", {&weapon_preferences[0][5]}, {8}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_7", {&weapon_preferences[0][6]}, {5}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_8", {&weapon_preferences[0][7]}, {7}, 0,9,
   def_int,ss_weap},
  {"weapon_choice_9", {&weapon_preferences[0][8]}, {1}, 0,9,
   def_int,ss_weap},


  {"Music", {NULL},{0},UL,UL,def_none,ss_none},
  {"mus_e1m1", {0,&S_music_files[mus_e1m1]}, {0,"e1m1.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m2", {0,&S_music_files[mus_e1m2]}, {0,"e1m2.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m3", {0,&S_music_files[mus_e1m3]}, {0,"e1m3.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m4", {0,&S_music_files[mus_e1m4]}, {0,"e1m4.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m5", {0,&S_music_files[mus_e1m5]}, {0,"e1m5.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m6", {0,&S_music_files[mus_e1m6]}, {0,"e1m6.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m7", {0,&S_music_files[mus_e1m7]}, {0,"e1m7.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m8", {0,&S_music_files[mus_e1m8]}, {0,"e1m8.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e1m9", {0,&S_music_files[mus_e1m9]}, {0,"e1m9.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m1", {0,&S_music_files[mus_e2m1]}, {0,"e2m1.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m2", {0,&S_music_files[mus_e2m2]}, {0,"e2m2.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m3", {0,&S_music_files[mus_e2m3]}, {0,"e2m3.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m4", {0,&S_music_files[mus_e2m4]}, {0,"e2m4.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m5", {0,&S_music_files[mus_e2m5]}, {0,"e1m7.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m6", {0,&S_music_files[mus_e2m6]}, {0,"e2m6.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m7", {0,&S_music_files[mus_e2m7]}, {0,"e2m7.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m8", {0,&S_music_files[mus_e2m8]}, {0,"e2m8.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e2m9", {0,&S_music_files[mus_e2m9]}, {0,"e3m1.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m1", {0,&S_music_files[mus_e3m1]}, {0,"e3m1.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m2", {0,&S_music_files[mus_e3m2]}, {0,"e3m2.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m3", {0,&S_music_files[mus_e3m3]}, {0,"e3m3.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m4", {0,&S_music_files[mus_e3m4]}, {0,"e1m8.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m5", {0,&S_music_files[mus_e3m5]}, {0,"e1m7.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m6", {0,&S_music_files[mus_e3m6]}, {0,"e1m6.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m7", {0,&S_music_files[mus_e3m7]}, {0,"e2m7.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m8", {0,&S_music_files[mus_e3m8]}, {0,"e3m8.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_e3m9", {0,&S_music_files[mus_e3m9]}, {0,"e1m9.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_inter", {0,&S_music_files[mus_inter]}, {0,"e2m3.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_intro", {0,&S_music_files[mus_intro]}, {0,"intro.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_bunny", {0,&S_music_files[mus_bunny]}, {0,"bunny.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_victor", {0,&S_music_files[mus_victor]}, {0,"victor.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_introa", {0,&S_music_files[mus_introa]}, {0,"intro.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_runnin", {0,&S_music_files[mus_runnin]}, {0,"runnin.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_stalks", {0,&S_music_files[mus_stalks]}, {0,"stalks.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_countd", {0,&S_music_files[mus_countd]}, {0,"countd.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_betwee", {0,&S_music_files[mus_betwee]}, {0,"betwee.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_doom", {0,&S_music_files[mus_doom]}, {0,"doom.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_the_da", {0,&S_music_files[mus_the_da]}, {0,"the_da.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_shawn", {0,&S_music_files[mus_shawn]}, {0,"shawn.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_ddtblu", {0,&S_music_files[mus_ddtblu]}, {0,"ddtblu.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_in_cit", {0,&S_music_files[mus_in_cit]}, {0,"in_cit.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_dead", {0,&S_music_files[mus_dead]}, {0,"dead.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_stlks2", {0,&S_music_files[mus_stlks2]}, {0,"stalks.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_theda2", {0,&S_music_files[mus_theda2]}, {0,"the_da.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_doom2", {0,&S_music_files[mus_doom2]}, {0,"doom.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_ddtbl2", {0,&S_music_files[mus_ddtbl2]}, {0,"ddtblu.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_runni2", {0,&S_music_files[mus_runni2]}, {0,"runnin.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_dead2", {0,&S_music_files[mus_dead2]}, {0,"dead.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_stlks3", {0,&S_music_files[mus_stlks3]}, {0,"stalks.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_romero", {0,&S_music_files[mus_romero]}, {0,"romero.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_shawn2", {0,&S_music_files[mus_shawn2]}, {0,"shawn.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_messag", {0,&S_music_files[mus_messag]}, {0,"messag.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_count2", {0,&S_music_files[mus_count2]}, {0,"countd.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_ddtbl3", {0,&S_music_files[mus_ddtbl3]}, {0,"ddtblu.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_ampie", {0,&S_music_files[mus_ampie]}, {0,"ampie.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_theda3", {0,&S_music_files[mus_theda3]}, {0,"the_da.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_adrian", {0,&S_music_files[mus_adrian]}, {0,"adrian.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_messg2", {0,&S_music_files[mus_messg2]}, {0,"messag.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_romer2", {0,&S_music_files[mus_romer2]}, {0,"romero.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_tense", {0,&S_music_files[mus_tense]}, {0,"tense.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_shawn3", {0,&S_music_files[mus_shawn3]}, {0,"shawn.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_openin", {0,&S_music_files[mus_openin]}, {0,"openin.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_evil", {0,&S_music_files[mus_evil]}, {0,"evil.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_ultima", {0,&S_music_files[mus_ultima]}, {0,"ultima.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_read_m", {0,&S_music_files[mus_read_m]}, {0,"read_m.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_dm2ttl", {0,&S_music_files[mus_dm2ttl]}, {0,"dm2ttl.mp3"},UL,UL,
   def_str,ss_none},
  {"mus_dm2int", {0,&S_music_files[mus_dm2int]}, {0,"dm2int.mp3"},UL,UL,
   def_str,ss_none},
};

int numdefaults;
static const char* defaultfile;

void M_SaveDefaults (void)
  {
  int   i;
  FILE* f;

  f = fopen (defaultfile, "w");
  if (!f)
    return;

  fprintf(f,"# Doom config file\n");
  fprintf(f,"# Format:\n");
  fprintf(f,"# variable   value\n");

  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].type == def_none) {

      fprintf(f, "\n# %s\n", defaults[i].name);
    } else

    if (!IS_STRING(defaults[i]))
      {

      if (defaults[i].type == def_hex)
  fprintf (f,"%-25s 0x%x\n",defaults[i].name,*(defaults[i].location.pi));
      else
  fprintf (f,"%-25s %5i\n",defaults[i].name,*(defaults[i].location.pi));
      }
    else
      {
      fprintf (f,"%-25s \"%s\"\n",defaults[i].name,*(defaults[i].location.ppsz));
      }
    }

  fclose (f);
  }

struct default_s *M_LookupDefault(const char *name)
{
  int i;
  for (i = 0 ; i < numdefaults ; i++)
    if ((defaults[i].type != def_none) && !strcmp(name, defaults[i].name))
      return &defaults[i];
  I_Error("M_LookupDefault: %s not found",name);
  return NULL;
}





#define NUMCHATSTRINGS 10

void M_LoadDefaults (void)
{
  int   i;
  int   len;
  FILE* f;
  char  def[80];
  char  strparm[100];
  char* newstring = NULL;
  int   parm;
  boolean isstring;



  numdefaults = sizeof(defaults)/sizeof(defaults[0]);
  for (i = 0 ; i < numdefaults ; i++) {
    if (defaults[i].location.ppsz)
      *defaults[i].location.ppsz = strdup(defaults[i].defaultvalue.psz);
    if (defaults[i].location.pi)
      *defaults[i].location.pi = defaults[i].defaultvalue.i;
  }



    {
    const char* exedir = I_DoomExeDir();
    defaultfile = malloc(PATH_MAX+1);
    snprintf((char *)defaultfile, PATH_MAX, "%s%s%sboom.cfg", exedir, HasTrailingSlash(exedir) ? "" : "/", "pr");
  }

  lprintf (LO_CONFIRM, " default file: %s\n",defaultfile);



  f = fopen (defaultfile, "r");
  if (f)
    {
    while (!feof(f))
      {
      isstring = false;
      if (fscanf (f, "%79s %[^\n]\n", def, strparm) == 2)
        {



        if (!isalnum(def[0]))
          continue;

        if (strparm[0] == '"') {


          isstring = true;
          len = strlen(strparm);
          newstring = (char *) malloc(len);
          strparm[len-1] = 0;
          strcpy(newstring, strparm+1);
  } else if ((strparm[0] == '0') && (strparm[1] == 'x')) {

    sscanf(strparm+2, "%x", &parm);
  } else {
          sscanf(strparm, "%i", &parm);

  }

        for (i = 0 ; i < numdefaults ; i++)
          if ((defaults[i].type != def_none) && !strcmp(def, defaults[i].name))
            {

            if (isstring != IS_STRING(defaults[i])) {
        lprintf(LO_WARN, "M_LoadDefaults: Type mismatch reading %s\n", defaults[i].name);
        continue;
      }
            if (!isstring)
              {



              if ((defaults[i].minvalue==UL || defaults[i].minvalue<=parm) &&
                  (defaults[i].maxvalue==UL || defaults[i].maxvalue>=parm))
                *(defaults[i].location.pi) = parm;
              }
            else
              {
              free((char*)*(defaults[i].location.ppsz));  /* phares 4/13/98 */
              *(defaults[i].location.ppsz) = newstring;
              }
            break;
            }
        }
      }

    fclose (f);
    }

  wad_files[MAXLOADFILES-1]="prboom.wad";
}

