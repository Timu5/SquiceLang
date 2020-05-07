#include "builtin.h"
#include <stdio.h>

#include "utils.h"

static void print(ctx_t *ctx)
{
    int n = (int)vector_pop(ctx->stack)->number;
    int end = (int)vector_size(ctx->stack);
    int start = end - n;
    for (int i = start; i < end; i++)
    {
        value_t *v = ctx->stack[i];
        if (v->type == V_NUMBER)
            printf("%g", v->number);
        else if (v->type == V_STRING)
            printf("%s", v->string);
    }
    printf("\n");
    fflush(stdout);
}

static void list(ctx_t *ctx)
{
    int n = (int)vector_pop(ctx->stack)->number;
    vector(value_t *) arr = NULL;
    for (int i = (int)vector_size(ctx->stack) - n; i < (int)vector_size(ctx->stack); i++)
    {
        vector_push(arr, ctx->stack[i]);
    }
    vector_shrinkby(ctx->stack, n);

    vector_push(ctx->stack, value_array(arr));
}

static void dict(ctx_t *ctx)
{
    int n = (int)vector_pop(ctx->stack)->number;
    if (n != 0 && n != 2)
        throw("Function dict takes 0 or 2 arguments");

    if (n == 0)
        vector_push(ctx->stack, value_dict(NULL, NULL));
    else
    {
        value_t *v = vector_pop(ctx->stack);
        value_t *k = vector_pop(ctx->stack);
        if (v->type != V_ARRAY || k->type != V_ARRAY)
            throw("Function dict takes arguments of type array");
        vector(char *) keys = NULL;
        for (int i = 0; i < vector_size(k->array); i++)
        {
            value_t *key = value_get(i, k);
            if (key->type != V_STRING)
                throw("Key of dictonary must be a string");
            vector_push(keys, strdup(key->string));
        }
        vector(value_t *) values = NULL;
        for (int i = 0; i < vector_size(v->array); i++)
        {
            vector_push(values, value_get(i, v));
        }

        vector_push(ctx->stack, value_dict(keys, values));
    }
}

static void len(ctx_t *ctx)
{
    int n = (int)vector_pop(ctx->stack)->number;
    if (n != 1)
        throw("Function len takes exactly 1 argument");

    value_t *v = vector_pop(ctx->stack);
    if (v->type == V_STRING)
    {
        vector_push(ctx->stack, value_number((double)strlen(v->string)));
        return;
    }
    else if (v->type == V_ARRAY)
    {
        vector_push(ctx->stack, value_number((double)vector_size(v->array)));
        return;
    }

    throw("Function len need argument of type string or array.");
}

void builtin_install(ctx_t *ctx)
{
    ctx_addfn(ctx, "print", 0, print);
    ctx_addfn(ctx, "list", 0, list);
    ctx_addfn(ctx, "dict", 0, dict);
    ctx_addfn(ctx, "len", 0, len);
}
