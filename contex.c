#include "ex.h"
#include "contex.h"

ctx_t* ctx_new(ctx_t* parent)
{
    ctx_t* ctx = (ctx_t*)malloc(sizeof(ctx_t));
    ctx->parent = parent;
    ctx->vars = NULL;
    ctx->funcs = NULL;
    ctx->stack = stack_new();
    ctx->ret = NULL;
    return ctx;
}

void ctx_setvar(ctx_t* ctx, char* name, value_t* value)
{
    ctx_t* c = ctx;
    while(c)
    {
        var_t* v = c->vars;
        while(v)
        {
            if(strcmp(v->name, name) == 0)
            {
                v->val = value;
                return;
            }
            v = v->next;
        }
        c = c->parent;
    }
    throw("Variable %s not found.", name);
}

value_t* ctx_getvar(ctx_t* ctx, char* name)
{
    ctx_t* c = ctx;
    while(c)
    {
        var_t* v = c->vars;
        while(v)
        {
            if(strcmp(v->name, name) == 0)
                return v->val;

            v = v->next;
        }
        c = c->parent;
    }
    return NULL;
}

void ctx_addvar(ctx_t* ctx, char* name, value_t* val)
{
    var_t* tmp = ctx->vars;
    ctx->vars = (var_t*)malloc(sizeof(var_t));
    ctx->vars->name = name;
    ctx->vars->val = val;
    ctx->vars->next = tmp;
}

fn_t* ctx_getfn(ctx_t* ctx, char* name)
{
    ctx_t* c = ctx;
    while(c)
    {
        fn_t* f = c->funcs;
        while(f)
        {
            if(strcmp(f->name, name) == 0)
                return f;
            f = f->next;
        }
        c = c->parent;
    }
    return NULL;
}

void ctx_addfn(ctx_t* ctx, char* name, node_t* body, value_t* (*fn)(ctx_t*))
{
    fn_t* func = (fn_t*)malloc(sizeof(fn_t));
    func->name = strdup(name);
    func->body = body;
    func->fn = fn;
    func->native = fn ? 1 : 0;
    func->next = ctx->funcs;
    ctx->funcs = func;

}
