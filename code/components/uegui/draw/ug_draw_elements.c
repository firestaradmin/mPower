/*************************************************
 * @brief a basic element draw func
 * @date 2020
 * @author firestaradmin
*************************************************/


#include "ug_draw_elements.h"
#include "ug_disp.h"
#include "ug_mem.h"




/* ug_Private_Fun */
static void _ug_fillCircle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, int16_t delta, ug_color_t color);
static void _ug_drawCircle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, ug_color_t color);
static void _ug_drawVLine(int16_t x, int16_t y, int16_t length, ug_color_t color);
static void _ug_drawHLine(int16_t x, int16_t y, int16_t length, ug_color_t color);
static void _ug_writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, ug_color_t color);
void _ug_drawPoint(uint16_t x, uint16_t y, ug_color_t color, ug_disp_t *disp);


void ug_drawPoint(uint16_t x, uint16_t y, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    if(x > disp->driver.hor_res || y > disp->driver.ver_res) return;
    *((ug_color_t *)(disp->driver.buffer->buf_act) + (y * disp->driver.hor_res) + x) = color;
}

inline void _ug_drawPoint(uint16_t x, uint16_t y, ug_color_t color, ug_disp_t *disp)
{
    *((ug_color_t *)(disp->driver.buffer->buf_act) + (y * disp->driver.hor_res) + x) = color;
}

void ug_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, ug_color_t color)
{
    
    // Update in subclasses if desired!
    if (x0 == x1)
    {
        if (y0 > y1)
            SWAP(y0, y1);
        _ug_drawVLine(x0, y0, y1 - y0, color);
    }
    else if (y0 == y1)
    {
        if (x0 > x1)
            SWAP(x0, x1);
        _ug_drawHLine(x0, y0, x1 - x0, color);
    }
    else
        _ug_writeLine(x0, y0, x1, y1, color);
}

void ug_drawWideLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t with, ug_color_t color)
{

    int32_t dx;     // 直线x轴差值变量
    int32_t dy;     // 直线y轴差值变量
    int16_t dx_sym; // x轴增长方向，为-1时减值方向，为1时增值方向
    int16_t dy_sym; // y轴增长方向，为-1时减值方向，为1时增值方向
    int32_t dx_x2;  // dx*2值变量，用于加快运算速度
    int32_t dy_x2;  // dy*2值变量，用于加快运算速度
    int32_t di;     // 决策变量

    int32_t wx, wy; // 线宽变量
    int32_t draw_a, draw_b;

    /* 参数过滤 */
    if (with == 0)
        return;
    if (with > 50)
        with = 50;

    dx = x1 - x0; // 求取两点之间的差值
    dy = y1 - y0;

    wx = with / 2;
    wy = with - wx - 1;

    /* 判断增长方向，或是否为水平线、垂直线、点 */
    if (dx > 0) // 判断x轴方向
    {
        dx_sym = 1; // dx>0，设置dx_sym=1
    }
    else
    {
        if (dx < 0)
        {
            dx_sym = -1; // dx<0，设置dx_sym=-1
        }
        else
        { /* dx==0，画垂直线，或一点 */
            wx = x0 - wx;
            if (wx < 0)
                wx = 0;
            wy = x0 + wy;

            while (1)
            {
                x0 = wx;

                if (y0 > y1)
                    SWAP(y0, y1);
                _ug_drawVLine(x0, y0, y1 - y0, color);
                if (wx >= wy)
                    break;
                wx++;
            }

            return;
        }
    }

    if (dy > 0) // 判断y轴方向
    {
        dy_sym = 1; // dy>0，设置dy_sym=1
    }
    else
    {
        if (dy < 0)
        {
            dy_sym = -1; // dy<0，设置dy_sym=-1
        }
        else
        { /* dy==0，画水平线，或一点 */
            wx = y0 - wx;
            if (wx < 0)
                wx = 0;
            wy = y0 + wy;

            while (1)
            {
                y0 = wx;
                if (x0 > x1)
                    SWAP(x0, x1);
                _ug_drawHLine(x0, y0, x1 - x0, color);
                if (wx >= wy)
                    break;
                wx++;
            }
            return;
        }
    }

    /* 将dx、dy取绝对值 */
    dx = dx_sym * dx;
    dy = dy_sym * dy;

    /* 计算2倍的dx及dy值 */
    dx_x2 = dx * 2;
    dy_x2 = dy * 2;

    /* 使用Bresenham法进行画直线 */
    if (dx >= dy) // 对于dx>=dy，则使用x轴为基准
    {
        di = dy_x2 - dx;
        while (x0 != x1)
        { /* x轴向增长，则宽度在y方向，即画垂直线 */
            draw_a = y0 - wx;
            if (draw_a < 0)
                draw_a = 0;
            draw_b = y0 + wy;
            if (draw_a > draw_b)
                SWAP(draw_a, draw_b);
            _ug_drawVLine(x0, draw_a, draw_b - draw_a, color);

            x0 += dx_sym;
            if (di < 0)
            {
                di += dy_x2; // 计算出下一步的决策值
            }
            else
            {
                di += dy_x2 - dx_x2;
                y0 += dy_sym;
            }
        }
        draw_a = y0 - wx;
        if (draw_a < 0)
            draw_a = 0;
        draw_b = y0 + wy;
        if (draw_a > draw_b)
            SWAP(draw_a, draw_b);
        _ug_drawVLine(x0, draw_a, draw_b - draw_a, color);
    }
    else // 对于dx<dy，则使用y轴为基准
    {
        di = dx_x2 - dy;
        while (y0 != y1)
        { /* y轴向增长，则宽度在x方向，即画水平线 */
            draw_a = x0 - wx;
            if (draw_a < 0)
                draw_a = 0;
            draw_b = x0 + wy;
            if (draw_a > draw_b)
                SWAP(draw_a, draw_b);
            _ug_drawHLine(draw_a, y0, draw_b - draw_a, color);

            y0 += dy_sym;
            if (di < 0)
            {
                di += dx_x2;
            }
            else
            {
                di += dx_x2 - dy_x2;
                x0 += dx_sym;
            }
        }
        draw_a = x0 - wx;
        if (draw_a < 0)
            draw_a = 0;
        draw_b = x0 + wy;
        if (draw_a > draw_b)
            SWAP(draw_a, draw_b);
        _ug_drawHLine(draw_a, y0, draw_b - draw_a, color);
    }
}


