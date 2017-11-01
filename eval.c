#include <stdio.h>

#include "eval.h"
#include "ast.h"

void eval_setvar(char* name, value_t* value, ctx_t* ctx)
{
	ctx_t* c = ctx;
	while(c != NULL)
	{
		var_t* v = c->vars;
		while(v != NULL)
		{
			if(strcmp(v->name, name) == 0)
			{
				v->val = value;
				return;
			}
			v = ctx->vars->next;
		}
		c = ctx->parent;
	}
	// error variable not found
	printf("Variable %s not found.\n", name);
	exit(-3);
}

value_t* eval_getvar(char* name, ctx_t* ctx)
{
	ctx_t* c = ctx;
	while(c != NULL)
	{
		var_t* v = c->vars;
		while(v != NULL)
		{
			if(strcmp(v->name, name) == 0)
				return v->val;

			v = ctx->vars->next;
		}
		c = ctx->parent;
	}
	return NULL;
}

void eval_ident(node_t* node, ctx_t* ctx)
{
	value_t* v = eval_getvar(node->ident, ctx);
	if(!v)
	{
		printf("Variable %s not found!\n", node->ident);// error not found
		exit(-3);
	}
	stack_push(ctx->stack, v);
}

void eval_unary(node_t* node, ctx_t* ctx)
{
	node->unary.val->eval(node->unary.val, ctx);
	value_t* a = stack_pop(ctx->stack);
	stack_push(ctx->stack, value_unary(node->unary.op, a));
}

void eval_binary(node_t* node, ctx_t* ctx)
{
	printf("a: %d b: %d\n", node->binary.a->type, node->binary.b->type);
	node->binary.a->eval(node->binary.a, ctx);
	node->binary.b->eval(node->binary.b, ctx);
	value_t* b = stack_pop(ctx->stack);
	value_t* a = stack_pop(ctx->stack);
	stack_push(ctx->stack, value_binary(node->binary.op, a, b));
}

void eval_value(node_t* node, ctx_t* ctx)
{
	stack_push(ctx->stack, node->value);
}

void eval_call(node_t* node, ctx_t* ctx)
{
	// TODO
}

void eval_cond(node_t* node, ctx_t* ctx)
{
	node->cond.arg->eval(node->cond.arg, ctx);
	value_t* arg = stack_pop(ctx->stack);
	if(arg->number != 0)
		node->cond.body->eval(node->cond.body, ctx);
}

void eval_loop(node_t* node, ctx_t* ctx)
{
label:
	node->loop.arg->eval(node->loop.arg, ctx);
	value_t* arg = stack_pop(ctx->stack);
	if(arg->number != 0)
	{
		node->loop.body->eval(node->loop.body, ctx);
		goto label;
	}
}

void eval_decl(node_t* node, ctx_t* ctx)
{
	if(node->decl.name->type != N_IDENT)
		; // error
	node->decl.value->eval(node->decl.value, ctx);
	var_t* tmp = ctx->vars;
	ctx->vars = (var_t*)malloc(sizeof(var_t));
	ctx->vars->name = node->decl.name->ident;
	ctx->vars->val = stack_pop(ctx->stack);
	ctx->vars->next = tmp;
}

void eval_block(node_t* node, ctx_t* ctx)
{
	ctx_t* c = (ctx_t*)malloc(sizeof(ctx_t));
	c->parent = ctx;
	c->vars = NULL;
	c->stack = stack_new();

	node_list_t* n = node->block;
	while(n)
	{
		n->el->eval(n->el, c);
		n = n->next;
	}
	printf("size: %d\n", c->stack->used);
}



