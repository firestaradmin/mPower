#include "ug_main.h"
#include "ug_task.h"
#include "ug_obj.h"
#include "ug_refr.h"
#include "ug_tick.h"
#include "ug_mem.h"

/*********************
 *      DEFINES
 *********************/
#define IDLE_MEAS_PERIOD 500 /*[ms]*/
#define DEF_PRIO UG_TASK_PRIO_MID
#define DEF_PERIOD 500 /*[ms]*/
/**********************
 *  STATIC VARIABLES
 **********************/
static bool ug_task_run  = false;   /* task running falg */
static uint8_t idle_last = 0;
static bool task_deleted;
static bool task_list_changed;      /* task list has changed flag */
static bool task_created;           /* task been created flag */

/**********************
 *  STATIC PROTOTYPES
 **********************/
static bool ug_task_exec(ug_task_t * task);
static uint32_t ug_task_time_remaining(ug_task_t * task);
/**
 * Init the ug_task module
 */
void _ug_task_core_init(void)
{
    _ug_ll_init(&_ug_task_ll, sizeof(ug_task_t));

    task_list_changed = false;
    /*Initially enable the ug_task handling*/
    ug_task_enable(true);
}





/**
 * Call it  periodically to handle ug_tasks.
 * @return the time after which it must be called again
 */
uint32_t ug_task_handler(void)
{


    //UG_LOG_TRACE("ug_task_handler started");

    /*Avoid concurrent running of the task handler*/
    static bool already_running = false;
    if(already_running) return 1;
    already_running = true;

    static uint32_t idle_period_start = 0;
    static uint32_t handler_start     = 0;
    static uint32_t busy_time         = 0;
    static uint32_t time_till_next;

    if(ug_task_run == false) {
        already_running = false; /*Release mutex*/
        return 1;
    }

    handler_start = ug_tick_get();

    /* Run all task from the highest to the lowest priority
     * If a lower priority task is executed check task again from the highest priority
     * but on the priority of executed tasks don't run tasks before the executed*/
    ug_task_t * task_interrupter = NULL;
    ug_task_t * next;
    bool end_flag;
    do {
        end_flag                 = true;
        task_deleted             = false;
        task_created             = false;
        _ug_task_act = _ug_ll_get_head(&_ug_task_ll);
        while(_ug_task_act) {
            /* The task might be deleted if it runs only once ('once = 1')
             * So get next element until the current is surely valid*/
            next = _ug_ll_get_next(&_ug_task_ll, _ug_task_act);

            /*We reach priority of the turned off task. There is nothing more to do.*/
            if(((ug_task_t *)_ug_task_act)->prio == UG_TASK_PRIO_OFF) {
                break;
            }

            /*Here is the interrupter task. Don't execute it again.*/
            if(_ug_task_act == task_interrupter) {
                task_interrupter = NULL; /*From this point only task after the interrupter comes, so
                                            the interrupter is not interesting anymore*/
                _ug_task_act = next;
                continue; /*Load the next task*/
            }

            /*Just try to run the tasks with highest priority.*/
            if(((ug_task_t *)_ug_task_act)->prio == UG_TASK_PRIO_HIGHEST) {
                ug_task_exec(_ug_task_act);
            }
            /*Tasks with higher priority than the interrupted shall be run in every case*/
            else if(task_interrupter) {
                if(((ug_task_t *)_ug_task_act)->prio > task_interrupter->prio) {
                    if(ug_task_exec(_ug_task_act)) {
                        if(!task_created && !task_deleted) {
                            /*Check all tasks again from the highest priority */
                            task_interrupter = _ug_task_act;
                            end_flag = false;
                            break;
                        }
                    }
                }
            }
            /* It is no interrupter task or we already reached it earlier.
             * Just run the remaining tasks*/
            else {
                if(ug_task_exec(_ug_task_act)) {
                    if(!task_created && !task_deleted) {
                        task_interrupter = _ug_task_act; /*Check all tasks again from the highest priority */
                        end_flag         = false;
                        break;
                    }
                }
            }

            /*If a task was created or deleted then this or the next item might be corrupted*/
            if(task_created || task_deleted) {
                task_interrupter = NULL;
                break;
            }

            if(task_list_changed) {
                task_interrupter = NULL;
                end_flag = false;
                task_list_changed = false;
                break;
            }

            _ug_task_act = next; /*Load the next task*/
        }
    } while(!end_flag);

    busy_time += ug_tick_elaps(handler_start);
    uint32_t idle_period_time = ug_tick_elaps(idle_period_start);
    if(idle_period_time >= IDLE_MEAS_PERIOD) {

        idle_last         = (uint32_t)((uint32_t)busy_time * 100) / IDLE_MEAS_PERIOD; /*Calculate the busy percentage*/
        idle_last         = idle_last > 100 ? 0 : 100 - idle_last;                    /*But we need idle time*/
        busy_time         = 0;
        idle_period_start = ug_tick_get();
    }

    time_till_next = UG_NO_TASK_READY;
    next = _ug_ll_get_head(&_ug_task_ll);
    while(next) {
        if(next->prio != UG_TASK_PRIO_OFF) {
            uint32_t delay = ug_task_time_remaining(next);
            if(delay < time_till_next)
                time_till_next = delay;
        }

        next = _ug_ll_get_next(&_ug_task_ll, next); /*Find the next task*/
    }

    already_running = false; /*Release the mutex*/

    //UG_LOG_TRACE("ug_task_handler ready");
    return time_till_next;
}
/**
 * Create an "empty" task. It needs to initialized with at least
 * `ug_task_set_cb` and `ug_task_set_period`
 * @return pointer to the created task
 */
