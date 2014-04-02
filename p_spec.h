#ifndef __P_SPEC__
#define __P_SPEC__

#include "r_defs.h"
#include "d_player.h"

#define MO_TELEPORTMAN                  14
#define ELEVATORSPEED                   (FRACUNIT * 4)
#define FLOORSPEED                      FRACUNIT
#define CEILSPEED                       FRACUNIT
#define CEILWAIT                        150
#define VDOORSPEED                      (FRACUNIT * 2)
#define VDOORWAIT                       150
#define PLATWAIT                        3
#define PLATSPEED                       FRACUNIT
#define MAXBUTTONS                      (MAXPLAYERS * 4)
#define BUTTONTIME                      TICRATE
#define GLOWSPEED                       8
#define STROBEBRIGHT                    5
#define FASTDARK                        15
#define SLOWDARK                        35
#define DAMAGE_MASK                     0x60
#define DAMAGE_SHIFT                    5
#define SECRET_MASK                     0x80
#define SECRET_SHIFT                    7
#define FRICTION_MASK                   0x100
#define FRICTION_SHIFT                  8
#define PUSH_MASK                       0x200
#define PUSH_SHIFT                      9
#define GenEnd                          0x8000
#define GenFloorBase                    0x6000
#define GenCeilingBase                  0x4000
#define GenDoorBase                     0x3c00
#define GenLockedBase                   0x3800
#define GenLiftBase                     0x3400
#define GenStairsBase                   0x3000
#define GenCrusherBase                  0x2F80
#define TriggerType                     0x0007
#define TriggerTypeShift                0
#define FloorCrush                      0x1000
#define FloorChange                     0x0c00
#define FloorTarget                     0x0380
#define FloorDirection                  0x0040
#define FloorModel                      0x0020
#define FloorSpeed                      0x0018
#define FloorCrushShift                 12
#define FloorChangeShift                10
#define FloorTargetShift                7
#define FloorDirectionShift             6
#define FloorModelShift                 5
#define FloorSpeedShift                 3
#define CeilingCrush                    0x1000
#define CeilingChange                   0x0c00
#define CeilingTarget                   0x0380
#define CeilingDirection                0x0040
#define CeilingModel                    0x0020
#define CeilingSpeed                    0x0018
#define CeilingCrushShift               12
#define CeilingChangeShift              10
#define CeilingTargetShift              7
#define CeilingDirectionShift           6
#define CeilingModelShift               5
#define CeilingSpeedShift               3
#define LiftTarget                      0x0300
#define LiftDelay                       0x00c0
#define LiftMonster                     0x0020
#define LiftSpeed                       0x0018
#define LiftTargetShift                 8
#define LiftDelayShift                  6
#define LiftMonsterShift                5
#define LiftSpeedShift                  3
#define StairIgnore                     0x0200
#define StairDirection                  0x0100
#define StairStep                       0x00c0
#define StairMonster                    0x0020
#define StairSpeed                      0x0018
#define StairIgnoreShift                9
#define StairDirectionShift             8
#define StairStepShift                  6
#define StairMonsterShift               5
#define StairSpeedShift                 3
#define CrusherSilent                   0x0040
#define CrusherMonster                  0x0020
#define CrusherSpeed                    0x0018
#define CrusherSilentShift              6
#define CrusherMonsterShift             5
#define CrusherSpeedShift               3
#define DoorDelay                       0x0300
#define DoorMonster                     0x0080
#define DoorKind                        0x0060
#define DoorSpeed                       0x0018
#define DoorDelayShift                  8
#define DoorMonsterShift                7
#define DoorKindShift                   5
#define DoorSpeedShift                  3
#define LockedNKeys                     0x0200
#define LockedKey                       0x01c0
#define LockedKind                      0x0020
#define LockedSpeed                     0x0018
#define LockedNKeysShift                9
#define LockedKeyShift                  6
#define LockedKindShift                 5
#define LockedSpeedShift                3

typedef enum
{

    WalkOnce,
    WalkMany,
    SwitchOnce,
    SwitchMany,
    GunOnce,
    GunMany,
    PushOnce,
    PushMany

} triggertype_e;

