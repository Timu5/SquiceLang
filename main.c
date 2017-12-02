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

value_t* len(ctx_t* ctx)
{
    int n = vector_pop(ctx->stack)->number;
    if(n != 1)
        throw("Function len get exacly 1 argument");

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

        ctx_t* global = ctx_new(NULL);
        ctx_addfn(global, "print", NULL, print);
        ctx_addfn(global, "list", NULL, list);
        ctx_addfn(global, "len", NULL, len);  
        
        tree->eval(tree, global);
        
     //   gc_freeall();
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
