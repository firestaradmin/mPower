#include "ug_draw_blend.h"
#include "ug_draw_elements.h"
#include "ug_user_config.h"



/**
 * Fill an area with a color
 * @param disp_area the current display area (destination area)
 * @param disp_buf destination buffer
 * @param draw_area fill this area (relative to `disp_area`)
 * @param color fill color
 * @param opa overall opacity in 0x00..0xff range
 * @param mask a mask to apply on every pixel (uint8_t array with 0x00..0xff values).
 *                It fits into draw_area.
 * @param mask_res UG_MASK_RES_COVER: the mask has only 0xff values (no mask),
 *                 UG_MASK_RES_TRANSP: the mask has only 0x00 values (full transparent),
 *                 UG_MASK_RES_CHANGED: the mask has mixed values
 */
UG_ATTRIBUTE_FAST_MEM void fill_normal(const ug_area_t * disp_area, ug_color_t * disp_buf,
                                              const ug_area_t * draw_area,
                                              ug_color_t color, ug_opa_t opa,
                                              const ug_opa_t * mask, ug_draw_mask_res_t mask_res)
{

    /*Get the width of the `disp_area` it will be used to go to the next line*/
    int32_t disp_w = ug_area_get_width(disp_area);

    int32_t draw_area_w = ug_area_get_width(draw_area);
    int32_t draw_area_h = ug_area_get_height(draw_area);

    /*Create a temp. disp_buf which always point to the first pixel of the destination area*/
    ug_color_t * disp_buf_first = disp_buf + disp_w * draw_area->y1 + draw_area->x1;

    int32_t x;
    int32_t y;

    /*Simple fill (maybe with opacity), no masking*/
    if(mask_res == UG_DRAW_MASK_RES_FULL_COVER) {
        if(opa > UG_OPA_MAX) {
            /*Software rendering*/
            for(y = 0; y < draw_area_h; y++) {
                ug_color_fill(disp_buf_first, color, draw_area_w);
                disp_buf_first += disp_w;
            }
        }
        /*No mask with opacity*/
        else {

            ug_color_t last_dest_color = UG_COLOR_BLACK;
            ug_color_t last_res_color = ug_color_mix(color, last_dest_color, opa);

            uint16_t color_premult[3];
            ug_color_premult(color, opa, color_premult);
            ug_opa_t opa_inv = 255 - opa;

            for(y = 0; y < draw_area_h; y++) {
                for(x = 0; x < draw_area_w; x++) {
                    if(last_dest_color.full != disp_buf_first[x].full) {
                        last_dest_color = disp_buf_first[x];
                        last_res_color = ug_color_mix_premult(color_premult, disp_buf_first[x], opa_inv);
                    }
                    disp_buf_first[x] = last_res_color;
                }
                disp_buf_first += disp_w;
            }
        }
    }

}
