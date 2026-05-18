#include "scr_soccer.h"
#include "scr_idle.h"
#include "task_life.h"
#include "ar_ball.h"
#include "ar_keeper.h"
#include "ar_striker.h"

/* Shared game state is declared in header; define it here for the module */
soccer_game_t game_state;
static uint32_t last_soccer_input_ms = 0;
static bool is_drawing = false;  /* Rendering lock to prevent nested view updates */
static bool needs_render = true; /* Flag to queue a render on next game tick */

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
void stop_soccer_timers()
{
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_COUNTDOWN);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_PLAY);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_RESULT);
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_SUPER_MODE);
}

// Clamps an integer value between a minimum and maximum range
int clamp_int(int value, int minimum, int maximum)
{
    if (value < minimum)
    {
        return minimum;
    }

    if (value > maximum)
    {
        return maximum;
    }

    return value;
}

bool accept_soccer_input()
{
    const uint32_t current_time_ms = sys_ctrl_millis();

    if (last_soccer_input_ms != 0 && (current_time_ms - last_soccer_input_ms) < SOCCER_INPUT_COOLDOWN_MS)
    {
        return false;
    }

    last_soccer_input_ms = current_time_ms;
    return true;
}

// Mark that we need to render on the next game tick
static inline void mark_render_needed()
{
    needs_render = true;
}

// Synchronizes the ball's display position with its fixed-point position
void sync_ball_display_position()
{
    game_state.ball_x = game_state.ball_x_fp / BALL_POSITION_SCALE;
    game_state.ball_y = game_state.ball_y_fp / BALL_POSITION_SCALE;
}

// Sets the ball's position using fixed-point values and updates the display position accordingly
void set_ball_position(int x, int y)
{
    game_state.ball_x_fp = x * BALL_POSITION_SCALE;
    game_state.ball_y_fp = y * BALL_POSITION_SCALE;
    sync_ball_display_position();
}

// Setting the striker's position while ensuring it stays within defined bounds, and keeping the ball aligned with the striker based on defined offsets
void set_striker_position(int x)
{
    game_state.striker_x = clamp_int(x, STRIKER_MIN_X, STRIKER_MAX_X);
    game_state.striker_y = STRIKER_Y;
}

// Keeps the ball's position aligned with the striker's position based on defined offsets
void align_ball_with_striker()
{
    set_ball_position(game_state.striker_x + STRIKER_BALL_OFFSET_X - 14, game_state.striker_y + STRIKER_BALL_OFFSET_Y);
}

// Moves the striker left or right by a fixed step, ensuring it stays within bounds, and keeps the ball aligned with the striker
void move_shooter_position(int delta_x)
{
    set_striker_position(game_state.striker_x + delta_x);
    align_ball_with_striker();
}

// Resets the ball to the starting position and velocity
void reset_ball()
{
    if (game_state.role == SOCCER_ROLE_GOALKEEPER)
    {
        set_striker_position(random(STRIKER_MIN_X, STRIKER_MAX_X + 1));
        align_ball_with_striker();
        game_state.ball_vx_fp = random(-BALL_GOALKEEPER_SPEED_X, BALL_GOALKEEPER_SPEED_X + 1);
        if (game_state.ball_vx_fp == 0)
        {
            game_state.ball_vx_fp = BALL_GOALKEEPER_SPEED_X;
        }
        game_state.ball_vy_fp = BALL_GOALKEEPER_SPEED_Y;
    }
    else
    {
        set_striker_position((128 - SOCCER_ACTOR_W) / 2);
        align_ball_with_striker();
        game_state.ball_vx_fp = 0;
        game_state.ball_vy_fp = 0;
        game_state.keeper_x = random(GOAL_POST_X, GOAL_POST_X + GOAL_POST_W - GOAL_KEEPER_W + 1);
        game_state.keeper_vx = random(KEEPER_AUTO_MIN_SPEED, KEEPER_AUTO_MAX_SPEED);
        if (random(0, 2) == 0)
        {
            game_state.keeper_vx = -game_state.keeper_vx;
        }
    }

    sync_ball_display_position();
}

