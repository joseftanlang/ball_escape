#include "ball_escape.h"
#include "app_eeprom.h"

#define BITMAP_BALL_WIDTH (5)
#define BITMAP_BALL_HEIGHT (5)

#define CIRCLE_CENTER_X (62)
#define CIRCLE_CENTER_Y (32)
#define CIRCLE_RADIUS (30)
#define CIRCLE_GAP_WIDTH_DEG (45)
#define CIRCLE_ROTATE_STEP_DEG (12)
#define CIRCLE_START_ROTATION_DEG (0)
#define CIRCLE_SHRINK_STEP (4)
#define CIRCLE_SPAWN_INTERVAL_TICKS (20)
#define CIRCLE_MIN_RADIUS (BITMAP_BALL_WIDTH + 1)
#define CIRCLE_HIT_MARGIN (5)

#define BALL_START_X (CIRCLE_CENTER_X - (BITMAP_BALL_WIDTH / 2))
#define BALL_START_Y (CIRCLE_CENTER_Y)

#define BALL_STEP_COUNT (4)
#define BALL_STEP_Y (2)

#define SCORE_START (0)
#define SCORE_INCREASE (1)

using namespace std;

struct circle_state;

static void view_circle_escape();
static void view_circle_draw();
static void update_circle_rotation(int delta_deg);
static void update_game_tick();
static void update_ball_motion();
static void push_ball_out_of_circle(const circle_state &circle);
static int normalize_angle(int angle_deg);
static bool angle_in_gap(int angle_deg, int gap_center_deg, int gap_width_deg);

struct circle_state
{
    int radius;
    int gap_rotation_deg;
};

static void spawn_circle();
static void shrink_circles();
static bool circle_hit_by_ball(const circle_state &circle);
static void scoring_system();

double pi = 4.0 * atan(1.0);
int cum_score = SCORE_START;

static int circle_rotation_deg = 0;
static int game_tick_count = 0;
static int ball_x = BALL_START_X;
static int ball_y = BALL_START_Y;
static int ball_vy = -BALL_STEP_Y;
static vector<circle_state> active_circles;

// this screen will show a simple game where a ball will bounce around the screen and there is a circle in the middle with a gap, the player needs to rotate the circle to let the ball pass through the gap to increase the score, if the ball hits the circle without passing through the gap, it will bounce back, this game is used to test the performance of the display and also to make the screen more interesting
static int normalize_angle(int angle_deg)
{
    while (angle_deg < 0)
    {
        angle_deg += 360;
    }

    while (angle_deg >= 360)
    {
        angle_deg -= 360;
    }

    return angle_deg;
}

// this function checks if a given angle is within a gap defined by its center and width, taking into account the circular nature of angles
static bool angle_in_gap(int angle_deg, int gap_center_deg, int gap_width_deg)
{
    int start = normalize_angle(gap_center_deg - (gap_width_deg / 2));
    int end = normalize_angle(gap_center_deg + (gap_width_deg / 2));

    angle_deg = normalize_angle(angle_deg);

    if (start <= end)
    {
        return (angle_deg >= start) && (angle_deg <= end);
    }

    return (angle_deg >= start) || (angle_deg <= end);
}

// this function updates the rotation of the circle by a certain degree step, it will be called periodically to make the circle rotate continuously
static void update_circle_rotation(int delta_deg)
{
    circle_rotation_deg = normalize_angle(circle_rotation_deg + delta_deg);
}

static void spawn_circle()
{
    circle_state new_circle;
    new_circle.radius = CIRCLE_RADIUS;
    new_circle.gap_rotation_deg = rand() % 360;
    active_circles.push_back(new_circle);
}

// This function is for the circle to be shrinking
static void shrink_circles()
{
    for (size_t index = 0; index < active_circles.size();)
    {
        if (active_circles[index].radius <= CIRCLE_MIN_RADIUS)
        {
            SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
            return;
        }

        active_circles[index].radius -= CIRCLE_SHRINK_STEP;

        if (active_circles[index].radius <= CIRCLE_MIN_RADIUS)
        {
            SCREEN_TRAN(scr_charts_game_handle, &scr_charts_game);
            return;
        }

        index++;
    }
}

static void scoring_system()
{
    gamescore.score_now = cum_score;

    if (gamescore.score_now == gamescore.score_1st)
    {
    }
    else if (gamescore.score_now > gamescore.score_1st)
    {
        gamescore.score_3rd = gamescore.score_2nd;
        gamescore.score_2nd = gamescore.score_1st;
        gamescore.score_1st = gamescore.score_now;
    }
    else if (gamescore.score_now == gamescore.score_2nd)
    {
    }
    else if (gamescore.score_now > gamescore.score_3rd)
    {
        gamescore.score_3rd = gamescore.score_now;
    }

    ar_game_score_write(&gamescore);
}

