#include <string.h>

#include "vector.h"
#include "value.h"
#include "utils.h"
#include "contex.h"
#include "gc.h"

sl_vector(sl_value_t *) values;

size_t maxmem = 128;
size_t usedmem = 0;

extern sl_ctx_t *global;

void *sl_safe_alloc(int size)
{
    void *data = malloc(size);
    if (!data)
        throw("Cannot alloc memory!");
    return data;
}

sl_value_t *sl_gc_alloc_value()
{
    void *v = sl_safe_alloc(sizeof(sl_value_t));
    sl_vector_push(values, v);

    usedmem += 1;
    if (usedmem >= maxmem)
    {
        //sl_gc_collect(global);
        usedmem = sl_vector_size(values);
        if (usedmem >= maxmem)
            maxmem = maxmem * 2;
    }

    return v;
}

static void gc_mark(sl_value_t *val)
{
    val->markbit = 1;
    if (val->type == SL_VALUE_REF)
    {
        val->markbit = 1;
        gc_mark(val->ref);
    }
    else if (val->type == SL_VALUE_ARRAY)
    {
        for (int j = (int)sl_vector_size(val->array) - 1; j >= 0; j--)
            val->array[j]->markbit = 1;
    }
    else if (val->type == SL_VALUE_DICT)
    {
        for (int j = (int)sl_vector_size(val->dict.values) - 1; j >= 0; j--)
            val->dict.values[j]->markbit = 1;
    }
}

void sl_gc_collect(sl_ctx_t *ctx)
{
    sl_ctx_t *c = ctx;
    while (c)
    {
        for (int i = 0; i < sl_vector_size(c->vars); i++)
        {
            gc_mark(c->vars[i]->val);
        }
        for (int i = 0; i < sl_vector_size(c->stack); i++)
        {
            gc_mark(c->stack[i]);
        }
        c = c->child;
    }

    for (int i = (int)sl_vector_size(values) - 1; i >= 0; i--)
    {
        if (values[i]->markbit == 0)
        {
            sl_value_free(values[i]);
            int l = (int)sl_vector_size(values) - 1;
            if (i != l)
                values[i] = values[l];
            sl_vector_pop(values);
        }
        else
            values[i]->markbit = 0;
    }
}

void sl_gc_freeall()
{
    for (int i = 0; i < sl_vector_size(values); i++)
    {
        sl_value_free(values[i]);
    }
    sl_vector_free(values);
}
