
/*********************
 *      INCLUDES
 *********************/
#include "ug_draw_text.h"
#include "ug_draw_blend.h"
#include "ug_math.h"
#include "ug_refr.h"
#include "ug_font.h"

#include "stdio.h"

#include "ug_font_declare.h"

/**********************
 *  CONST TABLE
 **********************/
 /* This is for phrasing font which different bpps. */
const uint8_t _ug_bpp1_opa_table[2]  = {0, 255};          /*Opacity mapping with bpp = 1 (Just for compatibility)*/
const uint8_t _ug_bpp2_opa_table[4]  = {0, 85, 170, 255}; /*Opacity mapping with bpp = 2*/

const uint8_t _ug_bpp3_opa_table[8]  = {0, 36,  73, 109,   /*Opacity mapping with bpp = 3*/
                                        146, 182,  219, 255
                                       };

const uint8_t _ug_bpp4_opa_table[16] = {0,  17, 34,  51,  /*Opacity mapping with bpp = 4*/
                                        68, 85, 102, 119,
                                        136, 153, 170, 187,
                                        204, 221, 238, 255
                                       };
const uint8_t _ug_bpp8_opa_table[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                         16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
                                         32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
                                         48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
                                         64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
                                         80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
                                         96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
                                         112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
                                         128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
                                         144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
                                         160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
                                         176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
                                         192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
                                         208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
                                         224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
                                         240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255
                                        };

void _ug_draw_letter(const ug_point_t *pos_p,
                     const ug_font_t *font_p,
                     const ug_font_glyph_dsc_t *g,
                     const ug_color_t color,
                     const ug_color_t backColor,
                     const uint32_t letter,
                     const ug_area_t *disp_area,
                     const ug_area_t *draw_area,
                     ug_color_t *disp_buf);

static void _draw_letter_normal(ug_coord_t pos_x,
                                ug_coord_t pos_y,
                                ug_font_glyph_dsc_t *g,
                                ug_color_t color,
                                ug_color_t backColor,
                                const uint8_t *map_p,
                                const ug_area_t *disp_area,
                                const ug_area_t *draw_area,
                                ug_color_t *disp_buf);

void ug_draw_str(const ug_area_t * coords, const ug_font_t * font_p, ug_color_t color, ug_color_t backColor, const unsigned char* str, const ug_area_t *clip)
{
	if(str == NULL) return;
	
	uint32_t bytes_num = 0;
	uint32_t unicode = 0;
	uint16_t letter_adv_w = 0;
	/*character's coordinates*/
	ug_point_t pos = {
		.x = coords->x1,
		.y = coords->y1
	};
	
	ug_font_glyph_dsc_t g;
	bool g_ret;
	

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



	while(*str != '\0'){
		
		if(*str == '\t'){	/*if character is tab*/
			bytes_num = 1;
			g_ret = font_p->get_glyph_dsc(font_p, &g, ' ', '\0');
			if(g_ret == false) {
				str += bytes_num;
				letter_adv_w = 0;
				bytes_num = 0;
				continue;
			}
			letter_adv_w = g.adv_w * UG_TAB_SIZE; 
		}
		else if(*str == '\r' || *str == '\n'){	/*change line*/
			bytes_num = 1;
			pos.y +=  font_p->line_height;
			pos.x = coords->x1;
			
		}
		else{
			bytes_num = utf8_to_unicode(&unicode, str);
			g_ret = font_p->get_glyph_dsc(font_p, &g, unicode, '\0');
			if(g_ret == false) {
				str += bytes_num;
				letter_adv_w = 0;
				bytes_num = 0;
				continue;
			}
			letter_adv_w = g.adv_w; 
			
			_ug_draw_letter(&pos, font_p, &g, color, backColor, unicode, disp_area, &draw_area, disp_buf);
		}
		
		pos.x += letter_adv_w;
		str += bytes_num;
		letter_adv_w = 0;
		bytes_num = 0;

	}
}

// UG_ATTRIBUTE_FAST_MEM void fill_normal(const ug_area_t * disp_area, ug_color_t * disp_buf,
//                                               const ug_area_t * draw_area,
//                                               ug_color_t color, ug_opa_t opa,
//                                               const ug_opa_t * mask, ug_draw_mask_res_t mask_res)




