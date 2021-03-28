#define SL_IMPLEMENTATION

#include <stdio.h>
#include <signal.h>

#include "SquiceLang.h"
#include "main.c"

#define RED "\033[31m"
#define GREEN "\033[32m"
#define RESET "\033[0m"

char *tests[] = {
    "tests/test1_logic.sqlang",
    "tests/test2_arithmetic.sqlang",
    "tests/test3_ifs.sqlang",
    "tests/test4_loop.sqlang",
    "tests/test5_string.sqlang",
    "tests/test6_list.sqlang",
    //"tests/test7_module.sqlang",
    "tests/test8_gc.sqlang",
    "tests/test9_class.sqlang",
    "tests/test10_fstring.sqlang",
    "tests/test11_trycatch.sqlang",
    "tests/test12_calc.sqlang",
    "tests/test12_lexer.sqlang",
    "tests/test12_multiassign.sqlang",
    "tests/test13_scope.sqlang"};

static void assertfn(int argc, sl_ctx_t *ctx)
{
    int b = (int)sl_vector_pop(ctx->stack)->number;
    if (!b)
    {
        sl_throw("Assert failed!");
    }
}

sl_vector(sl_binary_t *) modules = NULL;

sl_binary_t *load_module(char *name)
{
    FILE *fd;
    char fullname[255] = {0};
    sl_binary_t *module;
    strcpy(fullname, "../../tests/");
    strcat(fullname, name);
    strcat(fullname, ".sqlang");
    if ((fd = fopen(fullname, "r")) != NULL)
    {
        fclose(fd);
        module = sl_compile_file(fullname);
        sl_vector_push(modules, module);
        return module;
    }
    return NULL;
}

static void stop(int argc, sl_ctx_t *ctx)
{
    printf("stop\n");
}

static void ptr(int argc, sl_ctx_t *ctx)
{
    (void)argc;
    (void)ctx;
}

int main(void)
{

    for (size_t i = 0; i < sizeof(tests) / sizeof(char *); i++)
    {
        printf("Running %s: ", tests[i]);

        sl_ctx_t *ctx = sl_ctx_new(NULL);
        sl_builtin_install(ctx);

        sl_ctx_addfn(ctx, NULL, strdup("assert"), 1, 0, assertfn);
        sl_ctx_addfn(ctx, NULL, strdup("stop"), 0, 0, stop);
        sl_ctx_addfn(ctx, NULL, strdup("ptr"), 1, 0, ptr);

        char fullname[255] = {0};
        strcpy(fullname, "./");
        strcat(fullname, tests[i]);

        if (sl_eval_file(ctx, fullname, load_module, NULL))
        {
            printf(GREEN "passed" RESET "\n");
        }
        else
        {
            printf(RED "failed" RESET "(%s)\n", sl_ex_msg);
        }

        sl_gc_freeall();

        for (size_t j = 0; j < sl_vector_size(modules); j++)
        {
            sl_binary_free(modules[j]);
        }
        sl_vector_free(modules);
    }
    return 0;
}