// Draw a rectangle
void ug_drawRectangle(int16_t x, int16_t y, int16_t w, int16_t h, ug_color_t color)
{

    _ug_drawHLine(x, y, w, color);
    _ug_drawHLine(x, y + h, w, color);
    _ug_drawVLine(x, y, h, color);
    _ug_drawVLine(x + w, y, h, color);
}

void ug_fillRectangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2,ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
	for(int16_t x = x1; x <= x2; x++){
		for(int16_t y = y1; y <= y2; y ++){
            _ug_drawPoint(x, y, color, disp);//画点 
		}
	}
}




void ug_fillCircle(int16_t x0, int16_t y0, int16_t r, ug_color_t color)
{

    _ug_drawVLine(x0, y0 - r, 2 * r, color);
    _ug_fillCircle_Helper(x0, y0, r, 3, 0, color);
}


// Draw a circle outline
void ug_drawCircle(int16_t x0, int16_t y0, int16_t r, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    _ug_drawPoint(x0, y0 + r, color, disp);
    _ug_drawPoint(x0, y0 - r, color, disp);
    _ug_drawPoint(x0 + r, y0, color, disp);
    _ug_drawPoint(x0 - r, y0, color, disp);

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        _ug_drawPoint(x0 + x, y0 + y, color, disp);
        _ug_drawPoint(x0 - x, y0 + y, color, disp);
        _ug_drawPoint(x0 + x, y0 - y, color, disp);
        _ug_drawPoint(x0 - x, y0 - y, color, disp);
        _ug_drawPoint(x0 + y, y0 + x, color, disp);
        _ug_drawPoint(x0 - y, y0 + x, color, disp);
        _ug_drawPoint(x0 + y, y0 - x, color, disp);
        _ug_drawPoint(x0 - y, y0 - x, color, disp);
    }
}

void ug_drawQuarterCircle(int16_t x0, int16_t y0, int16_t r, ug_circle_part_t part, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;
    switch (part)
    {
    case UG_CIRCLE_PART_LOWRIGHT:
        _ug_drawPoint(x0, y0 + r, color, disp);
        _ug_drawPoint(x0 + r, y0, color, disp);
        break; //右下
    case UG_CIRCLE_PART_LOWLEFT:
        _ug_drawPoint(x0, y0 + r, color, disp);
        _ug_drawPoint(x0 - r, y0, color, disp);
        break; //左下
    case UG_CIRCLE_PART_UPRIGHT:
        _ug_drawPoint(x0, y0 - r, color, disp);
        _ug_drawPoint(x0 + r, y0, color, disp);
        break; //右上
    case UG_CIRCLE_PART_UPLEFT:
        _ug_drawPoint(x0 - r, y0, color, disp);
        _ug_drawPoint(x0, y0 - r, color, disp);
        break; //左上
    }

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        switch (part)
        {
        case UG_CIRCLE_PART_LOWRIGHT:
            _ug_drawPoint(x0 + x, y0 + y, color, disp);
            _ug_drawPoint(x0 + y, y0 + x, color, disp);
            break;
        case UG_CIRCLE_PART_LOWLEFT:
            _ug_drawPoint(x0 - x, y0 + y, color, disp);
            _ug_drawPoint(x0 - y, y0 + x, color, disp);
            break;
        case UG_CIRCLE_PART_UPRIGHT:
            _ug_drawPoint(x0 + x, y0 - y, color, disp);
            _ug_drawPoint(x0 + y, y0 - x, color, disp);
            break;
        case UG_CIRCLE_PART_UPLEFT:
            _ug_drawPoint(x0 - y, y0 - x, color, disp);
            _ug_drawPoint(x0 - x, y0 - y, color, disp);
            break;
        }
    }
}


