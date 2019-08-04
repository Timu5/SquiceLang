#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "ast.h"
#include "vector.h"

struct binary_s {
    vector(int) adresses;
    vector(char*) symbols;
    char* block;
    int index; // free label index
    int loop; // hold current loop index, needed by break
};

typedef struct binary_s binary_t;

binary_t* binary_new();

void codegen_root(node_t* node, binary_t* binary);
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

#endif
