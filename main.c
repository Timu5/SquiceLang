#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "value.h"
#include "ast.h"
#include "parser.h"
#include "stack.h"
#include "eval.h"
#include "contex.h"
#include "ex.h"

extern FILE* input;

void quit()
{
    fclose(input);
}

value_t* print(ctx_t* ctx)
{
    int n = ((value_t*)stack_pop(ctx->stack))->number;
    int end = ctx->stack->used;
    int start = end - n;
    for(int i = start; i < end; i++)
    {
        value_t* v = (value_t*)ctx->stack->data[i];
        if(v->type == V_NUMBER)
            printf("%g", v->number);
        else if(v->type == V_STRING)
            printf("%s", v->string);
    }   
    putchar('\n');
}

value_t* list(ctx_t* ctx)
{
    int n = ((value_t*)stack_pop(ctx->stack))->number;
    stack_t* stack = stack_new(sizeof(value_t));
    while(n > 0)
    {
        stack_push(stack, stack_pop(ctx->stack));
        n--;
    }
    value_t** arr = (value_t**)malloc(sizeof(value_t*)*stack->used);
    int i = 0;
    while(stack->used > 0)
    {
        arr[i] = (value_t*)stack_pop(stack);
        i++;
    }
    stack_push(ctx->stack, value_array(i, arr));
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


    try
    {   
        node_t* tree = parse();

        ctx_t* global = ctx_new(NULL);
        ctx_addfn(global, "print", NULL, print);
        ctx_addfn(global, "list", NULL, list);
        //node_print(tree, 0);  
        tree->eval(tree, global);
    }
    catch
    {
        puts(ex_msg);
    }

    return 0;
}