// This function checks if the ball hits the circle edge
static bool circle_hit_by_ball(const circle_state &circle)
{
    int ball_center_x = ball_x + (BITMAP_BALL_WIDTH / 2);
    int ball_center_y = ball_y + (BITMAP_BALL_HEIGHT / 2);
    int dx = ball_center_x - CIRCLE_CENTER_X;
    int dy = ball_center_y - CIRCLE_CENTER_Y;
    int distance_sq = dx * dx + dy * dy;
    int outer_radius_sq = (circle.radius + CIRCLE_HIT_MARGIN) * (circle.radius + CIRCLE_HIT_MARGIN);
    int inner_radius_sq = (circle.radius - CIRCLE_HIT_MARGIN) * (circle.radius - CIRCLE_HIT_MARGIN);

    return (distance_sq <= outer_radius_sq) && (distance_sq >= inner_radius_sq);
}

// Move the ball out of the collision band so the next tick does not instantly collide again.
static void push_ball_out_of_circle(const circle_state &circle)
{
    const int min_y = CIRCLE_CENTER_Y - CIRCLE_RADIUS;
    const int max_y = CIRCLE_CENTER_Y + CIRCLE_RADIUS - BITMAP_BALL_HEIGHT;
    const int move_dir = (ball_vy >= 0) ? 1 : -1;

    // Move in the new travel direction until we are out of this ring's hit zone.
    for (int i = 0; i < (CIRCLE_HIT_MARGIN * 4); i++)
    {
        if (!circle_hit_by_ball(circle))
        {
            break;
        }

        ball_y += move_dir;

        if (ball_y < min_y)
        {
            ball_y = min_y;
            break;
        }
        else if (ball_y > max_y)
        {
            ball_y = max_y;
            break;
        }
    }

    // Fallback if still overlapping (can happen with fast shrink/frame timing).
    if (circle_hit_by_ball(circle))
    {
        int ball_center_y = ball_y + (BITMAP_BALL_HEIGHT / 2);
        int dy = ball_center_y - CIRCLE_CENTER_Y;
        int sign = (dy >= 0) ? 1 : -1;
        int safe_center_y = CIRCLE_CENTER_Y + sign * (circle.radius + CIRCLE_HIT_MARGIN + 1);
        ball_y = safe_center_y - (BITMAP_BALL_HEIGHT / 2);

        if (ball_y < min_y)
        {
            ball_y = min_y;
        }
        else if (ball_y > max_y)
        {
            ball_y = max_y;
        }
    }
}

// This function will be called periodically to update the game state, it will update the ball motion, spawn new circles, shrink existing circles, and check for collisions between the ball and the circles
static void update_game_tick()
{
    game_tick_count++;
    update_ball_motion();

    if ((game_tick_count % CIRCLE_SPAWN_INTERVAL_TICKS) == 0)
    {
        spawn_circle();
        shrink_circles();
    }

    for (size_t index = 0; index < active_circles.size();)
    {
        if (circle_hit_by_ball(active_circles[index]))
        {
            int ball_center_x = ball_x + (BITMAP_BALL_WIDTH / 2);
            int ball_center_y = ball_y + (BITMAP_BALL_HEIGHT / 2);
            int dx = ball_center_x - CIRCLE_CENTER_X;
            int dy = ball_center_y - CIRCLE_CENTER_Y;
            int angle_deg = (int)round(atan2((double)dy, (double)dx) * 180.0 / pi);
            int gap_center_deg = normalize_angle(270 + circle_rotation_deg + active_circles[index].gap_rotation_deg);

            if (angle_in_gap(angle_deg, gap_center_deg, CIRCLE_GAP_WIDTH_DEG))
            {
                cum_score += SCORE_INCREASE;
                scoring_system();
                active_circles.erase(active_circles.begin() + index);
                continue;
            }
        }

        index++;
    }
}

