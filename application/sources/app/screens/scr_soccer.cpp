#include "scr_soccer.h"
#include "scr_idle.h"
#include "task_life.h"

#define GOAL_POST_X (0)
#define GOAL_POST_Y (0)
#define GOAL_POST_W (80)
#define GOAL_POST_H (15)

#define GOAL_KEEPER_X (30)
#define GOAL_KEEPER_Y (15)
#define GOAL_KEEPER_W (22)
#define GOAL_KEEPER_H (4)

#define BALL_X (40)
#define BALL_Y (58)
#define BALL_RADIUS (3)
#define BALL_SHOOT_SPEED_Y (-3)

#define STARTING_LIVES (3)
#define GOALKEEPER_WIN_SCORE (10)
#define SHOOTER_WIN_SCORE (3)
#define READY_COUNTDOWN_SECONDS (3)
#define SUPER_MODE_DURATION_MS (5000)
#define RESULT_SCREEN_DURATION_MS (3000)
#define PLAY_TICK_MS (150)
#define MOVE_STEP_Y (4)
#define MOVE_STEP_X (4)
#define SUPER_MODE_BLINK_COUNT (5)

enum soccer_role_t
{
    SOCCER_ROLE_GOALKEEPER = 0,
    SOCCER_ROLE_SHOOTER = 1,
};

enum soccer_phase_t
{
    SOCCER_PHASE_SELECT = 0,
    SOCCER_PHASE_COUNTDOWN,
    SOCCER_PHASE_PLAYING,
    SOCCER_PHASE_RESULT,
};

enum soccer_result_t
{
    SOCCER_RESULT_NONE = 0,
    SOCCER_RESULT_WIN,
    SOCCER_RESULT_LOSE,
};

struct soccer_game_t
{
    soccer_role_t role;
    soccer_phase_t phase;
    soccer_result_t result;
    int selection_index;
    int lives;
    int score;
    int streak;
    int countdown_seconds;
    int keeper_x;
    int ball_x;
    int ball_y;
    int ball_vx;
    int ball_vy;
    bool super_mode_active;
    int super_mode_led_toggle_remaining;
};

static soccer_game_t game_state;

static void view_scr_soccer();

// This function stops the super mode LED blink
static void stop_super_mode_led_blink()
{
    game_state.super_mode_led_toggle_remaining = 0;
    led_off(&led_life);
}

// This function is called when the player scores a goal or the goalkeeper saves a shot to activate super mode for 5 seconds
static void poll_super_mode_led_blink()
{
    if (!game_state.super_mode_active)
    {
        return;
    }

    if (game_state.super_mode_led_toggle_remaining > 0)
    {
        led_toggle(&led_life);
        game_state.super_mode_led_toggle_remaining--;

        if (game_state.super_mode_led_toggle_remaining == 0)
        {
            led_off(&led_life);
        }
    }
}

// Forward declarations of helper functions
static void stop_soccer_timers()
{
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_COUNTDOWN);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_PLAY);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_RESULT);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_SUPER_MODE);
}

// Resets the ball to the starting position and velocity
static void reset_ball()
{
    game_state.ball_x = BALL_X;
    game_state.ball_y = BALL_Y;
    if (game_state.role == SOCCER_ROLE_GOALKEEPER)
    {
        game_state.ball_vx = 2;
        game_state.ball_vy = -2;
    }
    else
    {
        game_state.ball_vx = 0;
        game_state.ball_vy = 0;
    }
}

// Schedules the next countdown tick after 1 second
static void schedule_countdown_tick()
{
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_COUNTDOWN, 1000, TIMER_PERIODIC);
}

// Starts the gameplay phase
static void begin_gameplay()
{
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_COUNTDOWN);
    game_state.phase = SOCCER_PHASE_PLAYING;
    game_state.countdown_seconds = 0;
    game_state.lives = STARTING_LIVES;
    game_state.score = 0;
    game_state.streak = 0;
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
    reset_ball();

    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_PLAY, PLAY_TICK_MS, TIMER_PERIODIC);
    view_scr_soccer();
}

