#ifndef _STACK_H_
#define _STACK_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ex.h"

struct stack_s {
    void** data;
    int allocated;
    int used;
};

typedef struct stack_s stack_t;

static inline stack_t* stack_new()
{
    stack_t* stack = (stack_t*)malloc(sizeof(stack_t*));
    stack->used = 0;
    stack->allocated = 128;
    stack->data = (void**)malloc(sizeof(void*) * 128);
    return stack;
}

static inline void stack_push(stack_t* stack, void* el)
{
    stack->used++;
    if(stack->used >= stack->allocated)
    {   
        stack->allocated *= 2;
        stack->data = realloc(stack->data, sizeof(void*) *  stack->allocated);
    }
    stack->data[stack->used -1] = el;   
}

static inline void* stack_peek(stack_t* stack)
{
    if(stack->used > 0)
        return stack->data[stack->used - 1];
    
    throw("Empty stack");
    
    return NULL;
}

static inline void* stack_pop(stack_t* stack)
{
    void* el = stack_peek(stack);
    if(el)
        stack->used--;
    return el;
}   

#endif
