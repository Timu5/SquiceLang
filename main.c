#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "value.h"
#include "ast.h"
#include "parser.h"
#include "stack.h"
#include "eval.h"

extern FILE* input;

void quit()
{
	fclose(input);
}

void print(ctx_t* ctx)
{
	int n = ((value_t*)stack_pop(ctx->stack))->number;
	while(n > 0)
	{
		value_t* v = (value_t*)stack_pop(ctx->stack);
		if(v->type == V_NUMBER)
			printf("%g", v->number);
		else if(v->type == V_STRING)
			printf("%s", v->string);
		n--;
	}
	putchar('\n');
}

int main(int argc, char ** argv)
{
	if(argc < 2)
	{
		printf("Usage: lang input\n");
		return -1;
	}

	input = fopen(argv[1], "r");

	if(!input)
	{
		printf("Cannot open file.\n");
		return -2;
	}
	atexit(quit);
	
	node_t* tree = parse();
	
	ctx_t* global = (ctx_t*)malloc(sizeof(ctx_t));
	global->parent = NULL;
	global->vars = NULL;
	global->funcs = (fn_t*)malloc(sizeof(fn_t));
	global->funcs->name = "print";
	global->funcs->body = NULL;
	global->funcs->native = 1;
	global->funcs->fn = print;
	global->funcs->next = NULL;
	
	tree->eval(tree, global);

	return 0;
}
