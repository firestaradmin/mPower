/*************************************************
 * @brief color for uvgui
 * @date 2020
 * @author firestaradmin
*************************************************/


#ifndef __UG_COLOR_H__
#define __UG_COLOR_H__



#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "ug_user_config.h"
#include "ug_math.h"

/*Error checking*/
#if UG_COLOR_DEPTH == 24
#error "UG_COLOR_DEPTH  24 is deprecated. Use UG_COLOR_DEPTH  32 instead (ug_conf.h)"
#endif

#if UG_COLOR_DEPTH != 32 && UG_COLOR_SCREEN_TRANSP != 0
#error "UG_COLOR_SCREEN_TRANSP requires UG_COLOR_DEPTH == 32. Set it in ug_conf.h"
#endif

#if UG_COLOR_DEPTH != 16 && UG_COLOR_16_SWAP != 0
#error "UG_COLOR_16_SWAP requires UG_COLOR_DEPTH == 16. Set it in ug_conf.h"
#endif

#include <stdint.h>

/*********************
 *      DEFINES
 *********************/
#define UG_COLOR_WHITE UG_COLOR_MAKE(0xFF, 0xFF, 0xFF)
#define UG_COLOR_SILVER UG_COLOR_MAKE(0xC0, 0xC0, 0xC0)
#define UG_COLOR_GRAY UG_COLOR_MAKE(0x80, 0x80, 0x80)
#define UG_COLOR_BLACK UG_COLOR_MAKE(0x00, 0x00, 0x00)
#define UG_COLOR_RED UG_COLOR_MAKE(0xFF, 0x00, 0x00)
#define UG_COLOR_MAROON UG_COLOR_MAKE(0x80, 0x00, 0x00)
#define UG_COLOR_YELLOW UG_COLOR_MAKE(0xFF, 0xFF, 0x00)
#define UG_COLOR_OLIVE UG_COLOR_MAKE(0x80, 0x80, 0x00)
#define UG_COLOR_LIME UG_COLOR_MAKE(0x00, 0xFF, 0x00)
#define UG_COLOR_GREEN UG_COLOR_MAKE(0x00, 0x80, 0x00)
#define UG_COLOR_CYAN UG_COLOR_MAKE(0x00, 0xFF, 0xFF)
#define UG_COLOR_AQUA UG_COLOR_CYAN
#define UG_COLOR_TEAL UG_COLOR_MAKE(0x00, 0x80, 0x80)
#define UG_COLOR_BLUE UG_COLOR_MAKE(0x00, 0x00, 0xFF)
#define UG_COLOR_NAVY UG_COLOR_MAKE(0x00, 0x00, 0x80)
#define UG_COLOR_MAGENTA UG_COLOR_MAKE(0xFF, 0x00, 0xFF)
#define UG_COLOR_PURPLE UG_COLOR_MAKE(0x80, 0x00, 0x80)
#define UG_COLOR_ORANGE UG_COLOR_MAKE(0xFF, 0xA5, 0x00)

/**
 * Opacity percentages.
 */
enum {
    UG_OPA_TRANSP = 0,
    UG_OPA_0      = 0,
    UG_OPA_10     = 25,
    UG_OPA_20     = 51,
    UG_OPA_30     = 76,
    UG_OPA_40     = 102,
    UG_OPA_50     = 127,
    UG_OPA_60     = 153,
    UG_OPA_70     = 178,
    UG_OPA_80     = 204,
    UG_OPA_90     = 229,
    UG_OPA_100    = 255,
    UG_OPA_COVER  = 255,
};

#define UG_OPA_MIN 2    /*Opacities below this will be transparent*/
#define UG_OPA_MAX 253  /*Opacities above this will fully cover*/

#if UG_COLOR_DEPTH == 1
#define UG_COLOR_SIZE 8
#elif UG_COLOR_DEPTH == 8
#define UG_COLOR_SIZE 8
#elif UG_COLOR_DEPTH == 16
#define UG_COLOR_SIZE 16
#elif UG_COLOR_DEPTH == 32
#define UG_COLOR_SIZE 32
#else
#error "Invalid UG_COLOR_DEPTH in ug_conf.h! Set it to 1, 8, 16 or 32!"
#endif


/* Adjust color mix functions rounding.
 * GPUs might calculate color mix (blending) differently.
 * Should be in range of 0..254
 * 0: no adjustment, get the integer part of the result (round down)
 * 64: round up from x.75
 * 128: round up from half
 * 192: round up from x.25
 * 254: round up */
