#ifndef _EVAL_H_
#define _EVAL_H_

#include "value.h"
#include "ast.h"
#include "stack.h"

struct var_s {
	char* name;
	value_t* val;
	struct var_s* next;
};

struct fn_s {
	char* name;
	node_t* body;
	int native; // if not 0 call fn else eval body
	value_t* (*fn)(struct var_s* args);
	struct fn_s* next;
};

struct ctx_s {
	struct ctx_s* parent;
	struct var_s* vars;
	struct fn_s* funcs;
	stack_t* stack;
};

typedef struct var_s var_t;
typedef struct fn_s fn_t;
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
