#include "scr_soccer.h"

// Goal post
#define GOAL_POST_X (10)
#define GOAL_POST_Y (40)
#define GOAL_POST_W (10)
#define GOAL_POST_H (20)

// Goal keeper
#define GOAL_KEEPER_X (20)
#define GOAL_KEEPER_Y (40)
#define GOAL_KEEPER_W (10)
#define GOAL_KEEPER_H (20)

// Soccer ball
#define BALL_X (62)
#define BALL_Y (50)
#define BALL_VX (0)
#define BALL_VY (0)

//scoring lives system
#define STARTING_LIVES (3)
#define MIN_LIVES (0) //so when the player score 3 goals then the game is over and the player lose. will be using fillCircle as lives and drawCircle as lost lives

//at any point of time within the game if they want restart they just need to long press the mode.
static void restart_game() {
    // reset the game state to initial conditions
    // reset the position of the ball, goal keeper, and goal post
    // reset the score and lives
    //reset timer for the game loop
}

//when first enter the game, the user can choose between goalkeeper or shooter
//Then will have the two bitmap "epd_bitmap_goal_keeper_saving" and "epd_bitmap_soccer_guy_shooting_action" to display the two options. the user can press the mode button to switch between the two options and then press the select button to confirm the choice. after the user choose, then start the game with a timer of 3 seconds to make sure that the user is ready.
//make sure that when selectign the one being select is highlighted and the other one is not highlighted. for example, if the user select the goalkeeper, then the "epd_bitmap_goal_keeper_saving" will be highlighted and the "epd_bitmap_soccer_guy_shooting_action" will not be highlighted. if the user select the shooter, then the "epd_bitmap_soccer_guy_shooting_action" will be highlighted and the "epd_bitmap_goal_keeper_saving" will not be highlighted.
//can be contorl with the button up and down to switch between the two options and then press the select button to confirm the choice. after the user choose, then start the game with a timer of 3 seconds to make sure that the user is ready.

//if choose goalkeeper, then the user can control the goal keeper to move left (up button) and right (down button) to save the ball. if the ball hit the goal post, then the user lose one life. if the user save the ball, then the user score one point. when the user lose all lives, then the game is over.
// If the goalkeeper failed to save 3 balls then the game is over
// If the goal keeper save 10 balls then the game win
//If winner then display this: bitmap_winner_goal_image for 3 seconds then go to main screen, if lose then display this: bitmap_lose_game for 3 seconds then go to main screen.
// If the goal keeper save 3 ball in a row the user can press the mode button to activate the super save mode, in this mode the goal keeper can save the ball even if the ball hit the goal post. the super save mode last for 5 seconds and then go back to normal mode.
//There is a timer of up to 3 seconds to make sure that the user is ready.

// If choose shooter, then the user can control the ball to move up and down to shoot the ball. if the ball hit the goal post, then the user score one point. if the ball hit the goal keeper, then the user lose one life. when the user lose all lives, then the game is over.
//The shooter can score max 3 goals and there is a timer of 15 seconds to shoot the ball. if the user score 3 goals before the timer run out, then the user win. if the timer run out before the user score 3 goals, then the user lose.
// If the shooter score 10 times then the game win
//If winner then display this: bitmap_winner_goal_image for 3 seconds then go to main screen, if lose then display this: bitmap_lose_game for 3 seconds then go to main screen.
// If the shooter score 3 goals in a row the user can press the mode button to activate the super shoot mode, in this mode the ball can hit the goal post and still score. the super shoot mode last for 5 seconds and then go back to normal mode.
//There is a timer of up to 3 seconds to make sure that the user is ready.



static void counter();

static void penaly_shoot()
{
    view_render.drawBitmap(0, 0, epd_bitmap_penalty_shoot_1, 128, 64, BLACK);
    view_render.update();
}

static void view_scr_soccer();

// this is to create the soccer ball using the circle filled radius.
class ball
{
public:
    int x;
    int y;
    int vx;
    int vy;
    int radius;

    ball()
    {
        x = BALL_X;
        y = BALL_Y;
    }

    int distance(ball &__ball)
    {
        uint8_t dx, dy;
        dx = abs(x - __ball.x);
        dy = abs(y - __ball.y);
        return sqrt(dx * dx + dy * dy);
    }
};

static void soccer_moving()
{
    // TODO: Implement ball movement logic
}

static std::vector<ball> v_idle_ball;

static void view_scr_soccer()
{
    for (ball _ball : v_idle_ball)
    {
        view_render.drawCircle(_ball.x, _ball.y, _ball.radius, 144);
    }
    soccer_moving();
}

view_dynamic_t dyn_view_item_soccer = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_soccer};

view_screen_t scr_soccer = {
    &dyn_view_item_soccer,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

static void counter()
{
    static int count = 0;
    count++;
    APP_DBG_SIG("counter: %d\n", count);
    view_render.setCursor(80, 5);
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);

    for (int i = 3; i >= 1; i--)
    {
        view_render.print("T: ");
        view_render.print(i);
    }
    BUZZER_PlaySound(BUZZER_SOUND_3BEEP);
}

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
        view_render.clear();
        penaly_shoot();
    }
    break;

    case AC_DISPLAY_BUTON_UP_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
        // make the goal keeper move up
    }
    break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
    {
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
        // make the goal keeper move down
    }
    break;
    }
}