#include "ug_main.h"
#include "ug_obj.h"
#include "ug_mem.h"
#include "ug_draw_rect.h"
#include "ug_type.h"
#include "ug_area.h"
#include "ug_ll.h"
#include "ug_refr.h"

#include "stdio.h"
/*********************
 *      DEFINES
 *********************/
#define UG_OBJX_NAME "ug_obj"
#define UG_OBJ_DEF_WIDTH    (UG_DPX(100))
#define UG_OBJ_DEF_HEIGHT   (UG_DPX(50))
/**********************
 *      TYPEDEFS
 **********************/
typedef struct _ug_event_temp_data {
    ug_obj_t * obj;
    bool deleted;
    struct _ug_event_temp_data * prev;
} ug_event_temp_data_t;

/**********************
 *  STATIC VARIABLES
 **********************/
static ug_event_temp_data_t * event_temp_data_head;
static const void * event_act_data;


/**********************
 *  STATIC PROTOTYPES
 **********************/

/* obj core func. */
static ug_design_res_t  ug_obj_design(ug_obj_t * obj, const ug_area_t * clip_area, ug_design_mode_t mode);
static ug_res_t         ug_obj_signal(ug_obj_t * obj, ug_signal_t sign, void * param);


/* obj delete */
static void ug_event_mark_deleted(ug_obj_t * obj);
static void obj_del_core(ug_obj_t * obj);
//static bool obj_valid_child(const ug_obj_t * parent, const ug_obj_t * obj_to_find);
static void ug_obj_del_async_cb(void * obj);


/* obj align */
static void obj_align_core(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align, bool x_set, bool y_set, ug_coord_t x_ofs, ug_coord_t y_ofs);
static void obj_align_origo_core(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align,  bool x_set, bool y_set, ug_coord_t x_ofs, ug_coord_t y_ofs);


static void refresh_children_position(ug_obj_t * obj, ug_coord_t x_diff, ug_coord_t y_diff);



/**
 * Create a basic object
 * @param parent pointer to a parent object.
 *                  If NULL then a screen will be created
 * @param copy pointer to a base object, if not NULL then the new object will be copied from it
 * @return pointer to the new object
 */
ug_obj_t * ug_obj_create(ug_obj_t * parent, const ug_obj_t * copy, char *name)
{
    ug_obj_t * new_obj = NULL;
	
    /*Create a screen*/
    if(parent == NULL) {
        //UG_LOG_TRACE("Screen create started");
        ug_disp_t * disp = ug_disp_get_actdisp();
        if(!disp) {
            //UG_LOG_WARN("ug_obj_create: not display created to so far. No place to assign the new screen");
            return NULL;
        }

        new_obj = _ug_ll_ins_head(&disp->scr_ll);
        //UG_ASSERT_MEM(new_obj);
        if(new_obj == NULL) return NULL;

        if(disp->act_scr == NULL){
            disp->act_scr = new_obj;
        }

        _ug_memset_00(new_obj, sizeof(ug_obj_t));

        /*Set the callbacks*/
        new_obj->signal_cb = ug_obj_signal;
        new_obj->design_cb = ug_obj_design;
        new_obj->event_cb = NULL;

        /*Set coordinates to full screen size*/
        new_obj->coords.x1    = 0;
        new_obj->coords.y1    = 0;
        new_obj->coords.x2    = ug_disp_get_hor_res(NULL) - 1;
        new_obj->coords.y2    = ug_disp_get_ver_res(NULL) - 1;

        new_obj->bg_color.full    = UG_OBJ_DEFAULT_BG_COLOR;
        new_obj->drawbackground = 1;
    }
    /*Create a normal object*/
    else {

        new_obj = _ug_ll_ins_head(&parent->child_ll);
        if(new_obj == NULL) return NULL;

        _ug_memset_00(new_obj, sizeof(ug_obj_t));

        new_obj->parent = parent;

        /*Set the callbacks (signal:cb is required in `ug_obj_get_base_dir` if `UG_USE_ASSERT_OBJ` is enabled)*/
        new_obj->signal_cb = ug_obj_signal;
        new_obj->design_cb = ug_obj_design;
        new_obj->event_cb = NULL;

        new_obj->coords.y1    = parent->coords.y1;
        new_obj->coords.y2    = parent->coords.y1 + UG_OBJ_DEF_HEIGHT;

        new_obj->bg_color.full     = UG_OBJ_DEFAULT_BG_COLOR;
        new_obj->bd_color.full     = UG_OBJ_DEFAULT_BD_COLOR;
        new_obj->drawbackground = 1;
        new_obj->bd_width = 2;
    }


    _ug_ll_init(&(new_obj->child_ll), sizeof(ug_obj_t));

    new_obj->ext_draw_pad = 0;

    /*Init realign*/
#if UG_USE_OBJ_REALIGN
    new_obj->realign.align        = UG_ALIGN_CENTER;
    new_obj->realign.xofs         = 0;
    new_obj->realign.yofs         = 0;
    new_obj->realign.base         = NULL;
    new_obj->realign.auto_realign = 0;
    new_obj->realign.origo_align  = 0;
#endif

    /*Init. user date*/
#if UG_USE_USER_DATA
    _ug_memset_00(&new_obj->user_data, sizeof(ug_obj_user_data_t));
#endif

    /*Set attributes*/
    new_obj->adv_hittest  = 0;
    new_obj->click        = 1;
    new_obj->drag         = 0;
    new_obj->drag_throw   = 0;
    new_obj->drag_parent  = 0;
    //new_obj->drag_dir     = UG_DRAG_DIR_BOTH;
    new_obj->hidden       = 0;
    new_obj->top          = 0;
    new_obj->protect      = UG_PROTECT_NONE;
    new_obj->parent_event = 0;
    new_obj->gesture_parent = parent ? 1 : 0;
    new_obj->focus_parent  = 0;
    new_obj->state = UG_STATE_DEFAULT;

    new_obj->ext_attr = NULL;


    /*Copy the attributes if required*/
    if(copy != NULL) {
        ug_area_copy(&new_obj->coords, &copy->coords);
        new_obj->ext_draw_pad = copy->ext_draw_pad;

        /*Set user data*/
#if UG_USE_USER_DATA
        _ug_memcpy(&new_obj->user_data, &copy->user_data, sizeof(ug_obj_user_data_t));
#endif

        /*Copy realign*/
#if UG_USE_OBJ_REALIGN
        new_obj->realign.align        = copy->realign.align;
        new_obj->realign.xofs         = copy->realign.xofs;
        new_obj->realign.yofs         = copy->realign.yofs;
        new_obj->realign.base         = copy->realign.base;
        new_obj->realign.auto_realign = copy->realign.auto_realign;
#endif

        /*Only copy the `event_cb`. `signal_cb` and `design_cb` will be copied in the derived
         * object type (e.g. `ug_btn`)*/
        new_obj->event_cb = copy->event_cb;

        /*Copy attributes*/
        new_obj->adv_hittest  = copy->adv_hittest;
        new_obj->click        = copy->click;
        new_obj->drag         = copy->drag;
        //new_obj->drag_dir     = copy->drag_dir;
        new_obj->drag_throw   = copy->drag_throw;
        new_obj->drag_parent  = copy->drag_parent;
        new_obj->hidden       = copy->hidden;
        new_obj->top          = copy->top;
        new_obj->parent_event = copy->parent_event;

        new_obj->protect      = copy->protect;
        new_obj->gesture_parent = copy->gesture_parent;

        new_obj->bg_color.full = copy->bg_color.full;
        new_obj->bd_color.full = copy->bd_color.full;
        new_obj->drawbackground = copy->drawbackground;
        new_obj->bd_width = copy->bd_width;

    }

    /*Send a signal to the parent to notify it about the new child*/
    if(parent != NULL) {
        parent->signal_cb(parent, UG_SIGNAL_CHILD_CHG, new_obj);

        /* Invalidate the area */
        ug_obj_invalidate(new_obj);

    }



    //UG_LOG_INFO("Object create ready");
	new_obj->name = name;
    return new_obj;
}


