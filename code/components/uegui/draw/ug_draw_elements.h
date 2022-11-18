/*************************************************
 * @brief a basic element draw func
 * @date 2020
 * @author firestaradmin
*************************************************/


#ifndef __UG_DRAW_ELEMENTS_H__
#define __UG_DRAW_ELEMENTS_H__
#include "ug_area.h"
#include "ug_user_config.h"
#include "ug_type.h"

#include "ug_color.h"

/* Represents a point  */
typedef ug_point_t ug_point_t;

/* Represents a line  */
typedef struct {
    ug_point_t head;
    ug_point_t tail;
} ug_line_t;

/* Represents a wide line  */
typedef struct {
    uint8_t width;
    ug_point_t head;
    ug_point_t tail;
} ug_wline_t;

/* Represents a triangle  */
typedef struct {
    uint8_t width;
    ug_point_t point0;
    ug_point_t point1;
    ug_point_t point2;
} ug_triangle_t;

/* Represents a rectangle  */
typedef struct {
    uint8_t width;
    ug_point_t point0;
    ug_point_t point1;
} ug_rectanlge_t;

typedef enum {
    UG_CIRCLE_PART_LOWRIGHT = 0,
    UG_CIRCLE_PART_LOWLEFT,
    UG_CIRCLE_PART_UPRIGHT,
    UG_CIRCLE_PART_UPLEFT,
}ug_circle_part_t;


/******************* ug_draw_elements API ******************************/

void ug_drawPoint(uint16_t x, uint16_t y, ug_color_t color);

/* Line */
void ug_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, ug_color_t color);
void ug_drawWideLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t with, ug_color_t color);

/* Rectangle */
void ug_drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, ug_color_t color);
void ug_fillRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,ug_color_t color);

/* RoundRectangle */
void ug_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r,ug_color_t color) ;

/* Circle */
void ug_drawCircle(int16_t x0, int16_t y0, int16_t r,ug_color_t color) ;
void ug_fillCircle(int16_t x0, int16_t y0, int16_t r,ug_color_t color);
void ug_drawQuarterCircle(int16_t x0, int16_t y0, int16_t r, ug_circle_part_t part,ug_color_t color) ;

/* Tirangle */
void ug_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2,ug_color_t color)  ;



#endif // !__UG_DRAW_ELEMENTS_H__
