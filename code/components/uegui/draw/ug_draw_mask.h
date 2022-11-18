/**
 * @file ug_mask.h
 *
 */

#ifndef UG_MASK_H
#define UG_MASK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include "ug_area.h"
#include "ug_color.h"
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#define UG_MASK_ID_INV  (-1)
#define _UG_MASK_MAX_NUM     16

/**********************
 *      TYPEDEFS
 **********************/

enum {
    UG_DRAW_MASK_RES_TRANSP,
    UG_DRAW_MASK_RES_FULL_COVER,
    UG_DRAW_MASK_RES_CHANGED,
    UG_DRAW_MASK_RES_UNKNOWN
};

typedef uint8_t ug_draw_mask_res_t;


enum {
    UG_DRAW_MASK_TYPE_LINE,
    UG_DRAW_MASK_TYPE_ANGLE,
    UG_DRAW_MASK_TYPE_RADIUS,
    UG_DRAW_MASK_TYPE_FADE,
    UG_DRAW_MASK_TYPE_MAP,
};

typedef uint8_t ug_draw_mask_type_t;

enum {
    UG_DRAW_MASK_LINE_SIDE_LEFT = 0,
    UG_DRAW_MASK_LINE_SIDE_RIGHT,
    UG_DRAW_MASK_LINE_SIDE_TOP,
    UG_DRAW_MASK_LINE_SIDE_BOTTOM,
};
#if 0
/**
 * A common callback type for every mask type.
 * Used internally by the library.
 */
typedef ug_draw_mask_res_t (*ug_draw_mask_xcb_t)(ug_opa_t * mask_buf, ug_coord_t abs_x, ug_coord_t abs_y,
                                                 ug_coord_t len,
                                                 void * p);

typedef uint8_t ug_draw_mask_line_side_t;

typedef struct {
    ug_draw_mask_xcb_t cb;
    ug_draw_mask_type_t type;
} ug_draw_mask_common_dsc_t;

typedef struct {
    /*The first element must be the common descriptor*/
    ug_draw_mask_common_dsc_t dsc;

    struct {
        /*First point */
        ug_point_t p1;

        /*Second point*/
        ug_point_t p2;

        /*Which side to keep?*/
        ug_draw_mask_line_side_t side : 2;
    } cfg;

    /*A point of the line*/
    ug_point_t origo;

    /* X / (1024*Y) steepness (X is 0..1023 range). What is the change of X in 1024 Y?*/
    int32_t xy_steep;

    /* Y / (1024*X) steepness (Y is 0..1023 range). What is the change of Y in 1024 X?*/
    int32_t yx_steep;

    /*Helper which stores yx_steep for flat lines and xy_steep for steep (non flat) lines */
    int32_t steep;

    /*Steepness in 1 px in 0..255 range. Used only by flat lines. */
    int32_t spx;

    /*1: It's a flat line? (Near to horizontal)*/
    uint8_t flat : 1;


    /* Invert the mask. The default is: Keep the left part.
     * It is used to select left/right/top/bottom*/
    uint8_t inv: 1;
} ug_draw_mask_line_param_t;

typedef struct {
    /*The first element must be the common descriptor*/
    ug_draw_mask_common_dsc_t dsc;

    struct {
        ug_point_t vertex_p;
        ug_coord_t start_angle;
        ug_coord_t end_angle;
    } cfg;

    ug_draw_mask_line_param_t start_line;
    ug_draw_mask_line_param_t end_line;
    uint16_t delta_deg;
} ug_draw_mask_angle_param_t;

typedef struct {
    /*The first element must be the common descriptor*/
    ug_draw_mask_common_dsc_t dsc;

    struct {
        ug_area_t rect;
        ug_coord_t radius;
        /* Invert the mask. 0: Keep the pixels inside.*/
        uint8_t outer: 1;
    } cfg;
    int32_t y_prev;
    ug_sqrt_res_t y_prev_x;

} ug_draw_mask_radius_param_t;

typedef struct {
    /*The first element must be the common descriptor*/
    ug_draw_mask_common_dsc_t dsc;

    struct {
        ug_area_t coords;
        ug_coord_t y_top;
        ug_coord_t y_bottom;
        ug_opa_t opa_top;
        ug_opa_t opa_bottom;
    } cfg;

} ug_draw_mask_fade_param_t;

typedef struct _ug_draw_mask_map_param_t {
    /*The first element must be the common descriptor*/
    ug_draw_mask_common_dsc_t dsc;

    struct {
        ug_area_t coords;
        const ug_opa_t * map;
    } cfg;
} ug_draw_mask_map_param_t;

typedef struct {
    void * param;
    void * custom_id;
} _ug_draw_mask_saved_t;

typedef _ug_draw_mask_saved_t _ug_draw_mask_saved_arr_t[_UG_MASK_MAX_NUM];

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Add a draw mask. Everything drawn after it (until removing the mask) will be affected by the mask.
 * @param param an initialized mask parameter. Only the pointer is saved.
 * @param custom_id a custom pointer to identify the mask. Used in `ug_draw_mask_remove_custom`.
 * @return the an integer, the ID of the mask. Can be used in `ug_draw_mask_remove_id`.
 */
int16_t ug_draw_mask_add(void * param, void * custom_id);


//! @cond Doxygen_Suppress

