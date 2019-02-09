#ifndef _CONTEX_H_
#define _CONTEX_H_

#include <setjmp.h>

#include "ast.h"
#include "vector.h"
#include "value.h"

struct var_s {
    char* name;
    value_t* val;
};

struct ctx_s;

struct fn_s {
    node_t* body;
    value_t* (*native)(struct ctx_s*); // call if not NULL
};

struct ctx_s {
    struct ctx_s* parent;
    struct ctx_s* child;
    vector(struct var_s*) vars;
    vector(value_t*) stack;
    jmp_buf* ret;
    jmp_buf* retLoop;
};

typedef struct var_s var_t;
typedef struct fn_s fn_t;
typedef struct ctx_s ctx_t;

ctx_t* ctx_new(ctx_t* parent);
void ctx_free(ctx_t* ctx);
value_t* ctx_getvar(ctx_t* ctx, char* name);
void ctx_addvar(ctx_t* ctx, char* name, value_t* val);
fn_t* ctx_getfn(ctx_t* ctx, char* name);
void ctx_addfn(ctx_t* ctx, char* name, node_t* body, value_t* (*fn)(ctx_t*));

#endif
