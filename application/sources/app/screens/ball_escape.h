#ifndef __SCR_BALL_ESCAPE_H__
#define __SCR_BALL_ESCAPE_H__

#include "fsm.h"
#include "port.h"
#include "message.h"
#include "timer.h"

#include "sys_ctrl.h"
#include "sys_dbg.h"

#include "app.h"
#include "app_dbg.h"
#include "task_list.h"
#include "task_display.h"
#include "view_render.h"

#include "buzzer.h"

#include <math.h>
#include <vector>
#include <algorithm>
// #include <mathcalls.h>

// #include "screens.h"
#include "screens_bitmap.h"

//codes here
extern view_dynamic_t dyn_view_baller;
extern view_screen_t scr_circle_escape;
extern void arc(int x, int y, int stangle, int endangle, int radius); //arc(320,240,-i,270-i,200)
extern void scr_circle_escape_handle(ak_msg_t* msg);

#endif //__SCR_BALL_ESCAPE_H__