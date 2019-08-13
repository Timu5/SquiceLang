#include "utils.h"
#include "contex.h"

ctx_t *ctx_new(ctx_t *parent)
{
    ctx_t *ctx = (ctx_t *)malloc(sizeof(ctx_t));
    ctx->parent = parent;
    if (parent)
        parent->child = ctx;
    ctx->child = NULL;
    ctx->vars = NULL;
    ctx->stack = NULL;
    ctx->ret = NULL;
    ctx->retLoop = NULL;
    return ctx;
}

void ctx_free(ctx_t *ctx)
{
    if (ctx->child)
    {
        ctx_free(ctx->child);
        ctx->child = NULL;
    }

    for (int i = 0; i < vector_size(ctx->vars); i++)
    {
        //free(ctx->vars[i]->name);
        //value_free(ctx->vars[i]->val, 1);
        free(ctx->vars[i]);
    }
    vector_free(ctx->vars);

    /* while(vector_size(ctx->stack))
        value_free(vector_pop(ctx->stack), 1);*/
    vector_free(ctx->stack);
    free(ctx->ret);
    free(ctx->retLoop);

    if (ctx->parent)
        ctx->parent->child = NULL;

    free(ctx);
}

value_t *ctx_getvar(ctx_t *ctx, char *name)
{
    ctx_t *c = ctx;
    while (c)
    {
        for (int i = vector_size(c->vars) - 1; i >= 0; i--)
        {
            var_t *v = c->vars[i];

            if (strcmp(v->name, name) == 0)
                return v->val;
        }
        c = c->parent;
    }
    return NULL;
}

void ctx_addvar(ctx_t *ctx, char *name, value_t *val)
{
    var_t *var = (var_t *)malloc(sizeof(var_t));
    var->name = name;
    var->val = val;

    vector_push(ctx->vars, var);
}

fn_t *ctx_getfn(ctx_t *ctx, char *name)
{
    value_t *v = ctx_getvar(ctx, name);
    if (v == NULL || v->type != V_FN)
        return NULL;
    return v->fn;
}

void ctx_addfn(ctx_t *ctx, char *name, int address, value_t *(*fn)(ctx_t *))
{
    fn_t *func = (fn_t *)malloc(sizeof(fn_t));
    func->address = address;
    func->native = fn;

    value_t *v = value_fn(func);

    ctx_addvar(ctx, name, v);
}