typedef enum
{

    SpeedSlow,
    SpeedNormal,
    SpeedFast,
    SpeedTurbo

} motionspeed_e;

typedef enum
{

    FtoHnF,
    FtoLnF,
    FtoNnF,
    FtoLnC,
    FtoC,
    FbyST,
    Fby24,
    Fby32

} floortarget_e;

typedef enum
{

    FNoChg,
    FChgZero,
    FChgTxt,
    FChgTyp

} floorchange_e;

typedef enum
{

    FTriggerModel,
    FNumericModel

} floormodel_t;

typedef enum
{

    CtoHnC,
    CtoLnC,
    CtoNnC,
    CtoHnF,
    CtoF,
    CbyST,
    Cby24,
    Cby32

} ceilingtarget_e;

typedef enum
{

    CNoChg,
    CChgZero,
    CChgTxt,
    CChgTyp

} ceilingchange_e;

typedef enum
{

    CTriggerModel,
    CNumericModel

} ceilingmodel_t;

typedef enum
{

    F2LnF,
    F2NnF,
    F2LnC,
    LnF2HnF

} lifttarget_e;

typedef enum
{

    OdCDoor,
    ODoor,
    CdODoor,
    CDoor

} doorkind_e;

typedef enum
{

    AnyKey,
    RCard,
    BCard,
    YCard,
    RSkull,
    BSkull,
    YSkull,
    AllKeys

} keykind_e;

typedef enum
{

    floor_special,
    ceiling_special,
    lighting_special

} special_e;

typedef enum
{

    trigChangeOnly,
    numChangeOnly

} change_e;

typedef enum
{

    up,
    down,
    waiting,
    in_stasis

} plat_e;

typedef enum
{

    perpetualRaise,
    downWaitUpStay,
    raiseAndChange,
    raiseToNearestAndChange,
    blazeDWUS,
    genLift,
    genPerpetual,
    toggleUpDn

} plattype_e;

typedef enum
{

    normal,
    close30ThenOpen,
    close,
    open,
    raiseIn5Mins,
    blazeRaise,
    blazeOpen,
    blazeClose,
    genRaise,
    genBlazeRaise,
    genOpen,
    genBlazeOpen,
    genClose,
    genBlazeClose,
    genCdO,
    genBlazeCdO

} vldoor_e;

typedef enum
{

    lowerToFloor,
    raiseToHighest,
    lowerToLowest,
    lowerToMaxFloor,
    lowerAndCrush,
    crushAndRaise,
    fastCrushAndRaise,
    silentCrushAndRaise,
    genCeiling,
    genCeilingChg,
    genCeilingChg0,
    genCeilingChgT,
    genCrusher,
    genSilentCrusher

} ceiling_e;

typedef enum
{

    lowerFloor,
    lowerFloorToLowest,
    turboLower,
    raiseFloor,
    raiseFloorToNearest,
    lowerFloorToNearest,
    lowerFloor24,
    lowerFloor32Turbo,
    raiseToTexture,
    lowerAndChange,
    raiseFloor24,
    raiseFloor32Turbo,
    raiseFloor24AndChange,
    raiseFloorCrush,
    raiseFloorTurbo,
    donutRaise,
    raiseFloor512,
    genFloor,
    genFloorChg,
    genFloorChg0,
    genFloorChgT,
    buildStair,
    genBuildStair

} floor_e;

typedef enum
{

    build8,
    turbo16

} stair_e;

typedef enum
{

    elevateUp,
    elevateDown,
    elevateCurrent

} elevator_e;

typedef enum
{

    top,
    middle,
    bottom

} bwhere_e;

typedef enum
{

    ok,
    crushed,
    pastdest

} result_e;

typedef struct
{

    char name1[9];
    char name2[9];
    short episode;

} __attribute__((packed)) switchlist_t;

typedef struct
{

    line_t* line;
    bwhere_e where;
    int   btexture;
    int   btimer;
    mobj_t* soundorg;

} button_t;

typedef struct
{

    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;

} fireflicker_t;

typedef struct
{

    thinker_t thinker;
    sector_t* sector;
    int count;
    int maxlight;
    int minlight;
    int maxtime;
    int mintime;

} lightflash_t;

