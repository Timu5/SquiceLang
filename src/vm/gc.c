#include <string.h>

#include "SquiceLang.h"

sl_vector(sl_value_t *) values;
sl_vector(sl_ctx_t *) ctxs;

size_t maxmem = 128;
size_t usedmem = 0;

extern sl_ctx_t *global;
extern sl_ctx_t **current;
extern sl_vector(sl_ctx_t *) ctx_stack;

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
    return v;
}

void sl_gc_trigger()
{
    if (usedmem >= maxmem)
    {
        sl_gc_collect(global);
        usedmem = sl_vector_size(values);
        if (usedmem >= maxmem)
            maxmem = maxmem * 2;
    }
}

sl_ctx_t *sl_gc_alloc_ctx()
{
    void *ctx = sl_safe_alloc(sizeof(sl_ctx_t));
    sl_vector_push(ctxs, ctx);
    return ctx;
}

static void gc_mark(sl_value_t *val);

static void gc_markctx(sl_ctx_t *c)
{
    while (c)
    {
        if (c->markbit != 1)
        {
            c->markbit = 1;
            for (int i = 0; i < sl_vector_size(c->vars); i++)
            {
                gc_mark(c->vars[i]->val);
            }
            for (int i = 0; i < sl_vector_size(c->stack); i++)
            {
                gc_mark(c->stack[i]);
            }
        }
        c = c->parent;
    }
}

static void gc_mark(sl_value_t *val)
{
    if (val->markbit == 1)
        return;

    val->markbit = 1;

    if (val->type == SL_VALUE_REF)
    {
        gc_mark(val->ref);
    }
    else if (val->type == SL_VALUE_ARRAY)
    {
        for (int j = (int)sl_vector_size(val->array) - 1; j >= 0; j--)
            gc_mark(val->array[j]);
    }
    else if (val->type == SL_VALUE_TUPLE)
    {
        for (int j = (int)sl_vector_size(val->tuple) - 1; j >= 0; j--)
            gc_mark(val->array[j]);
    }
    else if (val->type == SL_VALUE_DICT)
    {
        for (int j = (int)sl_vector_size(val->dict.values) - 1; j >= 0; j--)
            gc_mark(val->dict.values[j]);
    }
    else if (val->type == SL_VALUE_FN)
    {
        // mark function context variables
        gc_markctx(val->fn->ctx);
    }
}

void sl_gc_collect(sl_ctx_t *ctx)
{
    for (size_t i = 0; i < sl_vector_size(ctx_stack); i++)
    {
        sl_ctx_t *c = ctx_stack[i];
        gc_markctx(c);
    }

    gc_markctx(*current);

    for (int i = (int)sl_vector_size(values) - 1; i >= 0; i--)
    {
        sl_value_t *v = values[i];

        if (v->markbit == 0)
        {
            sl_value_free(v);
            int l = (int)sl_vector_size(values) - 1;
            if (i != l)
                values[i] = values[l];
            sl_vector_pop(values);
        }
        else
        {
            v->markbit = 0;
        }
    }

    for (int i = (int)sl_vector_size(ctxs) - 1; i >= 0; i--)
    {
        if (ctxs[i]->markbit == 0)
        {
            sl_ctx_free(ctxs[i]);
            int l = (int)sl_vector_size(ctxs) - 1;
            if (i != l)
                ctxs[i] = ctxs[l];
            sl_vector_pop(ctxs);
        }
        else
        {
            ctxs[i]->markbit = 0;
        }
    }
}

void sl_gc_freeall()
{
    for (int i = 0; i < sl_vector_size(ctxs); i++)
    {
        sl_ctx_free(ctxs[i]);
    }
    sl_vector_free(ctxs);
    for (int i = 0; i < sl_vector_size(values); i++)
    {
        sl_value_free(values[i]);
    }
    sl_vector_free(values);
    usedmem = 0;
    maxmem = 128;
}
