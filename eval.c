#include <stdio.h>

#include "lexer.h"
#include "eval.h"
#include "ast.h"
#include "contex.h"
#include "ex.h"

void eval_root(node_t* node, ctx_t* ctx)
{
    for(int i = 0; i < vector_size(node->root.funcs->block); i++)
    {
        node_t* f = node->root.funcs->block[i];
        ctx_addfn(ctx, f->func.name, f, NULL);
    }

    for(int i = 0; i < vector_size(node->root.stmts->block); i++)
    {
        node_t* n = node->root.stmts->block[i];
        n->eval(n, ctx);
    }
}

void eval_ident(node_t* node, ctx_t* ctx)
{
    value_t* v = ctx_getvar(ctx, node->ident);
    if(!v)
        throw("Variable %s not found!", node->ident);

    vector_push(ctx->stack, v);
}

void eval_unary(node_t* node, ctx_t* ctx)
{
    node->unary.val->eval(node->unary.val, ctx);
    value_t* a = vector_pop(ctx->stack);
    vector_push(ctx->stack, value_unary(node->unary.op, a));
}

void eval_binary(node_t* node, ctx_t* ctx)
{
    node->binary.a->eval(node->binary.a, ctx);
    node->binary.b->eval(node->binary.b, ctx);
    value_t* b = vector_pop(ctx->stack);
    value_t* a = vector_pop(ctx->stack);
    if(node->binary.op == T_ASSIGN) // special case
        memcpy(a, b, sizeof(value_t));
    else
        vector_push(ctx->stack, value_binary(node->binary.op, a, b));
    
}

void eval_value(node_t* node, ctx_t* ctx)
{
    vector_push(ctx->stack, node->value);
}

void eval_call(node_t* node, ctx_t* ctx)
{
    fn_t* f = ctx_getfn(ctx, node->call.name);
    if(!f)
        throw("Cannot find function %s", node->call.name);
    
    ctx_t* c = ctx_new(ctx);

    int i = 0;
    for(; i < vector_size(node->call.args->block); i++)
    {
        node_t* n = node->call.args->block[i];
        n->eval(n, c);
    }

    vector_push(c->stack, value_number((double)i));
    
    if(f->native)
    {
        value_t* v = f->native(c);
        //vector_push(ctx->stack, v);
    }
    else
    {   
        int argc = vector_size(c->stack);
        if(argc != vector_size(f->body->func.args))
            throw("Wrong number of arguments");

        for(int i = vector_size(f->body->func.args) - 1; i >= 0; i--)
            ctx_addvar(c, f->body->func.args[i], vector_pop(c->stack));

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
        vector_push(ctx->parent->stack, _ret_value);
        goto fend;
    }
    vector_push(ctx->parent->stack, value_null());
fend:
    free(ctx->ret);
    ctx->ret = NULL;
}

void eval_return(node_t* node, ctx_t* ctx)
{
    if(node->ret != NULL)
        node->ret->eval(node->ret, ctx);
    else
        vector_push(ctx->stack, value_null());

    ctx_t* ret = ctx;
    while(ret)
    {
        if(ret->ret != 0)
            break;
        ret = ret->parent;
    }
    
    if(!ret->ret)
        throw("Nothing to return from");

    _ret_value = vector_pop(ctx->stack);
    longjmp(*ret->ret, 1);
}

void eval_cond(node_t* node, ctx_t* ctx)
{
    node->cond.arg->eval(node->cond.arg, ctx);
    value_t* arg = vector_pop(ctx->stack);
    if(arg->number != 0)
        node->cond.body->eval(node->cond.body, ctx);
    else if(node->cond.elsebody != NULL)
        node->cond.elsebody->eval(node->cond.elsebody, ctx);
}

void eval_loop(node_t* node, ctx_t* ctx)
{
label:
    node->loop.arg->eval(node->loop.arg, ctx);
    value_t* arg = vector_pop(ctx->stack);
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
    ctx_addvar(ctx, node->decl.name->ident, vector_pop(ctx->stack));
}

void eval_index(node_t* node, ctx_t* ctx)
{
    node->index.var->eval(node->index.var, ctx);
    node->index.expr->eval(node->index.expr, ctx);
    value_t* idx = vector_pop(ctx->stack);
    if(idx->type != V_NUMBER)
        throw("Index must be a number");
    
    value_t* v = vector_pop(ctx->stack);
    vector_push(ctx->stack, value_get((int)idx->number, v)); 
}

void eval_block(node_t* node, ctx_t* ctx)
{
    ctx_t* c = ctx_new(ctx);

    for(int i = 0; i < vector_size(node->block); i++)
        node->block[i]->eval(node->block[i], c);
}