// Schedules the next countdown tick after 1 second
void schedule_countdown_tick()
{
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_COUNTDOWN, 1000, TIMER_PERIODIC);
}

// Starts the gameplay phase
void begin_gameplay()
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
}

// Initializes the game state and starts the selection phase
void begin_countdown()
{
    stop_soccer_timers();
    game_state.phase = SOCCER_PHASE_COUNTDOWN;
    game_state.countdown_seconds = READY_COUNTDOWN_SECONDS;
    schedule_countdown_tick();
}

// Ends the game and shows the result screen for a few seconds
void begin_result(soccer_result_t result)
{
    stop_soccer_timers();
    game_state.phase = SOCCER_PHASE_RESULT;
    game_state.result = result;
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_RESULT, RESULT_SCREEN_DURATION_MS, TIMER_ONE_SHOT);
}

// Resets the game state to start a new game
void restart_game()
{
    stop_soccer_timers();
    last_soccer_input_ms = 0;
    game_state.role = SOCCER_ROLE_GOALKEEPER;
    game_state.phase = SOCCER_PHASE_SELECT;
    game_state.result = SOCCER_RESULT_NONE;
    game_state.selection_index = SOCCER_ROLE_GOALKEEPER;
    game_state.lives = STARTING_LIVES;
    game_state.score = 0;
    game_state.streak = 0;
    game_state.countdown_seconds = READY_COUNTDOWN_SECONDS;
    game_state.keeper_x = GOAL_KEEPER_X;
    game_state.keeper_vx = KEEPER_AUTO_MIN_SPEED;
    game_state.striker_x = STRIKER_MIN_X;
    game_state.striker_y = STRIKER_Y;
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
    reset_ball();
}

// Toggles the selection between goalkeeper and shooter in the selection screen
void toggle_selection()
{
    game_state.selection_index = 1 - game_state.selection_index;
}

// keeps the goalkeeper within the goal area
void clamp_keeper_position()
{
    game_state.keeper_x = clamp_int(game_state.keeper_x, GOAL_POST_X, GOAL_POST_X + GOAL_POST_W - GOAL_KEEPER_W);
}

// ball should not go outside of the goal post
void clamp_ball_position()
{
    game_state.ball_x = clamp_int(game_state.ball_x, BALL_RADIUS, 128 - BALL_RADIUS - 1);
    game_state.ball_y = clamp_int(game_state.ball_y, BALL_RADIUS, 64 - BALL_RADIUS - 1);
    game_state.ball_x_fp = game_state.ball_x * BALL_POSITION_SCALE + 5;
    game_state.ball_y_fp = game_state.ball_y * BALL_POSITION_SCALE;
}

// Checks if the goalkeeper is covering the ball's position
bool keeper_covers_ball()
{
    const int keeper_left = game_state.keeper_x;
    const int keeper_right = game_state.keeper_x + GOAL_KEEPER_W;
    const int ball_left = game_state.ball_x - BALL_RADIUS;
    const int ball_right = game_state.ball_x + BALL_RADIUS;
    const int keeper_top = GOAL_KEEPER_Y;
    const int keeper_bottom = GOAL_KEEPER_Y + GOAL_KEEPER_H;

    return (ball_right >= keeper_left) &&
           (ball_left <= keeper_right) &&
           (game_state.ball_y >= keeper_top) &&
           (game_state.ball_y <= keeper_bottom);
}

// This function is called when the super mode timer expires to deactivate super mode
bool activate_super_mode()
{
    if (game_state.super_mode_active || game_state.streak < 3)
    {
        return false;
    }

    game_state.super_mode_active = true;
    game_state.super_mode_led_toggle_remaining = SUPER_MODE_BLINK_COUNT * 2;
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_SOCCER_SUPER_MODE, SUPER_MODE_DURATION_MS, TIMER_ONE_SHOT);
    return true;
}

