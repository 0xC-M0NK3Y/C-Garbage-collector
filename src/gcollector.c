#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>
#include <stddef.h>

#include <unistd.h>

#include <pthread.h>

#include "gcollector.h"

#define TRACKER_RELOC (collector.number >= collector.capacity-1)

typedef struct tracker {
    uintptr_t   ptr;
    size_t      size;
}   tracker_t;

typedef struct gcollector {
    tracker_t       *tracker;
    volatile size_t number;
    size_t          capacity;
    int             error_code;
    uintptr_t       stack_top;
    uintptr_t       stack_base;
    pthread_t       th;
    pthread_mutex_t mutex;
    volatile int    loop;
}   gcollector_t;


static gcollector_t collector;

static void *collector_routine(void *dummy);

//TODO: remove printfs

int gcollector_init(void) {

    memset(&collector, 0, sizeof(gcollector_t));

    register uintptr_t sptr asm("rbp"); // rsp in rbp, due to : mov rbp, rsp
    collector.stack_top = sptr + 0x10; // +0x10 due to precedent instruct push rbp, modifiyng rsp + function call
    __asm__("pop %rbp"); // rbp in top of the stack due to : push rbp
    register uintptr_t bptr asm("rbp"); // pop it get it
    collector.stack_base = bptr;
    __asm__("push %rbp"); // push it back !!

    collector.loop = 1;
    pthread_mutex_init(&collector.mutex, NULL);
    collector.error_code = 0;
    collector.tracker = malloc(10 * sizeof(tracker_t));
    if (collector.tracker == NULL)
        return collector.error_code = MALLOC_FAILED, -1;
    memset(collector.tracker, 0, 10 * sizeof(tracker_t));
    collector.capacity = 10;
    collector.number = 0;
    if (pthread_create(&collector.th, NULL, collector_routine, NULL) != 0)
        return collector.error_code = COLLECTOR_THREAD_FAILED, -1;
    return 1;
}

void *galloc(size_t size) {

    void *ret = malloc(size);

    pthread_mutex_lock(&collector.mutex);
    if (ret == NULL)
        return collector.error_code = MALLOC_FAILED, NULL;
    if (TRACKER_RELOC) {
        collector.tracker = realloc(collector.tracker, 2*collector.capacity * sizeof(tracker_t));
        if (collector.tracker == NULL)
            return collector.error_code = TRACKER_REALLOC_FAILED, NULL;
        collector.capacity *= 2;
    }
    collector.tracker[collector.number].ptr = (uintptr_t) ret;
    collector.tracker[collector.number].size = size;
    collector.number++;
    pthread_mutex_unlock(&collector.mutex);
    return ret;
}

static void *collector_routine(void *dummy) {

    time_t t = time(NULL);

    (void)dummy;
    while (collector.loop)
    {
        if (t != time(NULL)) {
            t = time(NULL);
            pthread_mutex_lock(&collector.mutex);
            for (size_t i = 0; i < collector.number;) {
                int valid = 0;
                for (uintptr_t ptr = collector.stack_top; ptr < collector.stack_base; ptr++) {
                    if ((uintptr_t)(*(void **)ptr) >= collector.tracker[i].ptr && (uintptr_t)(*(void **)ptr) <= collector.tracker[i].ptr + collector.tracker[i].size)
                        valid = 1;
                }
                if (!valid) {
                    printf("free 0x%lx\n", collector.tracker[i].ptr);
                    free((void *)collector.tracker[i].ptr);
                    memmove(&collector.tracker[i], &collector.tracker[i+1], (collector.number-i-1) * sizeof(tracker_t));
                    collector.number--;
                } else {
                    i++;
                }
            }
            pthread_mutex_unlock(&collector.mutex);
        }
        usleep(500);
    }
    pthread_exit(NULL);
    return NULL;
}

const char *gcollector_get_error(void) {
    switch (collector.error_code) {
    case MALLOC_FAILED: return "malloc failed";
    case TRACKER_REALLOC_FAILED: return "tracker realloc failed";
    case COLLECTOR_THREAD_FAILED: return "collector thread failed create";
    default: break;
    }
    return "Success";
}

int gcollector_quit(void) {
    collector.loop = 0;
    pthread_join(collector.th, NULL); // join to wait the thread to quit and free
    for (size_t i = 0; i < collector.number; i++) {
        printf("QUIT free 0x%lx\n", collector.tracker[i].ptr);
        free((void *)collector.tracker[i].ptr);
    }
    free(collector.tracker);
    pthread_mutex_destroy(&collector.mutex);
    return 1;
}