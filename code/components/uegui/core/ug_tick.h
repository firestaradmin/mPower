/**
 * @file ug_hal_tick.h
 * Provide access to the system tick with 1 millisecond resolution
 */

#ifndef LV_TICK_H
#define LV_TICK_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdbool.h>

/*********************
 *      DEFINES
 *********************/
#ifndef UG_ATTRIBUTE_TICK_INC
#define UG_ATTRIBUTE_TICK_INC
#endif


/**********************
 * GLOBAL PROTOTYPES
 **********************/
/**
 * You have to call this function periodically
 * @param tick_period the call period of this function in milliseconds
 */
UG_ATTRIBUTE_TICK_INC void ug_tick_inc(uint32_t tick_period);

//! @endcond

/**
 * Get the elapsed milliseconds since start up
 * @return the elapsed milliseconds
 */
uint32_t ug_tick_get(void);

/**
 * Get the elapsed milliseconds since a previous time stamp
 * @param prev_tick a previous time stamp (return value of systick_get() )
 * @return the elapsed milliseconds since 'prev_tick'
 */
uint32_t ug_tick_elaps(uint32_t prev_tick);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_TICK_H*/
