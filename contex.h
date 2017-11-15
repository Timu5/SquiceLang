#ifndef _CONTEX_H_
#define _CONTEX_H_

#include "ast.h"
#include "stack.h"
#include "value.h"

struct var_s {
	char* name;
	value_t* val;
	struct var_s* next;
};

struct ctx_s;

struct fn_s {
	char* name;
	node_t* body;
	int native; // if not 0 call fn else eval body
	value_t* (*fn)(struct ctx_s*);
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

ctx_t* ctx_new(ctx_t* parent);
void ctx_setvar(ctx_t* ctx, char* name, value_t* value);
value_t* ctx_getvar(ctx_t* ctx, char* name);
void ctx_addvar(ctx_t* ctx, char* name, value_t* val);
fn_t* ctx_getfn(ctx_t* ctx, char* name);
void ctx_addfn(ctx_t* ctx, char* name, node_t* body, value_t* (*fn)(ctx_t*));

#endif