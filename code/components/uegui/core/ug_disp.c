#include "ug_main.h"
#include "ug_disp.h"
#include "ug_mem.h"
#include "ug_obj.h"
#include "ug_refr.h"
#include "ug_color.h"
/**********************
 *  STATIC VARIABLES
 **********************/

static ug_disp_t *act_disp;     /* Point to act disp. now use disp_refr in 'ug_refr.c' */
static uint8_t disp_index = 0;  /* how much device have been registered. */



/**
 * Initialize a display buffer
 * @param disp_buf pointer `ug_disp_buf_t` variable to initialize
 * @param buf A buffer to be used by LVGL to draw the image.
 *             Always has to specified and can't be NULL.
 *             Can be an array allocated by the user. E.g. `static ug_color_t disp_buf1[1024 * 10]`
 *             Or a memory address e.g. in external SRAM
 * @param size_in_px_cnt size of the `buf` in pixel count.
 */
void ug_disp_buf_init(ug_disp_buf_t * disp_buf, void * buf1, void * buf2, uint32_t size_in_px_cnt)
{
    _ug_memset_00(disp_buf, sizeof(ug_disp_buf_t));

    disp_buf->buf1    = buf1;
    disp_buf->buf2    = buf2;
    disp_buf->buf_act = disp_buf->buf1;
    disp_buf->size    = size_in_px_cnt;
}

/**
 * Initialize a display driver with default values.
 * It is used to surly have known values in the fields ant not memory junk.
 * After it you can set the fields.
 * @param driver pointer to driver variable to initialize
 */
void ug_disp_drv_init(ug_disp_drv_t * driver)
{
    _ug_memset_00(driver, sizeof(ug_disp_drv_t));

    driver->flush_screen     = NULL;
    driver->hor_res          = UG_HOR_RES_MAX;
    driver->ver_res          = UG_VER_RES_MAX;
    driver->buffer           = NULL;
    driver->rotated          = 0;
    driver->dpi              = UG_DPI;

}



/**
 * Register an initialized display driver.
 * Automatically set the first display as active.
 * @param driver pointer to an initialized 'ug_disp_drv_t' variable (can be local variable)
 * @return pointer to the new display or NULL on error
 */
ug_disp_t * ug_disp_drv_register(ug_disp_drv_t * driver)
{
    

    ug_disp_t  *new_disp = _ug_ll_ins_head(&_ug_disp_ll);
    if(new_disp == NULL) return NULL;

    _ug_memset_00(new_disp, sizeof(ug_disp_t));
    _ug_memcpy(&new_disp->driver, driver, sizeof(ug_disp_drv_t));

    /* Init new_disp'screen linked list */
    _ug_ll_init(&new_disp->scr_ll, sizeof(ug_obj_t));

    /* Register disp struct */
    if(act_disp == NULL) act_disp = new_disp;

    new_disp->bg_color = UG_COLOR_WHITE;
    new_disp->bg_img = NULL;


    /* disp_index use to change active disp */
    new_disp->disp_index = disp_index++;

    /* create first screen for disp. */
    new_disp->act_scr   = ug_obj_create(NULL, NULL, "screen"); /*Create a default screen on the display*/

    /*Create a refresh task*/
    new_disp->refr_task = ug_task_create(_ug_disp_refr_task, UG_DISP_DEF_REFR_PERIOD, UG_REFR_TASK_PRIO, new_disp);
    if(new_disp->refr_task == NULL) return NULL;
    
    ug_task_ready(new_disp->refr_task); /*Be sure the screen will be refreshed immediately on start up*/

    return new_disp;
}


/**
 * Get the default display
 * @return pointer to the default display
 */
ug_disp_t * ug_disp_get_actdisp(void)
{
    return act_disp;
}

ug_obj_t * ug_disp_get_actscr(ug_disp_t * disp)
{
	return disp->act_scr;
}

ug_obj_t * ug_disp_get_actdisp_actscr(void)
{
    return act_disp->act_scr;
}

//ug_area_t * _ug_disp_get_actdisp_redrawn_area(void)
//{
//    return &act_disp->redrawn_area;
//}