// Initializes the game state and starts the selection phase
static void begin_countdown()
{
    stop_soccer_timers();
    game_state.phase = SOCCER_PHASE_COUNTDOWN;
    game_state.countdown_seconds = READY_COUNTDOWN_SECONDS;
    schedule_countdown_tick();
    view_scr_soccer();
}

// Ends the game and shows the result screen for a few seconds
static void begin_result(soccer_result_t result)
{
    stop_soccer_timers();
    game_state.phase = SOCCER_PHASE_RESULT;
    game_state.result = result;
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_RESULT, RESULT_SCREEN_DURATION_MS, TIMER_ONE_SHOT);
    view_scr_soccer();
}

// Resets the game state to start a new game
static void restart_game()
{
    stop_soccer_timers();
    game_state.role = SOCCER_ROLE_GOALKEEPER;
    game_state.phase = SOCCER_PHASE_SELECT;
    game_state.result = SOCCER_RESULT_NONE;
    game_state.selection_index = SOCCER_ROLE_GOALKEEPER;
    game_state.lives = STARTING_LIVES;
    game_state.score = 0;
    game_state.streak = 0;
    game_state.countdown_seconds = READY_COUNTDOWN_SECONDS;
    game_state.keeper_x = GOAL_KEEPER_X;
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
    reset_ball();
    view_scr_soccer();
}

// Toggles the selection between goalkeeper and shooter in the selection screen
static void toggle_selection()
{
    game_state.selection_index = 1 - game_state.selection_index;
    view_scr_soccer();
}

// keeps the goalkeeper within the goal area
static void clamp_keeper_position()
{
    if (game_state.keeper_x < 0)
    {
        game_state.keeper_x = 0;
    }
    if (game_state.keeper_x > (128 - GOAL_KEEPER_W))
    {
        game_state.keeper_x = 128 - GOAL_KEEPER_W;
    }
}

// ball should not go outside the screen vertically
static void clamp_ball_position()
{
    if (game_state.ball_x < BALL_RADIUS)
    {
        game_state.ball_x = BALL_RADIUS;
    }
    if (game_state.ball_x > (128 - BALL_RADIUS - 1))
    {
        game_state.ball_x = 128 - BALL_RADIUS - 1;
    }

    if (game_state.ball_y < BALL_RADIUS)
    {
        game_state.ball_y = BALL_RADIUS;
    }
    if (game_state.ball_y > (64 - BALL_RADIUS - 1))
    {
        game_state.ball_y = 64 - BALL_RADIUS - 1;
    }
}

// Checks if the goalkeeper is covering the ball's position
static bool keeper_covers_ball()
{
    const int keeper_left = game_state.keeper_x;
    const int keeper_right = game_state.keeper_x + GOAL_KEEPER_W;
    const int ball_left = game_state.ball_x - BALL_RADIUS;
    const int ball_right = game_state.ball_x + BALL_RADIUS;

    return (ball_right >= keeper_left) &&
           (ball_left <= keeper_right) &&
           (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H));
}

// This function is called when the super mode timer expires to deactivate super mode
static void activate_super_mode()
{
    if (game_state.super_mode_active || game_state.streak < 3)
    {
        return;
    }

    game_state.super_mode_active = true;
    game_state.super_mode_led_toggle_remaining = SUPER_MODE_BLINK_COUNT * 2;
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_SUPER_MODE, SUPER_MODE_DURATION_MS, TIMER_ONE_SHOT);
    view_scr_soccer();
}

// finish super mode when timer expires
static void finish_super_mode()
{
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
    view_scr_soccer();
}

// handles scoring a goal, updates score and checks for win condition
static void score_goal()
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

    view_scr_soccer();
}

// handles losing a life, updates lives and checks for lose condition
static void lose_life()
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

    view_scr_soccer();
}

// updating the game state for goalkeeper role on each play tick
static void update_goalkeeper_game()
{
    game_state.ball_x += game_state.ball_vx;
    game_state.ball_y += game_state.ball_vy;

    if (game_state.ball_x <= BALL_RADIUS || game_state.ball_x >= (128 - BALL_RADIUS - 1))
    {
        game_state.ball_vx = -game_state.ball_vx;
    }

    clamp_ball_position();

    if (game_state.super_mode_active || keeper_covers_ball())
    {
        if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
        {
            score_goal();
        }
        return;
    }

    if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
    {
        lose_life();
    }
}

