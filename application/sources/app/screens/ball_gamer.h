#ifndef __SCR_PEASHOOTER_H__
#define __SCR_PEASHOOTER_H__

//the project folder header files
#include "fsm.h"
#include "message.h"
#include "task_display.h"
#include "view_render.h"
#include "screens_bitmap.h"

//the system header files
#include <math.h>
#include <mathcalls.h>

struct Circle;
struct Ball;

extern view_dynamic_t dyn_view_peashooter;
extern view_screen_t game_ball_score;
extern void scr_peashooter_handle(ak_msg_t* msg);

#endif //__SCR_PEASHOOTER_H__
