#include "SquiceLang.h"

#ifdef SL_IMPLEMENTATION

#include "vm/builtin.c"
#include "vm/contex.c"
#include "vm/gc.c"
#include "vm/value.c"
#include "vm/vm.c"
#include "ast.c"
#include "bytecode.c"
#include "codegen.c"
#include "lexer.c"
#include "parser.c"
#include "utils.c"

char input_buffer[255];

int balanced()
{
    int i = 0;
    int par = 0;
    int bra = 0;
    int squ = 0;
    while(input_buffer[i] != '\0')
    {
        if(input_buffer[i] == '(')
            par++;
        if(input_buffer[i] == ')')
            par--;
        if(input_buffer[i] == '[')
            squ++;
        if(input_buffer[i] == ']')
            squ--;
        if(input_buffer[i] == '{')
            bra++;
        if(input_buffer[i] == '}')
            bra--;
        i++;
    }
    return par == 0 && bra == 0 && squ == 0;
}

void getstring()
{
    printf(">>> ");
    fgets(input_buffer, 255, stdin);

    while(!balanced()) {
        printf("... ");
        fgets(input_buffer + strlen(input_buffer), 255, stdin);
    }
}

int main(int argc, char **argv)
{
    sl_ctx_t *ctx = sl_ctx_new(NULL);
    sl_builtin_install(ctx);

    while (1)
    {
        try
        {
            getstring();

            sl_parser_t *parser = sl_parser_new(input_buffer);

            sl_node_t *tree = sl_parse(parser);
            sl_parser_free(parser);

            sl_binary_t *bin = sl_binary_new();
            tree->codegen(tree, bin);

            sl_bytecode_fill(bin);

            //sl_binary_save(bin, "test.bin");

            sl_node_free(tree);

            exec(ctx, bin->block, bin->size);
        }
        catch
        {
            puts(ex_msg);
        }
    }

    //fclose(input);

    return 0;
}

#endif