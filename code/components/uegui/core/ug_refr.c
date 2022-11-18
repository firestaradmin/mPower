#include "ug_refr.h"
#include "ug_mem.h"
#include "ug_math.h"
#include "ug_tick.h"
#include "ug_draw_rect.h"
#include "ug_log.h"
#include "ug_obj.h"
#include "ug_area.h"
#include "stdio.h"
/**********************
 *  STATIC VARIABLES
 **********************/
//static uint32_t px_num;       /* monitor for invalid areas */
static ug_disp_t * disp_refr;   /* disp will being refreshed */



/**********************
 *  STATIC PROTOTYPES
 **********************/
static void ug_refr_join_area(void);
static void ug_refr_areas(void);
static void ug_refr_area(const ug_area_t * area_p);
static void ug_refr_area_part(const ug_area_t * area_p);
static ug_obj_t * ug_refr_get_top_obj(const ug_area_t * area_p, ug_obj_t * obj);
static void ug_refr_obj_and_children(ug_obj_t * top_p, const ug_area_t * mask_p);
static void ug_refr_obj(ug_obj_t * obj, const ug_area_t * mask_ori_p);
static void ug_refr_vdb_flush(void);



/**
 * Called periodically to handle the refreshing
 * @param task pointer to the task itself
 */
void _ug_disp_refr_task(ug_task_t * task)
{

    uint32_t start = ug_tick_get();
    uint32_t elaps = 0;     /* elapsed time */

    disp_refr = task->user_data;

#if UG_USE_PERF_MONITOR == 0
    /* Ensure the task does not run again automatically.
     * This is done before refreshing in case refreshing invalidates something else.
     */
    ug_task_set_prio(task, UG_TASK_PRIO_OFF);
#endif

    /*Do nothing if there is no active screen*/
    if(disp_refr->act_scr == NULL) {
        disp_refr->inv_p = 0;
        return;
    }

    // 将需要重绘的区域合并，如果合并后区域更小的话
    ug_refr_join_area();

    ug_refr_areas();
    /*If refresh happened ...*/
    if(disp_refr->inv_p != 0) {

        /*Clean up*/
        _ug_memset_00(disp_refr->inv_areas, sizeof(disp_refr->inv_areas));
        _ug_memset_00(disp_refr->inv_area_joined, sizeof(disp_refr->inv_area_joined));
        disp_refr->inv_p = 0;

        elaps = ug_tick_elaps(start);   /* get elapsed time */

    }

}


/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Join the areas which has got common parts
 * 将需要重绘的区域合并，如果合并后区域更小的话
 * 
 */
static void ug_refr_join_area(void)
{
    uint32_t join_from;
    uint32_t join_in;
    ug_area_t joined_area;

    for(join_in = 0; join_in < disp_refr->inv_p; join_in++) {   //遍历每个inv区域
        /* if the area is already been joined , skip. */
        if(disp_refr->inv_area_joined[join_in] != 0) continue;

        /*Check all areas to join them in 'join_in'*/
        for(join_from = 0; join_from < disp_refr->inv_p; join_from++) {     //遍历每个inv区域
            /*Handle only unjoined areas and ignore itself*/
            if(disp_refr->inv_area_joined[join_from] != 0 || join_in == join_from) {
                continue;
            }

            /*Check if the areas are on each other*/
            if(_ug_area_is_on(&disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]) == false) {
                continue;
            }

            _ug_area_join(&joined_area, &disp_refr->inv_areas[join_in], &disp_refr->inv_areas[join_from]);

            /*Join two area only if the joined area size is smaller*/
            if(ug_area_get_size(&joined_area) < (ug_area_get_size(&disp_refr->inv_areas[join_in]) +
                                                 ug_area_get_size(&disp_refr->inv_areas[join_from]))) {
                ug_area_copy(&disp_refr->inv_areas[join_in], &joined_area);

                /*Mark 'join_form' is joined into 'join_in'*/
                disp_refr->inv_area_joined[join_from] = 1;
            }
        }
    }
}

/**
 * Refresh the joined areas
 * 重绘需要重绘的区域
 */
