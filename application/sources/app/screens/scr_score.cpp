#include "screens_bitmap.h"
#include "ball_escape.h"
#include "scr_startup.h"
#include "scr_idle.h"
#include "scr_info.h"
#include "scr_peashooter.h"
#include "scr_score.h"
#include "app_eeprom.h"

static void view_scr_charts_game();
static ar_game_score_t gamescore_charts;

view_dynamic_t dyn_view_item_charts_game = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_charts_game
};

view_screen_t scr_charts_game = {
    &dyn_view_item_charts_game,
    ITEM_NULL,
    ITEM_NULL,

    .focus_item = 0,
};

void view_scr_charts_game() {
	view_render.clear();
	view_render.fillScreen(WHITE);
	// Draw icon and frames
	view_render.drawBitmap(14, 22, iconLeaderBoard, 100, 40, 0); //drawbit(start_width,start_height,bitmap_icon, height, width, color)
	// view_render.fillRoundRect(1, 28, 126, 12, 5, 0);
	// view_render.fillRoundRect(1, 51, 126, 12, 5, 0);

	// Draw score text
	view_render.setTextSize(2);
	view_render.setTextColor(BLACK);
	view_render.setCursor(28, 18);
    // view_render.print("2nd");
	view_render.print(gamescore_charts.score_2nd);
	view_render.setCursor(58, 7);
    // view_render.print("1st");
	view_render.print(gamescore_charts.score_1st);
	view_render.setCursor(91, 23);
    // view_render.print("3rd");
	view_render.print(gamescore_charts.score_3rd);
}

void scr_charts_game_handle(ak_msg_t* msg) {
    switch (msg->sig)
    {
    case SCREEN_ENTRY:
        APP_DBG_SIG("SCREEN ENTRY");
        ar_game_score_read(&gamescore_charts);
        view_scr_charts_game();
        BUZZER_PlayTones(tones_3beep);
        break;
    case AC_DISPLAY_BUTON_DOWN_RELEASED:
		SCREEN_TRAN(scr_circle_escape_handle, &scr_circle_escape);
        break;
    case AC_DISPLAY_BUTON_UP_RELEASED:
        APP_DBG_SIG("AC_DISPLAY_BUTTON_UP_LONG_PRESSED\n");
		// Reset score charts
		gamescore_charts.score_1st = 0;
		gamescore_charts.score_2nd = 0;
		gamescore_charts.score_3rd = 0;
		ar_game_score_write(&gamescore_charts);
		// BUZZER_PlaySound(BUZZER_SOUND_CLICK);
        break;
    case AC_DISPLAY_BUTON_MODE_RELEASED:
        /* code */
        break;
    default:
        break;
    }
}