#ifndef _EVAL_H_
#define _EVAL_H_

#include "value.h"
#include "stack.h"

struct var_s {
	char* name;
	value_t* val;
	struct var_s* next;
};

struct ctx_s {
	struct ctx_s* parent;
	struct var_s* vars;
	stack_t* stack;
};

typedef struct var_s var_t;
typedef struct ctx_s ctx_t;

struct node_s;

void eval_ident(struct node_s* node, ctx_t* ctx);
void eval_unary(struct node_s* node, ctx_t* ctx);
void eval_binary(struct node_s* node, ctx_t* ctx);
void eval_value(struct node_s* node, ctx_t* ctx);
void eval_call(struct node_s* node, ctx_t* ctx);
void eval_cond(struct node_s* node, ctx_t* ctx);
void eval_loop(struct node_s* node, ctx_t* ctx);
void eval_decl(struct node_s* node, ctx_t* ctx);
void eval_block(struct node_s* node, ctx_t* ctx);

#endif
