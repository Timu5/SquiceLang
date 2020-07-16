#include <stdio.h>

#include "SquiceLang.h"

static void print(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    int end = (int)sl_vector_size(ctx->stack);
    int start = end - n;
    for (int i = start; i < end; i++)
    {
        sl_value_t *v = ctx->stack[i];
        if (v->type == SL_VALUE_NUMBER)
            printf("%g", v->number);
        else if (v->type == SL_VALUE_STRING)
            printf("%s", v->string);
    }
    printf("\n");
    fflush(stdout);
}

static void list(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    sl_vector(sl_value_t *) arr = NULL;
    for (int i = (int)sl_vector_size(ctx->stack) - n; i < (int)sl_vector_size(ctx->stack); i++)
    {
        sl_vector_push(arr, ctx->stack[i]);
    }
    sl_vector_shrinkby(ctx->stack, n);

    sl_vector_push(ctx->stack, sl_value_array(arr));
}

static void dict(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    if (n != 0 && n != 2)
        throw("Function dict takes 0 or 2 arguments");

    if (n == 0)
        sl_vector_push(ctx->stack, sl_value_dict(NULL, NULL));
    else
    {
        sl_value_t *v = sl_vector_pop(ctx->stack);
        sl_value_t *k = sl_vector_pop(ctx->stack);
        if (v->type != SL_VALUE_ARRAY || k->type != SL_VALUE_ARRAY)
            throw("Function dict takes arguments of type array");
        sl_vector(char *) keys = NULL;
        for (int i = 0; i < sl_vector_size(k->array); i++)
        {
            sl_value_t *key = sl_value_get(i, k);
            if (key->type != SL_VALUE_STRING)
                throw("Key of dictionary must be a string");
            sl_vector_push(keys, strdup(key->string));
        }
        sl_vector(sl_value_t *) values = NULL;
        for (int i = 0; i < sl_vector_size(v->array); i++)
        {
            sl_vector_push(values, sl_value_get(i, v));
        }

        sl_vector_push(ctx->stack, sl_value_dict(keys, values));
    }
}

static void len(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    if (n != 1)
        throw("Function len takes exactly 1 argument");

    sl_value_t *v = sl_vector_pop(ctx->stack);
    if (v->type == SL_VALUE_STRING)
    {
        sl_vector_push(ctx->stack, sl_value_number((double)strlen(v->string)));
        return;
    }
    else if (v->type == SL_VALUE_ARRAY)
    {
        sl_vector_push(ctx->stack, sl_value_number((double)sl_vector_size(v->array)));
        return;
    }

    throw("Function len need argument of type string or array.");
}

static void ord(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    if (n != 1)
        throw("Function ord takes exactly 1 argument");

    sl_value_t *v = sl_vector_pop(ctx->stack);
    if (v->type == SL_VALUE_STRING)
    {
        sl_vector_push(ctx->stack, sl_value_number(v->string[0]));
        return;
    }

    throw("Function ord need argument of type string.");
}

static void chr(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    if (n != 1)
        throw("Function chr takes exactly 1 argument");

    sl_value_t *v = sl_vector_pop(ctx->stack);
    if (v->type == SL_VALUE_NUMBER)
    {
        char *data = (char*)mallock(sizeof(char));
        data[0] = (char)v->number; 
        sl_vector_push(ctx->stack, sl_value_string(data));
        return;
    }

    throw("Function chr need argument of type number.");
}

void sl_builtin_install(sl_ctx_t *ctx)
{
    sl_ctx_addfn(ctx, "print", 0, print);
    sl_ctx_addfn(ctx, "list", 0, list);
    sl_ctx_addfn(ctx, "dict", 0, dict);
    sl_ctx_addfn(ctx, "len", 0, len);
    sl_ctx_addfn(ctx, "ord", 0, ord);
    sl_ctx_addfn(ctx, "chr", 0, ord);
}
