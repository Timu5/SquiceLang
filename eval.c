#include <stdio.h>

#include "lexer.h"
#include "eval.h"
#include "ast.h"
#include "contex.h"
#include "ex.h"

void eval_root(node_t* node, ctx_t* ctx)
{
    node_list_t* f = node->root.funcs->block;
    while(f)
    {
        ctx_addfn(ctx, f->el->func.name, f->el, NULL);
        f = f->next;
    }
    node_list_t* n = node->root.stmts->block;
    while(n)
    {
        n->el->eval(n->el, ctx);
        n = n->next;
    }
}

void eval_ident(node_t* node, ctx_t* ctx)
{
    value_t* v = ctx_getvar(ctx, node->ident);
    if(!v)
        throw("Variable %s not found!", node->ident);

    stack_push(ctx->stack, v);
}

void eval_unary(node_t* node, ctx_t* ctx)
{
    node->unary.val->eval(node->unary.val, ctx);
    value_t* a = stack_pop(ctx->stack);
    stack_push(ctx->stack, value_unary(node->unary.op, a));
}

void eval_binary(node_t* node, ctx_t* ctx)
{
    node->binary.a->eval(node->binary.a, ctx);
    node->binary.b->eval(node->binary.b, ctx);
    value_t* b = stack_pop(ctx->stack);
    value_t* a = stack_pop(ctx->stack);
    if(node->binary.op == T_ASSIGN) // special case
        memcpy(a, b, sizeof(value_t));
    else
        stack_push(ctx->stack, value_binary(node->binary.op, a, b));
    
}

void eval_value(node_t* node, ctx_t* ctx)
{
    stack_push(ctx->stack, node->value);
}

void eval_call(node_t* node, ctx_t* ctx)
{
    fn_t* f = ctx_getfn(ctx, node->call.name);
    if(!f)
        throw("Cannot find function %s", node->call.name);

    int i = 0;
    node_list_t* n = node->call.args->block;
    while(n)
    {
        n->el->eval(n->el, ctx);
        n = n->next;
        i++;
    }

    stack_push(ctx->stack, value_number((double)i));
    
    if(f->native)
    {
        f->fn(ctx);
    }
    else
    {
        ctx_t* c = ctx_new(ctx);
        double argc = ((value_t*)stack_pop(ctx->stack))->number;
        if(argc != f->body->func.argc)
            throw("Wrong number of arguments");

        for(int i = f->body->func.argc - 1; i >= 0; i--)
            ctx_addvar(c, f->body->func.argv[i], (value_t*)stack_pop(ctx->stack));

        f->body->eval(f->body, c);
    }
}

value_t* _ret_value;

void eval_func(node_t* node, ctx_t* ctx)
{
    ctx->ret = malloc(sizeof(jmp_buf));
    if(!setjmp(*ctx->ret))
    {
        node->func.body->eval(node->func.body, ctx);
    }
    else // return
    {
        stack_push(ctx->parent->stack, _ret_value);
        goto fend;
    }
    stack_push(ctx->parent->stack, value_null());
fend:
    free(ctx->ret);
    ctx->ret = NULL;
}

void eval_return(node_t* node, ctx_t* ctx)
{
    if(node->ret != NULL)
        node->ret->eval(node->ret, ctx);
    else
        stack_push(ctx->stack, value_null());

    ctx_t* ret = ctx;
    while(ret)
    {
        if(ret->ret != 0)
            break;
        ret = ret->parent;
    }
    
    if(!ret->ret)
        throw("Nothing to return from");

    _ret_value = stack_pop(ctx->stack);
    longjmp(*ret->ret, 1);
}

void eval_cond(node_t* node, ctx_t* ctx)
{
    node->cond.arg->eval(node->cond.arg, ctx);
    value_t* arg = stack_pop(ctx->stack);
    if(arg->number != 0)
        node->cond.body->eval(node->cond.body, ctx);
    else if(node->cond.elsebody != NULL)
        node->cond.elsebody->eval(node->cond.elsebody, ctx);
}

void eval_loop(node_t* node, ctx_t* ctx)
{
label:
    node->loop.arg->eval(node->loop.arg, ctx);
    value_t* arg = stack_pop(ctx->stack);
    if(arg->number != 0)
    {
        node->loop.body->eval(node->loop.body, ctx);
        goto label;
    }
}

void eval_decl(node_t* node, ctx_t* ctx)
{
    if(node->decl.name->type != N_IDENT)
        throw("Declaration name must be identifier"); // error
    node->decl.value->eval(node->decl.value, ctx);
    ctx_addvar(ctx, node->decl.name->ident, stack_pop(ctx->stack));
}

void eval_index(node_t* node, ctx_t* ctx)
{
    node->index.var->eval(node->index.var, ctx);
    node->index.expr->eval(node->index.expr, ctx);
    value_t* idx = (value_t*)stack_pop(ctx->stack);
    if(idx->type != V_NUMBER)
        throw("Index must be a number");
    
    stack_push(ctx->stack, value_get((int)idx->number, stack_pop(ctx->stack))); 
}

void eval_block(node_t* node, ctx_t* ctx)
{
    ctx_t* c = ctx_new(ctx);

    node_list_t* n = node->block;
    while(n)
    {
        n->el->eval(n->el, c);
        n = n->next;
    }
}

