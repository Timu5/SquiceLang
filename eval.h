#ifndef _EVAL_H_
#define _EVAL_H_

#include "value.h"
#include "ast.h"
#include "contex.h"

void eval_root(node_t* node, ctx_t* ctx);
void eval_ident(node_t* node, ctx_t* ctx);
void eval_unary(node_t* node, ctx_t* ctx);
void eval_binary(node_t* node, ctx_t* ctx);
void eval_value(node_t* node, ctx_t* ctx);
void eval_call(node_t* node, ctx_t* ctx);
void eval_func(node_t* node, ctx_t* ctx);
void eval_return(node_t* node, ctx_t* ctx);
void eval_cond(node_t* node, ctx_t* ctx);
void eval_loop(node_t* node, ctx_t* ctx);
void eval_decl(node_t* node, ctx_t* ctx);
void eval_index(node_t* node, ctx_t* ctx);
void eval_block(node_t* node, ctx_t* ctx);

#endif