#ifndef UG_COLOR_MIX_ROUND_OFS
#if UG_COLOR_DEPTH == 32
#define UG_COLOR_MIX_ROUND_OFS 0
#else
#define UG_COLOR_MIX_ROUND_OFS 128
#endif
#endif

/*---------------------------------------
 * Macros for all existing  color depths
 * to set/get values of the color channels
 *------------------------------------------*/
# define UG_COLOR_SET_R1(c, v) (c).ch.red = (uint8_t)((v) & 0x1);
# define UG_COLOR_SET_G1(c, v) (c).ch.green = (uint8_t)((v) & 0x1);
# define UG_COLOR_SET_B1(c, v) (c).ch.blue = (uint8_t)((v) & 0x1);
# define UG_COLOR_SET_A1(c, v)

# define UG_COLOR_GET_R1(c) (c).ch.red
# define UG_COLOR_GET_G1(c) (c).ch.green
# define UG_COLOR_GET_B1(c) (c).ch.blue
# define UG_COLOR_GET_A1(c) 1

# define UG_COLOR_SET_R8(c, v) (c).ch.red = (uint8_t)(v) & 0x7U;
# define UG_COLOR_SET_G8(c, v) (c).ch.green = (uint8_t)(v) & 0x7U;
# define UG_COLOR_SET_B8(c, v) (c).ch.blue = (uint8_t)(v) & 0x3U;
# define UG_COLOR_SET_A8(c, v) do {} while(0)

# define UG_COLOR_GET_R8(c) (c).ch.red
# define UG_COLOR_GET_G8(c) (c).ch.green
# define UG_COLOR_GET_B8(c) (c).ch.blue
# define UG_COLOR_GET_A8(c) 0xFF

# define UG_COLOR_SET_R16(c, v) (c).ch.red = (uint8_t)(v) & 0x1FU;
# define UG_COLOR_SET_G16(c, v) (c).ch.green = (uint8_t)(v) & 0x3FU;
# define UG_COLOR_SET_G16_SWAP(c, v) {(c).ch.green_h = (uint8_t)(((v) >> 3) & 0x7); (c).ch.green_l = (uint8_t)((v) & 0x7);}
# define UG_COLOR_SET_B16(c, v) (c).ch.blue = (uint8_t)(v) & 0x1FU;
# define UG_COLOR_SET_A16(c, v) do {} while(0)

# define UG_COLOR_GET_R16(c) (c).ch.red
# define UG_COLOR_GET_G16(c) (c).ch.green
# define UG_COLOR_GET_G16_SWAP(c) (((c).ch.green_h << 3) + (c).ch.green_l)
# define UG_COLOR_GET_B16(c) (c).ch.blue
# define UG_COLOR_GET_A16(c) 0xFF

# define UG_COLOR_SET_R32(c, v) (c).ch.red = (uint32_t)((v) & 0xFF);
# define UG_COLOR_SET_G32(c, v) (c).ch.green = (uint32_t)((v) & 0xFF);
# define UG_COLOR_SET_B32(c, v) (c).ch.blue = (uint32_t)((v) & 0xFF);
# define UG_COLOR_SET_A32(c, v) (c).ch.alpha = (uint32_t)((v) & 0xFF);

# define UG_COLOR_GET_R32(c) (c).ch.red
# define UG_COLOR_GET_G32(c) (c).ch.green
# define UG_COLOR_GET_B32(c) (c).ch.blue
# define UG_COLOR_GET_A32(c) (c).ch.alpha


/*---------------------------------------
 * Macros for the current color depth
 * to set/get values of the color channels
 *------------------------------------------*/
#if UG_COLOR_DEPTH == 1
# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R1(c,v)
# define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G1(c,v)
# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B1(c,v)
# define UG_COLOR_SET_A(c, v) UG_COLOR_SET_A1(c,v)

# define UG_COLOR_GET_R(c) UG_COLOR_GET_R1(c)
# define UG_COLOR_GET_G(c) UG_COLOR_GET_G1(c)
# define UG_COLOR_GET_B(c) UG_COLOR_GET_B1(c)
# define UG_COLOR_GET_A(c) UG_COLOR_GET_A1(c)

#elif UG_COLOR_DEPTH == 8
# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R8(c,v)
# define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G8(c,v)
# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B8(c,v)
# define UG_COLOR_SET_A(c, v) UG_COLOR_SET_A8(c,v)

