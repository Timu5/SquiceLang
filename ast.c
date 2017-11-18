#include <stdlib.h>
#include "ast.h"
#include "eval.h"
#include "lexer.h"

node_t* node_root(node_list_t* funcs, node_list_t* stmts)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_ROOT;
    node->root.funcs = funcs;
    node->root.stmts = stmts;
    node->eval = eval_root;
    return node;
}

node_t* node_ident(char* name)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_IDENT;
    node->ident = name;
    node->eval = eval_ident; 
    return node;
}

node_t* node_unary(int op, node_t* val)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_UNARY;
    node->unary.op = op;
    node->unary.val = val;
    node->eval = eval_unary;
    return node;
}

node_t* node_binary(int op, node_t* a, node_t* b)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_BINARY;
    node->binary.op = op;
    node->binary.a = a;
    node->binary.b = b;
    node->eval = eval_binary;
    return node;
}

node_t* node_value(value_t* value)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_VALUE;
    node->value = value;
    node->eval = eval_value;
    return node;
}

node_t* node_call(char* name, node_list_t* args)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_CALL;
    node->call.name = name;
    node->call.args = args;
    node->eval = eval_call;
    return node;
}

node_t* node_func(char* name, int argc, char** argv, node_t* body)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_FUNC;
    node->func.name = name;
    node->func.argc = argc;
    node->func.argv = argv;
    node->func.body = body;
    node->eval = eval_func;
    return node;
}

node_t* node_return(node_t* expr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_RETURN;
    node->ret = expr;
    node->eval = eval_return;
    return node;
}

node_t* node_cond(node_t* arg, node_t* body, node_t* elsebody)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_COND;
    node->cond.arg = arg;
    node->cond.body = body;
    node->cond.elsebody = elsebody;
    node->eval = eval_cond;
    return node;
}

node_t* node_loop(node_t* arg, node_t* body)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_LOOP;
    node->loop.arg = arg;
    node->loop.body = body;
    node->eval = eval_loop;
    return node;
}

node_t* node_decl(node_t* name, node_t* value)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_DECL;
    node->decl.name = name;
    node->decl.value = value;
    node->eval = eval_decl;
    return node;
}

node_t* node_index(node_t* var, node_t* expr)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_INDEX;
    node->index.var = var;
    node->index.expr = expr;
    node->eval = eval_index;
    return node;
}

node_t* node_block(node_list_t* list)
{
    node_t* node = (node_t*)malloc(sizeof(node_t));
    node->type = N_BLOCK;
    node->block = list;
    node->eval = eval_block;
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
        node_list_t* n4 = node->root.stmts;
        while(n4)
        {
            node_print(n4->el, ind + 1);
            n4 = n4->next;
        }
        break;
    case N_BLOCK:
        printf("block\n");
        node_list_t* n = node->block;
        while(n)
        {
            node_print(n->el, ind + 1);
            n = n->next;
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
    case N_VALUE:
        printf("value %f\n", node->value->number);
        break;
    case N_CALL:
        printf("call %s\n", node->call.name);
        node_list_t* n2 = node->call.args;
        while(n2)
        {
            node_print(n2->el, ind + 1);
            n2 = n2->next;
        }
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
    case N_DECL:
        printf("decl\n");
        printtab(ind);
        printf("->name:\n");
        node_print(node->decl.name, ind + 1);
        printtab(ind);
        printf("->value:\n");
        node_print(node->decl.value, ind + 1);
        break;
    }
}
