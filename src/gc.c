#include <string.h>

#include "vector.h"
#include "value.h"
#include "ex.h"
#include "contex.h"
#include "gc.h"

vector(value_t*) values;

size_t maxmem = 128;
size_t usedmem = 0;

extern ctx_t* global;

void* safe_alloc(int size)
{
    void* data = malloc(size);
    if(!data)
        throw("Cannot alloc memory!");
    return data;
}

value_t* gc_alloc_value()
{
    void* v = safe_alloc(sizeof(value_t));
    vector_push(values, v);

    usedmem += 1;
    if (usedmem >= maxmem)
    {
        gc_collect(global);
        usedmem = vector_size(values);
        if (usedmem >= maxmem)
            maxmem = maxmem * 2;
    }

    return v;
}

void gc_collect(ctx_t* ctx)
{
    ctx_t* c = ctx;
    while(c)
    {
        for(int i = 0; i < vector_size(c->vars); i++)
        {
            value_t* val = c->vars[i]->val;
            val->markbit = 1;
            while (val->type == V_REF)
            {
                val->markbit = 1;
                val = val->ref;
            }
            if (val->type == V_ARRAY)
            {
                for (int j = vector_size(val->array) - 1; j >= 0; j--)
                    val->array[j]->markbit = 1;
            }
            else if (val->type == V_DICT)
            {
                for (int j = vector_size(val->dict.values) - 1; j >= 0; j--)
                    val->dict.values[j]->markbit = 1;
            }
        }
        for(int i = 0; i < vector_size(c->stack); i++)
        {
            c->stack[i]->markbit = 1;
        }
        c = c->child;
    }

    for(int i = vector_size(values) - 1; i >= 0; i--)
    {
        if(values[i]->markbit == 0)
        {
            value_free(values[i]);
            int l = vector_size(values) - 1;
            if(i != l)
                values[i] = values[l];
            vector_pop(values);
        }
        else
            values[i]->markbit = 0;
    }
}

void gc_freeall()
{
    for(int i = 0; i < vector_size(values); i++)
    {
        value_free(values[i]);
    }
    vector_free(values);
}

