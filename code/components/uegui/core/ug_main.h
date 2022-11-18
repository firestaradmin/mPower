/**
 * @file ug_main.h
 * @author @firestaradmin
 * @brief 
 * @version 0.1
 * @date 2022-11-10
 * 
 * @copyright Copyright (c) 2022 LXG 
 * 
 */

#ifndef _UEGUI_H__
#define _UEGUI_H__

// #include "UVGUI_Animation.h"
// #include "UVGUI_Ascii_Code_Tab.h"
// #include "ug_user_config.h"
// #include "ug_type.h"
//#include "ug_port.h"
//#include "ug_disp.h"
#include "ug_refr.h"
// #include "ug_math.h"
// #include "ug_ll.h"
// #include "ug_draw_elements.h"
// #include "ug_task.h"
#include "ug_obj.h"
// #include "ug_area.h"
//#include "ug_tick.h"
#include "ug_font.h"


/* Init UVGUI Lib */
void ug_init(void);



extern ug_ll_t _ug_disp_ll;

extern ug_ll_t _ug_task_ll;

extern ug_task_t * _ug_task_act;





#endif