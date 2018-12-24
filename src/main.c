#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "value.h"
#include "ast.h"
#include "parser.h"
#include "vector.h"
#include "eval.h"
#include "contex.h"
#include "ex.h"
#include "gc.h"

extern FILE* input;

value_t* print(ctx_t* ctx)
{
    int n = vector_pop(ctx->stack)->number;
    int end = vector_size(ctx->stack);
    int start = end - n;
    for(int i = start; i < end; i++)
    {
        value_t* v = ctx->stack[i];
        if(v->type == V_NUMBER)
            printf("%g", v->number);
        else if(v->type == V_STRING)
            printf("%s", v->string);
    }   
    putchar('\n');
}

value_t* list(ctx_t* ctx)
{
	int n = vector_pop(ctx->stack)->number;
	vector(value_t*) arr = NULL;
	for(int i = vector_size(ctx->stack) - n; i < vector_size(ctx->stack); i++)
	{
		vector_push(arr, ctx->stack[i]);
	}
	vector_shrinkby(ctx->stack, n);

	vector_push(ctx->stack, value_array(arr));
}

value_t* dict(ctx_t* ctx)
{
    int n = vector_pop(ctx->stack)->number;
    if (n != 0 && n != 2)
        throw("Function dict takes 0 or 2 arguments");

    if (n == 0)
        vector_push(ctx->stack, value_dict(NULL, NULL));
    else
    {
        value_t* v = vector_pop(ctx->stack);
        value_t* k = vector_pop(ctx->stack);
        if (v->type != V_ARRAY || k->type != V_ARRAY)
            throw("Function dict takes arguments of type array");
        vector(char*) keys = NULL;
        for (int i = 0; i < vector_size(k->array); i++)
        {
            value_t* key = value_get(i, k);
            if (key->type != V_STRING)
                throw("Key of dictonary must be a string");
            vector_push(keys, strdup(key->string));
        }
        vector(char*) values = NULL;
        for (int i = 0; i < vector_size(v->array); i++)
        {
            vector_push(values, value_get(i, v));
        }

        vector_push(ctx->stack, value_dict(keys, values));
    }

}

value_t* addMember(ctx_t* ctx)
{
	int n = vector_pop(ctx->stack)->number;
	if (n != 3)
		throw("Function addmember takes exactly 3 argument");

	value_t* v = vector_pop(ctx->stack);
	value_t* k = vector_pop(ctx->stack);
	value_t* p = vector_pop(ctx->stack);

	if (k->type != V_STRING)
		throw("Key can only be of type string");

	vector_push(p->dict.names, strdup(k->string));
	vector_push(p->dict.values, v);

	vector_push(ctx->stack, value_dict(NULL, NULL));
}

value_t* len(ctx_t* ctx)
{
    int n = vector_pop(ctx->stack)->number;
    if(n != 1)
        throw("Function len takes exactly 1 argument");

    value_t* v = vector_pop(ctx->stack);
    if(v->type == V_STRING)
    {
        vector_push(ctx->stack, value_number(strlen(v->string)));
        return NULL;
    }
    else if(v->type == V_ARRAY)
    {
        vector_push(ctx->stack, value_number(vector_size(v->array)));
        return NULL;
    }
    
    throw("Function len need argument of type string or array.");
}

ctx_t* global = NULL;

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

    try
    {   
        node_t* tree = parse();

        global = ctx_new(NULL);
        ctx_addfn(global, "print", NULL, print);
		ctx_addfn(global, "list", NULL, list);
		ctx_addfn(global, "dict", NULL, dict);
		ctx_addfn(global, "addmember", NULL, addMember);
        ctx_addfn(global, "len", NULL, len);  
        
        tree->eval(tree, global);
        
        gc_freeall();
        ctx_free(global);
        node_free(tree);
    }
    catch
    {
        puts(ex_msg);
    }

    fclose(input);

    return 0;
}
