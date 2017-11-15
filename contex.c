#include "ex.h"
#include "contex.h"

void ctx_setvar(char* name, value_t* value, ctx_t* ctx)
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
	throw("Variable %s not found.", name);
}

value_t* ctx_getvar(char* name, ctx_t* ctx)
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

void ctx_declvar(char* name, value_t* val, ctx_t* ctx)
{
	var_t* tmp = ctx->vars;
	ctx->vars = (var_t*)malloc(sizeof(var_t));
	ctx->vars->name = name;
	ctx->vars->val = val;
	ctx->vars->next = tmp;
}

fn_t* ctx_getfn(char* name, ctx_t* ctx)
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


