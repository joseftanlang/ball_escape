#include "ar_striker.h"

/* Striker logic for shooter role 
   Handles shooter position management and ball shooting mechanics */
void ar_striker_move_position(int delta_x)
{
    move_shooter_position(delta_x);
}

void ar_striker_shoot_ball()
{
    game_state.ball_vx_fp = 0;
    game_state.ball_vy_fp = BALL_SHOOT_SPEED_Y;
}
