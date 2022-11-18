/**
 * @filename ugfont.h
 * @brief a component for UVGUI, you can also only use this lib to display glyph in your device.
 * @author LXG.firestaradmin
 * @version V1.0.0
 * @email 791836949@qq.com
 */

#ifndef __UGFONT_H__
#define __UGFONT_H__




/*********************
 *      INCLUDES
 *********************/
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>



#define USE_UVGUI_LIB

#ifdef USE_UVGUI_LIB
	#include "ug_area.h"
#endif

#define UG_FONT_DECLARE(font_name) extern struct _ug_font_struct font_name;


/*********************** User config ***********************/
/*if used in uvgui, enable this macro.*/
//#define USE_UVGUI_LIB

#define UG_TAB_SIZE 4

/*********************** User config END ***********************/





/**********************
 *      TYPEDEFS
 **********************/
#ifdef USE_UVGUI_LIB
	#define UG_LOG_WARN(x)  
	#define UG_LOG_ERR(x)  
#else
	#define UG_LOG_WARN(x)  
	#define UG_LOG_ERR(x)  
	#define UG_COLOR_DEPTH 16
	
	
	
	typedef uint8_t ug_opa_t;	
	typedef  uint16_t ug_coord_t;
	/**
	 * Represents a point on the screen.
	 */
	typedef struct {
		ug_coord_t x;
		ug_coord_t y;
	} ug_point_t;
	
	/*********** ugfont_color ***********/
	#define UG_MATH_UDIV255(x) ((uint32_t)((uint32_t) (x) * 0x8081) >> 0x17)
	
	#define UG_COLOR_SET_R16(c, v) 	(c).ch.red = (uint8_t)(v) & 0x1FU;  
	#define UG_COLOR_SET_G16(c, v) 	(c).ch.green = (uint8_t)(v) & 0x3FU;
	#define UG_COLOR_SET_B16(c, v) 	(c).ch.blue = (uint8_t)(v) & 0x1FU;
	#define UG_COLOR_GET_R16(c) 	(c).ch.red
	#define UG_COLOR_GET_G16(c) 	(c).ch.green
	#define UG_COLOR_GET_B16(c) 	(c).ch.blue
	
	#define UG_COLOR_WHITE UG_COLOR_MAKE(0xFF, 0xFF, 0xFF)
	#define UG_COLOR_SILVER UG_COLOR_MAKE(0xC0, 0xC0, 0xC0)
	#define UG_COLOR_GRAY UG_COLOR_MAKE(0x80, 0x80, 0x80)
	#define UG_COLOR_BLACK UG_COLOR_MAKE(0x00, 0x00, 0x00)
	#define UG_COLOR_RED UG_COLOR_MAKE(0xFF, 0x00, 0x00)

	/* The most simple macro to create a color from R,G and B values */
	#if UG_COLOR_DEPTH == 1
	#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){.full = (uint8_t)((b8 >> 7) | (g8 >> 7) | (r8 >> 7))})
	#elif UG_COLOR_DEPTH == 8
	#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint8_t)((b8 >> 6) & 0x3U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 5) & 0x7U)}})
	#elif UG_COLOR_DEPTH == 16
	//#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x3FU), (uint16_t)((r8 >> 3) & 0x1FU)}})
	#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){.ch.red = r8, .ch.green = g8, .ch.blue = b8})
	#elif UG_COLOR_DEPTH == 32
	#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{b8, g8, r8, 0xff}}) /*Fix 0xff alpha*/
	#endif
	/*---------------------------------------
	 * Macros for the current color depth
	 * to set/get values of the color channels
	 *------------------------------------------*/
	#if UG_COLOR_DEPTH == 1

	#elif UG_COLOR_DEPTH == 8

	#elif UG_COLOR_DEPTH == 16
	# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R16(c,v)
	# define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G16(c,v)
	# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B16(c,v)
	# define UG_COLOR_GET_R(c) UG_COLOR_GET_R16(c)
	# define UG_COLOR_GET_G(c) UG_COLOR_GET_G16(c)
	# define UG_COLOR_GET_B(c)   UG_COLOR_GET_B16(c)

	#elif UG_COLOR_DEPTH == 32

	#endif
	
	
	typedef union {
		struct {
			uint16_t blue : 5;
			uint16_t green : 6;
			uint16_t red : 5;
		} ch;
		uint16_t full;
	} ug_color16_t;

	typedef ug_color16_t ug_color_t;
	/*********** ugfont_color ***********/
	
	typedef void (*__ugfont_fill_bitmap_t)(ug_coord_t, ug_coord_t, ug_coord_t, ug_coord_t, const ug_color_t*) ;
	
	
	void ug_font_register_fill_bitmap_func(__ugfont_fill_bitmap_t func);
	
	
	/**
	 * Mix two colors with a given ratio.
	 * @param c1 the first color to mix (usually the foreground)
	 * @param c2 the second color to mix (usually the background)
	 * @param mix The ratio of the colors. 0: full `c2`, 255: full `c1`, 127: half `c1` and half`c2`
	 * @return the mixed color
	 */
	static inline ug_color_t ug_color_mix(ug_color_t c1, ug_color_t c2, uint8_t mix)
	{
		ug_color_t ret;
	#if UG_COLOR_DEPTH != 1
		/*UG_COLOR_DEPTH == 8, 16 or 32*/
		UG_COLOR_SET_R(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_R(c1) * mix + UG_COLOR_GET_R(c2) *
											(255 - mix) ));
		UG_COLOR_SET_G(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_G(c1) * mix + UG_COLOR_GET_G(c2) *
											(255 - mix) ));
		UG_COLOR_SET_B(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_B(c1) * mix + UG_COLOR_GET_B(c2) *
											(255 - mix) ));
	#else
		/*UG_COLOR_DEPTH == 1*/
		ret.full = mix > UG_OPA_50 ? c1.full : c2.full;
	#endif

		return ret;
	}