/**
 * Delete 'obj' and all of its children
 * @param obj pointer to an object to delete
 * @return UG_RES_INV because the object is deleted
 */
ug_res_t ug_obj_del(ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);
    ug_obj_invalidate(obj);

    ug_disp_t * disp = NULL;
    bool act_scr_del = false;
    ug_obj_t * par = ug_obj_get_parent(obj);
    if(par == NULL) {
        disp = ug_obj_get_disp(obj);
        if(!disp) return UG_RES_INV;   /*Shouldn't happen*/
        if(disp->act_scr == obj) act_scr_del = true;
    }


    obj_del_core(obj);

    /*Send a signal to the parent to notify it about the child delete*/
    if(par) {
        par->signal_cb(par, UG_SIGNAL_CHILD_CHG, NULL);
    }

    /*Handle if the active screen was deleted*/
    if(act_scr_del)  {
        disp->act_scr = NULL;
    }

    return UG_RES_INV;
}





/**
 * Send an event to the object
 * @param obj pointer to an object
 * @param event the type of the event from `ug_event_t`
 * @param data arbitrary data depending on the object type and the event. (Usually `NULL`)
 * @return UG_RES_OK: `obj` was not deleted in the event; UG_RES_INV: `obj` was deleted in the event
 */
ug_res_t ug_event_send(ug_obj_t * obj, ug_event_t event, const void * data)
{
    if(obj == NULL) return UG_RES_OK;

    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_res_t res;
    res = ug_event_send_func(obj->event_cb, obj, event, data);
    return res;
}


/**
 * Get the `data` parameter of the current event
 * @return the `data` parameter
 */
const void * ug_event_get_data(void)
{
    return event_act_data;
}
/**
 * Call an event function with an object, event, and data.
 * @param event_xcb an event callback function. If `NULL` `UG_RES_OK` will return without any actions.
 *        (the 'x' in the argument name indicates that its not a fully generic function because it not follows
 *         the `func_name(object, callback, ...)` convention)
 * @param obj pointer to an object to associate with the event (can be `NULL` to simply call the `event_cb`)
 * @param event an event
 * @param data pointer to a custom data
 * @return UG_RES_OK: `obj` was not deleted in the event; UG_RES_INV: `obj` was deleted in the event
 */
ug_res_t ug_event_send_func(ug_event_cb_t event_xcb, ug_obj_t * obj, ug_event_t event, const void * data)
{
    if(obj != NULL) {
        //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);
    }

    /* Build a simple linked list from the objects used in the events
     * It's important to know if an this object was deleted by a nested event
     * called from this `even_cb`. */
    ug_event_temp_data_t event_temp_data;
    event_temp_data.obj     = obj;
    event_temp_data.deleted = false;
    event_temp_data.prev    = NULL;

    if(event_temp_data_head) {
        event_temp_data.prev = event_temp_data_head;
    }
    event_temp_data_head = &event_temp_data;

    const void * event_act_data_save = event_act_data;
    event_act_data                   = data;

    /*Call the input device's feedback callback if set*/
    // ug_indev_t * indev_act = ug_indev_get_act();
    // if(indev_act) {
    //     if(indev_act->driver.feedback_cb) indev_act->driver.feedback_cb(&indev_act->driver, event);
    // }

    /*Call the event callback itself*/
    if(event_xcb) event_xcb(obj, event);

    /*Restore the event data*/
    event_act_data = event_act_data_save;

    /*Remove this element from the list*/
    event_temp_data_head = event_temp_data_head->prev;

    if(event_temp_data.deleted) {
        return UG_RES_INV;
    }

    if(obj) {
        if(obj->parent_event && obj->parent) {
            ug_res_t res = ug_event_send(obj->parent, event, data);
            if(res != UG_RES_OK) {
                return UG_RES_INV;
            }
        }
    }

    return UG_RES_OK;
}




/**
 * Send an event to the object
 * @param obj pointer to an object
 * @param event the type of the event from `ug_event_t`.
 * @return UG_RES_OK or UG_RES_INV
 */
ug_res_t ug_signal_send(ug_obj_t * obj, ug_signal_t signal, void * param)
{
    if(obj == NULL) return UG_RES_OK;

    ug_res_t res = UG_RES_OK;
    if(obj->signal_cb) res = obj->signal_cb(obj, signal, param);

    return res;
}




/**
 * Mark an area of an object as invalid.
 * This area will be redrawn by 'ug_refr_task'
 * @param obj pointer to an object
 * @param area the area to redraw
 */
