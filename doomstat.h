#ifndef __D_STATE__
#define __D_STATE__

#include "d_player.h"

#define BACKUPTICS                      12
#define compatibility                   (compatibility_level <= boom_compatibility_compatibility)

enum
{

    comp_telefrag,
    comp_dropoff,
    comp_vile,
    comp_pain,
    comp_skull,
    comp_blazing,
    comp_doorlight,
    comp_model,
    comp_god,
    comp_falloff,
    comp_floors,
    comp_skymap,
    comp_pursuit,
    comp_doorstuck,
    comp_staylift,
    comp_zombie,
    comp_stairs,
    comp_infcheat,
    comp_zerotags,
    comp_moveblock,
    comp_respawn,
    comp_sound,
    comp_666,
    comp_soul,
    comp_maskedanim,
    COMP_NUM,
    COMP_TOTAL = 32

};

extern GameMode_t gamemode;
extern complevel_t compatibility_level;
extern int comp[COMP_TOTAL], default_comp[COMP_TOTAL];
extern skill_t startskill;
extern int startepisode;
extern int startmap;
extern skill_t gameskill;
extern int gameepisode;
extern int gamemap;
extern boolean respawnmonsters;
extern int snd_SfxVolume;
extern int snd_MusicVolume;
extern unsigned int desired_screenwidth, desired_screenheight;
extern boolean menuactive;
extern int viewangleoffset;
extern int consoleplayer;
extern int displayplayer;
extern int totalkills, totallive;
extern int totalitems;
extern int totalsecret;
extern int basetic;
extern int leveltime;
extern gamestate_t gamestate;
extern int gametic;
extern player_t players[MAXPLAYERS];
extern boolean playeringame[MAXPLAYERS];
extern boolean realplayeringame[MAXPLAYERS];
extern mapthing_t playerstarts[];
extern wbstartstruct_t wminfo;
extern int mouseSensitivity_horiz;
extern int mouseSensitivity_vert;
extern int skyflatnum;
extern int maketic;
extern ticcmd_t netcmds[][BACKUPTICS];
extern int allow_pushers;
extern int default_allow_pushers;
extern int variable_friction;
extern int default_variable_friction;
extern int monsters_remember;
extern int default_monsters_remember;
extern int weapon_recoil;
extern int default_weapon_recoil;
extern int player_bobbing;
extern int default_player_bobbing;
extern int monster_backing, default_monster_backing;
extern int monster_avoid_hazards, default_monster_avoid_hazards;
extern int monster_friction, default_monster_friction;
extern int doom_weapon_toggles;
extern int monster_infighting, default_monster_infighting;

#endif
