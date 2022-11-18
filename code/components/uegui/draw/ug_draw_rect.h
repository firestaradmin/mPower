#ifndef __UG_DRAW_RECT_H__
#define __UG_DRAW_RECT_H__
#include "ug_user_config.h"
#include "ug_type.h"
#include "ug_area.h"
#include "ug_color.h"


typedef uint16_t ug_rect_int_t;

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    //ug_rect_int_t radius;

    /*Background*/
    ug_color_t bg_color;
    uint8_t drawbackground : 1;
    /*Border*/
    ug_color_t border_color;
    ug_rect_int_t border_width;

    /*Outline*/
    //ug_color_t outline_color;
    //ug_rect_int_t outline_width;

    /*Shadow*/
    // ug_color_t shadow_color;
    // ug_rect_int_t shadow_width;

    /*Pattern*/
    //const void * pattern_image;
    //const ug_font_t * pattern_font;
    //ug_color_t pattern_recolor;

    /*Value*/
    const char * value_str;
    //const ug_font_t * value_font;
    ug_color_t value_color;
    ug_rect_int_t value_ofs_x;
    ug_rect_int_t value_ofs_y;
    ug_rect_int_t value_letter_space;
    ug_rect_int_t value_line_space;
    ug_align_t value_align;

} ug_draw_rect_dsc_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

UG_ATTRIBUTE_FAST_MEM void ug_draw_rect_dsc_init(ug_draw_rect_dsc_t * dsc);

//! @endcond

/**
 * Draw a rectangle
 * @param coords the coordinates of the rectangle
 * @param mask the rectangle will be drawn only in this mask
 * @param style pointer to a style
 */
void ug_draw_rect(const ug_area_t * coords, const ug_area_t * mask, ug_draw_rect_dsc_t * dsc);




#endif //!__UG_DRAW_RECT_H__
