#include "scr_peashooter.h"
#include "timer.h"

#define PEA_START_X		(28)
#define PEA_START_Y		(23)
#define PEA_STEP_X		(4)
#define PEA_WIDTH		(25)
#define PEA_HEIGHT		(13)

#define SHOOTING_INTERVAL_STEP_MS (50)
#define MIN_SHOOTING_INTERVAL_MS (10)
#define MAX_SHOOTING_INTERVAL_MS (510)

static void view_scr_peashooter();
static void ball_flying_anim();
static void update_ball_flight();

static int pea_x = PEA_START_X;
static int pea_y = PEA_START_Y;
static uint8_t pea_frame = 0;
static uint16_t shooting_interval_ms = AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE_INTERVAL;

static void view_scr_peashooter() {
    view_render.clear();
    view_render.drawBitmap(0, 0, epd_bitmap_pea_helmet_shoot, 56, 64, WHITE);
    ball_flying_anim();
    view_render.drawRect(0, 0, 128, 64, WHITE);
    view_render.update();
}

view_dynamic_t dyn_view_peashooter = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_peashooter
};

view_screen_t scr_peashooter = {
    &dyn_view_peashooter,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

// animate the flying ball, there are 4 frames in total, and each frame is 25x13px.
static void ball_flying_anim() {
    switch (pea_frame) {
    case 0:
        view_render.drawBitmap(pea_x, pea_y, epd_bitmap_ball_flying_1, PEA_WIDTH, PEA_HEIGHT, WHITE);
        break;

    case 1:
        view_render.drawBitmap(pea_x, pea_y, epd_bitmap_ball_flying_2, PEA_WIDTH, PEA_HEIGHT, WHITE);
        break;

    case 2:
        view_render.drawBitmap(pea_x, pea_y, epd_bitmap_ball_flying_3, PEA_WIDTH, PEA_HEIGHT, WHITE);
        break;

    default:
        view_render.drawBitmap(pea_x, pea_y, epd_bitmap_ball_flying_4, PEA_WIDTH, PEA_HEIGHT, WHITE);
        break;
    }
}

//update the ball flying status, including position and frame index.
static void update_ball_flight() {
    pea_x += PEA_STEP_X;
    pea_frame = (pea_frame + 1) & 0x03;

    if (pea_x > (128 - PEA_WIDTH)) {
        pea_x = PEA_START_X;
        pea_y = PEA_START_Y;
    }

    view_scr_peashooter();
}

//set the shooting interval, and update the timer accordingly.
static void set_shooting_interval_ms(uint16_t interval_ms) {
    if (interval_ms < MIN_SHOOTING_INTERVAL_MS) {
        interval_ms = MIN_SHOOTING_INTERVAL_MS;
    }
    else if (interval_ms > MAX_SHOOTING_INTERVAL_MS) {
        interval_ms = MAX_SHOOTING_INTERVAL_MS;
    }

    shooting_interval_ms = interval_ms;
    timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE);
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE, shooting_interval_ms, TIMER_PERIODIC);
}

void scr_peashooter_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY:
        APP_DBG_SIG("SCREEN_ENTRY\n");
        view_render.initialize();
        view_render_display_on();
        pea_x = PEA_START_X;
        pea_y = PEA_START_Y;
        pea_frame = 0;
        shooting_interval_ms = AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE_INTERVAL;
        timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE, shooting_interval_ms, TIMER_PERIODIC);
        view_scr_peashooter();
        break;

    case AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE:
        update_ball_flight();
        break;

    case AC_DISPLAY_BUTON_MODE_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTON_MODE_RELEASED\n");
        timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_BALL_ESCAPE_UPDATE);
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
    }
        break;
    
    case AC_DISPLAY_BUTON_UP_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTON_UP_RELEASED\n");
        set_shooting_interval_ms(shooting_interval_ms - SHOOTING_INTERVAL_STEP_MS);
    }
        break;
    
    case AC_DISPLAY_BUTON_DOWN_RELEASED: {
        APP_DBG_SIG("AC_DISPLAY_BUTON_DOWN_RELEASED\n");
        set_shooting_interval_ms(shooting_interval_ms + SHOOTING_INTERVAL_STEP_MS);
    }
        break;

    default:
        break;
    }
}
