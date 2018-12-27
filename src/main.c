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
#include "builtin.h"

extern FILE* input;

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
        builtin_install(global);
        
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
