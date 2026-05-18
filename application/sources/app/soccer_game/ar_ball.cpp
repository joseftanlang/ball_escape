#include "ar_ball.h"

/* Ball update logic for goalkeeper role
   Handles ball physics: position, velocity, collision with edges,
   and scoring/losing when ball reaches goal line */
void ar_ball_update_goalkeeper_game()
{
    game_state.ball_x_fp += game_state.ball_vx_fp;
    game_state.ball_y_fp += game_state.ball_vy_fp;
    sync_ball_display_position();

    if (game_state.ball_x <= BALL_RADIUS || game_state.ball_x >= (128 - BALL_RADIUS - 1))
    {
        game_state.ball_vx_fp = -game_state.ball_vx_fp;
    }

    clamp_ball_position();

    if (game_state.super_mode_active || keeper_covers_ball())
    {
        if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
        {
            ar_ball_score_goal();
        }
        return;
    }

    if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
    {
        ar_ball_lose_life();
    }
}

/* Ball update logic for shooter role
   Handles ball physics when player is shooting:
   ball movement, keeper collision detection, and scoring */
void ar_ball_update_shooter_game()
{
    if (game_state.ball_vy_fp == 0)
    {
        align_ball_with_striker();
        return;
    }

    game_state.ball_x_fp += game_state.ball_vx_fp;
    game_state.ball_y_fp += game_state.ball_vy_fp;
    sync_ball_display_position();
    clamp_ball_position();

    if (!game_state.super_mode_active && keeper_covers_ball())
    {
        ar_ball_lose_life();
        return;
    }

    if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
    {
        ar_ball_score_goal();
    }
}

/* Handle scoring a goal: update score, streak, check win condition */
void ar_ball_score_goal()
{
    game_state.score++;
    game_state.streak++;
    reset_ball();

    if (game_state.role == SOCCER_ROLE_GOALKEEPER)
    {
        if (game_state.score >= GOALKEEPER_WIN_SCORE)
        {
            begin_result(SOCCER_RESULT_WIN);
            return;
        }
    }
    else if (game_state.score >= SHOOTER_WIN_SCORE)
    {
        begin_result(SOCCER_RESULT_WIN);
        return;
    }
}

/* Handle losing a life: decrement lives, reset streak, check lose condition */
void ar_ball_lose_life()
{
    if (game_state.lives > 0)
    {
        game_state.lives--;
    }
    game_state.streak = 0;
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
    reset_ball();

    if (game_state.lives <= 0)
    {
        begin_result(SOCCER_RESULT_LOSE);
        return;
    }
}