static void ug_refr_areas(void)
{
    if(disp_refr->inv_p == 0) return;

    /*Find the last area which will be drawn*/
    int32_t i;
    int32_t last_i = 0; 
    // find a area which not joined to last_i
    for(i = disp_refr->inv_p - 1; i >= 0; i--) {
        if(disp_refr->inv_area_joined[i] == 0) {
            last_i = i;
            break;
        }
    }

    // 清楚标志最后的区域正在被渲染的标志位
    disp_refr->driver.buffer->last_area = 0;
    disp_refr->driver.buffer->last_part = 0;

    for(i = 0; i < disp_refr->inv_p; i++) {
        /*Refresh the unjoined areas*/
        if(disp_refr->inv_area_joined[i] == 0) {

            if(i == last_i) disp_refr->driver.buffer->last_area = 1;
            disp_refr->driver.buffer->last_part = 0;
            ug_refr_area(&disp_refr->inv_areas[i]);

        }
    }
}

/**
 * Refresh an area if there is Virtual Display Buffer
 * @param area_p  pointer to an area to refresh
 */
static void ug_refr_area(const ug_area_t * area_p)
{
    /*True double buffering: there are two screen sized buffers. Just redraw directly into a * buffer */
    if(ug_disp_is_true_double_buf(disp_refr)) {
        ug_disp_buf_t * vdb = ug_disp_get_buf(disp_refr);
        vdb->area.x1        = 0;
        vdb->area.x2        = ug_disp_get_hor_res(disp_refr) - 1;
        vdb->area.y1        = 0;
        vdb->area.y2        = ug_disp_get_ver_res(disp_refr) - 1;
        disp_refr->driver.buffer->last_part = 1;
        ug_refr_area_part(area_p);
    }
    /*The buffer is smaller: refresh the area in parts： 部分刷新模式*/
    else {
        ug_disp_buf_t * vdb = ug_disp_get_buf(disp_refr);

        ug_coord_t w = ug_area_get_width(area_p);   // 绘制的宽度
        ug_coord_t h = ug_area_get_height(area_p);  // 绘制的高度

        /* Calculate the max y value */
        ug_coord_t y2 = area_p->y2 >= ug_disp_get_ver_res(disp_refr) ? ug_disp_get_ver_res(disp_refr) - 1 : area_p->y2;

        int32_t max_row = (uint32_t)vdb->size / w; // 一次最大能绘制的行数：VDB 能储存的行数
        if(max_row > h) max_row = h;    

        ug_coord_t row;
        ug_coord_t row_last = 0;
        /* 使用完整的VDB大小绘制 */
        for(row = area_p->y1; row + max_row - 1 <= y2; row += max_row) {
            /*Calc. the next y coordinates of VDB*/
            vdb->area.x1 = area_p->x1;
            vdb->area.x2 = area_p->x2;
            vdb->area.y1 = row;
            vdb->area.y2 = row + max_row - 1;
            if(vdb->area.y2 > y2) vdb->area.y2 = y2;
            row_last = vdb->area.y2;
            if(y2 == row_last) disp_refr->driver.buffer->last_part = 1;
            ug_refr_area_part(area_p);
        }

        /*If the last y coordinates are not handled yet ...*/
        /* 如果没绘制完，或者用不到完整的VDB，继续绘制剩余的部分 */
        if(y2 != row_last) {
            /*Calc. the next y coordinates of VDB*/
            vdb->area.x1 = area_p->x1;
            vdb->area.x2 = area_p->x2;
            vdb->area.y1 = row;
            vdb->area.y2 = y2;

            disp_refr->driver.buffer->last_part = 1;
            ug_refr_area_part(area_p);
        }
    }
}

/**
 * Refresh a part of an area which is on the actual Virtual Display Buffer
 * @param area_p pointer to an area to refresh
 */
