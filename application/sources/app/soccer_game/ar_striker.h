#ifndef __AR_STRIKER_H__
#define __AR_STRIKER_H__

#include "scr_soccer.h"

/* Striker/shooter gameplay functions */
void ar_striker_move_position(int delta_x);  /* Move shooter left/right */
void ar_striker_shoot_ball();                /* Execute ball shot */

#endif // __AR_STRIKER_H__
