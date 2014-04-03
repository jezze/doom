#ifndef __M_RANDOM__
#define __M_RANDOM__

typedef enum
{

    pr_skullfly,
    pr_damage,
    pr_crush,
    pr_genlift,
    pr_killtics,
    pr_damagemobj,
    pr_painchance,
    pr_lights,
    pr_explode,
    pr_respawn,
    pr_lastlook,
    pr_spawnthing,
    pr_spawnpuff,
    pr_spawnblood,
    pr_missile,
    pr_shadow,
    pr_plats,
    pr_punch,
    pr_punchangle,
    pr_saw,
    pr_plasma,
    pr_gunshot,
    pr_misfire,
    pr_shotgun,
    pr_bfg,
    pr_slimehurt,
    pr_dmspawn,
    pr_missrange,
    pr_trywalk,
    pr_newchase,
    pr_newchasedir,
    pr_see,
    pr_facetarget,
    pr_posattack,
    pr_sposattack,
    pr_cposattack,
    pr_spidrefire,
    pr_troopattack,
    pr_sargattack,
    pr_headattack,
    pr_bruisattack,
    pr_tracer,
    pr_skelfist,
    pr_scream,
    pr_brainscream,
    pr_cposrefire,
    pr_brainexp,
    pr_spawnfly,
    pr_misc,
    pr_all_in_one,
    pr_opendoor,
    pr_targetsearch,
    pr_friends,
    pr_threshold,
    pr_skiptarget,
    pr_enemystrafe,
    pr_avoidcrush,
    pr_stayonlift,
    pr_helpfriend,
    pr_dropoff,
    pr_randomjump,
    pr_defect,
    NUMPRCLASS

} pr_class_t;

typedef struct
{

    unsigned long seed[NUMPRCLASS];
    int rndindex, prndindex;

} rng_t;

extern rng_t rng;

int P_Random(pr_class_t);
void M_ClearRandom(unsigned long rngseed);

#endif
