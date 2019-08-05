#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

jmp_buf __ex_buf__;
char ex_msg[256];

void throw(char* msg, ...)
{
    va_list args;
    va_start(args, msg);
    vsnprintf(ex_msg, 256, msg, args);
    va_end(args);

    longjmp(__ex_buf__, 1);
}

char* mprintf(char *fmt, ...) {
    va_list va;
    va_start (va, fmt);
    char* buffer = (char*)malloc(124);
    vsprintf (buffer, fmt, va);
    va_end (va);
    return buffer;
}