/**
 * Apply the added buffers on a line. Used internally by the library's drawing routines.
 * @param mask_buf store the result mask here. Has to be `len` byte long. Should be initialized with `0xFF`.
 * @param abs_x absolute X coordinate where the line to calculate start
 * @param abs_y absolute Y coordinate where the line to calculate start
 * @param len length of the line to calculate (in pixel count)
 * @return One of these values:
 * - `UG_DRAW_MASK_RES_FULL_TRANSP`: the whole line is transparent. `mask_buf` is not set to zero
 * - `UG_DRAW_MASK_RES_FULL_COVER`: the whole line is fully visible. `mask_buf` is unchanged
 * - `UG_DRAW_MASK_RES_CHANGED`: `mask_buf` has changed, it shows the desired opacity of each pixel in the given line
 */
UG_ATTRIBUTE_FAST_MEM ug_draw_mask_res_t ug_draw_mask_apply(ug_opa_t * mask_buf, ug_coord_t abs_x, ug_coord_t abs_y,
                                                            ug_coord_t len);


//! @endcond

/**
 * Remove a mask with a given ID
 * @param id the ID of the mask.  Returned by `ug_draw_mask_add`
 * @return the parameter of the removed mask.
 * If more masks have `custom_id` ID then the last mask's parameter will be returned
 */
void * ug_draw_mask_remove_id(int16_t id);

/**
 * Remove all mask with a given custom ID
 * @param custom_id a pointer used in `ug_draw_mask_add`
 * @return return the parameter of the removed mask.
 * If more masks have `custom_id` ID then the last mask's parameter will be returned
 */
void * ug_draw_mask_remove_custom(void * custom_id);

//! @cond Doxygen_Suppress

/**
 * Count the currently added masks
 * @return number of active masks
 */
UG_ATTRIBUTE_FAST_MEM uint8_t ug_draw_mask_get_cnt(void);


//! @endcond

/**
 *Initialize a line mask from two points.
 * @param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param p1x X coordinate of the first point of the line
 * @param p1y Y coordinate of the first point of the line
 * @param p2x X coordinate of the second point of the line
 * @param p2y y coordinate of the second point of the line
 * @param side and element of `ug_draw_mask_line_side_t` to describe which side to keep.
 * With `UG_DRAW_MASK_LINE_SIDE_LEFT/RIGHT` and horizontal line all pixels are kept
 * With `UG_DRAW_MASK_LINE_SIDE_TOP/BOTTOM` and vertical line all pixels are kept
 */
void ug_draw_mask_line_points_init(ug_draw_mask_line_param_t * param, ug_coord_t p1x, ug_coord_t p1y, ug_coord_t p2x,
                                   ug_coord_t p2y, ug_draw_mask_line_side_t side);

/**
 *Initialize a line mask from a point and an angle.
 * @param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param px X coordinate of a point of the line
 * @param py X coordinate of a point of the line
 * @param angle right 0 deg, bottom: 90
 * @param side and element of `ug_draw_mask_line_side_t` to describe which side to keep.
 * With `UG_DRAW_MASK_LINE_SIDE_LEFT/RIGHT` and horizontal line all pixels are kept
 * With `UG_DRAW_MASK_LINE_SIDE_TOP/BOTTOM` and vertical line all pixels are kept
 */
void ug_draw_mask_line_angle_init(ug_draw_mask_line_param_t * param, ug_coord_t p1x, ug_coord_t py, int16_t angle,
                                  ug_draw_mask_line_side_t side);

/**
 * Initialize an angle mask.
 * @param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param vertex_x X coordinate of the angle vertex (absolute coordinates)
 * @param vertex_y Y coordinate of the angle vertex (absolute coordinates)
 * @param start_angle start angle in degrees. 0 deg on the right, 90 deg, on the bottom
 * @param end_angle end angle
 */
void ug_draw_mask_angle_init(ug_draw_mask_angle_param_t * param, ug_coord_t vertex_x, ug_coord_t vertex_y,
                             ug_coord_t start_angle, ug_coord_t end_angle);

/**
 * Initialize a fade mask.
 * @param param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param rect coordinates of the rectangle to affect (absolute coordinates)
 * @param radius radius of the rectangle
 * @param inv: true: keep the pixels inside the rectangle; keep the pixels outside of the rectangle
 */
void ug_draw_mask_radius_init(ug_draw_mask_radius_param_t * param, const ug_area_t * rect, ug_coord_t radius, bool inv);

/**
 * Initialize a fade mask.
 * @param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param coords coordinates of the area to affect (absolute coordinates)
 * @param opa_top opacity on the top
 * @param y_top at which coordinate start to change to opacity to `opa_bottom`
 * @param opa_bottom opacity at the bottom
 * @param y_bottom at which coordinate reach `opa_bottom`.
 */
void ug_draw_mask_fade_init(ug_draw_mask_fade_param_t * param, const ug_area_t * coords, ug_opa_t opa_top,
                            ug_coord_t y_top,
                            ug_opa_t opa_bottom, ug_coord_t y_bottom);

/**
 * Initialize a map mask.
 * @param param pointer to a `ug_draw_mask_param_t` to initialize
 * @param coords coordinates of the map (absolute coordinates)
 * @param map array of bytes with the mask values
 */
void ug_draw_mask_map_init(ug_draw_mask_map_param_t * param, const ug_area_t * coords, const ug_opa_t * map);

#endif
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UG_MASK_H*/
