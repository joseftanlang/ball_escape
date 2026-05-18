#include "ar_keeper.h"
/* Keeper update logic for shooter role
   Handles goalkeeper/keeper AI movement back and forth
   within the goal post boundaries */
void ar_keeper_update_game()
{
    game_state.keeper_x += game_state.keeper_vx;
    
    /* Bounce keeper off goal post boundaries */
    if (game_state.keeper_x <= GOAL_POST_X || game_state.keeper_x >= (GOAL_POST_X + GOAL_POST_W - GOAL_KEEPER_W))
    {
        game_state.keeper_vx = -game_state.keeper_vx;
        game_state.keeper_x = clamp_int(game_state.keeper_x, GOAL_POST_X, GOAL_POST_X + GOAL_POST_W - GOAL_KEEPER_W);
    }
}