typedef struct
{

    thinker_t thinker;
    sector_t* sector;
    int count;
    int minlight;
    int maxlight;
    int darktime;
    int brighttime;

} strobe_t;

typedef struct
{

    thinker_t thinker;
    sector_t* sector;
    int minlight;
    int maxlight;
    int direction;

} glow_t;

typedef struct
{

    thinker_t thinker;
    sector_t* sector;
    fixed_t speed;
    fixed_t low;
    fixed_t high;
    int wait;
    int count;
    plat_e status;
    plat_e oldstatus;
    boolean crush;
    int tag;
    plattype_e type;
    struct platlist *list;

} plat_t;

typedef struct platlist
{

    plat_t *plat;
    struct platlist *next,**prev;

} platlist_t;

typedef struct
{

    thinker_t thinker;
    vldoor_e type;
    sector_t* sector;
    fixed_t topheight;
    fixed_t speed;
    int direction;
    int topwait;
    int topcountdown;
    line_t *line;
    int lighttag;

} vldoor_t;

typedef struct
{

    thinker_t thinker;
    ceiling_e type;
    sector_t* sector;
    fixed_t bottomheight;
    fixed_t topheight;
    fixed_t speed;
    fixed_t oldspeed;
    boolean crush;
    int newspecial;
    int oldspecial;
    short texture;
    int direction;
    int tag;
    int olddirection;
    struct ceilinglist *list;

} ceiling_t;

typedef struct ceilinglist
{

    ceiling_t *ceiling;
    struct ceilinglist *next, **prev;

} ceilinglist_t;

typedef struct
{

    thinker_t thinker;
    floor_e type;
    boolean crush;
    sector_t* sector;
    int direction;
    int newspecial;
    int oldspecial;
    short texture;
    fixed_t floordestheight;
    fixed_t speed;

} floormove_t;

typedef struct
{

    thinker_t thinker;
    elevator_e type;
    sector_t* sector;
    int direction;
    fixed_t floordestheight;
    fixed_t ceilingdestheight;
    fixed_t speed;

} elevator_t;

typedef struct
{

    thinker_t thinker;
    fixed_t dx, dy;
    int affectee;
    int control;
    fixed_t last_height;
    fixed_t vdx, vdy;
    int accel;
    enum
    {
      sc_side,
      sc_floor,
      sc_ceiling,
      sc_carry,
      sc_carry_ceiling
    } type;

} scroll_t;

typedef struct
{

    thinker_t thinker;
    int friction;
    int movefactor;
    int affectee;

} friction_t;

typedef struct
{

    thinker_t thinker;
    enum
    {

        p_push,
        p_pull,
        p_wind,
        p_current

    } type;
    mobj_t* source;
    int x_mag;
    int y_mag;
    int magnitude;
    int radius;
    int x;
    int y;
    int affectee;

} pusher_t;

extern button_t buttonlist[MAXBUTTONS];
extern platlist_t *activeplats;

