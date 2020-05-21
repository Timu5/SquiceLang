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
    "tests/test2_arithmetic.sqlang"
    };

static void assertfn(sl_ctx_t *ctx)
{
    int n = (int)sl_vector_pop(ctx->stack)->number;
    int b = (int)sl_vector_pop(ctx->stack)->number;
    if(!b)
    {
        throw("Assert failed!");
    }
}

int main(void)
{
    for(int i = 0; i < sizeof(tests)/sizeof(char*);i++)
    {
        printf("Running %s: ", tests[i]);

        sl_ctx_t *ctx = sl_ctx_new(NULL);
        sl_builtin_install(ctx);
        sl_ctx_addfn(ctx, "assert", 0, assertfn);

        try
        {
            sl_eval_file(ctx, tests[i]);
            printf(GREEN "passed" RESET "\n");
        }
        catch
        {
            printf(RED "failed" RESET "(%s)\n", ex_msg);
        }
    }
    return 0;
}