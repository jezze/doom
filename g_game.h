#ifndef __G_GAME__
#define __G_GAME__

#include "doomdef.h"
#include "d_event.h"
#include "d_client.h"

#define GAME_OPTION_SIZE                64

boolean G_Responder(event_t *ev);
void G_InitNew(skill_t skill, int episode, int map);
void G_DeferedInitNew(skill_t skill, int episode, int map);
void G_ExitLevel(void);
void G_SecretExitLevel(void);
void G_WorldDone(void);
void G_EndGame(void);
void G_Ticker(void);
void G_ReloadDefaults(void);
void G_SetFastParms(int);
void G_DoNewGame(void);
void G_DoReborn(int playernum);
void G_DoCompleted(void);
void G_DoWorldDone(void);
void G_Compatibility(void);
void G_PlayerReborn(int player);
void G_RestartLevel(void);
void G_DoVictory(void);
void G_BuildTiccmd (ticcmd_t* cmd);
void G_ChangedPlayerColour(int pn, int cl);
void G_MakeSpecialEvent(buttoncode_t bc, ...);
void doom_printf(const char *, ...) __attribute__((format(printf,1,2)));

extern int key_right;
extern int key_left;
extern int key_up;
extern int key_down;
extern int key_menu_right;
extern int key_menu_left;
extern int key_menu_up;
extern int key_menu_down;
extern int key_menu_backspace;
extern int key_menu_escape;
extern int key_menu_enter;
extern int key_strafeleft;
extern int key_straferight;
extern int key_fire;
extern int key_use;
extern int key_strafe;
extern int key_speed;
extern int key_escape;
extern int key_savegame;
extern int key_loadgame;
extern int key_autorun;
extern int key_reverse;
extern int key_zoomin;
extern int key_zoomout;
extern int key_chat;
extern int key_backspace;
extern int key_enter;
extern int key_help;
extern int key_soundvolume;
extern int key_hud;
extern int key_quicksave;
extern int key_endgame;
extern int key_messages;
extern int key_quickload;
extern int key_quit;
extern int key_gamma;
extern int key_spy;
extern int key_pause;
extern int key_setup;
extern int key_forward;
extern int key_leftturn;
extern int key_rightturn;
extern int key_backward;
extern int key_weapontoggle;
extern int key_weapon1;
extern int key_weapon2;
extern int key_weapon3;
extern int key_weapon4;
extern int key_weapon5;
extern int key_weapon6;
extern int key_weapon7;
extern int key_weapon8;
extern int key_weapon9;
extern int destination_keys[MAXPLAYERS];
extern int key_map_right;
extern int key_map_left;
extern int key_map_up;
extern int key_map_down;
extern int key_map_zoomin;
extern int key_map_zoomout;
extern int key_map;
extern int key_map_gobig;
extern int key_map_follow;
extern int key_map_mark;
extern int key_map_clear;
extern int key_map_grid;
extern int key_map_rotate;
extern int key_map_overlay;
extern int key_screenshot;
extern int autorun;
extern int defaultskill;
extern boolean haswolflevels;
extern int bodyquesize;
extern int pars[4][10];
extern int cpars[32];
extern const char *comp_lev_str[];

#endif
