#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "ast.h"
#include "vector.h"
#include "bytecode.h"

void codegen_root(sl_node_t *node, sl_binary_t *binary);
void codegen_ident(sl_node_t *node, sl_binary_t *binary);
void codegen_unary(sl_node_t *node, sl_binary_t *binary);
void codegen_binary(sl_node_t *node, sl_binary_t *binary);
void codegen_int(sl_node_t *node, sl_binary_t *binary);
void codegen_string(sl_node_t *node, sl_binary_t *binary);
void codegen_call(sl_node_t *node, sl_binary_t *binary);
void codegen_func(sl_node_t *node, sl_binary_t *binary);
void codegen_return(sl_node_t *node, sl_binary_t *binary);
void codegen_cond(sl_node_t *node, sl_binary_t *binary);
void codegen_loop(sl_node_t *node, sl_binary_t *binary);
void codegen_break(sl_node_t *node, sl_binary_t *binary);
void codegen_decl(sl_node_t *node, sl_binary_t *binary);
void codegen_index(sl_node_t *node, sl_binary_t *binary);
void codegen_block(sl_node_t *node, sl_binary_t *binary);
void codegen_member(sl_node_t *node, sl_binary_t *binary);

#endif
