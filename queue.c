#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

int queue_init(queue_t *queue)
{
    queue->_head = NULL;
    queue->_tail = NULL;
    queue->_size = 0;
    pthread_mutex_t *mutex = malloc(sizeof(*mutex));

    if(mutex == NULL) {
        return NULL_ERR;
    }

    if(pthread_mutex_init(mutex, NULL) != 0){
        return MUTEX_ERR;
    }

    queue->mutex = mutex;
    return OK;
}
 

int queue_enqueue(queue_t *queue, void *data)
{
    if (data == NULL) {
        return NULL_ERR;
    }

    struct _node *to_add = malloc(sizeof(struct _node));

    if (to_add == NULL) {
        return MALLOC_ERR;
    }

    to_add->_next_item = NULL;
    to_add->_data = data;

    if(pthread_mutex_lock(queue->mutex) != 0) {
        return MUTEX_ERR;
    }

    if (queue->_size == 0) {
        queue->_head = queue->_tail = to_add;
    } else {
        queue->_tail->_next_item = to_add;
        queue->_tail = to_add;
    }

    //printf("ENQUEUING: %p\n", to_add->_data);
    queue->_size++;

    if(pthread_mutex_unlock(queue->mutex) != 0) {
        return MUTEX_ERR;
    }

    return OK;

}

void *queue_dequeue(queue_t *queue)
{
    if(pthread_mutex_lock(queue->mutex) != 0) {
        return NULL;
    }

    if (queue->_size == 0) {
       pthread_mutex_unlock(queue->mutex);
       return NULL;
    }

    struct _node *to_ret = queue->_head;
    queue->_head = to_ret->_next_item;

    if (queue->_head == NULL) {
        queue->_tail = NULL;
    }
    queue->_size--;

    void *result = to_ret->_data;
    free(to_ret);

    if(pthread_mutex_unlock(queue->mutex)  != 0) {
        return NULL;
    }

    //printf("DEQUEUING: %p\n", result);

    return result;
}

int queue_destroy(queue_t *queue, void (*deallocator)(void*))
{
    if (queue == NULL) {
        return NULL_ERR;
    }

    void *data;

    while ((data = queue_dequeue(queue)) != NULL) {
        if (deallocator != NULL) {
            deallocator(data);
        }
    }

    pthread_mutex_destroy(queue->mutex);
    free(queue->mutex);

    return OK;
}
