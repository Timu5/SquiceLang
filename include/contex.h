#ifndef _CONTEX_H_
#define _CONTEX_H_

#include <setjmp.h>

#include "ast.h"
#include "vector.h"
#include "value.h"

struct sl_var_s
{
    char *name;
    sl_value_t *val;
};

struct sl_ctx_s;

struct sl_fn_s
{
    int address;
    void (*native)(struct sl_ctx_s *); // call if not NULL
};

struct sl_ctx_s
{
    struct sl_ctx_s *parent;
    struct sl_ctx_s *child;
    sl_vector(struct sl_var_s *) vars;
    sl_vector(sl_value_t *) stack;
    //jmp_buf *ret;
    //jmp_buf *retLoop;
};

typedef struct sl_var_s sl_var_t;
typedef struct sl_fn_s sl_fn_t;
typedef struct sl_ctx_s sl_ctx_t;

sl_ctx_t *sl_ctx_new(sl_ctx_t *parent);
void sl_ctx_free(sl_ctx_t *ctx);
sl_value_t *sl_ctx_getvar(sl_ctx_t *ctx, char *name);
void sl_ctx_addvar(sl_ctx_t *ctx, char *name, sl_value_t *val);
sl_fn_t *sl_ctx_getfn(sl_ctx_t *ctx, char *name);
void sl_ctx_addfn(sl_ctx_t *ctx, char *name, int address, void (*fn)(sl_ctx_t *));

#endif