void ug_obj_invalidate_area(const ug_obj_t * obj, const ug_area_t * area)
{

    if(ug_obj_get_hidden(obj)) return;

    /*Invalidate the object only if it belongs to the curent or previous'*/
    ug_obj_t * obj_scr = ug_obj_get_screen(obj);
    ug_disp_t * disp   = ug_obj_get_disp(obj_scr);

    if(obj_scr == ug_disp_get_actscr(disp)){
        /*Truncate the area to the object*/
        ug_area_t obj_coords;
        ug_coord_t ext_size = obj->ext_draw_pad;
        ug_area_copy(&obj_coords, &obj->coords);
        obj_coords.x1 -= ext_size;
        obj_coords.y1 -= ext_size;
        obj_coords.x2 += ext_size;
        obj_coords.y2 += ext_size;

        bool is_common;
        ug_area_t area_trunc;

        is_common = _ug_area_intersect(&area_trunc, area, &obj_coords);
        if(is_common == false) return;  /*The area is not on the object*/

        /* 检查obj 是不是在 par obj 内部 */
        ug_obj_t * par = ug_obj_get_parent(obj);
        while(par != NULL) {
            is_common = _ug_area_intersect(&area_trunc, &area_trunc, &par->coords);
            if(is_common == false) break;       /*If no common parts with parent break;*/
            if(ug_obj_get_hidden(par)) return; /*If the parent is hidden then the child is hidden and won't be drawn*/

            par = ug_obj_get_parent(par);
        }

        if(is_common) _ug_inv_area(disp, &area_trunc);
    }
}

/**
 * Mark the object as invalid therefore its current position will be redrawn by 'ug_refr_task'
 * @param obj pointer to an object
 */
void ug_obj_invalidate(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    /*Truncate the area to the object*/
    ug_area_t obj_coords;
    ug_coord_t ext_size = obj->ext_draw_pad;
    ug_area_copy(&obj_coords, &obj->coords);
    obj_coords.x1 -= ext_size;
    obj_coords.y1 -= ext_size;
    obj_coords.x2 += ext_size;
    obj_coords.y2 += ext_size;


    ug_obj_invalidate_area(obj, &obj_coords);

}



/*=======================
 * Getter functions
 *======================*/


/*-----------------
 * Attribute get
 *----------------*/
/**
 * Copy the coordinates of an object to an area
 * @param obj pointer to an object
 * @param cords_p pointer to an area to store the coordinates
 */
void ug_obj_get_coords(const ug_obj_t * obj, ug_area_t * cords_p)
{
    //ug_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_area_copy(cords_p, &obj->coords);
}



/**
 * Get the x coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the left side of its parent
 */
ug_coord_t ug_obj_get_x(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_coord_t rel_x;
    ug_obj_t * parent = ug_obj_get_parent(obj);
    if(parent) {
        rel_x             = obj->coords.x1 - parent->coords.x1;
    }
    else {
        rel_x = obj->coords.x1;
    }
    return rel_x;
}

/**
 * Get the y coordinate of object
 * @param obj pointer to an object
 * @return distance of 'obj' from the top of its parent
 */
ug_coord_t ug_obj_get_y(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_coord_t rel_y;
    ug_obj_t * parent = ug_obj_get_parent(obj);
    if(parent) {
        rel_y             = obj->coords.y1 - parent->coords.y1;
    }
    else {
        rel_y = obj->coords.y1;
    }
    return rel_y;
}

/**
 * Get the width of an object
 * @param obj pointer to an object
 * @return the width
 */
ug_coord_t ug_obj_get_width(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return ug_area_get_width(&obj->coords);
}

/**
 * Get the height of an object
 * @param obj pointer to an object
 * @return the height
 */
ug_coord_t ug_obj_get_height(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return ug_area_get_height(&obj->coords);
}

/**
 * Get the hidden attribute of an object
 * @param obj pointer to an object
 * @return true: the object is hidden
 */
bool ug_obj_get_hidden(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->hidden == 0 ? false : true;
}

/**
 * Return with the screen of an object
 * @param obj pointer to an object
 * @return pointer to a screen
 */
ug_obj_t * ug_obj_get_screen(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    const ug_obj_t * par = obj;
    const ug_obj_t * act_p;

    do {
        act_p = par;
        par   = ug_obj_get_parent(act_p);
    } while(par != NULL);

    return (ug_obj_t *)act_p;
}

/**
 * Get the display of an object
 * @param scr pointer to an object
 * @return pointer the object's display
 */
ug_disp_t * ug_obj_get_disp(const ug_obj_t * obj)
{
    //ug_ASSERT_OBJ(obj, UG_OBJX_NAME);

    const ug_obj_t * scr;

    if(obj->parent == NULL)
        scr = obj; /*`obj` is a screen*/
    else
        scr = ug_obj_get_screen(obj); /*get the screen of `obj`*/

    ug_disp_t * d;

    _UG_LL_READ(_ug_disp_ll, d) {
        ug_obj_t * s;
        _UG_LL_READ(d->scr_ll, s) {
            if(s == scr) return d;
        }
    }

    //ug_LOG_WARN("ug_scr_get_disp: screen not found")
    return NULL;
}

/*---------------------
 * Parent/children get
 *--------------------*/

/**
 * Returns with the parent of an object
 * @param obj pointer to an object
 * @return pointer to the parent of  'obj'
 */
ug_obj_t * ug_obj_get_parent(const ug_obj_t * obj)
{
    //ug_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->parent;
}

/**
 * Iterate through the children of an object (start from the "youngest")
 * @param obj pointer to an object
 * @param child NULL at first call to get the next children
 *                  and the previous return value later
 * @return the child after 'act_child' or NULL if no more child
 */
ug_obj_t * ug_obj_get_child(const ug_obj_t * obj, const ug_obj_t * child)
{
    //ug_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_obj_t * result = NULL;

    if(child == NULL) {
        result = _ug_ll_get_head(&obj->child_ll);
    }
    else {
        result = _ug_ll_get_next(&obj->child_ll, child);
    }

    return result;
}

/**
 * Get the really focused object by taking `focus_parent` into account.
 * @param obj the start object
 * @return the object to really focus
 */