static void ug_refr_area_part(const ug_area_t * area_p)
{
    ug_disp_buf_t * vdb = ug_disp_get_buf(disp_refr);

    /*In non double buffered mode, before rendering the next part wait until the previous image is
     * flushed*/
    if(ug_disp_is_double_buf(disp_refr) == false) {
        while(vdb->flushing) {
            if(disp_refr->driver.wait_cb) disp_refr->driver.wait_cb(&disp_refr->driver);
        }
    }

    ug_obj_t * top_act_scr = NULL;
    ug_obj_t * top_prev_scr = NULL;

    /* 计算当前被绘制的区域 to start_mask */
    ug_area_t start_mask;
    _ug_area_intersect(&start_mask, area_p, &vdb->area);

    /*Get the most top object which is not covered by others*/
    top_act_scr = ug_refr_get_top_obj(&start_mask, ug_disp_get_actscr(disp_refr));
    if(disp_refr->prev_scr) {
        top_prev_scr = ug_refr_get_top_obj(&start_mask, disp_refr->prev_scr);
    }

    /*Draw a display background if there is no top object*/
    if(top_act_scr == NULL && top_prev_scr == NULL) {
        if(disp_refr->bg_img) {
            //TODO: ug_draw_img(&a, &start_mask, disp_refr->bg_img, &dsc);
        } else {
            ug_draw_rect_dsc_t dsc;
            ug_draw_rect_dsc_init(&dsc);
            dsc.bg_color = disp_refr->bg_color;
            ug_draw_rect(&start_mask, &start_mask, &dsc);

        }
    }
    /*Refresh the previous screen if any*/
    if(disp_refr->prev_scr) {
        /*Get the most top object which is not covered by others*/
        if(top_prev_scr == NULL) {
            top_prev_scr = disp_refr->prev_scr;
        }
        /*Do the refreshing from the top object*/
        ug_refr_obj_and_children(top_prev_scr, &start_mask);

    }


    if(top_act_scr == NULL) {
         top_act_scr = disp_refr->act_scr;
     }
    /*Do the refreshing from the top object*/
    ug_refr_obj_and_children(top_act_scr, &start_mask);

    /* In true double buffered mode flush only once when all areas were rendered.
     * In normal mode flush after every area */
    if(ug_disp_is_true_double_buf(disp_refr) == false) {
        ug_refr_vdb_flush();
    }
}

/**
 * Search the most top object which fully covers an area
 * @param area_p pointer to an area
 * @param obj the first object to start the searching (typically a screen)
 * @return
 */
static ug_obj_t * ug_refr_get_top_obj(const ug_area_t * area_p, ug_obj_t * obj)
{
    ug_obj_t * found_p = NULL;

    /*If this object is fully cover the draw area check the children too */
    if(_ug_area_is_in(area_p, &obj->coords, 0) && obj->hidden == 0) {
        ug_design_res_t design_res = obj->design_cb ? obj->design_cb(obj, area_p, UG_DESIGN_COVER_CHK) : UG_DESIGN_RES_NOT_COVER;
        if(design_res == UG_DESIGN_RES_MASKED) return NULL;
                                                                                                                
        ug_obj_t * i;
        _UG_LL_READ(obj->child_ll, i) {
            found_p = ug_refr_get_top_obj(area_p, i);

            /*If a children is ok then break*/
            if(found_p != NULL) {
                break;
            }
        }

        /*If no better children use this object*/
        if(found_p == NULL) {
            if(design_res == UG_DESIGN_RES_COVER) {
                found_p = obj;
            }
        }
    }

    return found_p;
}

/**
 * Make the refreshing from an object. Draw all its children and the youngers too.
 * @param top_p pointer to an objects. Start the drawing from it.
 * @param mask_p pointer to an area, the objects will be drawn only here
 */
static void ug_refr_obj_and_children(ug_obj_t * top_p, const ug_area_t * mask_p)
{
    /* Normally always will be a top_obj (at least the screen)
     * but in special cases (e.g. if the screen has alpha) it won't.
     * In this case use the screen directly */
    if(top_p == NULL) top_p = ug_disp_get_actscr(disp_refr);
    if(top_p == NULL) return;  /*Shouldn't happen*/
    ug_obj_t * par;
    par = ug_obj_get_parent(top_p);

    if(top_p->drawbackground == 0){
        ug_refr_obj(par, mask_p);
    }
    /* 绘制自己和孩子们 */
    ug_refr_obj(top_p, mask_p);

    /* 绘制top_p 的兄弟姐妹， 因为他们可能会覆盖到top_p */
    ug_obj_t * border_p = top_p;


    /*Do until not reach the screen*/
    while(par != NULL) {
        /*object before border_p has to be redrawn*/
        ug_obj_t * i = _ug_ll_get_prev(&(par->child_ll), border_p);

        while(i != NULL) {
            /*Refresh the objects*/
            ug_refr_obj(i, mask_p);
            i = _ug_ll_get_prev(&(par->child_ll), i);
        }

        /*Call the post draw design function of the parents of the to object*/
        if(par->design_cb) par->design_cb(par, mask_p, UG_DESIGN_DRAW_POST);

        /*The new border will be there last parents,
         *so the 'younger' brothers of parent will be refreshed*/
        border_p = par;
        /*Go a level deeper*/
        par = ug_obj_get_parent(par);
    }
}

/**
 * Refresh an object and all of its children. (Called recursively)
 * @param obj pointer to an object to refresh
 * @param mask_ori_p pointer to an area, the objects will be drawn only here
 */
