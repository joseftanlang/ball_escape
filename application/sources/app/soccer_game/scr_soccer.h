#ifndef __SCR_SOCCER_H__
#define __SCR_SOCCER_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"
#include "led.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include "eeprom.h"
#include "app_eeprom.h"

#include "screens_bitmap.h"

#include <math.h>
#include <vector>

/* Soccer definitions moved to header so helper translation units can use them */
#define GOAL_POST_X (24)
#define GOAL_POST_Y (0)
#define GOAL_POST_W (80)
#define GOAL_POST_H (15)

#define GOAL_KEEPER_X (30)
#define GOAL_KEEPER_Y (10)
#define GOAL_KEEPER_W (21)
#define GOAL_KEEPER_H (25)

#define SOCCER_ACTOR_W (21)
#define SOCCER_ACTOR_H (25)

#define STRIKER_MIN_X (30)
#define STRIKER_MAX_X (90)
#define STRIKER_Y (30)
#define STRIKER_BALL_OFFSET_X (16)
#define STRIKER_BALL_OFFSET_Y (21)

#define BALL_POSITION_SCALE (10)
#define BALL_X (40)
#define BALL_Y (58)
#define BALL_RADIUS (3)
#define BALL_SHOOT_SPEED_Y (-15)
#define BALL_GOALKEEPER_SPEED_Y (-10)
#define BALL_GOALKEEPER_SPEED_X (6)

#define STARTING_LIVES (3)
#define GOALKEEPER_WIN_SCORE (10)
#define SHOOTER_WIN_SCORE (3)
#define READY_COUNTDOWN_SECONDS (3)
#define SUPER_MODE_DURATION_MS (5000)
#define RESULT_SCREEN_DURATION_MS (3000)
#define PLAY_TICK_MS (150)
#define SOCCER_INPUT_COOLDOWN_MS (300)
#define MOVE_STEP_Y (4)
#define MOVE_STEP_X (4)
#define SUPER_MODE_BLINK_COUNT (5)
#define KEEPER_AUTO_MIN_SPEED (1)
#define KEEPER_AUTO_MAX_SPEED (4)

typedef enum soccer_role_t {
	SOCCER_ROLE_GOALKEEPER = 0,
	SOCCER_ROLE_SHOOTER = 1,
} soccer_role_t;

typedef enum soccer_phase_t {
	SOCCER_PHASE_SELECT = 0,
	SOCCER_PHASE_COUNTDOWN,
	SOCCER_PHASE_PLAYING,
	SOCCER_PHASE_RESULT,
} soccer_phase_t;

typedef enum soccer_result_t {
	SOCCER_RESULT_NONE = 0,
	SOCCER_RESULT_WIN,
	SOCCER_RESULT_LOSE,
} soccer_result_t;

typedef struct soccer_game_t {
	soccer_role_t role;
	soccer_phase_t phase;
	soccer_result_t result;
	int selection_index;
	int lives;
	int score;
	int streak;
	int countdown_seconds;
	int keeper_x;
	int keeper_vx;
	int striker_x;
	int striker_y;
	int ball_x_fp;
	int ball_y_fp;
	int ball_vx_fp;
	int ball_vy_fp;
	int ball_x;
	int ball_y;
	bool super_mode_active;
	int super_mode_led_toggle_remaining;
} soccer_game_t;

extern soccer_game_t game_state;

/* helpers used by other soccer translation units */
int clamp_int(int value, int minimum, int maximum);
void set_ball_position(int x, int y);
void sync_ball_display_position();
void set_striker_position(int x);
void align_ball_with_striker();
void move_shooter_position(int delta_x);
void reset_ball();
void clamp_keeper_position();
void clamp_ball_position();
bool keeper_covers_ball();

/* lifecycle functions */
void stop_soccer_timers();
void schedule_countdown_tick();
void begin_gameplay();
void begin_countdown();
void begin_result(soccer_result_t result);
void restart_game();
void toggle_selection();
bool accept_soccer_input();
void stop_super_mode_led_blink();
void view_scr_soccer();  /* Internal view rendering function used by ar_*.cpp */

extern view_dynamic_t dyn_view_item_soccer;
extern view_screen_t scr_soccer;
extern void scr_soccer_handle(ak_msg_t* msg);

#endif // __SCR_SOCCER_H__
