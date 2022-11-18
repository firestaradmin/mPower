
#ifndef _UG_PRINTF_H_
#define _UG_PRINTF_H_


#ifdef __cplusplus
extern "C" {
#endif

#include "ug_user_config.h"

#if UG_SPRINTF_CUSTOM == 0

#include <stdarg.h>
#include <stddef.h>

/**
 * Tiny snprintf/vsnprintf implementation
 * \param buffer A pointer to the buffer where to store the formatted string
 * \param count The maximum number of characters to store in the buffer, including a terminating null character
 * \param format A string that specifies the format of the output
 * \param va A value identifying a variable arguments list
 * \return The number of characters that COULD have been written into the buffer, not counting the terminating
 *         null character. A value equal or larger than count indicates truncation. Only when the returned value
 *         is non-negative and less than count, the string has been completely written.
 */
int  ug_snprintf(char * buffer, size_t count, const char * format, ...);
int ug_vsnprintf(char * buffer, size_t count, const char * format, va_list va);

#else
#include UG_SPRINTF_INCLUDE
#endif


#ifdef __cplusplus
}
#endif


#endif  // _PRINTF_H_