// updating the game state for shooter role on each play tick
static void update_shooter_game()
{
    if (game_state.ball_vy == 0)
    {
        return;
    }

    game_state.ball_y += game_state.ball_vy;
    clamp_ball_position();

    if (!game_state.super_mode_active && keeper_covers_ball())
    {
        lose_life();
        return;
    }

    if (game_state.ball_y <= (GOAL_POST_Y + GOAL_POST_H))
    {
        score_goal();
    }
}

// view screen select screen, countdown screen, playing screen, result screen
static void view_scr_select()
{
    view_render.clear();

    const int keeper_x = 10;
    const int shooter_x = 84;
    const int bitmap_y = 3;
    const bool keeper_selected = (game_state.selection_index == SOCCER_ROLE_GOALKEEPER);
    // view_render.drawRoundRect(0, 0, 128, 64, 4, WHITE); use this to check for the boundary
    view_render.drawBitmap(keeper_x, bitmap_y, epd_bitmap_goal_keeper_saving, 34, 34, WHITE);
    view_render.drawBitmap(shooter_x, bitmap_y, epd_bitmap_soccer_guy_shooting_action, 34, 34, WHITE);

    if (keeper_selected)
    {
        view_render.drawRect(keeper_x - 2, bitmap_y - 2, 38, 38, WHITE);
        // view_render.drawBitmap(keeper_x, bitmap_y, bitmap_keeper_stance, 21, 25, WHITE); // will be using bitmap later on
    }
    else
    {
        view_render.drawRect(shooter_x - 2, bitmap_y - 2, 38, 38, WHITE);
        // view_render.drawBitmap(keeper_x, bitmap_y, bitmap_keeper_stance, 21, 25, WHITE); // will be using bitmap later on
    }

    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(15, 43);
    view_render.print("UP/DOWN TO SELECT");
    view_render.setCursor(30, 53);
    view_render.print("MODE TO START");
}

// view screen for countdown phase, shows "GET READY" and the countdown timer
static void view_scr_countdown()
{
    view_render.clear();
    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(38, 12);
    view_render.print("GET READY");
    view_render.setTextSize(4);
    view_render.setCursor(52, 29);
    view_render.print(game_state.countdown_seconds);
}

// view screen for playing phase, shows the goal post, goalkeeper, ball, score, lives, and super mode status
static void draw_lives()
{
    for (int index = 0; index < STARTING_LIVES; index++)
    {
        const int life_x = 112 + (index * 5);
        if (index < game_state.lives)
        {
            view_render.fillCircle(life_x, 7, 2, WHITE);
        }
        else
        {
            view_render.drawCircle(life_x, 7, 2, WHITE);
        }
    }
}

// view screen for playing phase, shows the goal post, goalkeeper, ball, score, lives, and super mode status
static void view_scr_playing()
{
    view_render.clear();
    view_render.drawBitmap(24, 0, epd_bitmap_goal_post, 80, 20, WHITE);
    view_render.drawRect(game_state.keeper_x, GOAL_KEEPER_Y, GOAL_KEEPER_W, GOAL_KEEPER_H, WHITE);
    view_render.fillCircle(game_state.ball_x, game_state.ball_y, BALL_RADIUS, WHITE);

    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(0, 0);
    view_render.print("S:");
    view_render.print(game_state.score);
    view_render.setCursor(0, 10);
    view_render.print(game_state.role == SOCCER_ROLE_GOALKEEPER ? "KEEP" : "SHOOT");

    draw_lives();

    if (game_state.super_mode_active)
    {
        view_render.setCursor(44, 5);
        view_render.print("SUPER");
    }
}

// view screen for result phase, shows win or lose image based on the game result
static void view_scr_result()
{
    view_render.clear();
    if (game_state.result == SOCCER_RESULT_WIN)
    {
        view_render.drawBitmap(2, 0, bitmap_winner_goal_image, 124, 64, WHITE);
    }
    else
    {
        view_render.drawBitmap(0, 0, bitmap_lose_game, 128, 64, WHITE);
    }
}