# define UG_COLOR_GET_R(c) UG_COLOR_GET_R8(c)
# define UG_COLOR_GET_G(c) UG_COLOR_GET_G8(c)
# define UG_COLOR_GET_B(c) UG_COLOR_GET_B8(c)
# define UG_COLOR_GET_A(c) UG_COLOR_GET_A8(c)

#elif UG_COLOR_DEPTH == 16
# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R16(c,v)
# if UG_COLOR_16_SWAP == 0
#   define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G16(c,v)
# else
#   define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G16_SWAP(c,v)
# endif
# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B16(c,v)
# define UG_COLOR_SET_A(c, v) UG_COLOR_SET_A16(c,v)

# define UG_COLOR_GET_R(c) UG_COLOR_GET_R16(c)
# if UG_COLOR_16_SWAP == 0
#   define UG_COLOR_GET_G(c) UG_COLOR_GET_G16(c)
# else
#   define UG_COLOR_GET_G(c) UG_COLOR_GET_G16_SWAP(c)
# endif
# define UG_COLOR_GET_B(c)   UG_COLOR_GET_B16(c)
# define UG_COLOR_GET_A(c)   UG_COLOR_GET_A16(c)

#elif UG_COLOR_DEPTH == 32
# define UG_COLOR_SET_R(c, v) UG_COLOR_SET_R32(c,v)
# define UG_COLOR_SET_G(c, v) UG_COLOR_SET_G32(c,v)
# define UG_COLOR_SET_B(c, v) UG_COLOR_SET_B32(c,v)
# define UG_COLOR_SET_A(c, v) UG_COLOR_SET_A32(c,v)

# define UG_COLOR_GET_R(c) UG_COLOR_GET_R32(c)
# define UG_COLOR_GET_G(c) UG_COLOR_GET_G32(c)
# define UG_COLOR_GET_B(c) UG_COLOR_GET_B32(c)
# define UG_COLOR_GET_A(c) UG_COLOR_GET_A32(c)
#endif

/**********************
 *      TYPEDEFS
 **********************/

typedef union {
    union {
        uint8_t blue : 1;
        uint8_t green : 1;
        uint8_t red : 1;
    } ch;
    uint8_t full;
} ug_color1_t;

typedef union {
    struct {
        uint8_t blue : 2;
        uint8_t green : 3;
        uint8_t red : 3;
    } ch;
    uint8_t full;
} ug_color8_t;

typedef union {
    struct {
#if UG_COLOR_16_SWAP == 0
        uint16_t blue : 5;
        uint16_t green : 6;
        uint16_t red : 5;
#else
        // uint16_t green_l : 3;
        // uint16_t blue : 5;
        // uint16_t red : 5;
        // uint16_t green_h : 3;
        uint16_t green_h : 3;
        uint16_t red : 5;
        uint16_t blue : 5;
        uint16_t green_l : 3;
#endif
    } ch;
    uint16_t full;
} ug_color16_t;

typedef union {
    struct {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } ch;
    uint32_t full;
} ug_color32_t;

#if UG_COLOR_DEPTH == 1
typedef uint8_t ug_color_int_t;
typedef ug_color1_t ug_color_t;
#elif UG_COLOR_DEPTH == 8
typedef uint8_t ug_color_int_t;
typedef ug_color8_t ug_color_t;
#elif UG_COLOR_DEPTH == 16
typedef uint16_t ug_color_int_t;
typedef ug_color16_t ug_color_t;
//typedef uint16_t ug_color_t;
#elif UG_COLOR_DEPTH == 32
typedef uint32_t ug_color_int_t;
typedef ug_color32_t ug_color_t;
#else
#error "Invalid UG_COLOR_DEPTH in ug_conf.h! Set it to 1, 8, 16 or 32!"
#endif


typedef struct {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} ug_color_hsv_t;

//! @cond Doxygen_Suppress
/*No idea where the guard is required but else throws warnings in the docs*/
typedef uint8_t ug_opa_t;
//! @endcond

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*In color conversations:
 * - When converting to bigger color type the LSB weight of 1 LSB is calculated
 *   E.g. 16 bit Red has 5 bits
 *         8 bit Red has 2 bits
 *        ----------------------
 *        8 bit red LSB = (2^5 - 1) / (2^2 - 1) = 31 / 3 = 10
 *
 * - When calculating to smaller color type simply shift out the LSBs
 *   E.g.  8 bit Red has 2 bits
 *        16 bit Red has 5 bits
 *        ----------------------
 *         Shift right with 5 - 3 = 2
 */