ug_obj_t * ug_obj_get_focused_obj(const ug_obj_t * obj)
{
    // if(obj == NULL) return NULL;
    // const ug_obj_t * focus_obj = obj;
    // while(ug_obj_get_focus_parent(focus_obj) != false && focus_obj != NULL) {
    //     focus_obj = ug_obj_get_parent(focus_obj);
    // }

    // return (ug_obj_t *)focus_obj;
    return NULL;
}

/**
 * Get the protect field of an object
 * @param obj pointer to an object
 * @return protect field ('OR'ed values of `ug_protect_t`)
 */
uint8_t ug_obj_get_protect(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->protect;
}

/**
 * Check at least one bit of a given protect bitfield is set
 * @param obj pointer to an object
 * @param prot protect bits to test ('OR'ed values of `ug_protect_t`)
 * @return false: none of the given bits are set, true: at least one bit is set
 */
bool ug_obj_is_protected(const ug_obj_t * obj, uint8_t prot)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return (obj->protect & prot) == 0 ? false : true;
}

ug_state_t ug_obj_get_state(const ug_obj_t * obj, uint8_t part)
{
    // UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    // if(part < _UG_OBJ_PART_REAL_LAST) return ((ug_obj_t *)obj)->state;

    // /*If a real part is asked, then use the object's signal to get its state.
    //  * A real object can be in different state then the main part
    //  * and only the object itself knows who to get it's state. */
    // ug_get_state_info_t info;
    // info.part = part;
    // info.result = UG_STATE_DEFAULT;
    // ug_signal_send((ug_obj_t *)obj, UG_SIGNAL_GET_STATE_DSC, &info);

    // return info.result;
    return 0;

}

/**
 * Get the signal function of an object
 * @param obj pointer to an object
 * @return the signal function
 */
ug_signal_cb_t ug_obj_get_signal_cb(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->signal_cb;
}

/**
 * Get the design function of an object
 * @param obj pointer to an object
 * @return the design function
 */
ug_design_cb_t ug_obj_get_design_cb(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->design_cb;
}

/**
 * Get the event function of an object
 * @param obj pointer to an object
 * @return the event function
 */
ug_event_cb_t ug_obj_get_event_cb(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->event_cb;
}

/*------------------
 * Other get
 *-----------------*/

/**
 * Get the ext pointer
 * @param obj pointer to an object
 * @return the ext pointer but not the dynamic version
 *         Use it as ext->data1, and NOT da(ext)->data1
 */
void * ug_obj_get_ext_attr(const ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    return obj->ext_attr;
}


/**********************
 *   SET FUNCTIONS
 **********************/

/**
 * Set the a signal function of an object. Used internally by the library.
 * Always call the previous signal function in the new.
 * @param obj pointer to an object
 * @param cb the new signal function
 */
void ug_obj_set_signal_cb(ug_obj_t * obj, ug_signal_cb_t signal_cb)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    obj->signal_cb = signal_cb;
}
/**
 * Set a new design function for an object
 * @param obj pointer to an object
 * @param design_cb the new design function
 */
void ug_obj_set_design_cb(ug_obj_t * obj, ug_design_cb_t design_cb)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    obj->design_cb = design_cb;
}

/**
 * Set a an event handler function for an object.
 * Used by the user to react on event which happens with the object.
 * @param obj pointer to an object
 * @param event_cb the new event function
 */
void ug_obj_set_event_cb(ug_obj_t * obj, ug_event_cb_t event_cb)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    obj->event_cb = event_cb;
}


void ug_obj_set_color(ug_obj_t *obj, ug_color_t color)
{
	if(obj == NULL) return;
	if(obj->bg_color.full == color.full) return;
	
	obj->bg_color.full = color.full;
    /*Invalidate the new area*/
    ug_obj_invalidate(obj);

}



/*--------------------
 * Coordinate set
 * ------------------*/

void ug_obj_set_coords(ug_obj_t *obj, ug_area_t *area)
{
    /*Invalidate the original area*/
    ug_obj_invalidate(obj);


	if(obj == NULL) return;
	
	_ug_memcpy(&obj->coords, area, sizeof(ug_area_t));
	/*Invalidate the new area*/
    ug_obj_invalidate(obj);
}

void ug_obj_move(ug_obj_t *obj, int16_t dx, int16_t dy)
{
	ug_disp_t *disp = _ug_refr_get_refrdisp();
	ug_area_t area ;
	_ug_memcpy(&area, &obj->coords, sizeof(ug_area_t));
	ug_coord_t width, height;
	width = area.x2 - area.x1;
	height = area.y2 - area.y1;
	
	if(dx < 0){
		dx *= -1 ;
		if(area.x1 < dx){
			area.x1 = 0;
		}
		else{
			area.x1 -= dx;
		}
		area.x2 = area.x1 + width;
	}
	else{
		if((disp->driver.hor_res - area.x2 -1) < dx){
			area.x2 = disp->driver.hor_res - 1;
		}
		else{
			area.x2 += dx;
		}
		area.x1 = area.x2 - width;
	}
	
	if(dy < 0){
		dy *= -1 ;
		if(area.y1 < dy){
			area.y1 = 0;
		}
		else{
			area.y1 -= dy;
		}
		area.y2 = area.y1 + height;
	}
	else{
		if((disp->driver.ver_res - area.y2 - 1) < dy){
			area.y2 = disp->driver.ver_res - 1;
		}
		else{
			area.y2 += dy;
		}
		area.y1 = area.y2 - height;
	}
	ug_obj_set_coords(obj, &area);
    
    /*Tell the children the parent's size has changed*/
    ug_obj_t * i;
    _UG_LL_READ(obj->child_ll, i) {
        i->signal_cb(i, UG_SIGNAL_PARENT_SIZE_CHG,  NULL);
    }
}

/**
 * Set relative the position of an object (relative to the parent)
 * @param obj pointer to an object
 * @param x new distance from the left side of the parent
 * @param y new distance from the top of the parent
 */
