#include <stdio.h>

#include "codegen.h"
#include "ast.h"
#include "ex.h"

binary_t* binary_new()
{
    binary_t* bin = (binary_t*)malloc(sizeof(binary_t));
    bin->adresses = NULL;
    bin->symbols = NULL;
    bin->block = NULL;
    bin->index = 0;
    bin->loop = -1;
    return bin;
}

void codegen_root(node_t* node, binary_t* binary)
{
    for(int i = 0; i < vector_size(node->root.stmts->block); i++)
    {
        node_t* n = node->root.stmts->block[i];
        n->codegen(n, binary);
    }

    printf("retn\n");

    for(int i = 0; i < vector_size(node->root.funcs->block); i++)
    {
        node_t* f = node->root.funcs->block[i];
        f->codegen(f, binary);
        // save adress and symbol name to binary
    }
}
void codegen_ident(node_t* node, binary_t* binary)
{
    printf("push [%s]\n", node->ident);
}

void codegen_unary(node_t* node, binary_t* binary)
{
    node->unary.val->codegen(node->unary.val, binary);
    printf("unary %d\n", node->unary.op);
}

void codegen_binary(node_t* node, binary_t* binary)
{
    node->binary.a->codegen(node->binary.a, binary);
    node->binary.b->codegen(node->binary.b, binary);
    printf("binary %d\n", node->binary.op);
}

void codegen_int(node_t* node, binary_t* binary)
{
    printf("push %d\n", node->integer);
}

void codegen_string(node_t* node, binary_t* binary)
{
    printf("push \"%s\"\n", node->string);
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

    printf("push %d\n", i);
    if (node->call.func->type == N_MEMBER)
    {
        printf("callm\n"); // !!! TODO: add parent
    }
    else
    {
        printf("call\n");
    }
}

void codegen_func(node_t* node, binary_t* binary)
{
    printf("\nfunc_%s:\n", node->func.name);
    node->func.body->codegen(node->func.body, binary);
    printf("retn\n");
}

void codegen_return(node_t* node, binary_t* binary)
{
    if(node->ret != NULL)
    {
        node->ret->codegen(node->ret, binary);
        printf("ret\n");
    }
    else
    {
        printf("retn\n");
    }
}

void codegen_cond(node_t* node, binary_t* binary)
{
    node->cond.arg->codegen(node->cond.arg, binary);
    printf("brz cond_%d\n", binary->index); // branch if zero
    node->cond.body->codegen(node->cond.body, binary);
    printf("cond_%d:\n", binary->index++);
    if(node->cond.elsebody != NULL)
        node->cond.elsebody->codegen(node->cond.elsebody, binary);
}

void codegen_loop(node_t* node, binary_t* binary)
{
    int old = binary->loop;
    int i = binary->index++;
    binary->loop = i;

    printf("loops_%d:\n", i);
    node->loop.arg->codegen(node->loop.arg, binary);
    printf("brz loopend_%d\n", i);
    node->loop.body->codegen(node->loop.body, binary);
    printf("jmp loops_%d\n", i);
    printf("loopend_%d:\n", i);

    binary->loop = old;
}

void codegen_break(node_t* node, binary_t* binary)
{
    if(binary->loop < 0)
        throw("No loop to break from");

    printf("jmp loopend_%d\n", binary->loop);
}

void codegen_decl(node_t* node, binary_t* binary)
{
    if(node->decl.name->type != N_IDENT)
        throw("Declaration name must be identifier"); // error
    node->decl.value->codegen(node->decl.value, binary);
    printf("store %s\n", node->decl.name->ident);
}

void codegen_index(node_t* node, binary_t* binary)
{
    node->index.var->codegen(node->index.var, binary);
    node->index.expr->codegen(node->index.expr, binary);
    printf("index\n");
}

void codegen_block(node_t* node, binary_t* binary)
{
    //printf("block\n");

    for(int i = 0; i < vector_size(node->block); i++)
        node->block[i]->codegen(node->block[i], binary);
    
    //printf("blockend\n");
}

void codegen_member(node_t* node, binary_t* binary)
{
    node->member.parent->codegen(node->member.parent, binary);
    printf("member %s\n", node->member.name);
}