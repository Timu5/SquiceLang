#ifndef _AST_H_
#define _AST_H_

#include "value.h"
#include "vector.h"

enum {
    N_ROOT,
    N_IDENT,
    N_UNARY,
    N_BINARY,
    N_INT,
    N_STRING,
    N_CALL,
    N_FUNC,
    N_RETURN,
    N_COND,
    N_LOOP,
    N_BREAK,
    N_DECL,
    N_INDEX,
    N_BLOCK,
    N_MEMBER
};

struct binary_s;

struct node_s {
    int type;
    void (*codegen)(struct node_s* this, struct binary_s* binary);
    void (*free)(struct node_s* this);
    union {
        struct { struct node_s* funcs; struct node_s* stmts; } root;
        char* ident;
        struct { int op; struct node_s* val; } unary;
        struct { int op; struct node_s* a; struct node_s* b; } binary;
        int integer;
        char* string;
        struct { struct node_s* func; struct node_s* args; } call;
        struct { char* name; vector(char*) args; struct node_s* body; } func;
        struct node_s* ret;
        struct { struct node_s* arg; struct node_s* body; struct node_s* elsebody; } cond;
        struct { struct node_s* arg; struct node_s* body; } loop;
        struct { struct node_s* name; struct node_s* value; } decl;
        struct { struct node_s* var; struct node_s* expr; } index;
        vector(struct node_s*) block;
        struct { struct node_s* parent;  char* name; } member;
    };
};

typedef struct node_s node_t;

#define node_free(node) ((node)->free((node)))

node_t* node_root(node_t* funcs, node_t* stmts);
node_t* node_ident(char* name);
node_t* node_unary(int op, node_t* val);
node_t* node_binary(int op, node_t* a, node_t* b);
node_t* node_int(int integer);
node_t* node_string(char* string);
node_t* node_call(node_t* func, node_t* args);
node_t* node_func(char*name, vector(char*) args, node_t* body);
node_t* node_return(node_t* expr);
node_t* node_cond(node_t* arg, node_t* body, node_t* elsebody);
node_t* node_loop(node_t* arg, node_t* body);
node_t* node_break();
node_t* node_decl(node_t* name, node_t* value);
node_t* node_index(node_t* var, node_t* expr);
node_t* node_block(vector(node_t*) list);
node_t* node_member(node_t* parent, char* name);

void node_print(node_t* node, int ind);

#endif
