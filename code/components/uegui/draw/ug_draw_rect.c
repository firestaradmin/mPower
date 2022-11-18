/**
 * @file ug_draw_rect.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "ug_draw_rect.h"
#include "ug_draw_blend.h"
#include "ug_math.h"
#include "ug_refr.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/
UG_ATTRIBUTE_FAST_MEM static void draw_back_groung(const ug_area_t * coords, const ug_area_t * clip, ug_draw_rect_dsc_t * dsc);
UG_ATTRIBUTE_FAST_MEM static void draw_border(const ug_area_t *coords,
                                              const ug_area_t *clip,
                                              ug_draw_rect_dsc_t *dsc);
/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

UG_ATTRIBUTE_FAST_MEM void ug_draw_rect_dsc_init(ug_draw_rect_dsc_t * dsc)
{
    _ug_memset_00(dsc, sizeof(ug_draw_rect_dsc_t));
    dsc->bg_color = UG_COLOR_WHITE;

    // dsc->border_color = UG_COLOR_BLACK;
    // dsc->pattern_recolor = UG_COLOR_BLACK;
    dsc->value_color = UG_COLOR_BLACK;

}

/**
 * Draw a rectangle
 * @param coords the coordinates of the rectangle
 * @param mask the rectangle will be drawn only in this mask
 * @param style pointer to a style
 */
void ug_draw_rect(const ug_area_t * coords, const ug_area_t * clip, ug_draw_rect_dsc_t * dsc)
{
    if(ug_area_get_height(coords) < 1 || ug_area_get_width(coords) < 1) return;

    if(dsc->drawbackground !=0)
        draw_back_groung(coords, clip, dsc);

    if(dsc->border_width != 0)
        draw_border(coords, clip, dsc);


}


UG_ATTRIBUTE_FAST_MEM static void draw_back_groung(const ug_area_t * coords, const ug_area_t * clip, ug_draw_rect_dsc_t * dsc)
{
    ug_disp_t * disp    = _ug_refr_get_refrdisp();
    ug_disp_buf_t * vdb = ug_disp_get_buf(disp);
    const ug_area_t * disp_area = &vdb->area;
    ug_color_t * disp_buf = vdb->buf_act;
	
    ug_area_t draw_area;
	bool is_common;
    is_common = _ug_area_intersect(&draw_area, coords, clip);
    if(is_common == false) return;

	/* Now `draw_area` has absolute coordinates.
     * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
    draw_area.x1 -= disp_area->x1;
    draw_area.y1 -= disp_area->y1;
    draw_area.x2 -= disp_area->x1;
    draw_area.y2 -= disp_area->y1;
	
    fill_normal(disp_area, disp_buf, &draw_area, dsc->bg_color, 255, NULL, UG_DRAW_MASK_RES_FULL_COVER);
}

UG_ATTRIBUTE_FAST_MEM static void draw_border(const ug_area_t *coords,
                                              const ug_area_t *clip,
                                              ug_draw_rect_dsc_t *dsc)
{
    ug_disp_t * disp    = _ug_refr_get_refrdisp();
    ug_disp_buf_t * vdb = ug_disp_get_buf(disp);
    const ug_area_t * disp_area = &vdb->area;
    ug_color_t * disp_buf = vdb->buf_act;
	
	bool is_common;
    ug_area_t draw_area;
    ug_area_t draw_area_border;

    // draw top
    draw_area_border.x1 = coords->x1;
    draw_area_border.x2 = coords->x2;
    draw_area_border.y1 = coords->y1;
    draw_area_border.y2 = coords->y1 + dsc->border_width;
    is_common = _ug_area_intersect(&draw_area, &draw_area_border, clip);
    if(is_common == true) {
        /* Now `draw_area` has absolute coordinates.
        * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
        draw_area.x1 -= disp_area->x1;
        draw_area.y1 -= disp_area->y1;
        draw_area.x2 -= disp_area->x1;
        draw_area.y2 -= disp_area->y1;
        fill_normal(disp_area, disp_buf, &draw_area, dsc->border_color, 255, NULL, UG_DRAW_MASK_RES_FULL_COVER);
    }
    
    // draw bottom
    draw_area_border.x1 = coords->x1;
    draw_area_border.x2 = coords->x2;
    draw_area_border.y2 = coords->y2;
    draw_area_border.y1 = coords->y2 - dsc->border_width;
    is_common = _ug_area_intersect(&draw_area, &draw_area_border, clip);
    if(is_common == true) {
        /* Now `draw_area` has absolute coordinates.
        * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
        draw_area.x1 -= disp_area->x1;
        draw_area.y1 -= disp_area->y1;
        draw_area.x2 -= disp_area->x1;
        draw_area.y2 -= disp_area->y1;
        fill_normal(disp_area, disp_buf, &draw_area, dsc->border_color, 255, NULL, UG_DRAW_MASK_RES_FULL_COVER);
    }
    
    // draw left
    draw_area_border.x1 = coords->x1;
    draw_area_border.x2 = coords->x1 + dsc->border_width;
    draw_area_border.y1 = coords->y1;
    draw_area_border.y2 = coords->y2;
    is_common = _ug_area_intersect(&draw_area, &draw_area_border, clip);
    if(is_common == true) {
        /* Now `draw_area` has absolute coordinates.
        * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
        draw_area.x1 -= disp_area->x1;
        draw_area.y1 -= disp_area->y1;
        draw_area.x2 -= disp_area->x1;
        draw_area.y2 -= disp_area->y1;
        fill_normal(disp_area, disp_buf, &draw_area, dsc->border_color, 255, NULL, UG_DRAW_MASK_RES_FULL_COVER);
    }
    
    // draw right
    draw_area_border.x1 = coords->x2 - dsc->border_width;
    draw_area_border.x2 = coords->x2;
    draw_area_border.y1 = coords->y1;
    draw_area_border.y2 = coords->y2;
    is_common = _ug_area_intersect(&draw_area, &draw_area_border, clip);
    if(is_common == true) {
        /* Now `draw_area` has absolute coordinates.
        * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
        draw_area.x1 -= disp_area->x1;
        draw_area.y1 -= disp_area->y1;
        draw_area.x2 -= disp_area->x1;
        draw_area.y2 -= disp_area->y1;
        fill_normal(disp_area, disp_buf, &draw_area, dsc->border_color, 255, NULL, UG_DRAW_MASK_RES_FULL_COVER);
    }
    
}