static void ug_refr_obj(ug_obj_t * obj, const ug_area_t * mask_ori_p)
{
    /*Do not refresh hidden objects*/
    if(obj->hidden != 0) return;

    bool union_ok; /* Store the return value of area_union */
    /* Truncate the original mask to the coordinates of the parent
     * because the parent and its children are visible only here */
    ug_area_t obj_mask;
    ug_area_t obj_ext_mask;
    ug_area_t obj_area;
    ug_coord_t ext_size = obj->ext_draw_pad;
    ug_obj_get_coords(obj, &obj_area);
    obj_area.x1 -= ext_size;
    obj_area.y1 -= ext_size;
    obj_area.x2 += ext_size;
    obj_area.y2 += ext_size;
    union_ok = _ug_area_intersect(&obj_ext_mask, mask_ori_p, &obj_area);

    /*Draw the parent and its children only if they ore on 'mask_parent'*/
    if(union_ok == true) {

        /* Redraw the object */
        if(obj->design_cb) obj->design_cb(obj, &obj_ext_mask, UG_DESIGN_DRAW_MAIN);

        /*Create a new 'obj_mask' without 'ext_size' because the children can't be visible there*/
        ug_obj_get_coords(obj, &obj_area);
        union_ok = _ug_area_intersect(&obj_mask, mask_ori_p, &obj_area);
        if(union_ok == true) {
            ug_area_t mask_child; /*Mask from obj and its child*/
            ug_obj_t * child_p;
            ug_area_t child_area;
            _UG_LL_READ_BACK(obj->child_ll, child_p) {
                ug_obj_get_coords(child_p, &child_area);
                ext_size = child_p->ext_draw_pad;
                child_area.x1 -= ext_size;
                child_area.y1 -= ext_size;
                child_area.x2 += ext_size;
                child_area.y2 += ext_size;
                /* Get the union (common parts) of original mask (from obj)
                 * and its child */
                union_ok = _ug_area_intersect(&mask_child, &obj_mask, &child_area);

                /*If the parent and the child has common area then refresh the child */
                if(union_ok) {
                    /*Refresh the next children*/
                    ug_refr_obj(child_p, &mask_child);
                }
            }
        }

        /* If all the children are redrawn make 'post draw' design */
        if(obj->design_cb) obj->design_cb(obj, &obj_ext_mask, UG_DESIGN_DRAW_POST);
    }
}

/**
 * Flush the content of the VDB
 */
static void ug_refr_vdb_flush(void)
{
    ug_disp_buf_t * vdb = ug_disp_get_buf(disp_refr);

    /*In double buffered mode wait until the other buffer is flushed before flushing the current
     * one*/
    if(ug_disp_is_double_buf(disp_refr)) {
        while(vdb->flushing) {
            if(disp_refr->driver.wait_cb) disp_refr->driver.wait_cb(&disp_refr->driver);
        }
    }

    vdb->flushing = 1;

    if(disp_refr->driver.buffer->last_area && disp_refr->driver.buffer->last_part) vdb->flushing_last = 1;
    else vdb->flushing_last = 0;

    /*Flush the rendered content to the display*/
    ug_disp_t * disp = disp_refr;
    if(disp->driver.flush_screen) disp->driver.flush_screen(&disp->driver, &vdb->area, vdb->buf_act);

    if(vdb->buf1 && vdb->buf2) {
        if(vdb->buf_act == vdb->buf1)
            vdb->buf_act = vdb->buf2;
        else
            vdb->buf_act = vdb->buf1;
    }
}



/**
 * Invalidate an area on display to redraw it
 * @param area_p pointer to area which should be invalidated (NULL: delete the invalidated areas)
 * @param disp pointer to display where the area should be invalidated (NULL can be used if there is
 * only one display)
 */