static inline uint8_t ug_color_to1(ug_color_t color)
{
#if UG_COLOR_DEPTH == 1
    return color.full;
#elif UG_COLOR_DEPTH == 8
    if((UG_COLOR_GET_R(color) & 0x4) || (UG_COLOR_GET_G(color) & 0x4) || (UG_COLOR_GET_B(color) & 0x2)) {
        return 1;
    }
    else {
        return 0;
    }
#elif UG_COLOR_DEPTH == 16
    if((UG_COLOR_GET_R(color) & 0x10) || (UG_COLOR_GET_G(color) & 0x20) || (UG_COLOR_GET_B(color) & 0x10)) {
        return 1;
    }
    else {
        return 0;
    }
#elif UG_COLOR_DEPTH == 32
    if((UG_COLOR_GET_R(color) & 0x80) || (UG_COLOR_GET_G(color) & 0x80) || (UG_COLOR_GET_B(color) & 0x80)) {
        return 1;
    }
    else {
        return 0;
    }
#endif
}

static inline uint8_t ug_color_to8(ug_color_t color)
{
#if UG_COLOR_DEPTH == 1
    if(color.full == 0)
        return 0;
    else
        return 0xFF;
#elif UG_COLOR_DEPTH == 8
    return color.full;
#elif UG_COLOR_DEPTH == 16
    ug_color8_t ret;
    UG_COLOR_SET_R8(ret, UG_COLOR_GET_R(color) >> 2);   /* 5 - 3  = 2*/
    UG_COLOR_SET_G8(ret, UG_COLOR_GET_G(color) >> 3); /* 6 - 3  = 3*/
    UG_COLOR_SET_B8(ret, UG_COLOR_GET_B(color) >> 3);  /* 5 - 2  = 3*/
    return ret.full;
#elif UG_COLOR_DEPTH == 32
    ug_color8_t ret;
    UG_COLOR_SET_R8(ret, UG_COLOR_GET_R(color) >> 5);   /* 8 - 3  = 5*/
    UG_COLOR_SET_G8(ret, UG_COLOR_GET_G(color) >> 5); /* 8 - 3  = 5*/
    UG_COLOR_SET_B8(ret, UG_COLOR_GET_B(color) >> 6);  /* 8 - 2  = 6*/
    return ret.full;
#endif
}

