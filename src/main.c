#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "value.h"
#include "ast.h"
#include "parser.h"
#include "vector.h"
#include "bytecode.h"
#include "utils.h"
#include "codegen.h"

extern FILE *input;

int main(int argc, char **argv)
{
    /*if (argc < 2)
    {
        printf("Usage: lang input\n");
        return -1;
    }*/

    input = fopen("test.lang", "r");

    if (!input)
    {
        printf("Cannot open file.\n");
        return -2;
    }

    try
    {
        sl_node_t *tree = sl_parse();

        sl_binary_t *bin = sl_binary_new();
        tree->codegen(tree, bin);

        sl_bytecode_fill(bin);

        sl_binary_save(bin, "test.bin");

        sl_node_free(tree);
    }
    catch
    {
        puts(ex_msg);
    }

    fclose(input);

    return 0;
}