void _ug_inv_area(ug_disp_t * disp, const ug_area_t * area_p)
{
    if(!disp) disp = _ug_refr_get_refrdisp();
    if(!disp) return;

    /*Clear the invalidate buffer if the parameter is NULL*/
    if(area_p == NULL) {
        disp->inv_p = 0;
        return;
    }

    ug_area_t scr_area;
    scr_area.x1 = 0;
    scr_area.y1 = 0;
    scr_area.x2 = ug_disp_get_hor_res(disp) - 1;
    scr_area.y2 = ug_disp_get_ver_res(disp) - 1;

    ug_area_t com_area;
    bool suc;

    suc = _ug_area_intersect(&com_area, area_p, &scr_area);

    /*The area is truncated to the screen*/
    if(suc != false) {
        //if(disp->driver.rounder_cb) disp->driver.rounder_cb(&disp->driver, &com_area);

        /*Save only if this area is not in one of the saved areas*/
        uint16_t i;
        for(i = 0; i < disp->inv_p; i++) {
            if(_ug_area_is_in(&com_area, &disp->inv_areas[i], 0) != false) return;
        }

        /*Save the area*/
        if(disp->inv_p < UG_INV_BUF_SIZE) {
            ug_area_copy(&disp->inv_areas[disp->inv_p], &com_area);
        }
        else {   /*If no place for the area add the screen*/
            disp->inv_p = 0;
            ug_area_copy(&disp->inv_areas[disp->inv_p], &scr_area);
        }
        disp->inv_p++;
        ug_task_set_prio(disp->refr_task, UG_REFR_TASK_PRIO);

    }
}


ug_disp_t * _ug_refr_get_refrdisp(void)
{
    return disp_refr;
}

























#if 0
/* old way */

static ug_area_t * redrawn_area;

/* 刷新区域是否被更改标志 */
static uint8_t ug_redrawn_area_changed_flag;


/* screen refresh task, need to run in while */
void _ug_refr_task(void * task)
{
	
    ug_disp_t * disp = ((ug_task_t *)task)->user_data;
    uint32_t tick_now = ug_tick_get();
	_ug_refr_screen(disp);
	disp->last_activity_time = tick_now;

}


void _ug_refr_screen(ug_disp_t *disp)
{
    /* updata disp_buf */
    redrawn_area = _ug_disp_get_actdisp_redrawn_area();
    _ug_memset_00(redrawn_area, sizeof(ug_area_t));
    ug_redrawn_area_changed_flag = 0;

    ug_obj_t *act_scr = disp->act_scr;
    if(act_scr == NULL) return ;

    _ug_refr_draw_obj(act_scr);

    redrawn_area->x1 = 0;
    redrawn_area->x2 = disp->driver.hor_res - 1;
    disp->driver.flush_screen(  &disp->driver, 
                                redrawn_area, 
                                disp->driver.buffer->buf + (disp->driver.hor_res * redrawn_area->y1));
    

}

void _ug_refr_draw_obj(ug_obj_t * obj)
{
    
    if(obj->haschanged || _ug_refr_check_children_changed(obj)){

        _ug_refr_draw_obj_on_scr(obj);

        /* _ug_refr_updata_redrawn_area */
        if(ug_redrawn_area_changed_flag == 0){
            _ug_memcpy(redrawn_area, &obj->coords, sizeof(ug_area_t));
            ug_redrawn_area_changed_flag = 1;
        }
        else {
            redrawn_area->x1 = UG_MATH_MIN(redrawn_area->x1, obj->coords.x1);
            redrawn_area->x2 = UG_MATH_MAX(redrawn_area->x2, obj->coords.x2);
            redrawn_area->y1 = UG_MATH_MIN(redrawn_area->y1, obj->coords.y1);
            redrawn_area->x1 = UG_MATH_MAX(redrawn_area->y2, obj->coords.y2);
        }

        obj->haschanged = false;
    }

    ug_ll_node_t *obj_child_next = _ug_ll_get_head(&((ug_obj_t *)obj)->child_ll);
    if(obj_child_next == NULL) return;

    while(obj_child_next){
        _ug_refr_draw_obj((ug_obj_t *)obj_child_next);
        obj_child_next = _ug_ll_get_next(&((ug_obj_t *)obj)->child_ll, obj_child_next);
    }

}


uint8_t _ug_refr_check_children_changed(ug_obj_t * obj)
{
    ug_ll_node_t *nextchild = _ug_ll_get_head(&obj->child_ll);
    if(nextchild == NULL) return UG_OBJ_NO_CHANGED;

    while(nextchild){
        if(((ug_obj_t *)nextchild)->haschanged)
            return UG_OBJ_HAS_CHANGED;

        nextchild = _ug_ll_get_next(&obj->child_ll, nextchild);
    }

    return UG_OBJ_NO_CHANGED;
}




void _ug_refr_draw_obj_on_scr(ug_obj_t *obj)
{
    const ug_area_t *coords = &obj->coords;
    //ug_disp_t *disp = ug_disp_get_actdisp();

    ug_fillRectangle(coords->x1, coords->y1, coords->x2, coords->y2, obj->bg_color);

}



#endif