#endif /*END of USE_UVGUI_LIB*/


/*------------------
 * General types
 *-----------------*/

/** Describes the properties of a glyph. */
typedef struct {
    uint16_t adv_w; /**< The glyph needs this space. Draw the next glyph after this width. 8 bit integer, 4 bit fractional */
    uint16_t box_w;  /**< Width of the glyph's bounding box*/
    uint16_t box_h;  /**< Height of the glyph's bounding box*/
    int16_t ofs_x;   /**< x offset of the bounding box*/
    int16_t ofs_y;  /**< y offset of the bounding box*/
    uint8_t bpp;   /**< Bit-per-pixel: 1, 2, 4, 8*/
} ug_font_glyph_dsc_t;




/** Describe the properties of a font*/
typedef struct _ug_font_struct {
    /** Get a glyph's  descriptor from a font*/
    bool (*get_glyph_dsc)(const struct _ug_font_struct *, ug_font_glyph_dsc_t *, uint32_t letter, uint32_t letter_next);
    /** Get a glyph's bitmap from a font*/
    const uint8_t * (*get_glyph_bitmap)(const struct _ug_font_struct *, uint32_t);

    /*Pointer to the font in a font pack (must have the same line height)*/
    ug_coord_t line_height;         /**< The real line height where any text fits*/
    ug_coord_t base_line;           /**< Base line measured from the top of the line_height*/
   
    void * dsc;                     /**< Store implementation specific or run_time data or caching here*/

} ug_font_t;

/** This describes a glyph. */
typedef struct {
#if UG_FONT_FMT_TXT_LARGE == 0
    uint32_t bitmap_index : 20;     /**< Start index of the bitmap. A font can be max 1 MB. */
    uint32_t adv_w : 12;            /**< Draw the next glyph after this width. 8.4 format (real_value * 16 is stored). */
    uint8_t box_w;                  /**< Width of the glyph's bounding box*/
    uint8_t box_h;                  /**< Height of the glyph's bounding box*/
    int8_t ofs_x;                   /**< x offset of the bounding box*/
    int8_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
#else
    uint32_t bitmap_index;          /**< Start index of the bitmap. A font can be max 4 GB. */
    uint32_t adv_w;                 /**< Draw the next glyph after this width. 28.4 format (real_value * 16 is stored). */
    uint16_t box_w;                  /**< Width of the glyph's bounding box*/
    uint16_t box_h;                  /**< Height of the glyph's bounding box*/
    int16_t ofs_x;                   /**< x offset of the bounding box*/
    int16_t ofs_y;                  /**< y offset of the bounding box. Measured from the top of the line*/
#endif
} ug_font_fmt_txt_glyph_dsc_t;

/** Format of font character map. */
enum {
    UG_FONT_FMT_TXT_CMAP_FORMAT0_TINY,
    UG_FONT_FMT_TXT_CMAP_FORMAT0_FULL,
    UG_FONT_FMT_TXT_CMAP_SPARSE_TINY,
    UG_FONT_FMT_TXT_CMAP_SPARSE_FULL,
};


typedef uint8_t ug_font_fmt_txt_cmap_type_t;
/* Map codepoints to a `glyph_dsc`s
 * Several formats are supported to optimize memory usage
 */