/**
 * Set the active display
 * @param disp_index index of the display
 * @return UVGUI error codes
 */
ug_res_t ug_disp_set_actdisp(uint8_t disp_index)
{
    ug_ll_node_t *next = _ug_ll_get_head(&_ug_disp_ll);
    while(next){
        if(((ug_disp_t *)next)->disp_index == disp_index){
            act_disp = (ug_disp_t *)next;
            return UG_RES_OK;
        }
        next = _ug_ll_get_next(&_ug_disp_ll, next);
    }
    return UG_RES_INV;
}

/**
 * Get the DPI of the display
 * @param disp pointer to a display (NULL to use the default display)
 * @return dpi of the display
 */
ug_coord_t ug_disp_get_dpi(ug_disp_t * disp)
{
    if(disp == NULL) disp = ug_disp_get_actdisp();
    if(disp == NULL) return UG_DPI;  /*Do not return 0 because it might be a divider*/
    return disp->driver.dpi;
}


/**
 * Get the horizontal resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the horizontal resolution of the display
 */
ug_coord_t ug_disp_get_hor_res(ug_disp_t * disp)
{
    if(disp == NULL) disp = ug_disp_get_actdisp();

    if(disp == NULL)
        return UG_HOR_RES_MAX;
    else
        return disp->driver.rotated == 0 ? disp->driver.hor_res : disp->driver.ver_res;
}

/**
 * Get the vertical resolution of a display
 * @param disp pointer to a display (NULL to use the default display)
 * @return the vertical resolution of the display
 */
ug_coord_t ug_disp_get_ver_res(ug_disp_t * disp)
{
    if(disp == NULL) disp = ug_disp_get_actdisp();

    if(disp == NULL)
        return UG_VER_RES_MAX;
    else
        return disp->driver.rotated == 0 ? disp->driver.ver_res : disp->driver.hor_res;
}

static inline ug_coord_t ug_dpx(ug_coord_t n)
{
    return UG_DPX(n);
}


/**
 * Check the driver configuration if it's double buffered (both `buf1` and `buf2` are set)
 * @param disp pointer to to display to check
 * @return true: double buffered; false: not double buffered
 */
bool ug_disp_is_double_buf(ug_disp_t * disp)
{
    if(disp->driver.buffer->buf1 && disp->driver.buffer->buf2)
        return true;
    else
        return false;
}

/**
 * Check the driver configuration if it's TRUE double buffered (both `buf1` and `buf2` are set and
 * `size` is screen sized)
 * @param disp pointer to to display to check
 * @return true: double buffered; false: not double buffered
 */
bool ug_disp_is_true_double_buf(ug_disp_t * disp)
{
    uint32_t scr_size = disp->driver.hor_res * disp->driver.ver_res;

    if(ug_disp_is_double_buf(disp) && disp->driver.buffer->size == scr_size) {
        return true;
    }
    else {
        return false;
    }
}


/**
 * Get the internal buffer of a display
 * @param disp pointer to a display
 * @return pointer to the internal buffers
 */
ug_disp_buf_t * ug_disp_get_buf(ug_disp_t * disp)
{
    return disp->driver.buffer;
}

/**
 * Get the number of areas in the buffer
 * @return number of invalid areas
 */
uint16_t ug_disp_get_inv_buf_size(ug_disp_t * disp)
{
    return disp->inv_p;
}


/**
 * Call in the display driver's `flush_cb` function when the flushing is finished
 * @param disp_drv pointer to display driver in `flush_cb` where this function is called
 */
void ug_disp_flush_ready(ug_disp_drv_t * disp_drv)
{
    /*If the screen is transparent initialize it when the flushing is ready*/
#if UG_COLOR_SCREEN_TRANSP
    if(disp_drv->screen_transp) {
        _ug_memset_00(disp_drv->buffer->buf_act, disp_drv->buffer->size * sizeof(ug_color32_t));
    }
#endif

    disp_drv->buffer->flushing = 0;
    disp_drv->buffer->flushing_last = 0;
}