void ug_obj_set_pos(ug_obj_t * obj, ug_coord_t x, ug_coord_t y)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    /*Convert x and y to absolute coordinates*/
    ug_obj_t * par = obj->parent;

    if(par) {
        x = x + par->coords.x1;
        y = y + par->coords.y1;
    }


    /*Calculate and set the movement*/
    ug_point_t diff;
    diff.x = x - obj->coords.x1;
    diff.y = y - obj->coords.y1;

    /* Do nothing if the position is not changed */
    /* It is very important else recursive positioning can
     * occur without position change*/
    if(diff.x == 0 && diff.y == 0) return;

    /*Invalidate the original area*/
    ug_obj_invalidate(obj);

    /*Save the original coordinates*/
    ug_area_t ori;
    ug_obj_get_coords(obj, &ori);

    obj->coords.x1 += diff.x;
    obj->coords.y1 += diff.y;
    obj->coords.x2 += diff.x;
    obj->coords.y2 += diff.y;

    refresh_children_position(obj, diff.x, diff.y);

    /*Inform the object about its new coordinates*/
    obj->signal_cb(obj, UG_SIGNAL_COORD_CHG, &ori);

    /*Send a signal to the parent too*/
    if(par) par->signal_cb(par, UG_SIGNAL_CHILD_CHG, obj);

    /*Invalidate the new area*/
    ug_obj_invalidate(obj);
}

/**
 * Set the x coordinate of a object
 * @param obj pointer to an object
 * @param x new distance from the left side from the parent
 */
void ug_obj_set_x(ug_obj_t * obj, ug_coord_t x)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_obj_set_pos(obj, x, ug_obj_get_y(obj));
}

/**
 * Set the y coordinate of a object
 * @param obj pointer to an object
 * @param y new distance from the top of the parent
 */
void ug_obj_set_y(ug_obj_t * obj, ug_coord_t y)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_obj_set_pos(obj, ug_obj_get_x(obj), y);
}


/**
 * Set the size of an object
 * @param obj pointer to an object
 * @param w new width
 * @param h new height
 */
void ug_obj_set_size(ug_obj_t * obj, ug_coord_t w, ug_coord_t h)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    /* Do nothing if the size is not changed */
    /* It is very important else recursive resizing can
     * occur without size change*/
    if(ug_obj_get_width(obj) == w && ug_obj_get_height(obj) == h) {
        return;
    }

    /*Invalidate the original area*/
    ug_obj_invalidate(obj);

    /*Save the original coordinates*/
    ug_area_t ori;
    ug_obj_get_coords(obj, &ori);

    /*Set the length and height*/
    obj->coords.y2 = obj->coords.y1 + h - 1;
    // if(ug_obj_get_base_dir(obj) == UG_BIDI_DIR_RTL) {
    //     obj->coords.x1 = obj->coords.x2 - w + 1;
    // }
    // else {
    //     obj->coords.x2 = obj->coords.x1 + w - 1;
    // }
    obj->coords.x2 = obj->coords.x1 + w - 1;

    /*Send a signal to the object with its new coordinates*/
    obj->signal_cb(obj, UG_SIGNAL_COORD_CHG, &ori);

    /*Send a signal to the parent too*/
    ug_obj_t * par = ug_obj_get_parent(obj);
    if(par != NULL) par->signal_cb(par, UG_SIGNAL_CHILD_CHG, obj);

    /*Tell the children the parent's size has changed*/
    ug_obj_t * i;
    _UG_LL_READ(obj->child_ll, i) {
        i->signal_cb(i, UG_SIGNAL_PARENT_SIZE_CHG,  &ori);
    }

    /*Invalidate the new area*/
    ug_obj_invalidate(obj);

    /*Automatically realign the object if required*/
#if UG_USE_OBJ_REALIGN
    if(obj->realign.auto_realign) ug_obj_realign(obj);
#endif
}




/**
 * Align an object to an other object.
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'ug_align_t' enum)
 * @param x_ofs x coordinate offset after alignment
 * @param y_ofs y coordinate offset after alignment
 */
void ug_obj_align(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align, ug_coord_t x_ofs, ug_coord_t y_ofs)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    if(base == NULL) base = ug_obj_get_parent(obj);

    //UG_ASSERT_OBJ(base, UG_OBJX_NAME);

    obj_align_core(obj, base, align, true, true, x_ofs, y_ofs);

#if UG_USE_OBJ_REALIGN
    /*Save the last align parameters to use them in `ug_obj_realign`*/
    obj->realign.align       = align;
    obj->realign.xofs        = x_ofs;
    obj->realign.yofs        = y_ofs;
    obj->realign.base        = base;
    obj->realign.origo_align = 0;
#endif
}

/**
 * Realign the object based on the last `ug_obj_align` parameters.
 * @param obj pointer to an object
 */
void ug_obj_realign(ug_obj_t * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

#if UG_USE_OBJ_REALIGN
    if(obj->realign.origo_align)
        ug_obj_align_origo(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
    else
        ug_obj_align(obj, obj->realign.base, obj->realign.align, obj->realign.xofs, obj->realign.yofs);
#else
    (void)obj;
    //UG_LOG_WARN("ug_obj_realign: no effect because UG_USE_OBJ_REALIGN = 0");
#endif
}


/**
 * Align an object's middle point to an other object.
 * @param obj pointer to an object to align
 * @param base pointer to an object (if NULL the parent is used). 'obj' will be aligned to it.
 * @param align type of alignment (see 'ug_align_t' enum)
 * @param x_ofs x coordinate offset after alignment
 * @param y_ofs y coordinate offset after alignment
 */
void ug_obj_align_origo(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align, ug_coord_t x_ofs, ug_coord_t y_ofs)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    if(base == NULL) {
        base = ug_obj_get_parent(obj);
    }

    //UG_ASSERT_OBJ(base, UG_OBJX_NAME);


    obj_align_origo_core(obj, base, align, true, true, x_ofs, y_ofs);

#if UG_USE_OBJ_REALIGN
    /*Save the last align parameters to use them in `ug_obj_realign`*/
    obj->realign.align       = align;
    obj->realign.xofs        = x_ofs;
    obj->realign.yofs        = y_ofs;
    obj->realign.base        = base;
    obj->realign.origo_align = 1;
#endif
}

/**
 * Enable the automatic realign of the object when its size has changed based on the last
 * `ug_obj_align` parameters.
 * @param obj pointer to an object
 * @param en true: enable auto realign; false: disable auto realign
 */
void ug_obj_set_auto_realign(ug_obj_t * obj, bool en)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

#if UG_USE_OBJ_REALIGN
    obj->realign.auto_realign = en ? 1 : 0;
