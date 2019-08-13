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
    if (argc < 2)
    {
        printf("Usage: lang input\n");
        return -1;
    }

    input = fopen(argv[1], "r");

    if (!input)
    {
        printf("Cannot open file.\n");
        return -2;
    }

    try
    {
        node_t *tree = parse();

        binary_t *bin = binary_new();
        tree->codegen(tree, bin);

        bytecode_fill(bin);

        binary_save(bin, "test.bin");

        node_free(tree);
    }
    catch
    {
        puts(ex_msg);
    }

    fclose(input);

    return 0;
}
