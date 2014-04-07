#ifndef __D_STATE__
#define __D_STATE__

#include "d_player.h"

#define BACKUPTICS                      12

extern GameMode_t gamemode;
extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;
extern boolean respawnmonsters;
extern int snd_SfxVolume;
extern int snd_MusicVolume;
extern boolean menuactive;
extern int consoleplayer;
extern int totalkills, totallive;
extern int totalitems;
extern int totalsecret;
extern int basetic;
extern int leveltime;
extern gamestate_t gamestate;
extern int gametic;
extern player_t players[MAXPLAYERS];
extern boolean playeringame[MAXPLAYERS];
extern mapthing_t playerstarts[];
extern wbstartstruct_t wminfo;
extern int mouseSensitivity_horiz;
extern int mouseSensitivity_vert;
extern int skyflatnum;
extern int maketic;
extern ticcmd_t netcmds[][BACKUPTICS];
extern int monsters_remember;
extern int default_monsters_remember;
extern int weapon_recoil;
extern int default_weapon_recoil;
extern int player_bobbing;
extern int default_player_bobbing;
extern int monster_backing, default_monster_backing;
extern int monster_avoid_hazards, default_monster_avoid_hazards;
extern int monster_friction, default_monster_friction;
extern int monster_infighting, default_monster_infighting;
extern int acceleratestage;

#endif
