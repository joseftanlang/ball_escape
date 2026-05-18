#ifndef __AR_BALL_H__
#define __AR_BALL_H__

#include "../screens/scr_soccer.h"

/* Ball gameplay update functions */
void ar_ball_update_goalkeeper_game();  /* Update ball physics for goalkeeper role */
void ar_ball_update_shooter_game();     /* Update ball physics for shooter role */
void ar_ball_score_goal();              /* Handle goal scoring */
void ar_ball_lose_life();               /* Handle losing a life */


#endif // __AR_BALL_H__
