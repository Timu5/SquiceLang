#include <stdlib.h>
#include <stdio.h>
#include "ast.h"
#include "eval.h"
#include "lexer.h"

static void free_root(node_t* node)
{
    node_free(node->root.funcs);
    node_free(node->root.stmts);
    free(node);
}

node_t* node_root(node_t* funcs, node_t* stmts)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_ROOT;
    node->root.funcs = funcs;
    node->root.stmts = stmts;
    node->eval = eval_root;
    node->free = free_root;
    return node;
}

static void free_ident(node_t* node)
{
    free(node->ident);
    free(node);
}

node_t* node_ident(char* name)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_IDENT;
    node->ident = name;
    node->eval = eval_ident;
    node->free = free_ident; 
    return node;
}

static void free_unary(node_t* node)
{
    node_free(node->unary.val);
    free(node);
}

node_t* node_unary(int op, node_t* val)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_UNARY;
    node->unary.op = op;
    node->unary.val = val;
    node->eval = eval_unary;
    node->free = free_unary;
    return node;
}

static void free_binary(node_t* node)
{
    node_free(node->binary.a);
    node_free(node->binary.b);
    free(node);
}

node_t* node_binary(int op, node_t* a, node_t* b)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_BINARY;
    node->binary.op = op;
    node->binary.a = a;
    node->binary.b = b;
    node->eval = eval_binary;
    node->free = free_binary;
    return node;
}

static void free_int(node_t* node)
{
    free(node);
}

node_t* node_int(int integer)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_INT;
    node->integer = integer;
    node->eval = eval_int;
    node->free = free_int;
    return node;
}

static void free_string(node_t* node)
{
    free(node->string);
    free(node);
}

node_t* node_string(char* string)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_STRING;
    node->string = string;
    node->eval = eval_string;
    node->free = free_string;
    return node;
}

static void free_call(node_t* node)
{
    node_free(node->call.func);
    node_free(node->call.args);
    free(node);
}

node_t* node_call(node_t* func, node_t* args)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_CALL;
    node->call.func = func;
    node->call.args = args;
    node->eval = eval_call;
    node->free = free_call;
    return node;
}

static void free_func(node_t* node)
{
    node_free(node->func.body);
    for(int i = 0; i < vector_size(node->func.args); i++)
        free(node->func.args[i]);
    vector_free(node->func.args);
    free(node->func.name);
    free(node);
}

node_t* node_func(char* name, vector(char*) args, node_t* body)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_FUNC;
    node->func.name = name;
    node->func.args = args;
    node->func.body = body;
    node->eval = eval_func;
    node->free = free_func;
    return node;
}

static void free_return(node_t* node)
{
    node_free(node->ret);
    free(node);
}

node_t* node_return(node_t* expr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_RETURN;
    node->ret = expr;
    node->eval = eval_return;
    node->free = free_return;
    return node;
}

static void free_cond(node_t* node)
{
    node_free(node->cond.arg);
    node_free(node->cond.body);
    if(node->cond.elsebody)
        node_free(node->cond.elsebody);
    free(node);
}

node_t* node_cond(node_t* arg, node_t* body, node_t* elsebody)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_COND;
    node->cond.arg = arg;
    node->cond.body = body;
    node->cond.elsebody = elsebody;
    node->eval = eval_cond;
    node->free = free_cond;
    return node;
}

static void free_loop(node_t* node)
{
    node_free(node->loop.arg);
    node_free(node->loop.body);
    free(node);
}

node_t* node_loop(node_t* arg, node_t* body)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_LOOP;
    node->loop.arg = arg;
    node->loop.body = body;
    node->eval = eval_loop;
    node->free = free_loop;
    return node;
}

static void free_break(node_t* node)
{
    free(node);
}

node_t * node_break()
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_BREAK;
    node->eval = eval_break;
    node->free = free_break;
    return node;
}

static void free_decl(node_t* node)
{
    node_free(node->decl.name);
    node_free(node->decl.value);
    free(node);
}