#else
    (void)obj;
    (void)en;
    //UG_LOG_WARN("ug_obj_set_auto_realign: no effect because UG_USE_OBJ_REALIGN = 0");
#endif
}


/*----------------
 * Other set
 *--------------*/

/**
 * Allocate a new ext. data for an object
 * @param obj pointer to an object
 * @param ext_size the size of the new ext. data
 * @return pointer to the allocated ext.
 * If out of memory NULL is returned and the original ext is preserved
 */
void * ug_obj_allocate_ext_attr(ug_obj_t * obj, uint16_t ext_size)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    void * new_ext = ug_mem_realloc(obj->ext_attr, ext_size);
    if(new_ext == NULL) return NULL;

    obj->ext_attr = new_ext;
    return (void *)obj->ext_attr;
}




/**********************
 *   STATIC FUNCTIONS
 **********************/

static void ug_obj_del_async_cb(void * obj)
{
    //UG_ASSERT_OBJ(obj, UG_OBJX_NAME);

    ug_obj_del(obj);
}

static void obj_del_core(ug_obj_t * obj)
{
    /*Let the user free the resources used in `UG_EVENT_DELETE`*/
    ug_event_send(obj, UG_EVENT_DELETE, NULL);

    /*Recursively delete the children*/
    ug_obj_t * i;
    ug_obj_t * i_next;
    i = _ug_ll_get_head(&(obj->child_ll));
    while(i != NULL) {
        /*Get the next object before delete this*/
        i_next = _ug_ll_get_next(&(obj->child_ll), i);

        /*Call the recursive del to the child too*/
        obj_del_core(i);

        /*Set i to the next node*/
        i = i_next;
    }

    ug_event_mark_deleted(obj);


    /* All children deleted.
     * Now clean up the object specific data*/
    obj->signal_cb(obj, UG_SIGNAL_CLEANUP, NULL);

    /*Remove the object from parent's children list*/
    ug_obj_t * par = ug_obj_get_parent(obj);
    if(par == NULL) { /*It is a screen*/
        ug_disp_t * d = ug_obj_get_disp(obj);
        _ug_ll_remove(&d->scr_ll, obj);
    }
    else {
        _ug_ll_remove(&(par->child_ll), obj);
    }

    /*Delete the base objects*/
    if(obj->ext_attr != NULL) ug_mem_free(obj->ext_attr);
    ug_mem_free(obj); /*Free the object itself*/
}



/**
 * Handle the drawing related tasks of the base objects.
 * @param obj pointer to an object
 * @param clip_area the object will be drawn only in this area
 * @param mode UG_DESIGN_COVER_CHK: only check if the object fully covers the 'mask_p' area
 *                                  (return 'true' if yes)
 *             UG_DESIGN_DRAW: draw the object (always return 'true')
 * @param return an element of `ug_design_res_t`
 * @note 今天的风儿，声嘶力竭的喧嚣，我穿上了我的3级防护服，也抵抗不了它对我的蹂躏。
 * 它就像大海中的巨兽，鞭挞着我的脸颊，无情、残忍、凶狠。我无力抵抗，还好，内心深处
 * 的那一丝未来美好的希冀，总是在我即将倒下的时候，支持着我，我会努力的，我会得到我想要的生活，和你一起。
 */
static ug_design_res_t ug_obj_design(ug_obj_t * obj, const ug_area_t * clip_area, ug_design_mode_t mode)
{
    if(mode == UG_DESIGN_COVER_CHK) {

        /*Most trivial test. Is the mask fully IN the object? If no it surely doesn't cover it*/
        ug_area_t coords;
        ug_area_copy(&coords, &obj->coords);


        if(_ug_area_is_in(clip_area, &coords, 0) == false) return UG_DESIGN_RES_NOT_COVER;

        return  UG_DESIGN_RES_COVER;

    }
    else if(mode == UG_DESIGN_DRAW_MAIN) {
        ug_draw_rect_dsc_t draw_dsc;
        ug_draw_rect_dsc_init(&draw_dsc);
        ug_area_t coords;
        ug_area_copy(&coords, &obj->coords);

        draw_dsc.bg_color.full = obj->bg_color.full;
        draw_dsc.border_color.full = obj->bd_color.full;
        draw_dsc.border_width = obj->bd_width;
        draw_dsc.drawbackground = obj->drawbackground;

        ug_draw_rect(&coords, clip_area, &draw_dsc);

    }

    return UG_DESIGN_RES_OK;
}




/**
 * Signal function of the basic object
 * @param obj pointer to an object
 * @param sign signal type
 * @param param parameter for the signal (depends on signal type)
 * @return UG_RES_OK: the object is not deleted in the function; UG_RES_INV: the object is deleted
 */
static ug_res_t ug_obj_signal(ug_obj_t * obj, ug_signal_t sign, void * param)
{

    ug_res_t res = UG_RES_OK;

    if(sign == UG_SIGNAL_CHILD_CHG) {
        /*Return 'invalid' if the child change signal is not enabled*/
        if(ug_obj_is_protected(obj, UG_PROTECT_CHILD_CHG) != false) res = UG_RES_INV;
        

    }

#if UG_USE_OBJ_REALIGN
    else if(sign == UG_SIGNAL_PARENT_SIZE_CHG) {
        if(obj->realign.auto_realign) {
            ug_obj_realign(obj);
        }
    }
#endif

    return res;
}

static void ug_event_mark_deleted(ug_obj_t * obj)
{
    ug_event_temp_data_t * t = event_temp_data_head;

    while(t) {
        if(t->obj == obj) t->deleted = true;
        t = t->prev;
    }
}


static void obj_align_core(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align, bool x_set, bool y_set, ug_coord_t x_ofs, ug_coord_t y_ofs)
{
    ug_point_t new_pos;
    _ug_area_align(&base->coords, &obj->coords, align, &new_pos);

    /*Bring together the coordination system of base and obj*/
    ug_obj_t * par        = ug_obj_get_parent(obj);
    ug_coord_t par_abs_x  = par->coords.x1;
    ug_coord_t par_abs_y  = par->coords.y1;
    new_pos.x += x_ofs;
    new_pos.y += y_ofs;
    new_pos.x -= par_abs_x;
    new_pos.y -= par_abs_y;

    if(x_set && y_set) ug_obj_set_pos(obj, new_pos.x, new_pos.y);
    else if(x_set) ug_obj_set_x(obj, new_pos.x);
    else if(y_set) ug_obj_set_y(obj, new_pos.y);
}


