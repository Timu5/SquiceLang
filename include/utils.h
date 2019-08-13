#ifndef _UTILS_H_
#define _UTILS_H_

#include <setjmp.h>

extern jmp_buf __ex_buf__;
extern char ex_msg[256];

#define try if (!setjmp(__ex_buf__))
#define catch else

void throw(char *msg, ...);

char *mprintf(char *fmt, ...);

#endif
