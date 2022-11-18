#ifndef __UVGUI_TYPE_H__
#define __UVGUI_TYPE_H__
#include "ug_user_config.h"
#include <stdint.h>


#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0
    #else
        #define NULL ((void *)0)
    #endif
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef uintptr_t ug_uintptr_t;



/**
 * UVGUI error codes.
 */
enum {
    UG_RES_INV = 0, /*Typically indicates that the object is deleted (become invalid) in the action
                       function or an operation was failed*/
    UG_RES_OK,      /*The object is valid (no deleted) after the action*/
};
typedef uint8_t ug_res_t;


#ifndef __cplusplus /* In C++, 'bool', 'true' and 'false' and keywords */
    #define bool _Bool
    #define true 1
    #define false 0
#else
    #ifdef __GNUC__
        /* GNU C++ supports direct inclusion of stdbool.h to provide C99
            compatibility by defining _Bool */
        #define _Bool bool
    #endif
#endif





#endif // !__UVGUI_TYPE_H__