void _ug_draw_letter(const ug_point_t *pos_p,
                    const ug_font_t *font_p,
                    const ug_font_glyph_dsc_t *g,
                    const ug_color_t color,
                    const ug_color_t backColor,
                    const uint32_t letter,
                    const ug_area_t * disp_area,
                    const ug_area_t * draw_area,
                    ug_color_t * disp_buf)
{

    if(font_p == NULL) {
        return;
    }
	uint32_t unicode = letter;
    // ug_font_glyph_dsc_t g;
    bool g_ret = font_p->get_glyph_dsc(font_p, g, letter, '\0');
    if(g_ret == false)  {
        /* Add waring if the dsc is not found
         * but do not print warning for non printable ASCII chars (e.g. '\n')*/
        if(letter >= 0x20) {
            UG_LOG_WARN("ug_draw_letter: glyph dsc. not found");
        }
        return;
    }





    /* Don't draw anything if the character is empty. E.g. space */
    if((g->box_h == 0) && (g->box_w == 0)) return;

	/*pos_xy :字模在屏幕上的(左上角)坐标（去除空白区域的字模）*/
    int32_t pos_x = pos_p->x + g->ofs_x;
	//int32_t pos_x = pos_p->x;
    int32_t pos_y = pos_p->y + (font_p->line_height - font_p->base_line) - g->box_h - g->ofs_y;






	/*get glyph bitmap*/
    const uint8_t * map_p = font_p->get_glyph_bitmap(font_p, letter);
    if(map_p == NULL) {
        UG_LOG_WARN("ug_draw_letter: character's bitmap not found");
        return;
    }

	_draw_letter_normal(pos_x, pos_y, g, color, backColor, map_p, disp_area, draw_area, disp_buf);

}

static void _draw_letter_normal(ug_coord_t pos_x,
                                ug_coord_t pos_y,
                                ug_font_glyph_dsc_t *g,
                                ug_color_t color,
                                ug_color_t backColor,
                                const uint8_t *map_p,
                                const ug_area_t *disp_area,
                                const ug_area_t *draw_area,
                                ug_color_t *disp_buf)
{
// #ifdef USE_UVGUI_LIB
// 	//TODO: malloc mem of glyph_rect dynamically.
// #else
// 	static uint16_t glyph_rect_buf[1000];
// #endif
	
	const uint8_t * bpp_opa_table_p;
	uint16_t box_h = g->box_h;
	uint16_t box_w = g->box_w;
	
	
	uint32_t glyph_px_cnt = box_h * box_w;	/*pixels cnt of glyph*/
	uint8_t bpp = g->bpp;
	

    /*Get the width of the `disp_area` it will be used to go to the next line*/
    int32_t disp_w = ug_area_get_width(disp_area);

	/* Now `pos_x pos_y` has absolute coordinates.
     * Make it relative to `disp_area` to simplify draw to `disp_buf`*/
    pos_x -= disp_area->x1;
    pos_y -= disp_area->y1;

    // int32_t draw_area_w = ug_area_get_width(draw_area);
    // int32_t draw_area_h = ug_area_get_height(draw_area);

	
	//ug_color_t fill_blank = {.full = 0xFFFF};	//white

	// uint16_t col = 0 ,row = 0;
	uint16_t bit = 0;
	uint8_t byte = 0;
	uint8_t bitmask = 0x00;
    bpp_opa_table_p = _ug_bpp1_opa_table;
    switch (bpp)
    {
    case 1:
        bpp_opa_table_p = _ug_bpp1_opa_table;
        bitmask = 0x01;
        break;
    case 4:
        bpp_opa_table_p = _ug_bpp4_opa_table;
        bitmask = 0x0F;
        break;
    case 2:
        bpp_opa_table_p = _ug_bpp2_opa_table;
        bitmask = 0x03;
        break;
    case 8:
        bpp_opa_table_p = _ug_bpp8_opa_table;
        bitmask = 0xFF;
        break;
    default:
        UG_LOG_ERR("no supporting to this bpp!");
        break;
    }

    uint32_t px_cnt = 0;
    uint32_t col = 0, row = 0;
    uint32_t originIndex = pos_x + pos_y * disp_w;
    ug_point_t drawPoint;
    // printf("box_h:%d, box_w:%d\r\n", box_h, box_w);
    for(uint32_t i = 0; i < glyph_px_cnt; i ++){
        // printf("px_cnt:%d, row:%d, col:%d\r\n", px_cnt, row, col);
        drawPoint.x = col + pos_x;
        drawPoint.y = row + pos_y;
        byte = (*map_p >> ((8-bpp) - bit)) & bitmask;
        if(bpp_opa_table_p[byte] && _ug_area_is_point_on(draw_area, &drawPoint, 0)){
            
            disp_buf[originIndex + col + row*disp_w].full = ug_color_lighten_with_backcolor(color, backColor, 255 - bpp_opa_table_p[byte]).full;

        }
        // disp_buf[pos_x + i].full = (bpp_opa_table_p[byte]) ? ug_color_lighten(color, 255 - bpp_opa_table_p[byte]).full : back_color.full;
		bit += bpp;
		px_cnt++;
        col++;
        if(bit == 8) {
			bit = 0;
			map_p ++;
            if(px_cnt == box_w){
                px_cnt = 0;
                row++;
                col = 0;
            }
		}
		else if(px_cnt == box_w){
			bit = 0;
			px_cnt = 0;
			map_p ++;
            row++;
            col = 0;
        }
	}

}

