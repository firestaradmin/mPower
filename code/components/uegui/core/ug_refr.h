#ifndef __UG_REFR_H__
#define __UG_REFR_H__
#include "ug_obj.h"
#include "ug_task.h"
#include "ug_disp.h"
#include "ug_area.h"


#define UG_REFR_TASK_PRIO UG_TASK_PRIO_MID



enum {
    UG_OBJ_NO_CHANGED = 0,
    UG_OBJ_HAS_CHANGED
};


/**
 * Called periodically to handle the refreshing
 * @param task pointer to the task itself
 */
void _ug_disp_refr_task(ug_task_t * task);

void _ug_refr_screen(ug_disp_t *disp);
void _ug_refr_draw_obj(ug_obj_t * obj);
uint8_t _ug_refr_check_children_changed(ug_obj_t * obj);
void _ug_refr_draw_obj_on_scr(ug_obj_t *obj);

void _ug_inv_area(ug_disp_t * disp, const ug_area_t * area_p);

ug_disp_t * _ug_refr_get_refrdisp(void);

#endif //!__UG_REFR_H__




