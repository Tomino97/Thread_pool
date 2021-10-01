#include "worker.h"
#include <stdio.h>

#define UNUSED(VAR) (void)(VAR)

static void* worker_thread(void *worker_);

int worker_init(worker_ctx *worker, size_t number_of_threads)
{
    if (worker == NULL) {
        return NULL_ERR;
    }

    if(pthread_mutex_init(&(worker->mutex), NULL) != 0) {
        return MUTEX_ERR;
    }

    if(pthread_cond_init(&(worker->job_posted), NULL) != 0) {
        return MUTEX_ERR;
    }

    if(pthread_cond_init(&(worker->job_taken), NULL) != 0) {
        return MUTEX_ERR;
    }

    worker->end_all = false;
    worker->accept_more = true;

    if(queue_init(&(worker->_jobs_queue)) != 0) {
        return MUTEX_ERR;
    }

    worker->threads = (pthread_t *) malloc (number_of_threads * sizeof(pthread_t));
    if (worker->threads == NULL) {
        fprintf(stderr, "\n\nOut of memory allocating thread array!\n");
    }
    worker->threads_count = number_of_threads;
    for (size_t i = 0; i < number_of_threads; ++i) {
        if (pthread_create(worker->threads + i, NULL, worker_thread, (void *) worker) != 0) {
            perror("\n\nThread creation failed:");
            exit(THREAD_ERR);
        }
        //printf("DEBUG: Thread %lu created\n", i + 1);
    }

    return OK;
}


int worker_push_job(worker_ctx *worker, void (*job) (void *), void *data)
{
    if (pthread_mutex_lock(&worker->mutex) != 0)
    {
        perror("\nMutex lock failed!:");
        exit(MUTEX_ERR);
    }

    if(!worker->accept_more) {
        return -1;
    }

    if (worker == NULL || job == NULL) {
        return NULL_ERR;
    }

    worker_unit *to_queue = malloc(sizeof(worker_unit));
    to_queue->_func = job;
    to_queue->_data = data;

    int ret = queue_enqueue(&(worker->_jobs_queue), (void *) to_queue);
    //printf("DEBUG: Job pushed by %lu\n", pthread_self());
    pthread_cond_signal(&worker->job_posted);

    if (pthread_mutex_unlock(&worker->mutex) != 0) {
        perror("\n\nMutex unlock failed!:");
        exit(MUTEX_ERR);
    }

    return ret;
}


int worker_destroy(worker_ctx *worker_)
{
    worker_ctx *worker = (worker_ctx *)worker_;

    if (pthread_mutex_lock(&worker->mutex) != 0) {
        perror("Mutex lock failed (!!):");
        exit(MUTEX_ERR);
    }

    worker->end_all = true;
    worker->accept_more = false;

    //printf("DEBUG: Ending...\n");

    while(worker->_jobs_queue._size != 0) {
        if(worker->threads_count < 1) {
            break;
        }
        pthread_cond_signal(&worker->job_posted);
        pthread_cond_wait(&worker->job_taken, &worker->mutex);
    }

    if(pthread_cond_broadcast(&worker->job_posted) != 0) {
        perror("pthread_cond_broadcast() error");
        exit(2);
    }

    if (pthread_mutex_unlock(&worker->mutex) != 0) {
        perror("\n\nMutex unlock failed!:");
        exit(MUTEX_ERR);
    }

    for(int i = 0; i < worker->threads_count; i++) {
        //printf("DEBUG: Joining thread %lu\n", worker->threads[i]);
        pthread_join(worker->threads[i], NULL);
    }
    free(worker->threads);
    if(queue_destroy(&worker->_jobs_queue, free) != 0) {
        perror("\nQueue destruction failed!:");
        exit(MUTEX_ERR);
    }

    if (pthread_mutex_destroy(&worker->mutex) != 0) {
        perror("\nMutex destruction failed!:");
        exit(MUTEX_ERR);
    }

    if (pthread_cond_destroy(&worker->job_posted) != 0) {
        perror("\nCondition Variable 'job_posted' destruction failed!:");
        exit(MUTEX_ERR);
    }

    if (pthread_cond_destroy(&worker->job_taken) != 0) {
        perror("\nCondition Variable 'job_taken' destruction failed!:");
        exit(MUTEX_ERR);
    }

    worker->threads_count = 0;

    return OK;
}

static void* worker_thread(void *worker_)
{
    worker_ctx *worker = (worker_ctx *)worker_;

    if (pthread_mutex_lock(&worker->mutex) != 0)
    {
        perror("\nMutex lock failed!:");
        exit(MUTEX_ERR);
    }

    if (worker_ == NULL) {
        exit(NULL_ERR);
    }

    while (1) {
        //printf("IN WHILE\n");
        while(worker->_jobs_queue._size == 0) {
            if(worker->end_all && worker->_jobs_queue._size == 0) {
                break;
            }
            pthread_cond_wait(&worker->job_posted, &worker->mutex);
            //printf("DEBUG: Received job_posted signal in thread %lu\n", pthread_self());
        }
        //printf("OUT WHILE\n");

        if(worker->end_all && worker->_jobs_queue._size == 0) {
            //printf("DEBUG: Deleting thread %lu\n", pthread_self());
            break;
        }

        void *to_execute = NULL;
        //printf("Thread %lu ... ", pthread_self());
        to_execute = queue_dequeue(&(worker->_jobs_queue));

        worker_unit *job = (void *) to_execute;
        job->_func(job->_data);
        free(to_execute);

        if(worker->end_all) {
            pthread_cond_signal(&worker->job_taken);
        }

        if (pthread_mutex_unlock(&worker->mutex) != 0)
        {
            perror("\n\nMutex unlock failed!:");
            exit(MUTEX_ERR);
        }

        if (pthread_mutex_lock(&worker->mutex) != 0)
        {
            perror("\n\nMutex lock failed!:");
            exit(MUTEX_ERR);
        }
    }

    if (pthread_mutex_unlock(&worker->mutex) != 0)
    {
        perror("\n\nMutex unlock failed!:");
        exit(MUTEX_ERR);
    }
    return NULL;
}