// finish super mode when timer expires
void finish_super_mode()
{
    game_state.super_mode_active = false;
    stop_super_mode_led_blink();
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
    view_render.drawBitmap(game_state.keeper_x, GOAL_KEEPER_Y, bitmap_keeper_stance, GOAL_KEEPER_W, GOAL_KEEPER_H, WHITE);
    const unsigned char *striker_bitmap = bitmap_soccer_striker;
    int striker_w = SOCCER_ACTOR_W;
    int striker_h = SOCCER_ACTOR_H;
    if (game_state.role == SOCCER_ROLE_SHOOTER && game_state.ball_vy_fp != 0)
    {
        striker_bitmap = bitmap_socer_striker_finsh;
        striker_w = SOCCER_ACTOR_W;
        striker_h = SOCCER_ACTOR_H;
    }
    view_render.drawBitmap(game_state.striker_x, game_state.striker_y, striker_bitmap, striker_w, striker_h, WHITE);
    view_render.fillCircle(game_state.ball_x, game_state.ball_y, BALL_RADIUS, WHITE);

    view_render.setTextColor(WHITE);
    view_render.setTextSize(1);
    view_render.setCursor(0, 0);
    view_render.print("S:");
    view_render.print(game_state.score);
    view_render.setCursor(0, 10);
    view_render.print(game_state.role == SOCCER_ROLE_GOALKEEPER ? "KEEP" : "SHOT");

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
    if (is_drawing)
        return;        // If we are already drawing, ignore this request
    is_drawing = true; // Lock

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

    view_render.update(); // The heavy part

    is_drawing = false; // Unlock
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
        last_soccer_input_ms = 0;
        needs_render = true; /* Force first render */
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
            }
            mark_render_needed(); /* Queue render for PLAY_TICK */
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
                ar_ball_update_goalkeeper_game();
            }
            else
            {
                ar_keeper_update_game();
                ar_ball_update_shooter_game();
            }
            poll_super_mode_led_blink();
        }
        /* Always render on PLAY_TICK - this is the only render point */
        view_scr_soccer();
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
        if (!accept_soccer_input())
        {
            break;
        }

        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            toggle_selection();
            mark_render_needed();
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING)
        {
            if (game_state.role == SOCCER_ROLE_GOALKEEPER)
            {
                game_state.keeper_x -= MOVE_STEP_X;
                clamp_keeper_position();
                mark_render_needed();
            }
            else if (game_state.ball_vy_fp == 0)
            {
                ar_striker_move_position(-MOVE_STEP_X);
                mark_render_needed();
            }
        }
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
        if (!accept_soccer_input())
        {
            break;
        }

        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            toggle_selection();
            mark_render_needed();
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING)
        {
            if (game_state.role == SOCCER_ROLE_GOALKEEPER)
            {
                game_state.keeper_x += MOVE_STEP_X;
                clamp_keeper_position();
                mark_render_needed();
            }
            else if (game_state.ball_vy_fp == 0)
            {
                ar_striker_move_position(MOVE_STEP_X);
                mark_render_needed();
            }
        }
    }
    break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_RELEASED\n");
        if (!accept_soccer_input())
        {
            break;
        }

        bool handled = false;
        if (game_state.phase == SOCCER_PHASE_SELECT)
        {
            game_state.role = static_cast<soccer_role_t>(game_state.selection_index);
            begin_countdown();
            handled = true;
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING && game_state.role == SOCCER_ROLE_SHOOTER)
        {
            if (game_state.ball_vy_fp == 0)
            {
                ar_striker_shoot_ball();
                handled = true;
            }
        }
        else if (game_state.phase == SOCCER_PHASE_PLAYING && game_state.role == SOCCER_ROLE_GOALKEEPER)
        {
            handled = activate_super_mode();
        }
        else
        {
            restart_game();
            handled = true;
        }

        if (handled)
        {
            BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
            mark_render_needed(); /* Queue render for next game tick */
        }
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