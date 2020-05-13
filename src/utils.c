#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "SquiceLang.h"

jmp_buf __ex_buf__;
char ex_msg[256];

void throw(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    vsnprintf(ex_msg, 256, msg, args);
    va_end(args);

    longjmp(__ex_buf__, 1);
}

char *sl_mprintf(char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    char *buffer = (char *)malloc(124);
    vsprintf(buffer, fmt, va);
    va_end(va);
    return buffer;
}

void vec_grow(void **vector, size_t more, size_t type_size)
{
    sl_vector_t *meta = sl_vector_meta(*vector);
    size_t count = 0;
    void *data = NULL;

    if (*vector)
    {
        count = 2 * meta->allocated + more;
        data = realloc(meta, type_size * count + sizeof *meta);
    }
    else
    {
        count = more + 1;
        data = malloc(type_size * count + sizeof *meta);
        ((sl_vector_t *)data)->used = 0;
    }

    meta = (sl_vector_t *)data;
    meta->allocated = count;
    *vector = meta + 1;
}