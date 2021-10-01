#include "worker.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>


struct tuple {
    int first;
    int second;
    int result;
};

void add(void *data)
{
    struct tuple *my_data = (struct tuple *) data;
    my_data->result = my_data->first + my_data->second;
}

void add2(void *data)
{
    int *my_data = (int *) data;
    (*my_data)++;
}

int main()
{
    worker_ctx test;
    struct tuple a = {.first=1, .second=3, .result=0}, b = {.first=5, .second=10, .result=0};
    int f = 0;
    //NO THREADS SHOULD NOT DO ANY WORK
    assert(worker_init(&test, 0) == OK);
    assert(test.threads_count == 0);
    assert(test.end_all == false);
    assert(worker_push_job(&test, add, (void *) &a) == 0);
    assert(worker_push_job(&test, add2, (void *) &f) == 0);
    assert(worker_destroy(&test) == OK);
    assert(test.threads_count == 0);
    assert(a.result == 0);
    assert(f == 0);
    //
    //SIMPLE TASK WITH 1 THREAD ONLY
    assert(worker_init(&test, 1) == OK);
    assert(test.threads_count == 1);
    assert(test.end_all == false);
    assert(worker_push_job(&test, add, (void *) &a) == 0);
    assert(worker_push_job(&test, add2, (void *) &f) == 0);
    assert(worker_destroy(&test) == OK);
    assert(test.threads_count == 0);
    assert(a.result == 4);
    assert(f == 1);
    //
    a.result = 0;
    f = 0;
    //MORE COMPLEX TASK WITH 1 THREAD
    assert(worker_init(&test, 1) == OK);
    assert(test.threads_count == 1);
    assert(test.end_all == false);
    for(int i = 0; i < 10000; ++i) {
        assert(worker_push_job(&test, add2, (void *) &f) == 0);
    }
    assert(worker_destroy(&test) == OK);
    assert(test.threads_count == 0);
    assert(f == 10000);
    //
    f = 0;
    //MORE THREADS TESTING
    assert(worker_init(&test, 4) == OK);
    assert(worker_push_job(&test, add, (void *) &a) == 0);
    assert(worker_push_job(&test, add, (void *) &b) == 0);
    for(int i = 0; i < 1234; ++i) {
        assert(worker_push_job(&test, add, (void *) &a) == 0);
    }
    assert(worker_destroy(&test) == OK);
    assert(test.threads_count == 0);
    assert(a.result == 4);
    assert(b.result == 15);
    //
    a.result = 0;
    b.result = 0;
    //TOO MANY THREADS TESTING
    assert(worker_init(&test, 100) == OK);
    for(int i = 0; i < 1000; ++i) {
        assert(worker_push_job(&test, add, (void *) &a) == 0);
    }
    for(int i = 0; i < 1000; ++i) {
        assert(worker_push_job(&test, add, (void *) &b) == 0);
    }
    assert(worker_destroy(&test) == OK);
    assert(test.threads_count == 0);
    assert(a.result == 4);
    assert(b.result == 15);
    //
    return 0;
}