// Draw a rounded rectangle
void ug_drawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, ug_color_t color)
{
    // smarter version

    _ug_drawHLine(x + r, y, w - 2 * r, color);         // Top
    _ug_drawHLine(x + r, y + h - 1, w - 2 * r, color); // Bottom
    _ug_drawVLine(x, y + r, h - 2 * r, color);         // Left
    _ug_drawVLine(x + w - 1, y + r, h - 2 * r, color); // Right
    // draw four corners
    _ug_drawCircle_Helper(x + r, y + r, r, 1, color);
    _ug_drawCircle_Helper(x + w - r - 1, y + r, r, 2, color);
    _ug_drawCircle_Helper(x + w - r - 1, y + h - r - 1, r, 4, color);
    _ug_drawCircle_Helper(x + r, y + h - r - 1, r, 8, color);
}


// Draw a triangle
void ug_drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, ug_color_t color)
{
    ug_drawLine(x0, y0, x1, y1, color);
    ug_drawLine(x1, y1, x2, y2, color);
    ug_drawLine(x2, y2, x0, y0, color);
}


static void _ug_drawCircle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        if (corner & 0x4)
        {
            _ug_drawPoint(x0 + x, y0 + y, color, disp);
            _ug_drawPoint(x0 + y, y0 + x, color, disp);
        }
        if (corner & 0x2)
        {
            _ug_drawPoint(x0 + x, y0 - y, color, disp);
            _ug_drawPoint(x0 + y, y0 - x, color, disp);
        }
        if (corner & 0x8)
        {
            _ug_drawPoint(x0 - y, y0 + x, color, disp);
            _ug_drawPoint(x0 - x, y0 + y, color, disp);
        }
        if (corner & 0x1)
        {
            _ug_drawPoint(x0 - y, y0 - x, color, disp);
            _ug_drawPoint(x0 - x, y0 - y, color, disp);
        }
    }
}


static void _ug_drawHLine(int16_t x, int16_t y, int16_t length, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    // Bounds check
    int16_t x0 = x;
    do
    {
        _ug_drawPoint(x, y, color, disp); // 逐点显示，描出垂直线
        x++;
    } while (x0 + length >= x);
}

static void _ug_drawVLine(int16_t x, int16_t y, int16_t length, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    // Bounds check
    int16_t y0 = y;
    do
    {
        _ug_drawPoint(x, y, color, disp); // 逐点显示，描出垂直线
        y++;
    } while (y0 + length >= y);
}


static void _ug_writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, ug_color_t color)
{
    ug_disp_t *disp = ug_disp_get_actdisp();
    int16_t steep = UG_MATH_ABS(y1 - y0) > UG_MATH_ABS(x1 - x0);

    if (steep)
    {
        SWAP(x0, y0);
        SWAP(x1, y1);
    }
    if (x0 > x1)
    {
        SWAP(x0, x1);
        SWAP(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = UG_MATH_ABS(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1)
    {
        ystep = 1;
    }
    else
    {
        ystep = -1;
    }

    for (; x0 <= x1; x0++)
    {
        if (steep)
            _ug_drawPoint(y0, x0, color, disp);
        else
            _ug_drawPoint(x0, y0, color, disp);
        err -= dy;

        if (err < 0)
        {
            y0 += ystep;
            err += dx;
        }
    }
}


// Used to do circles and roundrects
static void _ug_fillCircle_Helper(int16_t x0, int16_t y0, int16_t r, uint8_t corner, int16_t delta, ug_color_t color)
{

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        if (corner & 0x1)
        {
            _ug_drawVLine(x0 + x, y0 - y, 2 * y + delta, color);
            _ug_drawVLine(x0 + y, y0 - x, 2 * x + delta, color);
        }
        if (corner & 0x2)
        {
            _ug_drawVLine(x0 - x, y0 - y, 2 * y + delta, color);
            _ug_drawVLine(x0 - y, y0 - x, 2 * x + delta, color);
        }
    }
}

