#include "d_think.h"
#include "doomstat.h"

GameMode_t gamemode = indetermined;
int allow_pushers = 1;
int default_allow_pushers;
int variable_friction = 1;
int default_variable_friction;
int weapon_recoil;
int default_weapon_recoil;
int player_bobbing;
int default_player_bobbing;
int monsters_remember;
int default_monsters_remember;
int monster_infighting = 1;
int default_monster_infighting = 1;
int monster_friction = 1;
int default_monster_friction = 1;
int monster_backing, default_monster_backing;
int monster_avoid_hazards, default_monster_avoid_hazards;

