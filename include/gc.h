#ifndef _GC_H_
#define _GC_H_

#include "contex.h"

#define ALLOC(type) safe_alloc(sizeof(type))

void *safe_alloc(int size);
value_t *gc_alloc_value();
void gc_collect(ctx_t *ctx);
void gc_freeall();

#endif
