#include <stdlib.h>
#include "ast.h"
#include "eval.h"

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

node_t* node_cond(node_t* arg, node_t* body)
{
	node_t* node = (node_t*)malloc(sizeof(node_t));
	node->type = N_COND;
	node->cond.arg = arg;
	node->cond.body = body;
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
	case N_BLOCK:
		printf("block\n");
		node_list_t* n = node->block;
		while(n)
		{
			node_print(n->el, ind+1);
			n = n->next;
		}
		break;
	case N_IDENT:
		printf("ident: %s\n", node->ident);
		break;
	case N_UNARY:
		printf("unary: %s\n", tokenstr(node->unary.op));
		node_print(node->unary.val, ind+1);
		break;
	case N_BINARY:
		printf("binary: %s\n", tokenstr(node->binary.op));
		node_print(node->binary.a, ind+1);
		node_print(node->binary.b, ind+1);
		break;
	case N_VALUE:
		printf("value %f\n", node->value->number);
		break;
	}
}
