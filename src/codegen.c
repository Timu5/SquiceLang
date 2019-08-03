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
void codegen_ident(node_t* node, binary_t* binary);
void codegen_unary(node_t* node, binary_t* binary);
void codegen_binary(node_t* node, binary_t* binary);
void codegen_int(node_t* node, binary_t* binary);
void codegen_string(node_t* node, binary_t* binary);
void codegen_call(node_t* node, binary_t* binary);
void codegen_func(node_t* node, binary_t* binary);
void codegen_return(node_t* node, binary_t* binary);
void codegen_cond(node_t* node, binary_t* binary);
void codegen_loop(node_t* node, binary_t* binary);
void codegen_break(node_t* node, binary_t* binary);
void codegen_decl(node_t* node, binary_t* binary);
void codegen_index(node_t* node, binary_t* binary);
void codegen_block(node_t* node, binary_t* binary);
void codegen_member(node_t* node, binary_t* binary);