typedef struct {
    /** First Unicode character for this range */
    uint32_t range_start;

    /** Number of Unicode characters related to this range.
     * Last Unicode character = range_start + range_length - 1*/
    uint16_t range_length;

    /** First glyph ID (array index of `glyph_dsc`) for this range */
    uint16_t glyph_id_start;


    const uint16_t * unicode_list;

    const void * glyph_id_ofs_list;

    /** Length of `unicode_list` and/or `glyph_id_ofs_list`*/
    uint16_t list_length;

    /** Type of this character map*/
    ug_font_fmt_txt_cmap_type_t type;
} ug_font_fmt_txt_cmap_t;

/*Describe store additional data for fonts */
typedef struct {
    /*The bitmaps of all glyphs*/
    const uint8_t * glyph_bitmap;

    /*Describe the glyphs*/
    const ug_font_fmt_txt_glyph_dsc_t * glyph_dsc;

    /* Map the glyphs to Unicode characters.
     * Array of `ug_font_cmap_fmt_txt_t` variables*/
    const ug_font_fmt_txt_cmap_t * cmaps;

    /* Store kerning values.
     * Can be  `ug_font_fmt_txt_kern_pair_t *  or `ug_font_kern_classes_fmt_txt_t *`
     * depending on `kern_classes`
     */
    const void * kern_dsc;

    /*Scale kern values in 12.4 format*/
    uint16_t kern_scale;

    /*Number of cmap tables*/
    uint16_t cmap_num       : 10;

    /*Bit per pixel: 1, 2, 3, 4, 8*/
    uint16_t bpp            : 4;

    /*Type of `kern_dsc`*/
    uint16_t kern_classes   : 1;

    /*
     * storage format of the bitmap
     * from `ug_font_fmt_txt_bitmap_format_t`
     */
    uint16_t bitmap_format  : 2;

    /*Cache the last letter and is glyph id*/
    uint32_t last_letter;
    uint32_t last_glyph_id;

} ug_font_fmt_txt_dsc_t;

/**********************
 * GLOBAL PROTOTYPES
 **********************/



/**
 * Get the line height of a font. All characters fit into this height
 * @param font_p pointer to a font
 * @return the height of a font
 */
static inline ug_coord_t ug_font_get_line_height(const ug_font_t * font_p)
{
    return font_p->line_height;
}







#ifdef USE_UVGUI_LIB
/**
 * @brief  Converter Unicode to UTF8.
 * UTF-8编码规则：如果只有一个字节则其最高二进制位为0；
 * 如果是多字节，其第一个字节从最高位开始，连续的二进制位值为1的个数决定了其编码的字节数，其余各字节均以10开头。
 * @param buf where store the UTF-8 data.
 * @param unicode which to be converted.
 * @return the bytes of the character in UTF-8 need.
 * 			[unicode range]         [bytes]
            0000 ~ 007F             1
            0080 ~ 07FF             2
            0800 ~ FFFF             3
            1 0000 ~ 1F FFFF        4
            20 0000 ~ 3FF FFFF      5
            400 0000 ~ 7FFF FFFF    6
*/
unsigned int unicode_to_utf8(unsigned char *buf, unsigned int unicode);

/**
 * @brief  Converter one charcater UTF-8 to Unicode.
 * UTF-8编码规则：如果只有一个字节则其最高二进制位为0；
 * 如果是多字节，其第一个字节从最高位开始，连续的二进制位值为1的个数决定了其编码的字节数，其余各字节均以10开头。
 * @param word the character to be converted
 * @return the character bytes_num in UTF-8.
*/
unsigned int utf8_to_unicode(unsigned int *unicode, const unsigned char *word);
#else

/**********************
 * GLOBAL FUNCTIONS
 **********************/
/** draw a letter */
void ug_draw_letter(const ug_point_t *pos_p, const ug_font_t *font_p, ug_color_t color, ug_color_t backcolor, uint32_t letter);
/** draw a string */
void ug_draw_str(const ug_point_t *pos_p, const ug_font_t *font_p, ug_color_t color, ug_color_t backcolor, const unsigned char *str);

void ug_font_register_fill_bitmap_func(__ugfont_fill_bitmap_t func);

ug_color_t ug_color_lighten(ug_color_t c, ug_opa_t lvl);
ug_color_t ug_color_darken(ug_color_t c, ug_opa_t lvl);
ug_color_t ug_color_lighten_with_backcolor(ug_color_t c, ug_color_t backcolor, ug_opa_t lvl);

inline ug_color_t ug_color_set_brightness(ug_color_t c, ug_opa_t lvl);
#endif /*END of USE_UVGUI_LIB*/













#endif