int twoSided(int sector, int line);
sector_t *getSector(int currentSector, int line, int side);
side_t *getSide(int currentSector, int line, int side);
fixed_t P_FindLowestFloorSurrounding(sector_t *sec);
fixed_t P_FindHighestFloorSurrounding(sector_t *sec);
fixed_t P_FindNextHighestFloor(sector_t *sec, int currentheight);
fixed_t P_FindNextLowestFloor(sector_t *sec, int currentheight);
fixed_t P_FindLowestCeilingSurrounding(sector_t *sec);
fixed_t P_FindHighestCeilingSurrounding(sector_t *sec);
fixed_t P_FindNextLowestCeiling(sector_t *sec, int currentheight);
fixed_t P_FindNextHighestCeiling(sector_t *sec, int currentheight);
fixed_t P_FindShortestTextureAround(int secnum);
fixed_t P_FindShortestUpperAround(int secnum);
sector_t *P_FindModelFloorSector(fixed_t floordestheight, int secnum);
sector_t *P_FindModelCeilingSector(fixed_t ceildestheight, int secnum);
int P_FindSectorFromLineTag(const line_t *line, int start);
int P_FindLineFromLineTag(const line_t *line, int start);
int P_FindMinSurroundingLight(sector_t *sector, int max);
sector_t* getNextSector(line_t *line, sector_t *sec);
int P_CheckTag(line_t *line);
boolean P_CanUnlockGenDoor(line_t *line, player_t *player);
boolean P_SectorActive(special_e t, const sector_t *s);
boolean P_IsSecret(const sector_t *sec);
boolean P_WasSecret(const sector_t *sec);
void P_ChangeSwitchTexture(line_t *line, int useAgain);
void T_LightFlash(lightflash_t *flash);
void T_StrobeFlash(strobe_t *flash);
void T_FireFlicker(fireflicker_t *flick);
void T_Glow(glow_t *g);
void T_PlatRaise(plat_t *plat);
void T_VerticalDoor(vldoor_t *door);
void T_MoveCeiling(ceiling_t *ceiling);
result_e T_MovePlane(sector_t *sector, fixed_t speed, fixed_t dest, boolean crush, int floorOrCeiling, int direction);
void T_MoveFloor(floormove_t *floor);
void T_MoveElevator(elevator_t *elevator);
void T_Scroll(scroll_t *);
void T_Friction(friction_t *);
void T_Pusher(pusher_t *);
int EV_Teleport(line_t *line, int side, mobj_t *thing);
int EV_SilentTeleport(line_t *line, int side, mobj_t *thing);
int EV_SilentLineTeleport(line_t *line, int side, mobj_t *thing, boolean reverse);
int EV_DoElevator(line_t *line, elevator_e type);
int EV_BuildStairs(line_t *line, stair_e type);
int EV_DoFloor(line_t *line, floor_e floortype);
int EV_DoCeiling(line_t *line, ceiling_e type);
int EV_CeilingCrushStop(line_t *line);
int EV_VerticalDoor(line_t *line, mobj_t *thing);
int EV_DoDoor(line_t *line, vldoor_e type);
int EV_DoLockedDoor(line_t *line, vldoor_e type, mobj_t *thing);
int EV_StartLightStrobing(line_t *line);
int EV_TurnTagLightsOff(line_t *line);
int EV_LightTurnOn(line_t *line, int bright);
int EV_LightTurnOnPartway(line_t *line, fixed_t level);
int EV_DoChange(line_t *line, change_e changetype);
int EV_DoDonut(line_t *line);
int EV_DoPlat(line_t *line, plattype_e type, int amount);
int EV_StopPlat(line_t *line);
int EV_DoGenFloor(line_t *line);
int EV_DoGenCeiling(line_t *line);
int EV_DoGenLift(line_t *line);
int EV_DoGenStairs(line_t *line);
int EV_DoGenCrusher(line_t *line);
int EV_DoGenDoor(line_t *line);
int EV_DoGenLockedDoor(line_t *line);
void P_InitPicAnims(void);
void P_InitSwitchList(void);
void P_SpawnSpecials(void);
void P_UpdateSpecials(void);
boolean P_UseSpecialLine(mobj_t *thing, line_t *line, int side);
void P_ShootSpecialLine(mobj_t *thing, line_t *line);
void P_CrossSpecialLine(line_t *line, int side, mobj_t *thing);
void P_PlayerInSpecialSector(player_t *player);
void P_SpawnFireFlicker(sector_t *sector);
void P_SpawnLightFlash(sector_t *sector);
void P_SpawnStrobeFlash(sector_t *sector, int fastOrSlow, int inSync);
void P_SpawnGlowingLight(sector_t *sector);
void P_AddActivePlat(plat_t *plat);
void P_RemoveActivePlat(plat_t *plat);
void P_RemoveAllActivePlats(void);
void P_ActivateInStasis(int tag);
void P_SpawnDoorCloseIn30(sector_t *sec);
void P_SpawnDoorRaiseIn5Mins(sector_t *sec, int secnum);
void P_RemoveActiveCeiling(ceiling_t *ceiling);
void P_RemoveAllActiveCeilings(void);
void P_AddActiveCeiling(ceiling_t *c);
int P_ActivateInStasisCeiling(line_t *line);
mobj_t *P_GetPushThing(int);

#endif
