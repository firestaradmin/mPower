
#include "lcd_st7735s.h"
#include "ug_disp.h"
#include "ug_area.h"
#include <stdio.h>
#include "esp_attr.h"
static void disp_init(void);
static void disp_flush(ug_disp_drv_t * disp_drv, const ug_area_t * area, ug_color_t * color_p);

DRAM_ATTR ug_color_t buf1[80*160];

void ug_port_disp_init(void)
{

    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

// 11

    static ug_disp_buf_t disp_buf;

    ug_disp_buf_init(&disp_buf, buf1, NULL, 80*160);   /*Initialize the display buffer*/


    /*-----------------------------------
     * Register the display in UVGUI
     *----------------------------------*/

    ug_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    ug_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/
    /*Set the resolution of the display*/
    disp_drv.hor_res = UG_HOR_RES_MAX;
    disp_drv.ver_res = UG_VER_RES_MAX;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_screen = disp_flush;

    /*Set a display buffer*/
    disp_drv.buffer = &disp_buf;

    /*Set DPI. */
	disp_drv.dpi = UG_DPI;
    
    /*Finally register the driver*/
    ug_disp_drv_register(&disp_drv);
	
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* Initialize your display and the required peripherals. */
static void disp_init(void)
{
    /*You code here*/
	lcd_init();

}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_disp_flush_ready()' has to be called when finished. */
static void disp_flush(ug_disp_drv_t * disp_drv, const ug_area_t * area, ug_color_t * color_p)
{
    /*The most simple case (but also the slowest) to put all pixels to the screen one-by-one*/
    uint32_t cnt = (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1);
    // printf("disp_flush: x1:%d, y1:%d, x2:%d, y2:%d, cnt:%d\r\n", area->x1,area->y1,area->x2,area->y2,cnt);
    // printf("color_p[0]: 0x%04X, color_p[10]: 0x%04X, color_p[50]: 0x%04X\r\n", color_p[0].full,color_p[10].full,color_p[50].full);
	lcd_address_set(area->x1,area->y1,area->x2,area->y2);//设置显示范围

    lcd_xfer((uint8_t *)color_p, cnt*2);



    /* IMPORTANT!!!
     * Inform the graphics library that you are ready with the flushing*/
    ug_disp_flush_ready(disp_drv);
}


