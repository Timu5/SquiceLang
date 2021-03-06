#include <stdio.h>

#include "SquiceLang.h"

static void print(int argc, sl_ctx_t *ctx)
{
    int end = (int)sl_vector_size(ctx->stack);
    for (int i = 0; i < argc; i++)
    {
        sl_value_t *v = sl_vector_pop(ctx->stack);
        if (v->type == SL_VALUE_NUMBER)
            printf("%g", v->number);
        else if (v->type == SL_VALUE_STRING)
            printf("%s", v->string);
    }
    printf("\n");
    fflush(stdout);
    sl_vector_push(ctx->stack, sl_value_null());
}

static void list(int argc, sl_ctx_t *ctx)
{
    sl_vector(sl_value_t *) arr = NULL;
    for (int i = (int)sl_vector_size(ctx->stack) - 1; i >= (int)sl_vector_size(ctx->stack) - argc; i--)
    {
        sl_vector_push(arr, ctx->stack[i]);
    }
    sl_vector_shrinkby(ctx->stack, argc);

    sl_vector_push(ctx->stack, sl_value_array(arr));
}

static void dict(int argc, sl_ctx_t *ctx)
{
    if (argc != 0 && argc != 2)
        sl_throw("Function dict() takes 0 or 2 arguments");

    if (argc == 0)
        sl_vector_push(ctx->stack, sl_value_dict(NULL, NULL));
    else
    {
        sl_value_t *k = sl_vector_pop(ctx->stack);
        sl_value_t *v = sl_vector_pop(ctx->stack);
        if (v->type != SL_VALUE_ARRAY || k->type != SL_VALUE_ARRAY)
            sl_throw("Function dict takes arguments of type array");
        sl_vector(char *) keys = NULL;
        for (size_t i = 0; i < sl_vector_size(k->array); i++)
        {
            sl_value_t *key = sl_value_get(i, k);
            if (key->type != SL_VALUE_STRING)
                sl_throw("Key of dictionary must be a string");
            sl_vector_push(keys, strdup(key->string));
        }
        sl_vector(sl_value_t *) values = NULL;
        for (size_t i = 0; i < sl_vector_size(v->array); i++)
        {
            sl_vector_push(values, sl_value_get(i, v));
        }

        sl_vector_push(ctx->stack, sl_value_dict(keys, values));
    }
}

static void len(int argc, sl_ctx_t *ctx)
{
    sl_value_t *v = sl_vector_pop(ctx->stack);
    while (v->type == SL_VALUE_REF)
        v = v->ref;
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

    sl_throw("Function len() need argument of type string or array.");
}

static void ord(int argc, sl_ctx_t *ctx)
{
    sl_value_t *v = sl_vector_pop(ctx->stack);
    while (v->type == SL_VALUE_REF)
        v = v->ref;

    if (v->type == SL_VALUE_STRING)
    {
        sl_vector_push(ctx->stack, sl_value_number(v->string[0]));
        return;
    }

    sl_throw("Function ord need argument of type string");
}

static void chr(int argc, sl_ctx_t *ctx)
{
    sl_value_t *v = sl_vector_pop(ctx->stack);
    if (v->type == SL_VALUE_NUMBER)
    {
        char *data = (char *)malloc(sizeof(char));
        data[0] = (char)v->number;
        sl_vector_push(ctx->stack, sl_value_string(data));
        return;
    }

    sl_throw("Function chr need argument of type number.");
}

static void super(int argc, sl_ctx_t *ctx)
{
    sl_value_t *a = sl_vector_pop(ctx->stack);
    sl_value_t *b = sl_vector_pop(ctx->stack);
    while (a->type == SL_VALUE_REF)
        a = a->ref;

    while (b->type == SL_VALUE_REF)
        b = b->ref;

    if (a->type == SL_VALUE_DICT && b->type == SL_VALUE_DICT)
    {
        for (size_t i = 0; i < sl_vector_size(b->dict.names); i++)
        {
            int flag = 0;
            for (size_t j = 0; j < sl_vector_size(a->dict.names); j++)
            {
                if (strcmp(a->dict.names[j], b->dict.names[i]) == 0)
                {
                    goto end;
                }
            }
            sl_vector_push(a->dict.names, strdup(b->dict.names[i]));
            sl_vector_push(a->dict.values, b->dict.values[i]);
        end:;
        }

        sl_vector_push(ctx->stack, a);
        return;
    }

    sl_throw("Function _super_ need arguments of type dict.");
}

static void str(int argc, sl_ctx_t *ctx)
{
    sl_value_t *v = sl_vector_pop(ctx->stack);
    if (v->type == SL_VALUE_STRING)
    {
        sl_vector_push(ctx->stack, v);
        return;
    }

    char *str = NULL;
    if (v->type == SL_VALUE_NUMBER)
        str = sl_mprintf("%g", v->number);
    else
        str = sl_mprintf("[object]");

    sl_vector_push(ctx->stack, sl_value_string(str));
}

static void isnull(int argc, sl_ctx_t *ctx)
{
    sl_value_t *val = sl_vector_pop(ctx->stack);
    sl_vector_push(ctx->stack, sl_value_number(val->type == SL_VALUE_NULL));
}

static void input(int argc, sl_ctx_t *ctx)
{
    if (argc > 0)
    {
        sl_value_t *v = sl_vector_pop(ctx->stack);
        printf("%s", v->string);
    }

    char buff[256];

    if (scanf("%255s", buff) == 0)
        sl_vector_push(ctx->stack, sl_value_string(strdup("")));
    else
        sl_vector_push(ctx->stack, sl_value_string(strdup(buff)));
}

void sl_builtin_install(sl_ctx_t *ctx)
{
    sl_ctx_addfn(ctx, NULL, strdup("print"), 0, 0, print);
    sl_ctx_addfn(ctx, NULL, strdup("list"), 0, 0, list);
    sl_ctx_addfn(ctx, NULL, strdup("dict"), 0, 0, dict);
    sl_ctx_addfn(ctx, NULL, strdup("len"), 1, 0, len);
    sl_ctx_addfn(ctx, NULL, strdup("ord"), 1, 0, ord);
    sl_ctx_addfn(ctx, NULL, strdup("chr"), 1, 0, chr);
    sl_ctx_addfn(ctx, NULL, strdup("_super_"), 2, 0, super);
    sl_ctx_addfn(ctx, NULL, strdup("str"), 1, 0, str);
    sl_ctx_addfn(ctx, NULL, strdup("isnull"), 1, 0, isnull);
    sl_ctx_addfn(ctx, NULL, strdup("input"), 0, 0, input);

    sl_ctx_addvar(ctx, strdup("null"), sl_value_null());
    sl_ctx_addvar(ctx, strdup("true"), sl_value_number(1.0));
    sl_ctx_addvar(ctx, strdup("false"), sl_value_number(0.0));
}