node_t* node_decl(node_t* name, node_t* value)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_DECL;
    node->decl.name = name;
    node->decl.value = value;
    node->eval = eval_decl;
    node->free = free_decl;
    return node;
}

static void free_index(node_t* node)
{
    node_free(node->index.var);
    node_free(node->index.expr);
    free(node);
}

node_t* node_index(node_t* var, node_t* expr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_INDEX;
    node->index.var = var;
    node->index.expr = expr;
    node->eval = eval_index;
    node->free = free_index;
    return node;
}

static void free_block(node_t* node)
{
    for(int i = 0; i < vector_size(node->block); i++)
        node_free(node->block[i]);
    vector_free(node->block);
    free(node);
}

node_t* node_block(vector(node_t*) list)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_BLOCK;
    node->block = list;
    node->eval = eval_block;
    node->free = free_block;
    return node;
}

static void free_member(node_t* node)
{
    node_free(node->member.parent);
    free(node->member.name);
    free(node);
}

node_t * node_member(node_t * parent, char * name)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_MEMBER;
    node->member.name = name;
    node->member.parent = parent;
    node->eval = eval_member;
    node->free = free_member;
    return node;
}

static void printtab(int n)
{
    for(int i=0; i < n; i++)
        putchar('\t');
}

void node_print(node_t* node, int ind)
{
    printtab(ind);
    switch(node->type)
    {
    case N_ROOT:
        printf("root\n");
        node_print(node->root.funcs, ind + 1);
        node_print(node->root.stmts, ind + 1);
        break;
    case N_BLOCK:
        printf("block\n");
        for(int i = 0; i < vector_size(node->block); i++)
        {
            node_print(node->block[i], ind + 1);
        }
        break;
    case N_IDENT:
        printf("ident: %s\n", node->ident);
        break;
    case N_UNARY:
        printf("unary: %s\n", tokenstr(node->unary.op));
        node_print(node->unary.val, ind + 1);
        break;
    case N_BINARY:
        printf("binary: %s\n", tokenstr(node->binary.op));
        node_print(node->binary.a, ind + 1);
        node_print(node->binary.b, ind + 1);
        break;
    case N_INT:
        printf("int %i\n", node->integer);
        break;
    case N_STRING:
        printf("string \"%s\"", node->string);
        break;
    case N_CALL:
        printf("call\n");
        node_print(node->call.func, ind + 1);
        node_print(node->call.args, ind + 1);
        break;
    case N_FUNC:
        printf("function: %s(", node->func.name);
        for(int i = 0; i < vector_size(node->func.args); i++)
        {
            printf("%s", node->func.args[i]);
            if(i != vector_size(node->func.args) - 1)
                printf(", ");
        }
        printf(")\n");
        node_print(node->func.body, ind + 1);
    case N_RETURN:
        printf("return\n");
       if(node->ret)
            node_print(node->ret, ind + 1);
        break;
    case N_COND:
        printf("cond\n");
        printtab(ind);
        printf("->arg:\n");
        node_print(node->cond.arg, ind + 1);
        printtab(ind);
        printf("->body:\n");
        node_print(node->cond.body, ind + 1);
        break;
    case N_LOOP:
        printf("loop\n");
        printtab(ind);
        printf("->arg:\n");
        node_print(node->loop.arg, ind + 1);
        printtab(ind);
        printf("->body:\n");
        node_print(node->loop.body, ind + 1);
        break;
    case N_BREAK:
        printf("break\n");
        break;
    case N_DECL:
        printf("decl\n");
        printtab(ind);
        printf("->name:\n");
        node_print(node->decl.name, ind + 1);
        printtab(ind);
        printf("->value:\n");
        node_print(node->decl.value, ind + 1);
        break;
    case N_INDEX:
        printf("index\n");
        node_print(node->index.var, ind + 1);
        node_print(node->index.expr, ind + 1);
    }
}