static void obj_align_origo_core(ug_obj_t * obj, const ug_obj_t * base, ug_align_t align,  bool x_set, bool y_set, ug_coord_t x_ofs, ug_coord_t y_ofs)
{
    ug_coord_t new_x = ug_obj_get_x(obj);
    ug_coord_t new_y = ug_obj_get_y(obj);

    ug_coord_t obj_w_half = ug_obj_get_width(obj) / 2;
    ug_coord_t obj_h_half = ug_obj_get_height(obj) / 2;


    switch(align) {
        case UG_ALIGN_CENTER:
            new_x = ug_obj_get_width(base) / 2 - obj_w_half;
            new_y = ug_obj_get_height(base) / 2 - obj_h_half;
            break;

        case UG_ALIGN_IN_TOP_LEFT:
            new_x = -obj_w_half;
            new_y = -obj_h_half;
            break;
        case UG_ALIGN_IN_TOP_MID:
            new_x = ug_obj_get_width(base) / 2 - obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_IN_TOP_RIGHT:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_IN_BOTTOM_LEFT:
            new_x = -obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;
        case UG_ALIGN_IN_BOTTOM_MID:
            new_x = ug_obj_get_width(base) / 2 - obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_IN_BOTTOM_RIGHT:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_IN_LEFT_MID:
            new_x = -obj_w_half;
            new_y = ug_obj_get_height(base) / 2 - obj_h_half;
            break;

        case UG_ALIGN_IN_RIGHT_MID:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = ug_obj_get_height(base) / 2 - obj_h_half;
            break;

        case UG_ALIGN_OUT_TOP_LEFT:
            new_x = -obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_OUT_TOP_MID:
            new_x = ug_obj_get_width(base) / 2 - obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_OUT_TOP_RIGHT:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_OUT_BOTTOM_LEFT:
            new_x = -obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_OUT_BOTTOM_MID:
            new_x = ug_obj_get_width(base) / 2 - obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_OUT_BOTTOM_RIGHT:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_OUT_LEFT_TOP:
            new_x = -obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_OUT_LEFT_MID:
            new_x = -obj_w_half;
            new_y = ug_obj_get_height(base) / 2 - obj_h_half;
            break;

        case UG_ALIGN_OUT_LEFT_BOTTOM:
            new_x = -obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;

        case UG_ALIGN_OUT_RIGHT_TOP:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = -obj_h_half;
            break;

        case UG_ALIGN_OUT_RIGHT_MID:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = ug_obj_get_height(base) / 2 - obj_h_half;
            break;

        case UG_ALIGN_OUT_RIGHT_BOTTOM:
            new_x = ug_obj_get_width(base) - obj_w_half;
            new_y = ug_obj_get_height(base) - obj_h_half;
            break;
    }

    /*Bring together the coordination system of base and obj*/
    ug_obj_t * par        = ug_obj_get_parent(obj);
    ug_coord_t base_abs_x = base->coords.x1;
    ug_coord_t base_abs_y = base->coords.y1;
    ug_coord_t par_abs_x  = par->coords.x1;
    ug_coord_t par_abs_y  = par->coords.y1;
    new_x += x_ofs + base_abs_x;
    new_y += y_ofs + base_abs_y;
    new_x -= par_abs_x;
    new_y -= par_abs_y;
    if(x_set && y_set) ug_obj_set_pos(obj, new_x, new_y);
    else if(x_set) ug_obj_set_x(obj, new_x);
    else if(y_set) ug_obj_set_y(obj, new_y);

}




/**
 * Reposition the children of an object. (Called recursively)
 * @param obj pointer to an object which children will be repositioned
 * @param x_diff x coordinate shift
 * @param y_diff y coordinate shift
 */
static void refresh_children_position(ug_obj_t * obj, ug_coord_t x_diff, ug_coord_t y_diff)
{
    ug_obj_t * i;
    _UG_LL_READ(obj->child_ll, i) {
        i->coords.x1 += x_diff;
        i->coords.y1 += y_diff;
        i->coords.x2 += x_diff;
        i->coords.y2 += y_diff;

        refresh_children_position(i, x_diff, y_diff);
    }
}


/**
 * Initialize a rectangle descriptor from an object's styles
 * @param obj pointer to an object
 * @param type type of style. E.g.  `UG_OBJ_PART_MAIN`, `UG_BTN_STYLE_REL` or `UG_PAGE_STYLE_SCRL`
 * @param draw_dsc the descriptor the initialize
 * @note Only the relevant fields will be set.
 * E.g. if `border width == 0` the other border properties won't be evaluated.
 */
// void ug_obj_init_draw_rect_dsc(ug_obj_t * obj, uint8_t part, ug_draw_rect_dsc_t * draw_dsc)
// {
//     //draw_dsc->radius = ug_obj_get_style_radius(obj, part);

// #if UG_USE_OPA_SCALE
//     ug_opa_t opa_scale = ug_obj_get_style_opa_scale(obj, part);
//     if(opa_scale <= UG_OPA_MIN) {
//         draw_dsc->bg_opa = UG_OPA_TRANSP;
//         draw_dsc->border_opa = UG_OPA_TRANSP;
//         draw_dsc->shadow_opa = UG_OPA_TRANSP;
//         draw_dsc->pattern_opa = UG_OPA_TRANSP;
//         draw_dsc->value_opa = UG_OPA_TRANSP;
//         return;
//     }
// #endif

//     if(draw_dsc->bg_opa != UG_OPA_TRANSP) {
//         draw_dsc->bg_opa = ug_obj_get_style_bg_opa(obj, part);
//         if(draw_dsc->bg_opa > UG_OPA_MIN) {
//             draw_dsc->bg_color = ug_obj_get_style_bg_color(obj, part);
//             draw_dsc->bg_grad_dir =  ug_obj_get_style_bg_grad_dir(obj, part);
//             if(draw_dsc->bg_grad_dir != UG_GRAD_DIR_NONE) {
//                 draw_dsc->bg_grad_color = ug_obj_get_style_bg_grad_color(obj, part);
//                 draw_dsc->bg_main_color_stop =  ug_obj_get_style_bg_main_stop(obj, part);
//                 draw_dsc->bg_grad_color_stop =  ug_obj_get_style_bg_grad_stop(obj, part);
//             }

