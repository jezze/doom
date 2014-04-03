#ifndef __G_GAME__
#define __G_GAME__

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
void G_PlayerReborn(int player);
void G_RestartLevel(void);
void G_DoVictory(void);
void G_BuildTiccmd(ticcmd_t* cmd);

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
extern int key_autorun;
extern int key_reverse;
extern int key_backspace;
extern int key_enter;
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
extern int autorun;
extern int defaultskill;
extern boolean haswolflevels;
extern int pars[4][10];
extern int cpars[32];
extern int mousebfire;
extern int mousebstrafe;
extern int mousebforward;

#endif
