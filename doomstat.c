#include "doomstat.h"

GameMode_t gamemode = indetermined;
GameMission_t gamemission = doom;
complevel_t compatibility_level, default_compatibility_level;
int comp[COMP_TOTAL], default_comp[COMP_TOTAL];
int pitched_sounds;
int demo_insurance, default_demo_insurance;
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
int distfriend = 128, default_distfriend = 128;
int monster_backing, default_monster_backing;
int monster_avoid_hazards, default_monster_avoid_hazards;
int help_friends, default_help_friends;
int flashing_hom;
int doom_weapon_toggles;
int monkeys, default_monkeys;