// This function updates the motion of the ball based on its current velocity, it also checks for collisions with the walls and bounces the ball back if it hits the walls, it also checks for collisions with the circles and bounces the ball back if it hits the circles without passing through the gap
static void update_ball_motion()
{
    const int min_y = CIRCLE_CENTER_Y - CIRCLE_RADIUS;
    const int max_y = CIRCLE_CENTER_Y + CIRCLE_RADIUS - BITMAP_BALL_HEIGHT;

    for (int step = 0; step < BALL_STEP_COUNT; step++)
    {
        ball_x = BALL_START_X;
        ball_y += ball_vy;

        if (ball_y <= min_y)
        {
            ball_y = min_y;
            ball_vy = -ball_vy;
        }
        else if (ball_y >= max_y)
        {
            ball_y = max_y;
            ball_vy = -ball_vy;
        }

        for (size_t index = 0; index < active_circles.size(); index++)
        {
            if (!circle_hit_by_ball(active_circles[index]))
            {
                continue;
            }

            int ball_center_x = ball_x + (BITMAP_BALL_WIDTH / 2);
            int ball_center_y = ball_y + (BITMAP_BALL_HEIGHT / 2);
            int dx = ball_center_x - CIRCLE_CENTER_X;
            int dy = ball_center_y - CIRCLE_CENTER_Y;
            int angle_deg = (int)round(atan2((double)dy, (double)dx) * 180.0 / pi);
            int gap_center_deg = normalize_angle(270 + circle_rotation_deg + active_circles[index].gap_rotation_deg);

            if (angle_in_gap(angle_deg, gap_center_deg, CIRCLE_GAP_WIDTH_DEG))
            {
                cum_score += SCORE_INCREASE;
                scoring_system();
                active_circles.erase(active_circles.begin() + index);
                index--;
                break;
            }

            ball_vy = -ball_vy;
            push_ball_out_of_circle(active_circles[index]);
            break;
        }
    }
}

view_dynamic_t dyn_view_baller = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_circle_escape};

view_screen_t scr_circle_escape = {
    &dyn_view_baller,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

void view_circle_escape()
{
    // the codes to draw the game screen
    update_game_tick();
    view_circle_draw();
}

static void view_circle_draw()
{
    // draw the rectangle as the background of the game
    view_render.drawRoundRect(0, 0, 128, 64, 5, WHITE);

    // draw the score so it stays visible after each refresh
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(90, 5);
    view_render.print("SCR:");
    view_render.setCursor(113, 5);
    view_render.print(cum_score);

    // draw shrinking circles and their rotating gaps
    for (size_t index = 0; index < active_circles.size(); index++)
    {
        view_render.drawCircle(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, active_circles[index].radius, WHITE);
        arc(CIRCLE_CENTER_X, CIRCLE_CENTER_Y, 270 + circle_rotation_deg + active_circles[index].gap_rotation_deg - (CIRCLE_GAP_WIDTH_DEG / 2), 270 + circle_rotation_deg + active_circles[index].gap_rotation_deg + (CIRCLE_GAP_WIDTH_DEG / 2), active_circles[index].radius);
    }
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE, 130, TIMER_PERIODIC);
    // draw the ball
    view_render.drawBitmap(ball_x, ball_y, epd_bitmap_black_ball, BITMAP_BALL_WIDTH, BITMAP_BALL_HEIGHT, WHITE);
}

// draw an arc (inclusive) of pixels along the circumference between start and end angles
// angles in degrees, 0 = right, 90 = down, 180 = left, 270 = up (screen coordinates)
void arc(int x0, int y0, int stangle, int endangle, int radius)
{
    const double PI = pi;
    int start = normalize_angle(stangle);
    int end = normalize_angle(endangle);

    if (end < start)
    {
        end += 360;
    }

    for (int a = start; a <= end; a++)
    {
        double ang = (a % 360) * PI / 180.0;
        int xr = (int)round(x0 + cos(ang) * radius);
        int yr = (int)round(y0 + sin(ang) * radius);
        view_render.drawPixel(xr, yr, BLACK);
        view_render.drawPixel((int)round(x0 + cos(ang) * (radius - 1)), (int)round(y0 + sin(ang) * (radius - 1)), BLACK);
        view_render.drawPixel((int)round(x0 + cos(ang) * (radius + 1)), (int)round(y0 + sin(ang) * (radius + 1)), BLACK);
    }
}

// this function handles the messages for the ball escape screen, it will update the game state and redraw the screen based on the messages received, such as button presses to rotate the circle and timer events to update the ball's motion
void scr_circle_escape_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
    {
        APP_DBG_SIG("SCREEN_ENTRY\n");
        view_render.initialize();
        view_render_display_on();
        timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE_INTERVAL, TIMER_PERIODIC);
        circle_rotation_deg = CIRCLE_START_ROTATION_DEG;
        game_tick_count = 0;
        active_circles.clear();
        spawn_circle();
        cum_score = SCORE_START;
        ball_x = BALL_START_X;
        ball_y = BALL_START_Y;
        ball_vy = -BALL_STEP_Y;
    }
    break;

    case AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE:
    {
        APP_DBG_SIG("AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE\n");
        update_ball_motion();
    }
    break;

    case AC_DISPLAY_BUTON_UP_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTON_UP_RELEASED\n");
        update_circle_rotation(-CIRCLE_ROTATE_STEP_DEG);
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTON_DOWN_RELEASED\n");
        update_circle_rotation(CIRCLE_ROTATE_STEP_DEG);
    }
    break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_RELEASED\n");
        timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE);
        SCREEN_TRAN(scr_info_handle, &scr_info);
    }
    break;

    default:
        break;
    }
}