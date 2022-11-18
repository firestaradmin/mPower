
#ifndef __UG_DISP_H__
#define __UG_DISP_H__

#include "ug_task.h"
#include "ug_disp.h"
#include "ug_math.h"
#include "ug_color.h"
#include "ug_obj.h"
/*********************
 *      DEFINES
 *********************/
#ifndef UG_INV_BUF_SIZE
#define UG_INV_BUF_SIZE 32 /*Buffer size for invalid areas */
#endif
/**
 * Same as Android's DIP. (Different name is chosen to avoid mistype between UG_DPI and UG_DIP)
 * 1 dip is 1 px on a 160 DPI screen
 * 1 dip is 2 px on a 320 DPI screen
 * https://stackoverflow.com/questions/2025282/what-is-the-difference-between-px-dip-dp-and-sp
 */
#define UG_DPX(n)   UG_MATH_MAX((( ug_disp_get_dpi(NULL) * (n) + 80) / 160), 1) /*+80 for rounding*/


/**
 * Structure for holding display buffer information.
 */
typedef struct {
    void * buf1; /**< First display buffer. */
    void * buf2; /**< Second display buffer. */

    /*Internal, used by the library*/
    void * buf_act;
    uint32_t size; /*In pixel count*/

    /* the real area of the display, the true area's map in the physical screen. */
    ug_area_t area;
    /*1: flushing is in progress. (It can't be a bit field because when it's cleared from IRQ Read-Modify-Write issue might occur)*/
    volatile int flushing;
    /*1: It was the last chunk to flush. (It can't be a bi tfield because when it's cleared from IRQ Read-Modify-Write issue might occur)*/
    volatile int flushing_last;
    volatile uint32_t last_area         : 1; /*1: 标志最后一个需要绘制的区域正在被渲染 */
    volatile uint32_t last_part         : 1; /*1: 标志绘制的区域的最后一个部分正在被渲染 */
}ug_disp_buf_t;


/**
 * Display Driver structure to be registered by HAL
 */
typedef struct _disp_drv_t {

    ug_coord_t hor_res; /**< Horizontal resolution. */
    ug_coord_t ver_res; /**< Vertical resolution. */

    /** Pointer to a buffer initialized with `ug_disp_buf_init()`.
     * UVGUI will use this buffer(s) to draw the screens contents */
    ug_disp_buf_t * buffer;

    uint32_t rotated : 1; /**< 1: turn the display by 90 degree. @warning Does not update coordinates for you!*/

    /** DPI (dot per inch) of the display.
     * Set to `UG_DPI` from `ug_user_config.h` by default.
     */
    uint32_t dpi : 10;

    /** MANDATORY: Write the internal buffer (VDB) to the display. 'ug_disp_flush_ready()' has to be
     * called when finished */
    void (*flush_screen)(struct _disp_drv_t * disp_drv, const ug_area_t * area, ug_color_t * color_p);

    /** OPTIONAL: Called periodically while lvgl waits for operation to be completed.
     * For example flushing or GPU
     * User can execute very simple tasks here or yield the task */
    void (*wait_cb)(struct _disp_drv_t * disp_drv);


} ug_disp_drv_t;

/**
 * Display structure.
 * @note `ug_disp_drv_t` should be the first member of the structure.
 */
typedef struct _disp_t {
    
    /**< Driver to the display*/
    ug_disp_drv_t driver;

    
    /**< A task which periodically checks the dirty areas and refreshes them*/
    ug_task_t * refr_task;

    
    /** Screens of the display*/
    ug_ll_t scr_ll;
    struct _ug_obj_t * act_scr;   /**< Currently active screen on this display */
    struct _ug_obj_t * prev_scr;  /**< Previous screen. Used during screen animations */

    ug_color_t bg_color;          /**< Default display color when screens are transparent*/
    const void * bg_img;       /**< An image source to display as wallpaper*/

    ug_area_t inv_areas[UG_INV_BUF_SIZE];       // 需要重绘的区域
    uint8_t inv_area_joined[UG_INV_BUF_SIZE];   // 区域合并标志
    uint32_t inv_p : 10;    /* invalid parts */ // 需要重绘的区域数

    /*Miscellaneous data*/
    uint8_t disp_index;     /* Used to select/change which physical screen to display. */

} ug_disp_t;




void ug_disp_buf_init(ug_disp_buf_t * disp_buf, void * buf1, void * buf2, uint32_t size_in_px_cnt);

void ug_disp_drv_init(ug_disp_drv_t * driver);

ug_disp_t * ug_disp_drv_register(ug_disp_drv_t * driver);


ug_area_t * _ug_disp_get_actdisp_redrawn_area(void);



struct _ug_obj_t * ug_disp_get_actscr(ug_disp_t * disp);

ug_coord_t ug_disp_get_dpi(ug_disp_t * disp);

ug_coord_t ug_disp_get_hor_res(ug_disp_t * disp);

ug_coord_t ug_disp_get_ver_res(ug_disp_t * disp);

bool ug_disp_is_true_double_buf(ug_disp_t * disp);

bool ug_disp_is_double_buf(ug_disp_t * disp);

ug_disp_buf_t * ug_disp_get_buf(ug_disp_t * disp);

uint16_t ug_disp_get_inv_buf_size(ug_disp_t * disp);

void ug_disp_flush_ready(ug_disp_drv_t * disp_drv);

ug_disp_t * ug_disp_get_actdisp(void);

struct _ug_obj_t * ug_disp_get_actdisp_actscr(void);

ug_res_t ug_disp_set_actdisp(uint8_t disp_index);
#endif // !__UG_DISP_H__
