#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include "queue.h"
#include <stdlib.h>
#include <stdbool.h>

#define MUTEX_ERR -3
#define THREAD_ERR -4

/**
 * Context of worker.
 */
typedef struct _worker_ctx {
    queue_t _jobs_queue;
    pthread_t* threads;
    int threads_count;
    pthread_cond_t job_posted;
    pthread_cond_t job_taken;
    pthread_mutex_t mutex;
    bool end_all;
    bool accept_more;
} worker_ctx;

/**
 * Unit of runable job.
 */
typedef struct {
    void (*_func) (void *);
    void *_data;
} worker_unit;



/**
 * Initialize worker context
 * 
 *
 * @param worker holder to be initialized
 * @param number_of_threads unused
 * @return OK if worker is initialized, NULL_ERR if worker is NULL 
 */
int worker_init(worker_ctx *worker, size_t nubmer_of_threads);

/**
 * Pushes job into worker queue to be processed.
 *
 * @param worker worker which should execute job
 * @param job pointer to function which is to be executed
 * @param  data for job
 * @return OK if everything is successfull, NULL_ERR if worker or job are null, MALLOC_ERR if malloc fails 
 */
int worker_push_job(worker_ctx *worker, void (*job) (void *), void *data);

/**
 * Function executes jobs in queue.
 *
 * @param worker worker whose jobs should be runned
 * @return OK if everything is successfull
 */
int worker_exec(worker_ctx *worker);

/**
 *
 * @param worker worker to be destroyed
 * @return OK if everything is successfull, error code otherwise
 */
int worker_destroy(worker_ctx *worker);


#endif /*_THREAD_POOL_H*/
