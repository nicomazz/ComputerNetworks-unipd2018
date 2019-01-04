#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>

void
fatal(char *fmt, ...);

void
assert_fatal(bool condition, char *fmt, ... );

void *
xmalloc(size_t size);

void *
xrealloc(void *ptr, size_t newsize);


#endif
