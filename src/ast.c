#include <stdlib.h>
#include <stdio.h>

#include "SquiceLang.h"

static void free_root(sl_node_t *node)
{
    sl_node_free(node->root.funcs);
    sl_node_free(node->root.stmts);
    free(node);
}

sl_node_t *node_root(sl_node_t *funcs, sl_node_t *stmts)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_ROOT;
    node->root.funcs = funcs;
    node->root.stmts = stmts;
    node->codegen = sl_codegen_root;
    node->free = free_root;
    return node;
}

static void free_ident(sl_node_t *node)
{
    free(node->ident);
    free(node);
}

sl_node_t *node_ident(char *name)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_IDENT;
    node->ident = name;
    node->codegen = sl_codegen_ident;
    node->free = free_ident;
    return node;
}

static void free_unary(sl_node_t *node)
{
    sl_node_free(node->unary.val);
    free(node);
}

sl_node_t *node_unary(int op, sl_node_t *val)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_UNARY;
    node->unary.op = op;
    node->unary.val = val;
    node->codegen = sl_codegen_unary;
    node->free = free_unary;
    return node;
}

static void free_binary(sl_node_t *node)
{
    sl_node_free(node->binary.a);
    sl_node_free(node->binary.b);
    free(node);
}

sl_node_t *node_binary(int op, sl_node_t *a, sl_node_t *b)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_BINARY;
    node->binary.op = op;
    node->binary.a = a;
    node->binary.b = b;
    node->codegen = sl_codegen_binary;
    node->free = free_binary;
    return node;
}

static void free_number(sl_node_t *node)
{
    free(node);
}

sl_node_t *node_number(double number)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_NUMBER;
    node->number = number;
    node->codegen = sl_codegen_double;
    node->free = free_number;
    return node;
}

static void free_string(sl_node_t *node)
{
    free(node->string);
    free(node);
}

sl_node_t *node_string(char *string)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_STRING;
    node->string = string;
    node->codegen = sl_codegen_string;
    node->free = free_string;
    return node;
}

static void free_call(sl_node_t *node)
{
    sl_node_free(node->call.func);
    sl_node_free(node->call.args);
    free(node);
}

sl_node_t *node_call(sl_node_t *func, sl_node_t *args)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_CALL;
    node->call.func = func;
    node->call.args = args;
    node->codegen = sl_codegen_call;
    node->free = free_call;
    return node;
}

static void free_func(sl_node_t *node)
{
    sl_node_free(node->func.body);
    for (int i = 0; i < sl_vector_size(node->func.args); i++)
        free(node->func.args[i]);
    sl_vector_free(node->func.args);
    free(node->func.name);
    free(node);
}

sl_node_t *node_func(char *name, sl_vector(char *) args, sl_node_t *body)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_FUNC;
    node->func.name = name;
    node->func.args = args;
    node->func.body = body;
    node->codegen = sl_codegen_func;
    node->free = free_func;
    return node;
}

static void free_return(sl_node_t *node)
{
    if (node->ret)
        sl_node_free(node->ret);
    free(node);
}

sl_node_t *node_return(sl_node_t *expr)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_RETURN;
    node->ret = expr;
    node->codegen = sl_codegen_return;
    node->free = free_return;
    return node;
}

static void free_cond(sl_node_t *node)
{
    sl_node_free(node->cond.arg);
    sl_node_free(node->cond.body);
    if (node->cond.elsebody)
        sl_node_free(node->cond.elsebody);
    free(node);
}

sl_node_t *node_cond(sl_node_t *arg, sl_node_t *body, sl_node_t *elsebody)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_COND;
    node->cond.arg = arg;
    node->cond.body = body;
    node->cond.elsebody = elsebody;
    node->codegen = sl_codegen_cond;
    node->free = free_cond;
    return node;
}

static void free_loop(sl_node_t *node)
{
    sl_node_free(node->loop.arg);
    sl_node_free(node->loop.body);
    free(node);
}

sl_node_t *node_loop(sl_node_t *arg, sl_node_t *body)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_LOOP;
    node->loop.arg = arg;
    node->loop.body = body;
    node->codegen = sl_codegen_loop;
    node->free = free_loop;
    return node;
}

static void free_break(sl_node_t *node)
{
    free(node);
}

sl_node_t *node_break()
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_BREAK;
    node->codegen = sl_codegen_break;
    node->free = free_break;
    return node;
}

static void free_decl(sl_node_t *node)
{
    sl_node_free(node->decl.name);
    sl_node_free(node->decl.value);
    free(node);
}

sl_node_t *node_decl(sl_node_t *name, sl_node_t *value)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_DECL;
    node->decl.name = name;
    node->decl.value = value;
    node->codegen = sl_codegen_decl;
    node->free = free_decl;
    return node;
}

static void free_index(sl_node_t *node)
{
    sl_node_free(node->index.var);
    sl_node_free(node->index.expr);
    free(node);
}

sl_node_t *node_index(sl_node_t *var, sl_node_t *expr)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_INDEX;
    node->index.var = var;
    node->index.expr = expr;
    node->codegen = sl_codegen_index;
    node->free = free_index;
    return node;
}

static void free_block(sl_node_t *node)
{
    for (int i = 0; i < sl_vector_size(node->block); i++)
        sl_node_free(node->block[i]);
    sl_vector_free(node->block);
    free(node);
}

sl_node_t *node_block(sl_vector(sl_node_t *) list)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_BLOCK;
    node->block = list;
    node->codegen = sl_codegen_block;
    node->free = free_block;
    return node;
}

