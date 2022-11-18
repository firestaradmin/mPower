#ifndef __UG_MEM_H__
#define __UG_MEM_H__

#include <stdint.h>
//#include "ug_main.h"
#include <string.h>
#include "ug_type.h"
/*********************
 *      DEFINES
 *********************/
//#define UG_TEMPORAL_BUF
#ifndef UG_MEM_BUF_MAX_NUM
#define UG_MEM_BUF_MAX_NUM    16
#endif
/**********************
 *      TYPEDEFS
 **********************/

/**
 * Heap information structure.
 */
typedef struct {
    uint32_t total_size; /**< Total heap size */
    uint32_t free_cnt;
    uint32_t free_size; /**< Size of available memory */
    uint32_t free_biggest_size;
    uint32_t used_cnt;
    uint32_t max_used; /**< Max size of Heap memory used */
    uint8_t used_pct; /**< Percentage used */
    uint8_t frag_pct; /**< Amount of fragmentation */
} ug_mem_monitor_t;

typedef struct {
    void * p;
    uint16_t size;
    uint8_t used    : 1;
} ug_mem_buf_t;

typedef ug_mem_buf_t ug_mem_buf_arr_t[UG_MEM_BUF_MAX_NUM];
extern ug_mem_buf_arr_t _ug_mem_buf;




/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the dyn_mem module (work memory and other variables)
 */
void _ug_mem_init(void);

/**
 * Clean up the memory buffer which frees all the allocated memories.
 * @note It work only if `UG_MEM_CUSTOM == 0`
 */
void _ug_mem_deinit(void);

/**
 * Allocate a memory dynamically
 * @param size size of the memory to allocate in bytes
 * @return pointer to the allocated memory
 */
void * ug_mem_alloc(size_t size);

/**
 * Free an allocated data
 * @param data pointer to an allocated memory
 */
void ug_mem_free(const void * data);

/**
 * Reallocate a memory with a new size. The old content will be kept.
 * @param data pointer to an allocated memory.
 * Its content will be copied to the new memory block and freed
 * @param new_size the desired new size in byte
 * @return pointer to the new memory
 */
void * ug_mem_realloc(void * data_p, size_t new_size);

/**
 * Join the adjacent free memory blocks
 */
void ug_mem_defrag(void);

/**
 *
 * @return
 */
ug_res_t ug_mem_test(void);

/**
 * Give information about the work memory of dynamic allocation
 * @param mon_p pointer to a dm_mon_p variable,
 *              the result of the analysis will be stored here
 */
void ug_mem_monitor(ug_mem_monitor_t * mon_p);

/**
 * Give the size of an allocated memory
 * @param data pointer to an allocated memory
 * @return the size of data memory in bytes
 */
uint32_t _ug_mem_get_size(const void * data);
#ifdef UG_TEMPORAL_BUF
/**
 * Get a temporal buffer with the given size.
 * @param size the required size
 */
void * _ug_mem_buf_get(uint32_t size);

/**
 * Release a memory buffer
 * @param p buffer to release
 */
void _ug_mem_buf_release(void * p);

/**
 * Free all memory buffers
 */
void _ug_mem_buf_free_all(void);
#endif

//! @cond Doxygen_Suppress

/**
 * Same as `memcpy` but optimized for 4 byte operation.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
void * _ug_memcpy(void * dst, const void * src, size_t len);

/**
 * Same as `memcpy` but optimized to copy only a few bytes.
 * @param dst pointer to the destination buffer
 * @param src pointer to the source buffer
 * @param len number of byte to copy
 */
static inline void * _ug_memcpy_small(void * dst, const void * src, size_t len)
{
    uint8_t * d8 = (uint8_t *)dst;
    const uint8_t * s8 = (const uint8_t *)src;

    while(len) {
        *d8 = *s8;
        d8++;
        s8++;
        len--;
    }

    return dst;
}



void _ug_memset_00(void * dst, size_t len);

void _ug_memset(void * dst, uint8_t v, size_t len);

void * _ug_memcpy(void * dst, const void * src, size_t len);
#endif // !__UG_MEM_H__



