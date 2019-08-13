#include <stdio.h>

#include "codegen.h"
#include "ast.h"
#include "utils.h"
#include "bytecode.h"

void codegen_root(node_t *node, binary_t *binary)
{
    for (int i = 0; i < vector_size(node->root.funcs->block); i++)
    {
        int adr = bytecode_emitint(binary, O_PUSHN, 0x22222222);
        bytecode_emitstr(binary, O_STOREFN, node->root.funcs->block[i]->func.name);

        char *name = mprintf("func_%s", node->root.funcs->block[i]->func.name);
        bytecode_addtofill(binary, name, adr + 1);
    }

    for (int i = 0; i < vector_size(node->root.stmts->block); i++)
    {
        node_t *n = node->root.stmts->block[i];
        n->codegen(n, binary);
    }

    bytecode_emit(binary, O_RETN);

    for (int i = 0; i < vector_size(node->root.funcs->block); i++)
    {
        node_t *f = node->root.funcs->block[i];
        f->codegen(f, binary);
        // save adress and symbol name to binary
    }
}
void codegen_ident(node_t *node, binary_t *binary)
{
    bytecode_emitstr(binary, O_PUSHV, node->ident);
}

void codegen_unary(node_t *node, binary_t *binary)
{
    node->unary.val->codegen(node->unary.val, binary);
    bytecode_emitint(binary, O_UNARY, node->unary.op);
}

void codegen_binary(node_t *node, binary_t *binary)
{
    node->binary.a->codegen(node->binary.a, binary);
    node->binary.b->codegen(node->binary.b, binary);
    bytecode_emitint(binary, O_BINARY, node->binary.op);
}

void codegen_int(node_t *node, binary_t *binary)
{
    bytecode_emitint(binary, O_PUSHN, node->integer);
}

void codegen_string(node_t *node, binary_t *binary)
{
    bytecode_emitstr(binary, O_PUSHS, node->string);
}

void codegen_call(node_t *node, binary_t *binary)
{
    int i = 0;
    for (; i < vector_size(node->call.args->block); i++)
    {
        node_t *n = node->call.args->block[i];
        n->codegen(n, binary);
    }

    bytecode_emitint(binary, O_PUSHN, i);

    node->call.func->codegen(node->call.func, binary);

    if (node->call.func->type == N_MEMBER)
    {
        bytecode_emit(binary, O_CALL); // !!! TODO: add parent
    }
    else
    {
        bytecode_emit(binary, O_CALL);
    }
}

void codegen_func(node_t *node, binary_t *binary)
{
    //printf("\nfunc_%s:\n", node->func.name); // hmm, label !!!!
    char *name = mprintf("func_%s", node->func.name);
    bytecode_addlabel(binary, name, binary->size);

    bytecode_emitstr(binary, O_STORE, "argc");
    for (int i = vector_size(node->func.args) - 1; i >= 0; i--)
    {
        bytecode_emitstr(binary, O_STORE, node->func.args[i]);
    }

    node->func.body->codegen(node->func.body, binary);
    bytecode_emit(binary, O_RETN);
}

void codegen_return(node_t *node, binary_t *binary)
{
    if (node->ret != NULL)
    {
        node->ret->codegen(node->ret, binary);
        bytecode_emit(binary, O_RET);
    }
    else
    {
        bytecode_emit(binary, O_RETN);
    }
}

void codegen_cond(node_t *node, binary_t *binary)
{
    node->cond.arg->codegen(node->cond.arg, binary);
    int adr = bytecode_emitint(binary, O_BRZ, 0x22222222); // fill it later with adress!!!
    char *name = mprintf("cond_%d", binary->index);
    char *ename = mprintf("condend_%d", binary->index++);
    bytecode_addtofill(binary, name, adr + 1);
    node->cond.body->codegen(node->cond.body, binary);

    if (node->cond.elsebody != NULL)
    {
        adr = bytecode_emitint(binary, O_JMP, 0x22222222);
        bytecode_addlabel(binary, name, binary->size);
        bytecode_addtofill(binary, ename, adr + 1);
        node->cond.elsebody->codegen(node->cond.elsebody, binary);
        bytecode_addlabel(binary, ename, binary->size);
    }
    else
    {
        bytecode_addlabel(binary, name, binary->size);
    }
}

void codegen_loop(node_t *node, binary_t *binary)
{
    int old = binary->loop;
    int i = binary->index++;
    binary->loop = i;

    char *lname = mprintf("loops_%d", i);
    char *lename = mprintf("loopend_%d", i);

    bytecode_addlabel(binary, lname, binary->size);
    node->loop.arg->codegen(node->loop.arg, binary);
    int adr = bytecode_emitint(binary, O_BRZ, 0x22222222);
    bytecode_addtofill(binary, lename, adr + 1);
    node->loop.body->codegen(node->loop.body, binary);
    adr = bytecode_emitint(binary, O_JMP, 0x22222222);
    bytecode_addtofill(binary, lname, adr + 1);
    bytecode_addlabel(binary, lename, binary->size);

    binary->loop = old;
}

void codegen_break(node_t *node, binary_t *binary)
{
    if (binary->loop < 0)
        throw("No loop to break from");

    char *lename = mprintf("loopend_%d", binary->loop);

    int adr = bytecode_emitint(binary, O_JMP, 0x22222222);
    bytecode_addtofill(binary, lename, adr + 1);
    //bytecode_addlabel(binary, lename, binary->size);
}

void codegen_decl(node_t *node, binary_t *binary)
{
    if (node->decl.name->type != N_IDENT)
        throw("Declaration name must be identifier"); // error
    node->decl.value->codegen(node->decl.value, binary);
    bytecode_emitstr(binary, O_STORE, node->decl.name->ident);
}

void codegen_index(node_t *node, binary_t *binary)
{
    node->index.var->codegen(node->index.var, binary);
    node->index.expr->codegen(node->index.expr, binary);
    printf("index\n");
    bytecode_emit(binary, O_INDEX);
}

void codegen_block(node_t *node, binary_t *binary)
{
    //printf("block\n");

    for (int i = 0; i < vector_size(node->block); i++)
        node->block[i]->codegen(node->block[i], binary);

    //printf("blockend\n");
}

void codegen_member(node_t *node, binary_t *binary)
{
    node->member.parent->codegen(node->member.parent, binary);
    bytecode_emitstr(binary, O_MEMBER, node->member.name);
    printf("member %s\n", node->member.name);
}