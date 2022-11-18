#include "ug_area.h"
#include "ug_math.h"


/**********************
 *  STATIC PROTOTYPES
 **********************/

static bool ug_point_within_circle(const ug_area_t * area, const ug_point_t * p);


/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Initialize an area
 * @param area_p pointer to an area
 * @param x1 left coordinate of the area
 * @param y1 top coordinate of the area
 * @param x2 right coordinate of the area
 * @param y2 bottom coordinate of the area
 */
void ug_area_set(ug_area_t * area_p, ug_coord_t x1, ug_coord_t y1, ug_coord_t x2, ug_coord_t y2)
{
    area_p->x1 = x1;
    area_p->y1 = y1;
    area_p->x2 = x2;
    area_p->y2 = y2;
}

/**
 * Set the width of an area
 * @param area_p pointer to an area
 * @param w the new width of the area (w == 1 makes x1 == x2)
 */
void ug_area_set_width(ug_area_t * area_p, ug_coord_t w)
{
    area_p->x2 = area_p->x1 + w - 1;
}

/**
 * Set the height of an area
 * @param area_p pointer to an area
 * @param h the new height of the area (h == 1 makes y1 == y2)
 */
void ug_area_set_height(ug_area_t * area_p, ug_coord_t h)
{
    area_p->y2 = area_p->y1 + h - 1;
}

/**
 * Set the position of an area (width and height will be kept)
 * @param area_p pointer to an area
 * @param x the new x coordinate of the area
 * @param y the new y coordinate of the area
 */
void _ug_area_set_pos(ug_area_t * area_p, ug_coord_t x, ug_coord_t y)
{
    ug_coord_t w = ug_area_get_width(area_p);
    ug_coord_t h = ug_area_get_height(area_p);
    area_p->x1   = x;
    area_p->y1   = y;
    ug_area_set_width(area_p, w);
    ug_area_set_height(area_p, h);
}

/**
 * Return with area of an area (x * y)
 * @param area_p pointer to an area
 * @return size of area
 */
uint32_t ug_area_get_size(const ug_area_t * area_p)
{
    uint32_t size;

    size = (uint32_t)(area_p->x2 - area_p->x1 + 1) * (area_p->y2 - area_p->y1 + 1);

    return size;
}

/**
 * Get the common parts of two areas
 * @param res_p pointer to an area, the result will be stored here
 * @param a1_p pointer to the first area
 * @param a2_p pointer to the second area
 * @return false: the two area has NO common parts, res_p is invalid
 */
bool _ug_area_intersect(ug_area_t * res_p, const ug_area_t * a1_p, const ug_area_t * a2_p)
{
    /* Get the smaller area from 'a1_p' and 'a2_p' */
    res_p->x1 = UG_MATH_MAX(a1_p->x1, a2_p->x1);
    res_p->y1 = UG_MATH_MAX(a1_p->y1, a2_p->y1);
    res_p->x2 = UG_MATH_MIN(a1_p->x2, a2_p->x2);
    res_p->y2 = UG_MATH_MIN(a1_p->y2, a2_p->y2);

    /*If x1 or y1 greater then x2 or y2 then the areas union is empty*/
    bool union_ok = true;
    if((res_p->x1 > res_p->x2) || (res_p->y1 > res_p->y2)) {
        union_ok = false;
    }

    return union_ok;
}
/**
 * Join two areas into a third which involves the other two
 * @param res_p pointer to an area, the result will be stored here
 * @param a1_p pointer to the first area
 * @param a2_p pointer to the second area
 */
void _ug_area_join(ug_area_t * a_res_p, const ug_area_t * a1_p, const ug_area_t * a2_p)
{
    a_res_p->x1 = UG_MATH_MIN(a1_p->x1, a2_p->x1);
    a_res_p->y1 = UG_MATH_MIN(a1_p->y1, a2_p->y1);
    a_res_p->x2 = UG_MATH_MAX(a1_p->x2, a2_p->x2);
    a_res_p->y2 = UG_MATH_MAX(a1_p->y2, a2_p->y2);
}

/**
 * Check if a point is on an area
 * @param a_p pointer to an area
 * @param p_p pointer to a point
 * @param radius radius of area (e.g. for rounded rectangle)
 * @return false:the point is out of the area
 */
