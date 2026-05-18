#include "scr_dvd.h"
#include "scr_soccer.h"

static void view_scr_dvd();
static void dvd_moving();

view_dynamic_t dyn_view_item_dvd = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_dvd};

view_screen_t scr_dvd = {
    &dyn_view_item_dvd,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

// DVD parameters
#define DVD_WIDTH 38
#define DVD_HEIGHT 19
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DVD_UPDATE_INTERVAL 50 // ms between updates

// DVD movement update interval in ms (smaller = faster)
#define DVD_START_INTERVAL 100
#define DVD_INTERVAL_STEP 20
#define DVD_MIN_INTERVAL 40
#define DVD_MAX_INTERVAL 140

// DVD state
static int dvd_x = 0;
static int dvd_y = 0;
static int dvd_vx = 2; // velocity in x direction
static int dvd_vy = 1; // velocity in y direction
static uint32_t last_update_time = 0;
static uint32_t dvd_update_interval = DVD_START_INTERVAL;


static void view_scr_dvd()
{
    view_render.drawRoundRect(0, 0, 128, 64, 5, WHITE);
    dvd_moving();
}

static void dvd_apply_update_interval()
{
    timer_set(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_DVD_UPDATE, dvd_update_interval, TIMER_PERIODIC);
}

// This function updates the DVD's position and handles bouncing off the walls
static void dvd_moving()
{
    uint32_t current_time = sys_ctrl_millis();

    // Update position at regular intervals
    if (current_time - last_update_time >= DVD_UPDATE_INTERVAL)
    {
        last_update_time = current_time;

        // Update position
        dvd_x += dvd_vx;
        dvd_y += dvd_vy;

        // Bounce off walls
        if (dvd_x <= 0)
        {
            dvd_x = 0;
            dvd_vx = -dvd_vx;
        }
        if (dvd_x + DVD_WIDTH >= DISPLAY_WIDTH)
        {
            dvd_x = DISPLAY_WIDTH - DVD_WIDTH;
            dvd_vx = -dvd_vx;
        }
        // This check ensures the DVD doesn't go off the top edge of the display
        if (dvd_y <= 0)
        {
            dvd_y = 0;
            dvd_vy = -dvd_vy;
        }
        // This check ensures the DVD doesn't go off the bottom edge of the display
        if (dvd_y + DVD_HEIGHT >= DISPLAY_HEIGHT)
        {
            dvd_y = DISPLAY_HEIGHT - DVD_HEIGHT;
            dvd_vy = -dvd_vy;
        }
    }

    // Clear and draw
    view_render.clear();
    view_render.drawBitmap(dvd_x, dvd_y, bitmap_DVD, DVD_WIDTH, DVD_HEIGHT, WHITE);
}

void scr_dvd_handle(ak_msg_t *msg)
{
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
        APP_DBG_SIG("SCREEN_ENTRY\n");
        BUZZER_Disable();
        view_render.initialize();
        // view_render_display_on();
        dvd_x = 10; // Start at the top-left corner
        dvd_y = 0;  // Start at the top-left corner
        dvd_vx = 2;
        dvd_vy = 1;
        last_update_time = sys_ctrl_millis();
        dvd_update_interval = DVD_START_INTERVAL;
        dvd_apply_update_interval();
        // view_render.drawRoundRect(0, 0, 128, 64, 5, WHITE); // Initial draw
        break;

    case AC_DISPLAY_BUTON_UP_RELEASED:
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_RELEASED\n");
        BUZZER_Disable();
        if (dvd_update_interval > DVD_MIN_INTERVAL)
        {
            if (dvd_update_interval <= DVD_MIN_INTERVAL + DVD_INTERVAL_STEP)
            {
                dvd_update_interval = DVD_MIN_INTERVAL;
            }
            else
            {
                dvd_update_interval -= DVD_INTERVAL_STEP;
            }
            dvd_apply_update_interval();
        }
        break;

    case AC_DISPLAY_BUTON_DOWN_RELEASED:
        APP_DBG_SIG("AC_DISPLAY_BUTTON_DOWN_RELEASED\n");
        BUZZER_Disable();
        if (dvd_update_interval < DVD_MAX_INTERVAL)
        {
            if (dvd_update_interval >= DVD_MAX_INTERVAL - DVD_INTERVAL_STEP)
            {
                dvd_update_interval = DVD_MAX_INTERVAL;
            }
            else
            {
                dvd_update_interval += DVD_INTERVAL_STEP;
            }
            dvd_apply_update_interval();
        }
        break;

    case AC_DISPLAY_BUTON_MODE_RELEASED:
        APP_DBG_SIG("AC_DISPLAY_BUTTON_MODE_LONG_PRESSED\n");
        BUZZER_Disable();
        // timer_remove_attr(AC_TASK_DISPLAY_ID, AC_DISPLAY_SHOW_DVD_UPDATE);
        SCREEN_TRAN(scr_soccer_handle, &scr_soccer);
        break;  

    case AC_DISPLAY_SHOW_DVD_UPDATE:
        dvd_moving();
        break;

    default:
        break;
    }
}