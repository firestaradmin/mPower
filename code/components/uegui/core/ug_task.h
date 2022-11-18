#ifndef __UG_TASK_H__
#define __UG_TASK_H__

#include <stdint.h>
#include "ug_type.h"

#define UG_NO_TASK_READY 0xFFFFFFFF

struct _ug_task_t;

/**
 * Tasks execute this type type of functions.
 */
typedef void (*ug_task_cb_t)(struct _ug_task_t * task);



/**
 * Possible priorities for ug_tasks
 */
enum {
    UG_TASK_PRIO_OFF = 0,
    UG_TASK_PRIO_LOWEST,
    UG_TASK_PRIO_LOW,
    UG_TASK_PRIO_MID,
    UG_TASK_PRIO_HIGH,
    UG_TASK_PRIO_HIGHEST,
    _UG_TASK_PRIO_NUM,
};
typedef uint8_t ug_task_prio_t;
/**
 * Descriptor of a ug_task
 */
typedef struct _ug_task_t {
    uint32_t period; /**< How often the task should run */
    uint32_t last_run; /**< Last time the task ran */
    ug_task_cb_t task_cb; /**< Task function */

    void * user_data; /**< Custom user data */

    int32_t repeat_count; /**< 1: Task times;  -1 : infinity;  0 : stop ;  n>0: residual times */
    uint8_t prio : 3; /**< Task priority */
} ug_task_t;



/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Init the ug_task module
 */
void _ug_task_core_init(void);

//! @cond Doxygen_Suppress

/**
 * Call it  periodically to handle ug_tasks.
 * @return time till it needs to be run next (in ms)
 */
uint32_t ug_task_handler(void);

//! @endcond

/**
 * Create an "empty" task. It needs to initialized with at least
 * `ug_task_set_cb` and `ug_task_set_period`
 * @return pointer to the created task
 */
ug_task_t * ug_task_create_basic(void);

/**
 * Create a new ug_task
 * @param task_xcb a callback which is the task itself. It will be called periodically.
 *                 (the 'x' in the argument name indicates that its not a fully generic function because it not follows
 *                  the `func_name(object, callback, ...)` convention)
 * @param period call period in ms unit
 * @param prio priority of the task (UG_TASK_PRIO_OFF means the task is stopped)
 * @param user_data custom parameter
 * @return pointer to the new task
 */
ug_task_t * ug_task_create(ug_task_cb_t task_xcb, uint32_t period, ug_task_prio_t prio, void * user_data);

/**
 * Delete a ug_task
 * @param task pointer to task_cb created by task
 */
void ug_task_del(ug_task_t * task);

/**
 * Set the callback the task (the function to call periodically)
 * @param task pointer to a task
 * @param task_cb the function to call periodically
 */
void ug_task_set_cb(ug_task_t * task, ug_task_cb_t task_cb);

/**
 * Set new priority for a ug_task
 * @param task pointer to a ug_task
 * @param prio the new priority
 */
void ug_task_set_prio(ug_task_t * task, ug_task_prio_t prio);

/**
 * Set new period for a ug_task
 * @param task pointer to a ug_task
 * @param period the new period
 */
void ug_task_set_period(ug_task_t * task, uint32_t period);

/**
 * Make a ug_task ready. It will not wait its period.
 * @param task pointer to a ug_task.
 */
void ug_task_ready(ug_task_t * task);

/**
 * Set the number of times a task will repeat.
 * @param task pointer to a ug_task.
 * @param repeat_count -1 : infinity;  0 : stop ;  n>0: residual times
 */
void ug_task_set_repeat_count(ug_task_t * task, int32_t repeat_count);

/**
 * Reset a ug_task.
 * It will be called the previously set period milliseconds later.
 * @param task pointer to a ug_task.
 */
void ug_task_reset(ug_task_t * task);

/**
 * Enable or disable the whole  ug_task handling
 * @param en: true: ug_task handling is running, false: ug_task handling is suspended
 */
void ug_task_enable(bool en);

/**
 * Get idle percentage
 * @return the ug_task idle in percentage
 */
uint8_t ug_task_get_idle(void);








#endif //!__UG_TASK_H__

