#include "utils.h"
#include <stdlib.h>

#include <stdarg.h>
#include <stdio.h>


static void
_fatal(char *fmt, va_list ap )
{
    vfprintf(stderr, fmt, ap);
    abort();
}


void
fatal(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    _fatal(fmt, ap);
    va_end(ap);
}


void
assert_fatal(bool condition, char *fmt, ... )
{
    if (!condition) {
        va_list ap;
        va_start(ap, fmt);
        _fatal(fmt, ap);
        va_end(ap);
    }
}





void *
xmalloc(size_t size)
{
    void *result = malloc(size);
    if ( !result ) {
        perror("MEM: Memory allocation failed");
        abort();
    }
    return result;
}

void *
xrealloc(void *ptr, size_t newsize)
{
    void *result = realloc(ptr, newsize);
    if (!result) {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg), "MEM: Failed to reallocate %zu bytes of memory -> ERRNO", newsize);
        perror(err_msg);
        abort();
    }
    return result;
}