ug_task_t * ug_task_create_basic(void)
{
    ug_task_t * new_task = NULL;
    ug_task_t * tmp;

    /*Create task lists in order of priority from high to low*/
    tmp = _ug_ll_get_head(&_ug_task_ll);

    /*It's the first task*/
    if(NULL == tmp) {
        new_task = _ug_ll_ins_head(&_ug_task_ll);
        //UG_ASSERT_MEM(new_task);
        if(new_task == NULL) return NULL;
    }
    /*Insert the new task to proper place according to its priority*/
    else {
        do {
            if(tmp->prio <= DEF_PRIO) {
                new_task = _ug_ll_ins_prev(&_ug_task_ll, tmp);
                //UG_ASSERT_MEM(new_task);
                if(new_task == NULL) return NULL;
                break;
            }
            tmp = _ug_ll_get_next(&_ug_task_ll, tmp);
        } while(tmp != NULL);

        /*Only too high priority tasks were found. Add the task to the end*/
        if(tmp == NULL) {
            new_task = _ug_ll_ins_tail(&_ug_task_ll);
            //UG_ASSERT_MEM(new_task);
            if(new_task == NULL) return NULL;
        }
    }
    task_list_changed = true;

    new_task->period  = DEF_PERIOD;
    new_task->task_cb = NULL;
    new_task->prio    = DEF_PRIO;

    new_task->repeat_count = -1;
    new_task->last_run = ug_tick_get();

    new_task->user_data = NULL;

    task_created = true;

    return new_task;
}

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
ug_task_t * ug_task_create(ug_task_cb_t task_xcb, uint32_t period, ug_task_prio_t prio, void * user_data)
{
    ug_task_t * new_task = ug_task_create_basic();
    //UG_ASSERT_MEM(new_task);
    if(new_task == NULL) return NULL;

    ug_task_set_cb(new_task, task_xcb);
    ug_task_set_period(new_task, period);
    ug_task_set_prio(new_task, prio);
    new_task->user_data = user_data;

    return new_task;
}

/**
 * Set the callback the task (the function to call periodically)
 * @param task pointer to a task
 * @param task_cb the function to call periodically
 */
void ug_task_set_cb(ug_task_t * task, ug_task_cb_t task_cb)
{
    task->task_cb = task_cb;
}

/**
 * Delete a ug_task
 * @param task pointer to task created by task
 */
void ug_task_del(ug_task_t * task)
{
    _ug_ll_remove(&_ug_task_ll, task);
    task_list_changed = true;

    ug_mem_free(task);

    if(_ug_task_act == task) task_deleted = true; /*The active task was deleted*/
}

/**
 * Set new priority for a ug_task
 * @param task pointer to a ug_task
 * @param prio the new priority
 */
void ug_task_set_prio(ug_task_t * task, ug_task_prio_t prio)
{
    if(task->prio == prio) return;

    /*Find the tasks with new priority*/
    ug_task_t * i;
    _UG_LL_READ(_ug_task_ll, i) {
        if(i->prio <= prio) {
            if(i != task) _ug_ll_move_before(&_ug_task_ll, task, i);
            break;
        }
    }

    /*There was no such a low priority so far then add the node to the tail*/
    if(i == NULL) {
        _ug_ll_move_before(&_ug_task_ll, task, NULL);
    }
    task_list_changed = true;

    task->prio = prio;
}

/**
 * Set new period for a ug_task
 * @param task pointer to a ug_task
 * @param period the new period
 */
void ug_task_set_period(ug_task_t * task, uint32_t period)
{
    task->period = period;
}

/**
 * Make a ug_task ready. It will not wait its period.
 * @param task pointer to a ug_task.
 */
void ug_task_ready(ug_task_t * task)
{
    task->last_run = ug_tick_get() - task->period - 1;
}

/**
 * Set the number of times a task will repeat.
 * @param task pointer to a ug_task.
 * @param repeat_count -1 : infinity;  0 : stop ;  n>0: residual times
 */
void ug_task_set_repeat_count(ug_task_t * task, int32_t repeat_count)
{
    task->repeat_count = repeat_count;
}

/**
 * Reset a ug_task.
 * It will be called the previously set period milliseconds later.
 * @param task pointer to a ug_task.
 */
void ug_task_reset(ug_task_t * task)
{
    task->last_run = ug_tick_get();
}

/**
 * Enable or disable the whole ug_task handling
 * @param en: true: ug_task handling is running, false: ug_task handling is suspended
 */
void ug_task_enable(bool en)
{
    ug_task_run = en;
}

/**
 * Get idle percentage
 * @return the ug_task idle in percentage
 */
uint8_t ug_task_get_idle(void)
{
    return idle_last;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * Execute task if its the priority is appropriate
 * @param task pointer to ug_task
 * @return true: execute, false: not executed
 */
static bool ug_task_exec(ug_task_t * task)
{
    bool exec = false;

    if(ug_task_time_remaining(task) == 0) {
        task->last_run = ug_tick_get();
        task_deleted   = false;
        task_created   = false;
        if(task->task_cb) task->task_cb(task);

        /*Delete if it was a one shot ug_task*/
        if(task_deleted == false) { /*The task might be deleted by itself as well*/
            if(task->repeat_count > 0) {
                task->repeat_count--;
            }
            if(task->repeat_count == 0) {
                ug_task_del(task);
            }
        }
        exec = true;
    }

    return exec;
}

/**
 * Find out how much time remains before a task must be run.
 * @param task pointer to ug_task
 * @return the time remaining, or 0 if it needs to be run again
 */
static uint32_t ug_task_time_remaining(ug_task_t * task)
{
    /*Check if at least 'period' time elapsed*/
    uint32_t elp = ug_tick_elaps(task->last_run);
    if(elp >= task->period)
        return 0;
    return task->period - elp;
}







