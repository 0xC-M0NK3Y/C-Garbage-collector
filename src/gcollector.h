#ifndef GCOLLECTOR_H
# define GCOLLECTOR_H

#include <stdint.h>
#include <stddef.h>

#define MALLOC_FAILED           1
#define TRACKER_REALLOC_FAILED  2
#define COLLECTOR_THREAD_FAILED 3

int gcollector_init(void);
int gcollector_quit(void);
const char *gcollector_get_error(void);
void *galloc(size_t size);

#endif /* gcollector.h */