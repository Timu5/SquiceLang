#ifndef _GC_H_
#define _GC_H_

#include "contex.h"

#define SL_ALLOC(type) sl_safe_alloc(sizeof(type))

void *sl_safe_alloc(int size);
sl_value_t *sl_gc_alloc_value();
void sl_gc_collect(ctx_t *ctx);
void sl_gc_freeall();

#endif
