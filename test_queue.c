#include "worker.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

#define UNUSED(VAR) (void)(VAR)

static queue_t queue;

void *push( void *ptr ) {
    queue_enqueue(&queue, ptr);
    return NULL;
}

void *pop ( void *ptr ) {
    UNUSED(ptr);
    queue_dequeue(&queue);
    return NULL;
}

void *push_1000_times( void *ptr ) {
    for(int i = 0; i < 1000; ++i) {
        //printf("PUSH: %lu\n", pthread_self());
        queue_enqueue(&queue, ptr);
    }
    return NULL;
}

void *pop_1000_times( void *ptr ) {
    UNUSED(ptr);
    for(int i = 0; i < 1000; ++i) {
        //printf("POP: %lu\n", pthread_self());
        queue_dequeue(&queue);
    }
    return NULL;
}

void *pop_until_empty( void *ptr ) {
    UNUSED(ptr);
    while(queue._size != 0) {
        queue_dequeue(&queue);
    }
    return NULL;
}

int main()
{
    int a = 1;
    char b[5] = "TEST\0";
    pthread_t thread0, thread1, thread2, thread3;
    //EMPTY QUEUE
    assert(queue_init(&queue) == OK);
    assert(queue._size == 0);
    assert(queue._head == NULL);
    assert(queue._tail == NULL);
    //
    //ENQUEUE WITHOUT THREAD
    assert(queue_enqueue(&queue, &a) == 0);
    assert(queue._size == 1);
    assert(*((int *)queue._head->_data) == a);
    assert(*((int *)queue._tail->_data) == a);
    assert(queue._tail->_next_item == NULL);
    assert(queue._head->_next_item == NULL);
    //
    //ENQUEUE SECOND ITEM WITHOUT THREAD
    assert(queue_enqueue(&queue, &b) == 0);
    assert(queue._size == 2);
    assert(*((int *)queue._head->_data) == a);
    assert((char*)queue._tail->_data == b);
    assert(queue._tail->_next_item == NULL);
    assert(queue._head->_next_item == queue._tail);
    //
    //DEQUEUE WHEN ITEMS IN QUEUE WITHOUT THREAD
    assert(*((int *)queue_dequeue(&queue)) == a);
    assert(queue._size == 1);
    assert((char *)queue._head->_data == b);
    assert((char *)queue._tail->_data == b);
    assert((char *)queue_dequeue(&queue) == b);
    assert(queue._size == 0);
    assert(queue._head == NULL);
    assert(queue._tail == NULL);
    //
    //DEQUE WITH EMPTY QUEUE WITHOUT THREAD
    assert(queue_dequeue(&queue) == NULL);
    assert(queue._size == 0);
    assert(queue._head == NULL);
    assert(queue._tail == NULL);
    //
    //ENQUEUE WITH THREAD
    pthread_create( &thread0, NULL, push, &a);
    pthread_join(thread0, NULL);
    assert(queue._size == 1);
    assert(*((int *)queue._head->_data) == a);
    assert(*((int *)queue._tail->_data) == a);
    assert(queue._tail->_next_item == NULL);
    assert(queue._head->_next_item == NULL);
    pthread_create( &thread1, NULL, push, &b);
    pthread_join(thread1, NULL);
    assert(queue._size == 2);
    assert(*((int *)queue._head->_data) == a);
    assert((char*)queue._tail->_data == b);
    assert(queue._tail->_next_item == NULL);
    assert(queue._head->_next_item == queue._tail);
    //
    //DEQUEUE WITH THREAD
    pthread_create( &thread2, NULL, pop, NULL);
    pthread_join(thread2, NULL);
    assert(queue._size == 1);
    assert((char *)queue._head->_data == b);
    assert((char *)queue._tail->_data == b);
    pthread_create( &thread3, NULL, pop_until_empty, NULL);
    pthread_join(thread2, NULL);
    assert(queue._size == 0);
    assert(queue._head == NULL);
    assert(queue._tail == NULL);
    //
    //ENQUE MANY ITEMS WITH MORE THREADS
    pthread_create( &thread0, NULL, push_1000_times, &a);
    pthread_create( &thread1, NULL, push_1000_times, &a);
    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);
    pthread_create( &thread2, NULL, pop_1000_times, &a);
    pthread_create( &thread3, NULL, pop_1000_times, &a);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    assert(queue._size == 0);
    assert(queue._head == NULL);
    assert(queue._tail == NULL);
    //
    //WORK WITH MANY THREADS
    pthread_t threads[300];
    for (size_t i = 0; i < 300; ++i) {
        if (pthread_create(threads + i, NULL, push_1000_times, &a) != 0) {
            perror("\n\nThread creation failed:");
            exit(MALLOC_ERR);
        }
    }
    for(int i = 0; i < 300; i++) {
        pthread_join(threads[i], NULL);
    }
    assert(queue._size == 300000);
    pthread_create(&thread0, NULL, pop_until_empty, &a);
    pthread_join(thread0, NULL);
    assert(queue._size == 0);
    //
    assert(queue_destroy(&queue, NULL) == 0);
    return 0;
}
