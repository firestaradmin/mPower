/**
 * @file ug_draw_blend.h
 *
 */


#ifndef UG_DRAW_BLEND_H
#define UG_DRAW_BLEND_H


#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "ug_color.h"
#include "ug_area.h"
#include "ug_draw_mask.h"

UG_ATTRIBUTE_FAST_MEM void fill_normal(const ug_area_t * disp_area, ug_color_t * disp_buf,
                                              const ug_area_t * draw_area,
                                              ug_color_t color, ug_opa_t opa,
                                              const ug_opa_t * mask, ug_draw_mask_res_t mask_res);

//void _ug_fill_normal(ug_area_t * clip, ug_color_t color);

// /*********************
//  *      DEFINES
//  *********************/

// /**********************
//  *      TYPEDEFS
//  **********************/
// enum {
//     UG_BLEND_MODE_NORMAL,
//     UG_BLEND_MODE_ADDITIVE,
//     UG_BLEND_MODE_SUBTRACTIVE,
// };

// typedef uint8_t ug_blend_mode_t;

// /**********************
//  * GLOBAL PROTOTYPES
//  **********************/

// //! @cond Doxygen_Suppress
// UG_ATTRIBUTE_FAST_MEM void _ug_blend_fill(const ug_area_t * clip_area, const ug_area_t * fill_area, ug_color_t color,
//                                           ug_opa_t * mask, ug_draw_mask_res_t mask_res, ug_opa_t opa, ug_blend_mode_t mode);


// UG_ATTRIBUTE_FAST_MEM void _ug_blend_map(const ug_area_t * clip_area, const ug_area_t * map_area,
//                                          const ug_color_t * map_buf,
//                                          ug_opa_t * mask, ug_draw_mask_res_t mask_res, ug_opa_t opa, ug_blend_mode_t mode);

// //! @endcond
// /**********************
//  *      MACROS
//  **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UG_DRAW_BLEND_H*/
