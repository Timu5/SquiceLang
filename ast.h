#ifndef _AST_H_
#define _AST_H_

#include "value.h"

enum {
	N_ROOT,
	N_IDENT,
	N_UNARY,
	N_BINARY,
	N_VALUE,
	N_CALL,
	N_FUNC,
	N_COND,
	N_LOOP,
	N_DECL,
	N_BLOCK
};

struct node_list_s;
struct ctx_s;

struct node_s {
	int type;
	void (*eval)(struct node_s* this, struct ctx_s* ctx);
	void (*free)(struct node_s* this);
	union {
		struct { struct node_list_s* funcs; struct node_list_s* stmts; } root;
		char* ident;
		struct { int op; struct node_s* val; } unary;
		struct { int op; struct node_s* a; struct node_s* b; } binary;
		value_t* value;
		struct { char* name; struct node_list_s* args; } call;
		struct { char* name; int argc; char** argv; struct node_s* body; } func;
		struct { struct node_s* arg; struct node_s* body; struct node_s* elsebody; } cond;
		struct { struct node_s* arg; struct node_s* body; } loop;
		struct { struct node_s* name; struct node_s* value; } decl;
		struct node_list_s* block;
	};
};

struct node_list_s {
	struct node_s* el;
	struct node_list_s* next;
};

typedef struct node_s node_t;
typedef struct node_list_s node_list_t;

node_t* node_root(node_list_t* funcs, node_list_t* stmts);
node_t* node_ident(char* name);
node_t* node_unary(int op, node_t* val);
node_t* node_binary(int op, node_t* a, node_t* b);
node_t* node_value(value_t* value);
node_t* node_call(char* name, node_list_t* args);
node_t* node_func(char*name, int argc, char** argv, node_t* body);
node_t* node_cond(node_t* arg, node_t* body, node_t* elsebody);
node_t* node_loop(node_t* arg, node_t* body);
node_t* node_decl(node_t* name, node_t* value);
node_t* node_block(node_list_t* list);

void node_print(node_t* node, int ind);

#endif