bool _ug_area_is_point_on(const ug_area_t * a_p, const ug_point_t * p_p, ug_coord_t radius)
{
    /*First check the basic area*/
    bool is_on_rect = false;
    if((p_p->x >= a_p->x1 && p_p->x <= a_p->x2) && ((p_p->y >= a_p->y1 && p_p->y <= a_p->y2))) {
        is_on_rect = true;
    }
    if(!is_on_rect)
        return false;
    /*Now handle potential rounded rectangles*/
    if(radius <= 0) {
        /*No radius, it is within the rectangle*/
        return true;
    }
    ug_coord_t w = ug_area_get_width(a_p) / 2;
    ug_coord_t h = ug_area_get_height(a_p) / 2;
    ug_coord_t max_radius = UG_MATH_MIN(w, h);
    if(radius > max_radius)
        radius = max_radius;

    /*Check if it's in one of the corners*/
    ug_area_t corner_area;
    /*Top left*/
    corner_area.x1 = a_p->x1;
    corner_area.x2 = a_p->x1 + radius;
    corner_area.y1 = a_p->y1;
    corner_area.y2 = a_p->y1 + radius;
    if(_ug_area_is_point_on(&corner_area, p_p, 0)) {
        corner_area.x2 += radius;
        corner_area.y2 += radius;
        return ug_point_within_circle(&corner_area, p_p);
    }
    /*Bottom left*/
    corner_area.y1 = a_p->y2 - radius;
    corner_area.y2 = a_p->y2;
    if(_ug_area_is_point_on(&corner_area, p_p, 0)) {
        corner_area.x2 += radius;
        corner_area.y1 -= radius;
        return ug_point_within_circle(&corner_area, p_p);
    }
    /*Bottom right*/
    corner_area.x1 = a_p->x2 - radius;
    corner_area.x2 = a_p->x2;
    if(_ug_area_is_point_on(&corner_area, p_p, 0)) {
        corner_area.x1 -= radius;
        corner_area.y1 -= radius;
        return ug_point_within_circle(&corner_area, p_p);
    }
    /*Top right*/
    corner_area.y1 = a_p->y1;
    corner_area.y2 = a_p->y1 + radius;
    if(_ug_area_is_point_on(&corner_area, p_p, 0)) {
        corner_area.x1 -= radius;
        corner_area.y2 += radius;
        return ug_point_within_circle(&corner_area, p_p);
    }
    /*Not within corners*/
    return true;
}

/**
 * Check if two area has common parts
 * @param a1_p pointer to an area.
 * @param a2_p pointer to an other area
 * @return false: a1_p and a2_p has no common parts
 */
