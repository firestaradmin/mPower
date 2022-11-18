/**
 * @file ug_log.h
 *
 */

#ifndef UG_LOG_H
#define UG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#include <stdint.h>
#include "ug_user_config.h"
/*********************
 *      DEFINES
 *********************/

/*Possible log level. For compatibility declare it independently from `UG_USE_LOG`*/

#define UG_LOG_LEVEL_TRACE 0 /**< A lot of logs to give detailed information*/
#define UG_LOG_LEVEL_INFO 1  /**< Log important events*/
#define UG_LOG_LEVEL_WARN 2  /**< Log if something unwanted happened but didn't caused problem*/
#define UG_LOG_LEVEL_ERROR 3 /**< Only critical issue, when the system may fail*/
#define UG_LOG_LEVEL_USER 4 /**< Custom logs from the user*/
#define UG_LOG_LEVEL_NONE 5 /**< Do not log anything*/
#define _UG_LOG_LEVEL_NUM 6 /**< Number of log levels */

UG_EXPORT_CONST_INT(UG_LOG_LEVEL_TRACE);
UG_EXPORT_CONST_INT(UG_LOG_LEVEL_INFO);
UG_EXPORT_CONST_INT(UG_LOG_LEVEL_WARN);
UG_EXPORT_CONST_INT(UG_LOG_LEVEL_ERROR);
UG_EXPORT_CONST_INT(UG_LOG_LEVEL_USER);
UG_EXPORT_CONST_INT(UG_LOG_LEVEL_NONE);

typedef int8_t ug_log_level_t;

#if UG_USE_LOG
/**********************
 *      TYPEDEFS
 **********************/

/**
 * Log print function. Receives "Log Level", "File path", "Line number", "Function name" and "Description".
 */
typedef void (*ug_log_print_g_cb_t)(ug_log_level_t level, const char *, uint32_t, const char *, const char *);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Register custom print/write function to call when a log is added.
 * It can format its "File path", "Line number" and "Description" as required
 * and send the formatted log message to a console or serial port.
 * @param print_cb a function pointer to print a log
 */
void ug_log_register_print_cb(ug_log_print_g_cb_t print_cb);

/**
 * Add a log
 * @param level the level of log. (From `ug_log_level_t` enum)
 * @param file name of the file when the log added
 * @param line line number in the source code where the log added
 * @param func name of the function when the log added
 * @param format printf-like format string
 * @param ... parameters for `format`
 */
void _ug_log_add(ug_log_level_t level, const char * file, int line, const char * func, const char * format, ...);

/**********************
 *      MACROS
 **********************/

#if UG_LOG_LEVEL <= UG_LOG_LEVEL_TRACE
#define UG_LOG_TRACE(...) _ug_log_add(UG_LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define UG_LOG_TRACE(...)                                                                                              \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif

#if UG_LOG_LEVEL <= UG_LOG_LEVEL_INFO
#define UG_LOG_INFO(...) _ug_log_add(UG_LOG_LEVEL_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define UG_LOG_INFO(...)                                                                                               \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif

#if UG_LOG_LEVEL <= UG_LOG_LEVEL_WARN
#define UG_LOG_WARN(...) _ug_log_add(UG_LOG_LEVEL_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define UG_LOG_WARN(...)                                                                                               \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif

#if UG_LOG_LEVEL <= UG_LOG_LEVEL_ERROR
#define UG_LOG_ERROR(...) _ug_log_add(UG_LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define UG_LOG_ERROR(...)                                                                                              \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif

#if UG_LOG_LEVEL <= UG_LOG_LEVEL_USER
#define UG_LOG_USER(...) _ug_log_add(UG_LOG_LEVEL_USER, __FILE__, __LINE__, __func__, __VA_ARGS__);
#else
#define UG_LOG_USER(...)                                                                                               \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif

#else /*UG_USE_LOG*/

/*Do nothing if `UG_USE_LOG  0`*/
#define _ug_log_add(level, file, line, ...)                                                                             \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#define UG_LOG_TRACE(...)                                                                                              \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#define UG_LOG_INFO(...)                                                                                               \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#define UG_LOG_WARN(...)                                                                                               \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#define UG_LOG_ERROR(...)                                                                                              \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#define UG_LOG_USER(...)                                                                                              \
    {                                                                                                                  \
        ;                                                                                                              \
    }
#endif /*UG_USE_LOG*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*UG_LOG_H*/

