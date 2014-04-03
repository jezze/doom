#ifndef __P_ENEMY__
#define __P_ENEMY__

void P_NoiseAlert (mobj_t *target, mobj_t *emmiter);
void P_SpawnBrainTargets(void);

extern struct brain_s
{

    int easy, targeton;

} brain;

void A_Explode();
void A_Pain();
void A_PlayerScream();
void A_Fall();
void A_XScream();
void A_Look();
void A_Chase();
void A_FaceTarget();
void A_PosAttack();
void A_Scream();
void A_SPosAttack();
void A_VileChase();
void A_VileStart();
void A_VileTarget();
void A_VileAttack();
void A_StartFire();
void A_Fire();
void A_FireCrackle();
void A_Tracer();
void A_SkelWhoosh();
void A_SkelFist();
void A_SkelMissile();
void A_FatRaise();
void A_FatAttack1();
void A_FatAttack2();
void A_FatAttack3();
void A_BossDeath();
void A_CPosAttack();
void A_CPosRefire();
void A_TroopAttack();
void A_SargAttack();
void A_HeadAttack();
void A_BruisAttack();
void A_SkullAttack();
void A_Metal();
void A_SpidRefire();
void A_BabyMetal();
void A_BspiAttack();
void A_Hoof();
void A_CyberAttack();
void A_PainAttack();
void A_PainDie();
void A_KeenDie();
void A_BrainPain();
void A_BrainScream();
void A_BrainDie();
void A_BrainAwake();
void A_BrainSpit();
void A_SpawnSound();
void A_SpawnFly();
void A_BrainExplode();
void A_Die();
void A_Detonate();
void A_Mushroom();
void A_Spawn();
void A_Turn();
void A_Face();
void A_Scratch();
void A_PlaySound();
void A_RandomJump();
void A_LineEffect();

#endif
