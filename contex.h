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

void ctx_setvar(char* name, value_t* value, ctx_t* ctx);
value_t* ctx_getvar(char* name, ctx_t* ctx);
void ctx_declvar(char* name, value_t* val, ctx_t* ctx);
fn_t* ctx_getfn(char* name, ctx_t* ctx);

#endif
