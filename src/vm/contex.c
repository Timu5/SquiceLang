#include "SquiceLang.h"

sl_ctx_t *sl_ctx_new(sl_ctx_t *parent)
{
    sl_ctx_t *ctx = (sl_ctx_t *)malloc(sizeof(sl_ctx_t));
    ctx->parent = parent;
    if (parent)
        parent->child = ctx;
    ctx->child = NULL;
    ctx->vars = NULL;
    ctx->stack = NULL;
    return ctx;
}

void sl_ctx_free(sl_ctx_t *ctx)
{
    if (ctx->child)
    {
        sl_ctx_free(ctx->child);
        ctx->child = NULL;
    }

    for (int i = 0; i < sl_vector_size(ctx->vars); i++)
    {
        //free(ctx->vars[i]->name);
        //sl_value_free(ctx->vars[i]->val, 1);
        free(ctx->vars[i]);
    }
    sl_vector_free(ctx->vars);

    /* while(sl_vector_size(ctx->stack))
        sl_value_free(sl_vector_pop(ctx->stack), 1);*/
    sl_vector_free(ctx->stack);

    if (ctx->parent)
        ctx->parent->child = NULL;

    free(ctx);
}

sl_value_t *sl_ctx_getvar(sl_ctx_t *ctx, char *name)
{
    sl_ctx_t *c = ctx;
    while (c)
    {
        for (int i = (int)sl_vector_size(c->vars) - 1; i >= 0; i--)
        {
            sl_var_t *v = c->vars[i];

            if (strcmp(v->name, name) == 0)
                return v->val;
        }
        c = c->parent;
    }
    return NULL;
}

void sl_ctx_addvar(sl_ctx_t *ctx, char *name, sl_value_t *val)
{
    sl_var_t *var = (sl_var_t *)malloc(sizeof(sl_var_t));
    var->name = name;
    var->val = val;

    sl_vector_push(ctx->vars, var);
}

sl_fn_t *sl_ctx_getfn(sl_ctx_t *ctx, char *name)
{
    sl_value_t *v = sl_ctx_getvar(ctx, name);
    if (v == NULL || v->type != SL_VALUE_FN)
        return NULL;
    return v->fn;
}

void sl_ctx_addfn(sl_ctx_t *ctx, sl_binary_t *binary, char *name, int address, void (*fn)(sl_ctx_t *))
{
    sl_fn_t *func = (sl_fn_t *)malloc(sizeof(sl_fn_t));
    func->address = address;
    func->native = fn;
    func->binary = binary;
    func->ctx = ctx;

    sl_value_t *v = sl_value_fn(func);

    sl_ctx_addvar(ctx, name, v);
}
