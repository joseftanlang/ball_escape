// Since \(\tan(\pi/4) = 1\), then \(\pi = 4 \times \text{atan}(1)\).
// Code: double pi = 4.0 * atan(1.0);

#include "ball_gamer.h"

#define START_BALL_X (0)  // at the center of the screen when start then it will start to drop
#define START_BALL_Y (32) // at the center of the screen when start then it will start to drop

// this is for the ball which will drop and bounce in the peashooter screen, it will be used to test the performance of the display and also to make the screen more interesting
#define BALL_RADIUS (7)
#define BALL_MOVE_INTERVAL (25)   // ball move update interval
#define BALL_MOVE_STEP (1)        // ball move step
#define BALL_MAX_FALL_SPEED (5)   // max fall speed
#define BALL_BOUNCE_DAMPING (0.7) // bounce damping factor

// this is for the circle which will continuously grow and shrink in the peashooter screen, it will be used to test the performance of the display and also to make the screen more interesting
#define Circle_GAME_START_X (64)    // circle game start x axis
#define Circle_GAME_START_Y (32)    // circle game start y axis
#define CIRCLE_GAME_MIN_RADIUS (4)  // circle game min radius
#define CIRCLE_GAME_MAX_RADIUS (40) // circle game max radius

double pi = 4.0 * atan(1.0); // since the library does not have a constant for pi, we will define it ourselves

double game_ball_score = 0; // the score of the ball game, it will be updated based on the distance between the ball and the circle, the closer the ball is to the circle, the higher the score

class Ball
{
public:
    Ball() : x(START_BALL_X), y(START_BALL_Y), vx(0), vy(0)
    {
    }

    int distance_to_the_circle()
    {
        int dx = x - Circle_GAME_START_X;
        int dy = y - Circle_GAME_START_Y;
        return sqrt(dx * dx + dy * dy);
    }

    void update()
    {
        // apply gravity
        vy += GRAVITY;
        if (vy > BALL_MAX_FALL_SPEED)
        {
            vy = BALL_MAX_FALL_SPEED;
        }

        // update position
        x += vx;
        y += vy;

        // check for collision with the ground
        if (y + BALL_RADIUS > 64)
        {
            y = 64 - BALL_RADIUS;           // reset position to the ground level
            vy = -vy * BALL_BOUNCE_DAMPING; // reverse velocity and apply damping

            // if the bounce is too small, stop the ball
            if (abs(vy) < 1)
            {
                vy = 0;
            }
        }
    }
}

// produce the circle which will continuously grow and shrink in the peashooter screen, it will be used to test the performance of the display and also to make the screen more interesting
class Circle
{
public:
    Circle() : radius(CIRCLE_GAME_MIN_RADIUS), growing(true) {}
    int radius;
    bool growing;

    void update()
    {
        if (growing)
        {
            radius += 1;
            if (radius >= CIRCLE_GAME_MAX_RADIUS)
            {
                growing = false;
            }
        }
        else
        {
            radius -= 1;
            if (radius <= CIRCLE_GAME_MIN_RADIUS)
            {
                growing = true;
            }
        }
    }

    if (distance_to_the_circle() < radius + BALL_RADIUS)
    {
        // collision detected, bounce the ball
        vx = -vx;                       // reverse horizontal velocity
        vy = -vy * BALL_BOUNCE_DAMPING; // reverse vertical velocity and apply damping
    }
    else
    {
        // no collision, apply normal gravity
        update();
    }

    void draw()
    {
        view_render.drawCircle(Circle_GAME_START_X, Circle_GAME_START_Y, radius, WHITE);
    }

    void update_and_draw()
    {
        update();
        draw();
    }
};

void ball_game_restart()
{
    Ball ball;
    Circle circle;
    while (true)
    {
        ball.update();
        circle.update_and_draw();
        scoring_system();
        game_collision();
        delay(BALL_MOVE_INTERVAL);
    }
}

static void view_scr_peashooter()
{
    view_render.drawBitmap(0, 0, bitmap_peashooter, 128, 64, WHITE);
}

view_dynamic_t dyn_view_peashooter = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_peashooter};

view_screen_t scr_peashooter = {
    &dyn_view_peashooter,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

void scoring_system()
{
    if (Ball.distance_to_the_circle() < Circle.radius)
    {
        game_ball_score += 10; // increase score if the ball is inside the circle
    }
    else
    {
        // it will get nothing
    }
}

void game_loop()
{
    Ball.update();
    Circle.update_and_draw();
    scoring_system();
}

void game_collision()
{
    if (Ball.distance_to_the_circle() < Circle.radius + BALL_RADIUS)
    {
        // collision detected, bounce the ball
        Ball.vx = -Ball.vx;                       // reverse horizontal velocity
        Ball.vy = -Ball.vy * BALL_BOUNCE_DAMPING; // reverse vertical velocity and apply damping
    }
}

void game_over_screen()
{
    view_render.clear();
    view_render.setTextSize(2);
    view_render.setTextColor(WHITE);
    view_render.setCursor(4, 5);
    view_render.print("Game Over");
    view_render.setCursor(4, 20);
    view_render.print("Your Score: %d", game_ball_score);
    BUZZER_PlayTones(tones_3beep);
}

void scr_peashooter_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
        break;

    case AC_DISPLAY_BUTON_UP_RELEASED:
    {
        // we will move the screen's circle right to let the ball out if possible
    }
    break;
    case AC_DISPLAY_BUTON_DOWN_RELEASED:
    {
        // we will move the screen's circle left to let the ball out if possible
    }
    break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
    {
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
    }
    break;

    default:
        break;
    }
}
