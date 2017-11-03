#include <stdio.h>

#include "lexer.h"
#include "eval.h"
#include "ast.h"

void eval_setvar(char* name, value_t* value, ctx_t* ctx)
{
	ctx_t* c = ctx;
	while(c)
	{
		var_t* v = c->vars;
		while(v)
		{
			if(strcmp(v->name, name) == 0)
			{
				v->val = value;
				return;
			}
			v = v->next;
		}
		c = c->parent;
	}
	printf("Variable %s not found.\n", name);
	exit(-3);
}

value_t* eval_getvar(char* name, ctx_t* ctx)
{
	ctx_t* c = ctx;
	while(c)
	{
		var_t* v = c->vars;
		while(v)
		{
			if(strcmp(v->name, name) == 0)
				return v->val;

			v = v->next;
		}
		c = c->parent;
	}
	return NULL;
}

fn_t* eval_getfn(char* name, ctx_t* ctx)
{
	ctx_t* c = ctx;
	while(c)
	{
		fn_t* f = c->funcs;
		while(f)
		{
			if(strcmp(f->name, name) == 0)
				return f;
			f = f->next;
		}
		c = c->parent;
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
	node->binary.a->eval(node->binary.a, ctx);
	node->binary.b->eval(node->binary.b, ctx);
	value_t* b = stack_pop(ctx->stack);
	value_t* a = stack_pop(ctx->stack);
	if(node->binary.op == T_ASSIGN) // special case
		memcpy(a, b, sizeof(value_t));
	else
		stack_push(ctx->stack, value_binary(node->binary.op, a, b));
	
}

void eval_value(node_t* node, ctx_t* ctx)
{
	stack_push(ctx->stack, node->value);
}

void eval_call(node_t* node, ctx_t* ctx)
{
	fn_t* f = eval_getfn(node->call.name, ctx);
	if(!f)
	{
		printf("Cannot find function %s\n", node->call.name);
		exit(-4);
	}
	var_t* args = NULL;
	var_t* last = NULL;
	node_list_t* n = node->call.args;
	while(n)
	{
		n->el->eval(n->el, ctx);
		var_t* tmp = (var_t*)malloc(sizeof(var_t));
		tmp->name = NULL;
		tmp->val = (value_t*)stack_pop(ctx->stack);
		tmp->next = NULL;
		if(!args)
			args = tmp;	
		else
			last->next = tmp;
		last = tmp;
		n = n->next;
	}
	if(f->native)
	{
		f->fn(args);
	}
	else
	{
		// TODO
	}
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
	c->funcs = NULL;
	c->stack = stack_new();

	node_list_t* n = node->block;
	while(n)
	{
		n->el->eval(n->el, c);
		n = n->next;
	}
}