// #if UG_USE_BLEND_MODES
//             draw_dsc->bg_blend_mode = ug_obj_get_style_bg_blend_mode(obj, part);
// #endif
//         }
//     }

//     if(draw_dsc->border_opa != UG_OPA_TRANSP) {
//         draw_dsc->border_width = ug_obj_get_style_border_width(obj, part);
//         if(draw_dsc->border_width) {
//             draw_dsc->border_opa = ug_obj_get_style_border_opa(obj, part);
//             if(draw_dsc->border_opa > UG_OPA_MIN) {
//                 draw_dsc->border_side = ug_obj_get_style_border_side(obj, part);
//                 draw_dsc->border_color = ug_obj_get_style_border_color(obj, part);
//             }
// #if UG_USE_BLEND_MODES
//             draw_dsc->border_blend_mode = ug_obj_get_style_border_blend_mode(obj, part);
// #endif
//         }
//     }


//     if(draw_dsc->outline_opa != UG_OPA_TRANSP) {
//         draw_dsc->outline_width = ug_obj_get_style_outline_width(obj, part);
//         if(draw_dsc->outline_width) {
//             draw_dsc->outline_opa = ug_obj_get_style_outline_opa(obj, part);
//             if(draw_dsc->outline_opa > UG_OPA_MIN) {
//                 draw_dsc->outline_pad = ug_obj_get_style_outline_pad(obj, part);
//                 draw_dsc->outline_color = ug_obj_get_style_outline_color(obj, part);
//             }
// #if UG_USE_BLEND_MODES
//             draw_dsc->outline_blend_mode = ug_obj_get_style_outline_blend_mode(obj, part);
// #endif
//         }
//     }

//     if(draw_dsc->pattern_opa != UG_OPA_TRANSP) {
//         draw_dsc->pattern_image = ug_obj_get_style_pattern_image(obj, part);
//         if(draw_dsc->pattern_image) {
//             draw_dsc->pattern_opa = ug_obj_get_style_pattern_opa(obj, part);
//             if(draw_dsc->pattern_opa > UG_OPA_MIN) {
//                 draw_dsc->pattern_recolor_opa = ug_obj_get_style_pattern_recolor_opa(obj, part);
//                 draw_dsc->pattern_repeat = ug_obj_get_style_pattern_repeat(obj, part);
//                 if(ug_img_src_get_type(draw_dsc->pattern_image) == UG_IMG_SRC_SYMBOL) {
//                     draw_dsc->pattern_recolor = ug_obj_get_style_pattern_recolor(obj, part);
//                     draw_dsc->pattern_font = ug_obj_get_style_text_font(obj, part);
//                 }
//                 else if(draw_dsc->pattern_recolor_opa > UG_OPA_MIN) {
//                     draw_dsc->pattern_recolor = ug_obj_get_style_pattern_recolor(obj, part);
//                 }
// #if UG_USE_BLEND_MODES
//                 draw_dsc->pattern_blend_mode = ug_obj_get_style_pattern_blend_mode(obj, part);
// #endif
//             }
//         }
//     }
// #if UG_USE_SHADOW
//     if(draw_dsc->shadow_opa > UG_OPA_MIN) {
//         draw_dsc->shadow_width = ug_obj_get_style_shadow_width(obj, part);
//         if(draw_dsc->shadow_width) {
//             draw_dsc->shadow_opa = ug_obj_get_style_shadow_opa(obj, part);
//             if(draw_dsc->shadow_opa > UG_OPA_MIN) {
//                 draw_dsc->shadow_ofs_x = ug_obj_get_style_shadow_ofs_x(obj, part);
//                 draw_dsc->shadow_ofs_y = ug_obj_get_style_shadow_ofs_y(obj, part);
//                 draw_dsc->shadow_spread = ug_obj_get_style_shadow_spread(obj, part);
//                 draw_dsc->shadow_color = ug_obj_get_style_shadow_color(obj, part);
// #if UG_USE_BLEND_MODES
//                 draw_dsc->shadow_blend_mode = ug_obj_get_style_shadow_blend_mode(obj, part);
// #endif
//             }
//         }
//     }
// #endif

//     if(draw_dsc->value_opa > UG_OPA_MIN) {
//         draw_dsc->value_str = ug_obj_get_style_value_str(obj, part);
//         if(draw_dsc->value_str) {
//             draw_dsc->value_opa = ug_obj_get_style_value_opa(obj, part);
//             if(draw_dsc->value_opa > UG_OPA_MIN) {
//                 draw_dsc->value_ofs_x = ug_obj_get_style_value_ofs_x(obj, part);
//                 draw_dsc->value_ofs_y = ug_obj_get_style_value_ofs_y(obj, part);
//                 draw_dsc->value_color = ug_obj_get_style_value_color(obj, part);
//                 draw_dsc->value_font = ug_obj_get_style_value_font(obj, part);
//                 draw_dsc->value_letter_space = ug_obj_get_style_value_letter_space(obj, part);
//                 draw_dsc->value_line_space = ug_obj_get_style_value_line_space(obj, part);
//                 draw_dsc->value_align = ug_obj_get_style_value_align(obj, part);
// #if UG_USE_BLEND_MODES
//                 draw_dsc->value_blend_mode = ug_obj_get_style_value_blend_mode(obj, part);
// #endif
//             }
//         }
//     }

// #if UG_USE_OPA_SCALE
//     if(opa_scale < UG_OPA_MAX) {
//         draw_dsc->bg_opa = (uint16_t)((uint16_t)draw_dsc->bg_opa * opa_scale) >> 8;
//         draw_dsc->border_opa = (uint16_t)((uint16_t)draw_dsc->border_opa * opa_scale) >> 8;
//         draw_dsc->shadow_opa = (uint16_t)((uint16_t)draw_dsc->shadow_opa * opa_scale) >> 8;
//         draw_dsc->pattern_opa = (uint16_t)((uint16_t)draw_dsc->pattern_opa * opa_scale) >> 8;
//         draw_dsc->value_opa = (uint16_t)((uint16_t)draw_dsc->value_opa * opa_scale) >> 8;
//     }
// #endif
// }