// main view function that decides which screen to show based on the current game phase
static void view_scr_soccer()
{
    switch (game_state.phase)
    {
    case SOCCER_PHASE_SELECT:
        view_scr_select();
        break;

    case SOCCER_PHASE_COUNTDOWN:
        view_scr_countdown();
        break;

    case SOCCER_PHASE_PLAYING:
        view_scr_playing();
        break;

    case SOCCER_PHASE_RESULT:
        view_scr_result();
        break;
    }

    view_render.update();
}

// dynamic view item for the soccer screen, uses the view_scr_soccer function to render the screen
view_dynamic_t dyn_view_item_soccer = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_soccer};

// main screen struct for the soccer screen, contains the dynamic view item and the message handler function
view_screen_t scr_soccer = {
    &dyn_view_item_soccer,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

// message handler function for the soccer screen, handles all the game logic and state updates based on incoming messages/events
void scr_soccer_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
        
    case SCREEN_ENTRY:
    {
        APP_DBG_SIG("SCREEN_ENTRY\n");
        BUZZER_Disable();
        view_render.initialize();
        view_render_display_on();
        view_render.drawRect(0, 0, 128, 64, WHITE); // use this to check for the boundary
        restart_game();
    }
    break;

    // Soccer timer countdown tick, updates the countdown timer and transitions to gameplay when countdown reaches 0
    case AC_DISPLAY_SHOW_SOCCER_COUNTDOWN:
    {
        if (game_state.phase == SOCCER_PHASE_COUNTDOWN)
        {
            if (game_state.countdown_seconds <= 1)
            {
                begin_gameplay();
            }
            else
            {
                game_state.countdown_seconds--;
                schedule_countdown_tick();
                view_scr_soccer();
            }
        }
    }
    break;

    // Soccer timer play tick, updates the ball position and checks for collisions and scoring
    case AC_DISPLAY_SHOW_SOCCER_PLAY:
    {
        if (game_state.phase == SOCCER_PHASE_PLAYING)
        {
            if (game_state.role == SOCCER_ROLE_GOALKEEPER)
            {
                update_goalkeeper_game();
            }
            else
            {
                update_shooter_game();
            }
            poll_super_mode_led_blink();
            view_scr_soccer();
        }
    }
    break;

    // Soccer timer result tick, transitions back to selection screen after showing result for a few seconds
    case AC_DISPLAY_SHOW_SOCCER_RESULT:
    {
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
    }
    break;

    // Soccer timer super mode tick, deactivates super mode when timer expires
    case AC_DISPLAY_SHOW_SOCCER_SUPER_MODE:
    {
        finish_super_mode();
    }
    break;

    case AC_DISPLAY_BUTON_UP_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            toggle_selection();
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING)
        {
            if (game_state.role == SOCCER_ROLE_GOALKEEPER)
            {
                game_state.keeper_x -= MOVE_STEP_X;
                clamp_keeper_position();
            }
            else
            {
                game_state.ball_x -= MOVE_STEP_X;
                clamp_ball_position();
            }
            view_scr_soccer();
        }
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            toggle_selection();
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING)
        {
            if (game_state.role == SOCCER_ROLE_GOALKEEPER)
            {
                game_state.keeper_x += MOVE_STEP_X;
                clamp_keeper_position();
            }
            else
            {
                game_state.ball_x += MOVE_STEP_X;
                clamp_ball_position();
            }
            view_scr_soccer();
        }
    }
    break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_RELEASED\n");
        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            game_state.role = static_cast<soccer_role_t>(game_state.selection_index);
            begin_countdown();
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING && game_state.role == SOCCER_ROLE_SHOOTER)
        {
            if (game_state.ball_vy == 0)
            {
                game_state.ball_vy = BALL_SHOOT_SPEED_Y;
            }
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING && game_state.role == SOCCER_ROLE_GOALKEEPER)
        {
            activate_super_mode();
        }
        else
        {
            restart_game();
        }
        BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
    }
    break;

    case AC_DISPLAY_BUTON_LONG_MODE_PRESSED:
    case AC_DISPLAY_BUTON_LONG_UP_RELEASED:
    case AC_DISPLAY_BUTON_LONG_DOWN_RELEASED:
        break;

    default:
        break;
    }
}