static inline uint16_t ug_color_to16(ug_color_t color)
{
#if UG_COLOR_DEPTH == 1
    if(color.full == 0)
        return 0;
    else
        return 0xFFFF;
#elif UG_COLOR_DEPTH == 8
    ug_color16_t ret;
    UG_COLOR_SET_R16(ret, UG_COLOR_GET_R(color) * 4);     /*(2^5 - 1)/(2^3 - 1) = 31/7 = 4*/
#if UG_COLOR_16_SWAP == 0
    UG_COLOR_SET_G16(ret,  UG_COLOR_GET_G(color) * 9); /*(2^6 - 1)/(2^3 - 1) = 63/7 = 9*/
#else
    UG_COLOR_SET_G16_SWAP(ret, (UG_COLOR_GET_G(color) * 9)); /*(2^6 - 1)/(2^3 - 1) = 63/7 = 9*/
#endif
    UG_COLOR_SET_B16(ret, UG_COLOR_GET_B(color) * 10);  /*(2^5 - 1)/(2^2 - 1) = 31/3 = 10*/
    return ret.full;
#elif UG_COLOR_DEPTH == 16
    return color.full;
#elif UG_COLOR_DEPTH == 32
    ug_color16_t ret;
    UG_COLOR_SET_R16(ret, UG_COLOR_GET_R(color) >> 3);   /* 8 - 5  = 3*/

#if UG_COLOR_16_SWAP == 0
    UG_COLOR_SET_G16(ret, UG_COLOR_GET_G(color) >> 2); /* 8 - 6  = 2*/
#else
    UG_COLOR_SET_G16_SWAP(ret, ret.ch.green_h = (UG_COLOR_GET_G(color) >> 2); /*(2^6 - 1)/(2^3 - 1) = 63/7 = 9*/
#endif
    UG_COLOR_SET_B16(ret, UG_COLOR_GET_B(color) >> 3);  /* 8 - 5  = 3*/
    return ret.full;
#endif
}

static inline uint32_t ug_color_to32(ug_color_t color)
{
#if UG_COLOR_DEPTH == 1
    if(color.full == 0)
        return 0xFF000000;
    else
        return 0xFFFFFFFF;
#elif UG_COLOR_DEPTH == 8
    ug_color32_t ret;
    UG_COLOR_SET_R32(ret, UG_COLOR_GET_R(color) * 36);     /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
    UG_COLOR_SET_G32(ret, UG_COLOR_GET_G(color) * 36); /*(2^8 - 1)/(2^3 - 1) = 255/7 = 36*/
    UG_COLOR_SET_B32(ret, UG_COLOR_GET_B(color) * 85);   /*(2^8 - 1)/(2^2 - 1) = 255/3 = 85*/
    UG_COLOR_SET_A32(ret, 0xFF);
    return ret.full;
#elif UG_COLOR_DEPTH == 16
    /**
     * The floating point math for conversion is:
     *  valueto = valuefrom * ( (2^bitsto - 1) / (float)(2^bitsfrom - 1) )
     * The faster integer math for conversion is:
     *  valueto = ( valuefrom * multiplier + adder ) >> divisor
     *   multiplier = FLOOR( ( (2^bitsto - 1) << divisor ) / (float)(2^bitsfrom - 1) )
     *
     * Find the first divisor where ( adder >> divisor ) <= 0
     *
     * 5-bit to 8-bit: ( 31 * multiplier + adder ) >> divisor = 255
     * divisor  multiplier  adder  min (0)  max (31)
     *       0           8      7        7       255
     *       1          16     14        7       255
     *       2          32     28        7       255
     *       3          65     25        3       255
     *       4         131     19        1       255
     *       5         263      7        0       255
     *
     * 6-bit to 8-bit: 255 = ( 63 * multiplier + adder ) >> divisor
     * divisor  multiplier  adder  min (0)  max (63)
     *       0           4      3        3       255
     *       1           8      6        3       255
     *       2          16     12        3       255
     *       3          32     24        3       255
     *       4          64     48        3       255
     *       5         129     33        1       255
     *       6         259      3        0       255
     */

    ug_color32_t ret;
    UG_COLOR_SET_R32(ret, (UG_COLOR_GET_R(color) * 263 + 7) >> 5);
    UG_COLOR_SET_G32(ret, (UG_COLOR_GET_G(color) * 259 + 3) >> 6);
    UG_COLOR_SET_B32(ret, (UG_COLOR_GET_B(color) * 263 + 7) >> 5);
    UG_COLOR_SET_A32(ret, 0xFF);
    return ret.full;
#elif UG_COLOR_DEPTH == 32
    return color.full;
#endif
}


//! @cond Doxygen_Suppress

/**
 * Mix two colors with a given ratio.
 * @param c1 the first color to mix (usually the foreground)
 * @param c2 the second color to mix (usually the background)
 * @param mix The ratio of the colors. 0: full `c2`, 255: full `c1`, 127: half `c1` and half`c2`
 * @return the mixed color
 */
UG_ATTRIBUTE_FAST_MEM static inline ug_color_t ug_color_mix(ug_color_t c1, ug_color_t c2, uint8_t mix)
{
    ug_color_t ret;
#if UG_COLOR_DEPTH != 1
    /*UG_COLOR_DEPTH == 8, 16 or 32*/
    UG_COLOR_SET_R(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_R(c1) * mix + UG_COLOR_GET_R(c2) *
                                        (255 - mix) + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_G(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_G(c1) * mix + UG_COLOR_GET_G(c2) *
                                        (255 - mix) + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_B(ret, UG_MATH_UDIV255((uint16_t) UG_COLOR_GET_B(c1) * mix + UG_COLOR_GET_B(c2) *
                                        (255 - mix) + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_A(ret, 0xFF);
#else
    /*UG_COLOR_DEPTH == 1*/
    ret.full = mix > UG_OPA_50 ? c1.full : c2.full;
#endif

    return ret;
}

UG_ATTRIBUTE_FAST_MEM static inline void ug_color_premult(ug_color_t c, uint8_t mix, uint16_t * out)
{
#if UG_COLOR_DEPTH != 1
    out[0] = (uint16_t) UG_COLOR_GET_R(c) * mix;
    out[1] = (uint16_t) UG_COLOR_GET_G(c) * mix;
    out[2] = (uint16_t) UG_COLOR_GET_B(c) * mix;
#else
    (void) mix;
    /*Pre-multiplication can't be used with 1 bpp*/
    out[0] = UG_COLOR_GET_R(c);
    out[1] = UG_COLOR_GET_G(c);
    out[2] = UG_COLOR_GET_B(c);
#endif

}


/**
 * Mix two colors with a given ratio. It runs faster then `ug_color_mix` but requires some pre computation.
 * @param c1 The first color. Should be preprocessed with `ug_color_premult(c1)`
 * @param c2 The second color. As it is no pre computation required on it
 * @param mix The ratio of the colors. 0: full `c2`, 255: full `c1`, 127: half `c1` and half `c2`.
 *            Should be modified like mix = `255 - mix`
 * @return the mixed color
 * @note 255 won't give clearly `c1`.
 */
UG_ATTRIBUTE_FAST_MEM static inline ug_color_t ug_color_mix_premult(uint16_t * premult_c1, ug_color_t c2, uint8_t mix)
{
    ug_color_t ret;
#if UG_COLOR_DEPTH != 1
    /*UG_COLOR_DEPTH == 8, 16 or 32*/
    UG_COLOR_SET_R(ret, UG_MATH_UDIV255((uint16_t) premult_c1[0] + UG_COLOR_GET_R(c2) * mix + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_G(ret, UG_MATH_UDIV255((uint16_t) premult_c1[1] + UG_COLOR_GET_G(c2) * mix + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_B(ret, UG_MATH_UDIV255((uint16_t) premult_c1[2] + UG_COLOR_GET_B(c2) * mix + UG_COLOR_MIX_ROUND_OFS));
    UG_COLOR_SET_A(ret, 0xFF);
#else
    /*UG_COLOR_DEPTH == 1*/
    /*Restore color1*/
    ug_color_t c1;
    UG_COLOR_SET_R(c1, premult_c1[0]);
    UG_COLOR_SET_G(c1, premult_c1[1]);
    UG_COLOR_SET_B(c1, premult_c1[2]);
    ret.full = mix > UG_OPA_50 ? c1.full : c2.full;
#endif

    return ret;
}


/**
 * Mix two colors. Both color can have alpha value. It requires ARGB888 colors.
 * @param bg_color background color
 * @param bg_opa alpha of the background color
 * @param fg_color foreground color
 * @param fg_opa alpha of the foreground color
 * @param res_color the result color
 * @param res_opa the result opacity
 */
UG_ATTRIBUTE_FAST_MEM static inline void ug_color_mix_with_alpha(ug_color_t bg_color, ug_opa_t bg_opa,
                                                                 ug_color_t fg_color, ug_opa_t fg_opa,
                                                                 ug_color_t * res_color, ug_opa_t * res_opa)
{
    /* Pick the foreground if it's fully opaque or the Background is fully transparent*/
    if(fg_opa >= UG_OPA_MAX || bg_opa <= UG_OPA_MIN) {
        res_color->full = fg_color.full;
        *res_opa = fg_opa;
    }
    /*Transparent foreground: use the Background*/
    else if(fg_opa <= UG_OPA_MIN) {
        res_color->full = bg_color.full;
        *res_opa = bg_opa;
    }
    /*Opaque background: use simple mix*/
    else if(bg_opa >= UG_OPA_MAX) {
        *res_color = ug_color_mix(fg_color, bg_color, fg_opa);
        *res_opa = UG_OPA_COVER;
    }
    /*Both colors have alpha. Expensive calculation need to be applied*/
    else {
        /*Save the parameters and the result. If they will be asked again don't compute again*/
        static ug_opa_t fg_opa_save     = 0;
        static ug_opa_t bg_opa_save     = 0;
        static ug_color_t fg_color_save = {.full = 0};
        static ug_color_t bg_color_save = {.full = 0};
        static ug_color_t res_color_saved = {.full = 0};
        static ug_opa_t res_opa_saved = 0;

        if(fg_opa != fg_opa_save || bg_opa != bg_opa_save || fg_color.full != fg_color_save.full ||
           bg_color.full != bg_color_save.full) {
            fg_opa_save        = fg_opa;
            bg_opa_save        = bg_opa;
            fg_color_save.full = fg_color.full;
            bg_color_save.full = bg_color.full;
            /*Info:
             * https://en.wikipedia.org/wiki/Alpha_compositing#Analytical_derivation_of_the_over_operator*/
            res_opa_saved = 255 - ((uint16_t)((uint16_t)(255 - fg_opa) * (255 - bg_opa)) >> 8);
            if(res_opa_saved == 0) {
                while(1)
                    ;
            }
            ug_opa_t ratio = (uint16_t)((uint16_t)fg_opa * 255) / res_opa_saved;
            res_color_saved  = ug_color_mix(fg_color, bg_color, ratio);

        }

        res_color->full = res_color_saved.full;
        *res_opa = res_opa_saved;
    }
}

//! @endcond

/**
 * Get the brightness of a color
 * @param color a color
 * @return the brightness [0..255]
 */
static inline uint8_t ug_color_brightness(ug_color_t color)
{
    ug_color32_t c32;
    c32.full        = ug_color_to32(color);
    uint16_t bright = (uint16_t)(3u * UG_COLOR_GET_R32(c32) + UG_COLOR_GET_B32(c32) + 4u * UG_COLOR_GET_G32(c32));
    return (uint8_t)(bright >> 3);
}

/* The most simple macro to create a color from R,G and B values */
#if UG_COLOR_DEPTH == 1
#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){.full = (uint8_t)((b8 >> 7) | (g8 >> 7) | (r8 >> 7))})
#elif UG_COLOR_DEPTH == 8
#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint8_t)((b8 >> 6) & 0x3U), (uint8_t)((g8 >> 5) & 0x7U), (uint8_t)((r8 >> 5) & 0x7U)}})
#elif UG_COLOR_DEPTH == 16
#if UG_COLOR_16_SWAP == 0
#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x3FU), (uint16_t)((r8 >> 3) & 0x1FU)}})
#else
#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint16_t)((g8 >> 5) & 0x7U), (uint16_t)((r8 >> 3) & 0x1FU), (uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x7U)}})
// #define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{(uint16_t)((g8 >> 5) & 0x7U), (uint16_t)((b8 >> 3) & 0x1FU), (uint16_t)((r8 >> 3) & 0x1FU), (uint16_t)((g8 >> 2) & 0x7U)}})
#endif
#elif UG_COLOR_DEPTH == 32
#define UG_COLOR_MAKE(r8, g8, b8) ((ug_color_t){{b8, g8, r8, 0xff}}) /*Fix 0xff alpha*/
#endif

static inline ug_color_t ug_color_make(uint8_t r, uint8_t g, uint8_t b)
{
    return UG_COLOR_MAKE(r, g, b);
}

static inline ug_color_t ug_color_hex(uint32_t c)
{
    return ug_color_make((uint8_t)((c >> 16) & 0xFF), (uint8_t)((c >> 8) & 0xFF), (uint8_t)(c & 0xFF));
}

static inline ug_color_t ug_color_hex3(uint32_t c)
{
    return ug_color_make((uint8_t)(((c >> 4) & 0xF0) | ((c >> 8) & 0xF)), (uint8_t)((c & 0xF0) | ((c & 0xF0) >> 4)),
                         (uint8_t)((c & 0xF) | ((c & 0xF) << 4)));
}


//! @cond Doxygen_Suppress
//!
UG_ATTRIBUTE_FAST_MEM void ug_color_fill(ug_color_t * buf, ug_color_t color, uint32_t px_num);

//! @endcond
ug_color_t ug_color_lighten(ug_color_t c, ug_opa_t lvl);
ug_color_t ug_color_lighten_with_backcolor(ug_color_t c, ug_color_t backcolor, ug_opa_t lvl);
ug_color_t ug_color_darken(ug_color_t c, ug_opa_t lvl);

/**
 * Convert a HSV color to RGB
 * @param h hue [0..359]
 * @param s saturation [0..100]
 * @param v value [0..100]
 * @return the given RGB color in RGB (with UG_COLOR_DEPTH depth)
 */
ug_color_t ug_color_hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v);

/**
 * Convert a 32-bit RGB color to HSV
 * @param r8 8-bit red
 * @param g8 8-bit green
 * @param b8 8-bit blue
 * @return the given RGB color in HSV
 */
ug_color_hsv_t ug_color_rgb_to_hsv(uint8_t r8, uint8_t g8, uint8_t b8);

/**
 * Convert a color to HSV
 * @param color color
 * @return the given color in HSV
 */
ug_color_hsv_t ug_color_to_hsv(ug_color_t color);


/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif // !__UG_COLOR_H__