bool _ug_area_is_on(const ug_area_t * a1_p, const ug_area_t * a2_p)
{
    if((a1_p->x1 <= a2_p->x2) && (a1_p->x2 >= a2_p->x1) && (a1_p->y1 <= a2_p->y2) && (a1_p->y2 >= a2_p->y1)) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Check if an area is fully on an other
 * @param ain_p pointer to an area which could be in 'aholder_p'
 * @param aholder_p pointer to an area which could involve 'ain_p'
 * @param radius radius of `aholder_p` (e.g. for rounded rectangle)
 * @return true: `ain_p` is fully inside `aholder_p`
 */
bool _ug_area_is_in(const ug_area_t * ain_p, const ug_area_t * aholder_p, ug_coord_t radius)
{
    bool is_in = false;

    if(ain_p->x1 >= aholder_p->x1 && ain_p->y1 >= aholder_p->y1 && ain_p->x2 <= aholder_p->x2 &&
       ain_p->y2 <= aholder_p->y2) {
        is_in = true;
    }

    if(radius == 0) return is_in;

    /*Check if the corner points are inside the radius or not*/
    ug_point_t p;

    p.x = ain_p->x1;
    p.y = ain_p->y1;
    if(_ug_area_is_point_on(aholder_p, &p, radius) == false) return false;

    p.x = ain_p->x2;
    p.y = ain_p->y1;
    if(_ug_area_is_point_on(aholder_p, &p, radius) == false) return false;

    p.x = ain_p->x1;
    p.y = ain_p->y2;
    if(_ug_area_is_point_on(aholder_p, &p, radius) == false) return false;

    p.x = ain_p->x2;
    p.y = ain_p->y2;
    if(_ug_area_is_point_on(aholder_p, &p, radius) == false) return false;

    return true;
}

/**
 * Align an area to an other
 * @param base an are where the other will be aligned
 * @param to_align the area to align
 * @param align `UG_ALIGN_...`
 * @param res x/y coordinates where `to_align` align area should be placed
 */
void _ug_area_align(const ug_area_t * base, const ug_area_t * to_align, ug_align_t align, ug_point_t * res)
{

    switch(align) {
        case UG_ALIGN_CENTER:
            res->x = ug_area_get_width(base) / 2 - ug_area_get_width(to_align) / 2;
            res->y = ug_area_get_height(base) / 2 - ug_area_get_height(to_align) / 2;
            break;

        case UG_ALIGN_IN_TOP_LEFT:
            res->x = 0;
            res->y = 0;
            break;
        case UG_ALIGN_IN_TOP_MID:
            res->x = ug_area_get_width(base) / 2 - ug_area_get_width(to_align) / 2;
            res->y = 0;
            break;

        case UG_ALIGN_IN_TOP_RIGHT:
            res->x = ug_area_get_width(base) - ug_area_get_width(to_align);
            res->y = 0;
            break;

        case UG_ALIGN_IN_BOTTOM_LEFT:
            res->x = 0;
            res->y = ug_area_get_height(base) - ug_area_get_height(to_align);
            break;
        case UG_ALIGN_IN_BOTTOM_MID:
            res->x = ug_area_get_width(base) / 2 - ug_area_get_width(to_align) / 2;
            res->y = ug_area_get_height(base) - ug_area_get_height(to_align);
            break;

        case UG_ALIGN_IN_BOTTOM_RIGHT:
            res->x = ug_area_get_width(base) - ug_area_get_width(to_align);
            res->y = ug_area_get_height(base) - ug_area_get_height(to_align);
            break;

        case UG_ALIGN_IN_LEFT_MID:
            res->x = 0;
            res->y = ug_area_get_height(base) / 2 - ug_area_get_height(to_align) / 2;
            break;

        case UG_ALIGN_IN_RIGHT_MID:
            res->x = ug_area_get_width(base) - ug_area_get_width(to_align);
            res->y = ug_area_get_height(base) / 2 - ug_area_get_height(to_align) / 2;
            break;

        case UG_ALIGN_OUT_TOP_LEFT:
            res->x = 0;
            res->y = -ug_area_get_height(to_align);
            break;

        case UG_ALIGN_OUT_TOP_MID:
            res->x = ug_area_get_width(base) / 2 - ug_area_get_width(to_align) / 2;
            res->y = -ug_area_get_height(to_align);
            break;

        case UG_ALIGN_OUT_TOP_RIGHT:
            res->x = ug_area_get_width(base) - ug_area_get_width(to_align);
            res->y = -ug_area_get_height(to_align);
            break;

        case UG_ALIGN_OUT_BOTTOM_LEFT:
            res->x = 0;
            res->y = ug_area_get_height(base);
            break;

        case UG_ALIGN_OUT_BOTTOM_MID:
            res->x = ug_area_get_width(base) / 2 - ug_area_get_width(to_align) / 2;
            res->y = ug_area_get_height(base);
            break;

        case UG_ALIGN_OUT_BOTTOM_RIGHT:
            res->x = ug_area_get_width(base) - ug_area_get_width(to_align);
            res->y = ug_area_get_height(base);
            break;

        case UG_ALIGN_OUT_LEFT_TOP:
            res->x = -ug_area_get_width(to_align);
            res->y = 0;
            break;

        case UG_ALIGN_OUT_LEFT_MID:
            res->x = -ug_area_get_width(to_align);
            res->y = ug_area_get_height(base) / 2 - ug_area_get_height(to_align) / 2;
            break;

        case UG_ALIGN_OUT_LEFT_BOTTOM:
            res->x = -ug_area_get_width(to_align);
            res->y = ug_area_get_height(base) - ug_area_get_height(to_align);
            break;

        case UG_ALIGN_OUT_RIGHT_TOP:
            res->x = ug_area_get_width(base);
            res->y = 0;
            break;

        case UG_ALIGN_OUT_RIGHT_MID:
            res->x = ug_area_get_width(base);
            res->y = ug_area_get_height(base) / 2 - ug_area_get_height(to_align) / 2;
            break;

        case UG_ALIGN_OUT_RIGHT_BOTTOM:
            res->x = ug_area_get_width(base);
            res->y = ug_area_get_height(base) - ug_area_get_height(to_align);
            break;
    }

    res->x += base->x1;
    res->y += base->y1;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static bool ug_point_within_circle(const ug_area_t * area, const ug_point_t * p)
{
    ug_coord_t r = (area->x2 - area->x1) / 2;

    /* Circle center */
    ug_coord_t cx = area->x1 + r;
    ug_coord_t cy = area->y1 + r;

    /*Simplify the code by moving everything to (0, 0) */
    ug_coord_t px = p->x - cx;
    ug_coord_t py = p->y - cy;

    int32_t r_sqrd = r * r;
    int32_t dist = (px * px) + (py * py);

    if(dist <= r_sqrd)
        return true;
    else
        return false;
}