static void free_member(sl_node_t *node)
{
    sl_node_free(node->member.parent);
    free(node->member.name);
    free(node);
}

sl_node_t *node_member(sl_node_t *parent, char *name)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_MEMBER;
    node->member.name = name;
    node->member.parent = parent;
    node->codegen = sl_codegen_member;
    node->free = free_member;
    return node;
}

static void free_import(sl_node_t *node)
{
    free(node->import);
    free(node);
}

sl_node_t *node_import(char *name)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_MEMBER;
    node->import = name;
    node->codegen = sl_codegen_import;
    node->free = free_import;
    return node;
}

static void free_class(sl_node_t *node)
{
    sl_vector_free(node->class.methods);
    free(node->class.name);
    free(node);
}

sl_node_t *node_class(char *name, sl_vector(sl_node_t *) list)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_CLASS;
    node->class.name = name;
    node->class.methods = list;
    node->codegen = sl_codegen_class;
    node->free = free_class;
    return node;
}

static void free_trycatch(sl_node_t *node)
{
    sl_node_free(node->trycatch.tryblock);
    sl_node_free(node->trycatch.catchblock);
    free(node);
}

sl_node_t *node_trycatch(sl_node_t *tryblock, sl_node_t *catchblock)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_TRYCATCH;
    node->trycatch.tryblock = tryblock;
    node->trycatch.catchblock = catchblock;
    node->codegen = sl_codegen_trycatch;
    node->free = free_trycatch;
    return node;
}

static void free_throw(sl_node_t *node)
{
    sl_node_free(node->throw);
    free(node);
}

sl_node_t *node_throw(sl_node_t *expr)
{
    sl_node_t *node = (sl_node_t *)malloc(sizeof(sl_node_t));
    node->type = SL_NODETYPE_THROW;
    node->throw = expr;
    node->codegen = sl_codegen_throw;
    node->free = free_throw;
    return node;
}

static void printtab(int n)
{
    for (int i = 0; i < n; i++)
        putchar('\t');
}

void sl_node_print(sl_node_t *node, int ind)
{
    printtab(ind);
    switch (node->type)
    {
    case SL_NODETYPE_ROOT:
        printf("root\n");
        sl_node_print(node->root.funcs, ind + 1);
        sl_node_print(node->root.stmts, ind + 1);
        break;
    case SL_NODETYPE_BLOCK:
        printf("block\n");
        for (int i = 0; i < sl_vector_size(node->block); i++)
        {
            sl_node_print(node->block[i], ind + 1);
        }
        break;
    case SL_NODETYPE_IDENT:
        printf("ident: %s\n", node->ident);
        break;
    case SL_NODETYPE_UNARY:
        printf("unary: %s\n", sl_tokenstr(node->unary.op));
        sl_node_print(node->unary.val, ind + 1);
        break;
    case SL_NODETYPE_BINARY:
        printf("binary: %s\n", sl_tokenstr(node->binary.op));
        sl_node_print(node->binary.a, ind + 1);
        sl_node_print(node->binary.b, ind + 1);
        break;
    case SL_NODETYPE_NUMBER:
        printf("num %g\n", node->number);
        break;
    case SL_NODETYPE_STRING:
        printf("string \"%s\"\n", node->string);
        break;
    case SL_NODETYPE_CALL:
        printf("call\n");
        printf("->func:\n");
        sl_node_print(node->call.func, ind + 1);
        printf("->args:\n");
        sl_node_print(node->call.args, ind + 1);
        break;
    case SL_NODETYPE_FUNC:
        printf("function: %s(", node->func.name);
        for (int i = 0; i < sl_vector_size(node->func.args); i++)
        {
            printf("%s", node->func.args[i]);
            if (i != sl_vector_size(node->func.args) - 1)
                printf(", ");
        }
        printf(")\n");
        sl_node_print(node->func.body, ind + 1);
    case SL_NODETYPE_RETURN:
        printf("return\n");
        if (node->ret)
            sl_node_print(node->ret, ind + 1);
        break;
    case SL_NODETYPE_COND:
        printf("cond\n");
        printtab(ind);
        printf("->arg:\n");
        sl_node_print(node->cond.arg, ind + 1);
        printtab(ind);
        printf("->body:\n");
        sl_node_print(node->cond.body, ind + 1);
        break;
    case SL_NODETYPE_LOOP:
        printf("loop\n");
        printtab(ind);
        printf("->arg:\n");
        sl_node_print(node->loop.arg, ind + 1);
        printtab(ind);
        printf("->body:\n");
        sl_node_print(node->loop.body, ind + 1);
        break;
    case SL_NODETYPE_BREAK:
        printf("break\n");
        break;
    case SL_NODETYPE_DECL:
        printf("decl\n");
        printtab(ind);
        printf("->name:\n");
        sl_node_print(node->decl.name, ind + 1);
        printtab(ind);
        printf("->value:\n");
        sl_node_print(node->decl.value, ind + 1);
        break;
    case SL_NODETYPE_INDEX:
        printf("index\n");
        sl_node_print(node->index.var, ind + 1);
        sl_node_print(node->index.expr, ind + 1);
        break;
    case SL_NODETYPE_IMPORT:
        printf("import \"%s\"\n", node->string);
        break;
    case SL_NODETYPE_MEMBER:
        printf("member \"%s\"\n", node->member.name);
        sl_node_print(node->member.parent, ind + 1);
        break;
    }
}
