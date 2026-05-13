#ifndef __APP_EEPROM_H__
#define __APP_EEPROM_H__

#include <stdint.h>
#include "app.h"

/**
  *****************************************************************************
  * EEPROM define address.
  *
  *****************************************************************************
  */
#define EEPROM_START_ADDR           (0X0000)
#define EEPROM_END_ADDR             (0X1000)

#define EEPROM_SCORE_START_ADDR     (0X0010)
#define EEPROM_SCORE_PLAY_ADDR      (0X00FA)

#define EEPROM_SETTING_START_ADDR   (0X0100)

#define AR_GAME_EEPROM_MAGIC_NUMBER   ((uint32_t)0x123123BB)

/******************************************************************************/
/* Archey game */
/******************************************************************************/

/* setting data */
#define AR_GAME_SETTING_SILENT_OFF               (0)
#define AR_GAME_SETTING_SILENT_ON                (1)
#define AR_GAME_SETTING_NUM_ARROW_MIN            (1)
#define AR_GAME_SETTING_NUM_ARROW_MAX            (9)
#define AR_GAME_SETTING_NUM_ARROW_DEFAULT        (5)
#define AR_GAME_SETTING_ARROW_SPEED_DEFAULT      (5)
#define AR_GAME_SETTING_METEOROID_SPEED_MIN      (1)
#define AR_GAME_SETTING_METEOROID_SPEED_MAX      (5)
#define AR_GAME_SETTING_METEOROID_SPEED_DEFAULT  (1)

typedef struct {
  bool silent;
  uint8_t num_arrow;
  uint8_t arrow_speed;
  uint8_t meteoroid_speed;
  uint8_t day_night; // 0 = day, 1 = night (persisted)
} ar_game_setting_t;

/* score data */
typedef struct {
  uint32_t score_now;
  uint32_t score_1st;
  uint32_t score_2nd;
  uint32_t score_3rd;
} ar_game_score_t;

#ifdef __cplusplus
extern "C" {
#endif

extern ar_game_score_t gamescore;
extern bool ar_game_score_read(ar_game_score_t* data);
extern bool ar_game_score_write(ar_game_score_t* data);

extern ar_game_setting_t settingdata;
extern bool ar_game_setting_read(ar_game_setting_t* data);
extern bool ar_game_setting_write(ar_game_setting_t* data);

#ifdef __cplusplus
}
#endif

#endif //__APP_EEPROM_H__
