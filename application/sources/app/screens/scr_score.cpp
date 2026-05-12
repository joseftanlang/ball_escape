#include "scr_score.h"

static void view_scr_setting();

static ar_game_score_t game_score;

view_dynamic_t dyn_view_item_charts_game = {
    {
        .item_type = ITEM_TYPE_DYNAMIC,
    },
    view_scr_setting
};

view_screen_t scr_charts_game = {
    &dyn_view_item_charts_game,
    ITEM_NULL,
    ITEM_NULL,
    .focus_item = 0,
};

//top 3 game score
void view_scr_setting() {
    view_render.clear();
    view_render.setTextSize(1);
    view_render.setTextColor(WHITE);
    view_render.setCursor(0, 0);
    view_render.print("Game Score");
    view_render.setCursor(0, 20);
    view_render.printf("Sun: %d", game_ball_score.sun);
    view_render.setCursor(0, 40);
    view_render.printf("Level: %d", game_ball_score.level);
}

void game_score_update() {
    game_ball_score = ar_game_get_score();
    view_scr_setting();
}

void scr_charts_game_handle(ak_msg_t* msg) {
    switch (msg->sig) {
    case SCREEN_ENTRY:
        game_score = ar_game_get_score();
        break;
    case AC_DISPLAY_BUTON_MODE_RELEASED: {
        SCREEN_TRAN(scr_idle_handle, &scr_idle);
    }
        break;
    case AC_DISPLAY_BUTON_DOWN_RELEASED: {
        SCREEN_TRAN(scr_peashooter_handle, &scr_peashooter);
        BUZZER_PlayTones(tones_startup);
    }
        break;
    case AC_DISPLAY_BUTON_UP_RELEASED: {
        SCREEN_TRAN(scr_startup_handle, &scr_startup);
        BUZZER_PlayTones(tones_startup);
    }
        break;
    default:
        break;
    }
}