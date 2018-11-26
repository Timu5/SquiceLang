#ifndef _EX_H_
#define _EX_H_

#include <setjmp.h>

extern jmp_buf __ex_buf__;
extern char ex_msg[256];

#define try if(!setjmp(__ex_buf__))
#define catch else

void throw(char* msg, ...);

#endif
