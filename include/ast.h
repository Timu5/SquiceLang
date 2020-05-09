#ifndef _AST_H_
#define _AST_H_

#include "vector.h"

enum SL_NODETYPE
{
    SL_NODETYPE_ROOT,
    SL_NODETYPE_IDENT,
    SL_NODETYPE_UNARY,
    SL_NODETYPE_BINARY,
    SL_NODETYPE_INT,
    SL_NODETYPE_STRING,
    SL_NODETYPE_CALL,
    SL_NODETYPE_FUNC,
    SL_NODETYPE_RETURN,
    SL_NODETYPE_COND,
    SL_NODETYPE_LOOP,
    SL_NODETYPE_BREAK,
    SL_NODETYPE_DECL,
    SL_NODETYPE_INDEX,
    SL_NODETYPE_BLOCK,
    SL_NODETYPE_MEMBER
};

struct sl_binary_s;

struct sl_node_s
{
    enum SL_NODETYPE type;
    void (*codegen)(struct sl_node_s *this, struct sl_binary_s *binary);
    void (*free)(struct sl_node_s *this);
    union {
        struct
        {
            struct sl_node_s *funcs;
            struct sl_node_s *stmts;
        } root;
        char *ident;
        struct
        {
            int op;
            struct sl_node_s *val;
        } unary;
        struct
        {
            int op;
            struct sl_node_s *a;
            struct sl_node_s *b;
        } binary;
        int integer;
        char *string;
        struct
        {
            struct sl_node_s *func;
            struct sl_node_s *args;
        } call;
        struct
        {
            char *name;
            vector(char *) args;
            struct sl_node_s *body;
        } func;
        struct sl_node_s *ret;
        struct
        {
            struct sl_node_s *arg;
            struct sl_node_s *body;
            struct sl_node_s *elsebody;
        } cond;
        struct
        {
            struct sl_node_s *arg;
            struct sl_node_s *body;
        } loop;
        struct
        {
            struct sl_node_s *name;
            struct sl_node_s *value;
        } decl;
        struct
        {
            struct sl_node_s *var;
            struct sl_node_s *expr;
        } index;
        vector(struct sl_node_s *) block;
        struct
        {
            struct sl_node_s *parent;
            char *name;
        } member;
    };
};

typedef struct sl_node_s sl_node_t;

#define sl_node_free(node) ((node)->free((node)))

sl_node_t *node_root(sl_node_t *funcs, sl_node_t *stmts);
sl_node_t *node_ident(char *name);
sl_node_t *node_unary(int op, sl_node_t *val);
sl_node_t *node_binary(int op, sl_node_t *a, sl_node_t *b);
sl_node_t *node_int(int integer);
sl_node_t *node_string(char *string);
sl_node_t *node_call(sl_node_t *func, sl_node_t *args);
sl_node_t *node_func(char *name, vector(char *) args, sl_node_t *body);
sl_node_t *node_return(sl_node_t *expr);
sl_node_t *node_cond(sl_node_t *arg, sl_node_t *body, sl_node_t *elsebody);
sl_node_t *node_loop(sl_node_t *arg, sl_node_t *body);
sl_node_t *node_break();
sl_node_t *node_decl(sl_node_t *name, sl_node_t *value);
sl_node_t *node_index(sl_node_t *var, sl_node_t *expr);
sl_node_t *node_block(vector(sl_node_t *) list);
sl_node_t *node_member(sl_node_t *parent, char *name);

void sl_node_print(sl_node_t *node, int ind);

#endif
