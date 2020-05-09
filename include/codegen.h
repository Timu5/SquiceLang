#ifndef _CODEGEN_H_
#define _CODEGEN_H_

#include "ast.h"
#include "vector.h"
#include "bytecode.h"

void sl_codegen_root(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_ident(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_unary(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_binary(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_int(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_string(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_call(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_func(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_return(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_cond(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_loop(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_break(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_decl(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_index(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_block(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_member(sl_node_t *node, sl_binary_t *binary);

#endif
