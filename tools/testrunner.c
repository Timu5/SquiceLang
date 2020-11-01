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
    "tests/test7_module.sqlang",
    "tests/test8_gc.sqlang",
    "tests/test9_class.sqlang"
    };

sl_ctx_t *global;

static void assertfn(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    int b = (int)sl_vector_pop(ctx->stack)->number;
    if(!b)
    {
        throw("Assert failed!");
    }
}

sl_vector(sl_binary_t*) modules = NULL;

sl_binary_t *load_module(char *name)
{
    FILE *fd;
    char fullname[255] = { 0 };
    strcpy(fullname, "tests/");
    strcat(fullname, name);
    strcat(fullname, ".sqlang");
    if((fd = fopen(fullname, "r")) != NULL)
    {
        fclose(fd);
        sl_binary_t *module = sl_compile_file(fullname);
        sl_vector_push(modules, module);
        return module;
    }
    return NULL;
}

int main(void)
{
    for(int i = 0; i < sizeof(tests)/sizeof(char*);i++)
    {
        printf("Running %s: ", tests[i]);

        sl_ctx_t *ctx = sl_ctx_new(NULL);
        sl_builtin_install(ctx);
        global = ctx;
        sl_ctx_addfn(ctx, NULL, strdup("assert"), 1, 0, assertfn);

        try
        {
            sl_eval_file(ctx, tests[i], load_module);
            printf(GREEN "passed" RESET "\n");
        }
        catch
        {
            printf(RED "failed" RESET "(%s)\n", ex_msg);
        }
        sl_gc_freeall();
        //sl_ctx_free(ctx);
        for(int i = 0; i < sl_vector_size(modules); i++)
        {
            sl_binary_free(modules[i]);
        }
        sl_vector_free(modules);
    }
    return 0;
}