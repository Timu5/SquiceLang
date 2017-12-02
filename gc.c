#include <string.h>

#include "vector.h"
#include "value.h"
#include "ex.h"
#include "contex.h"
#include "gc.h"

vector(value_t*) values;

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
    return v;
}

void gc_collect(ctx_t* ctx)
{
    ctx_t* c = ctx;
    while(c)
    {
        for(int i = 0; i < vector_size(c->vars); i++)
        {
            c->vars[i]->val->markbit = 1;
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
            int l = vector_size(values);
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

