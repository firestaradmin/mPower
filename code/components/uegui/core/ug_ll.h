/**
 * @brief a link list implemention for uvgui
 * @date 2020
 * @author firestaradmin
 * @ps it is study from LVGL
*/

#ifndef __UG_LL_H__
#define __UG_LL_H__

#include "ug_type.h"

/**********************
 *      MACROS
 **********************/

#define _UG_LL_READ(list, i) for(i = _ug_ll_get_head(&list); i != NULL; i = _ug_ll_get_next(&list, i))

#define _UG_LL_READ_BACK(list, i) for(i = _ug_ll_get_tail(&list); i != NULL; i = _ug_ll_get_prev(&list, i))



/** Dummy type to make handling easier*/
typedef uint8_t ug_ll_node_t;
/* in fact, the node type is as follows  */
/*
struct _ug_ll_node{
    uint8_t node[node_size];
    ug_ll_node_t * prev;
    ug_ll_node_t * next;
}
*/

/** Description of a linked list*/
typedef struct {
    uint32_t n_size;        /* node size in bytes */
    ug_ll_node_t * head;
    ug_ll_node_t * tail;
} ug_ll_t;




/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize linked list
 * @param ll_dsc pointer to ll_dsc variable
 * @param node_size the size of 1 node in bytes
 */
void _ug_ll_init(ug_ll_t * ll_p, uint32_t node_size);

/**
 * Add a new head to a linked list
 * @param ll_p pointer to linked list
 * @return pointer to the new head
 */
void * _ug_ll_ins_head(ug_ll_t * ll_p);

/**
 * Insert a new node in front of the n_act node
 * @param ll_p pointer to linked list
 * @param n_act pointer a node
 * @return pointer to the new head
 */
void * _ug_ll_ins_prev(ug_ll_t * ll_p, void * n_act);

/**
 * Add a new tail to a linked list
 * @param ll_p pointer to linked list
 * @return pointer to the new tail
 */
void * _ug_ll_ins_tail(ug_ll_t * ll_p);

/**
 * Remove the node 'node_p' from 'll_p' linked list.
 * It does not free the the memory of node.
 * @param ll_p pointer to the linked list of 'node_p'
 * @param node_p pointer to node in 'll_p' linked list
 */
void _ug_ll_remove(ug_ll_t * ll_p, void * node_p);

/**
 * Remove and free all elements from a linked list. The list remain valid but become empty.
 * @param ll_p pointer to linked list
 */
void _ug_ll_clear(ug_ll_t * ll_p);

/**
 * Move a node to a new linked list
 * @param ll_ori_p pointer to the original (old) linked list
 * @param ll_new_p pointer to the new linked list
 * @param node pointer to a node
 * @param head true: be the head in the new list
 *             false be the head in the new list
 */
void _ug_ll_chg_list(ug_ll_t * ll_ori_p, ug_ll_t * ll_new_p, void * node, bool head);

/**
 * Return with head node of the linked list
 * @param ll_p pointer to linked list
 * @return pointer to the head of 'll_p'
 */
void * _ug_ll_get_head(const ug_ll_t * ll_p);

/**
 * Return with tail node of the linked list
 * @param ll_p pointer to linked list
 * @return pointer to the head of 'll_p'
 */
void * _ug_ll_get_tail(const ug_ll_t * ll_p);

/**
 * Return with the pointer of the next node after 'n_act'
 * @param ll_p pointer to linked list
 * @param n_act pointer a node
 * @return pointer to the next node
 */
void * _ug_ll_get_next(const ug_ll_t * ll_p, const void * n_act);

/**
 * Return with the pointer of the previous node after 'n_act'
 * @param ll_p pointer to linked list
 * @param n_act pointer a node
 * @return pointer to the previous node
 */
void * _ug_ll_get_prev(const ug_ll_t * ll_p, const void * n_act);

/**
 * Return the length of the linked list.
 * @param ll_p pointer to linked list
 * @return length of the linked list
 */
uint32_t _ug_ll_get_len(const ug_ll_t * ll_p);

/**
 * TODO
 * @param ll_p
 * @param n1_p
 * @param n2_p
void ug_ll_swap(ug_ll_t * ll_p, void * n1_p, void * n2_p);
 */

/**
 * Move a node before an other node in the same linked list
 * @param ll_p pointer to a linked list
 * @param n_act pointer to node to move
 * @param n_after pointer to a node which should be after `n_act`
 */
void _ug_ll_move_before(ug_ll_t * ll_p, void * n_act, void * n_after);

/**
 * Check if a linked list is empty
 * @param ll_p pointer to a linked list
 * @return true: the linked list is empty; false: not empty
 */
bool _ug_ll_is_empty(ug_ll_t * ll_p);




#endif // !__UG_LL_H__

