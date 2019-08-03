#include <stdio.h>

#include "codegen.h"
#include "ast.h"
#include "ex.h"

void codegen_root(node_t* node, binary_t* binary)
{
    for(int i = 0; i < vector_size(node->root.stmts->block); i++)
    {
        node_t* n = node->root.stmts->block[i];
        n->codegen(n, binary);
    }

    for(int i = 0; i < vector_size(node->root.funcs->block); i++)
    {
        node_t* f = node->root.funcs->block[i];
        f->codegen(f, binary);
        // save adress and symbol name to binary
    }
}
void codegen_ident(node_t* node, binary_t* binary)
{
    printf("push [%s]", node->ident);
}

void codegen_unary(node_t* node, binary_t* binary)
{
    node->unary.val->codegen(node->unary.val, binary);
    printf("unary %d", node->unary.op);
}

void codegen_binary(node_t* node, binary_t* binary)
{
    node->binary.a->codegen(node->binary.a, binary);
    node->binary.b->codegen(node->binary.b, binary);
    printf("binary %d", node->binary.op);
}

void codegen_int(node_t* node, binary_t* binary)
{
    printf("push %d", node->integer);
}

void codegen_string(node_t* node, binary_t* binary)
{
        printf("push %s", node->string);
}

void codegen_call(node_t* node, binary_t* binary)
{
    node->call.func->codegen(node->call.func, binary);

    int i = 0;
    for(; i < vector_size(node->call.args->block); i++)
    {
        node_t* n = node->call.args->block[i];
        n->codegen(n, binary);
    }

    printf("push %d", i);
    if (node->call.func->type == N_MEMBER)
    {
        printf("callm"); // !!! TODO: add parent
    }
    else
    {
        printf("call");
    }
}

void codegen_func(node_t* node, binary_t* binary)
{
    node->func.body->codegen(node->func.body, binary);
}

void codegen_return(node_t* node, binary_t* binary)
{
    printf("ret");
}

int ic = 0;

void codegen_cond(node_t* node, binary_t* binary)
{
    node->cond.arg->codegen(node->cond.arg, binary);
    printf("brz cond_%d", ic); // branch if zero
    node->cond.body->codegen(node->cond.body, binary);
    printf("cond_%d:", ic++);
    node->cond.elsebody->codegen(node->cond.elsebody, binary);
}

void codegen_loop(node_t* node, binary_t* binary)
{
    int i = ic++;
    printf("loops_%d:", i);
    node->loop.arg->codegen(node->loop.arg, binary);
    printf("brz loope_%d", i);
    node->loop.body->codegen(node->loop.body, binary);
    printf("jmp loops_%d", i);
    printf("loope_%d:", i);
}

void codegen_break(node_t* node, binary_t* binary);
void codegen_decl(node_t* node, binary_t* binary);
void codegen_index(node_t* node, binary_t* binary);
void codegen_block(node_t* node, binary_t* binary);
void codegen_member(node_t* node, binary_t* binary);