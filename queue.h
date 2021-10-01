#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdlib.h>
#include <pthread.h>

#define OK 0
#define NULL_ERR -1
#define MALLOC_ERR -2
#define MUTEX_ERR -3

struct _node {
    struct _node *_next_item;
    void *_data;
};

/**
 * Implementation of queue using linked list
 */
typedef struct _queue {
    struct _node *_head;
    struct _node *_tail;
    size_t _size;
    pthread_mutex_t *mutex;
} queue_t;


/**
 * Initialize new queue
 *
 * @param queue Queue to initi
 * @return OK on success NULL_ERR when NULL is passed
 */
int queue_init(queue_t *queue);

/**
 * Enqueue element into queue
 *
 * @param queue where will be element stored
 * @param data data to be stored 
 */
int queue_enqueue(queue_t *queue, void *data);

/**
 * Pops first element from queue
 *
 * @param queue 
 * @return data from first element, NULL if queue is empty
 */
void *queue_dequeue(queue_t *queue);

/**
 * Cleans queue, when deallocator is passed it is used
 * to deallocate data in queue
 *
 * @param queue queue to clean
 * @param deallocator pointer to function which will be called on all data poped from queue
 *
 * @return OK on success, NULL_ERR if passed queue is null 
 */
int queue_destroy(queue_t *queue, void (*deallocator)(void *));

#endif /* _QUEUE_H */
