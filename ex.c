#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>

#include "ex.h"

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
