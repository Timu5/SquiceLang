#include "SquiceLang.h"

#ifdef SL_IMPLEMENTATION

#include "vm/builtin.c"
#include "vm/context.c"
#include "vm/gc.c"
#include "vm/value.c"
#include "vm/vm.c"
#include "ast.c"
#include "bytecode.c"
#include "codegen.c"
#include "lexer.c"
#include "parser.c"
#include "utils.c"

sl_binary_t *sl_compile_str(char *code)
{
    sl_try
    {
        sl_parser_t *parser = sl_parser_new(code);
        sl_node_t *tree = sl_parse(parser);
        sl_parser_free(parser);

        sl_binary_t *bin = sl_binary_new();
        tree->codegen(tree, bin);
        sl_node_free(tree);
        sl_bytecode_fill(bin);

        return bin;
    }
    sl_catch
    {
        return NULL;
    }
}

int sl_eval_str(sl_ctx_t *ctx, char *code, sl_binary_t *(*load_module)(char *name), void *(trap)(sl_ctx_t *ctx))
{
    sl_binary_t *bin = sl_compile_str(code);
    if (!bin)
        return 0;
    sl_try
    {
        sl_exec(ctx, ctx, bin, 0, load_module, trap);
        sl_binary_free(bin);
    }
    sl_catch
    {
        return 0;
    }
    return 1;
}

sl_binary_t *sl_compile_file(char *filename)
{
    FILE *fd = fopen(filename, "r");
    if (!fd)
        sl_throw("Cannot open file %s", filename);

    fseek(fd, 0, SEEK_END);
    long fsize = ftell(fd);
    fseek(fd, 0, SEEK_SET);

    char *string = (char *)malloc(fsize + 1);
    size_t r = fread(string, 1, fsize, fd);
    fclose(fd);

    string[r] = 0;

    sl_binary_t *bin = sl_compile_str(string);

    free(string);

    return bin;
}

int sl_eval_file(sl_ctx_t *ctx, char *filename, sl_binary_t *(*load_module)(char *name), void *(trap)(sl_ctx_t *ctx))
{
    sl_binary_t *bin = sl_compile_file(filename);
    if (!bin)
        return 0;
    sl_try
    {
        sl_exec(ctx, ctx, bin, 0, load_module, trap);
        sl_binary_free(bin);
    }
    sl_catch
    {
        return 0;
    }
    return 1;
}

int sl_dis_str(sl_ctx_t *ctx, char *code, sl_binary_t *(*load_module)(char *name), void *(trap)(sl_ctx_t *ctx))
{
    sl_try
    {
        sl_parser_t *parser = sl_parser_new(code);
        sl_node_t *tree = sl_parse(parser);
        sl_parser_free(parser);

        sl_binary_t *bin = sl_binary_new();
        tree->codegen(tree, bin);
        sl_node_free(tree);

        sl_bytecode_fill(bin);

        for (int i = 0; i < bin->size; i++)
        {
            printf("%02hhx ", bin->block[i]);
        }
        printf("\n");
        dis(bin->block, bin->size);
        sl_binary_free(bin);
    }
    sl_catch
    {
        return 0;
    }
    return 1;
}

#endif