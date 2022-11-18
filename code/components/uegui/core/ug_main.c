/**
 * @file UVGUI.c
 * @author @firestaradmin
 * @brief main of UEGUI
 * @version 0.1
 * @date 2022-11-10
 * 
 * @copyright Copyright (c) 2022 LXG 
 * 
 */
#include "ug_main.h"

#include "ug_mem.h"
#include "ug_disp.h"

/**********************
 *  GLOBAL VARIABLES
 **********************/
ug_ll_t _ug_disp_ll;
ug_ll_t _ug_task_ll;
ug_task_t * _ug_task_act;


/**********************
 *  STATIC VARIABLES
 **********************/
static bool ug_initialized = false;

/**
 * Init. the 'ug' library.
 */
void ug_init(void)
{
    /* Do nothing if already initialized */
    if (ug_initialized)
    {
        return;
    }

    /*Initialize the ug_mem modules*/
    _ug_mem_init();

    /* TODO add task support */
    _ug_task_core_init();

    /* Initialize disp_ll */
    _ug_ll_init(&_ug_disp_ll, sizeof(ug_disp_t));


    /*Test if the IDE has UTF-8 encoding*/
    char *txt = "Á";

    uint8_t *txt_u8 = (uint8_t *)txt;
    if (txt_u8[0] != 0xc3 || txt_u8[1] != 0x81 || txt_u8[2] != 0x00)
    {
        //TODO: UG_LOG_WARN("The strings has no UTF-8 encoding. Some characters won't be displayed.")
    }

    ug_initialized = true;

}

