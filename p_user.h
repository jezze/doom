#ifndef __P_USER__
#define __P_USER__

void P_PlayerThink(player_t *player);
void P_CalcHeight(player_t *player);
void P_DeathThink(player_t *player);
void P_MovePlayer(player_t *player);
void P_Thrust(player_t *player, angle_t angle, fixed_t move);

#endif
