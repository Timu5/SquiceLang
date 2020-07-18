#define SL_IMPLEMENTATION

#include <stdio.h>
#include "SquiceLang.h"
#include "main.c"

char input_buffer[255];

int balanced()
{
    int i = 0;
    int par = 0;
    int bra = 0;
    int squ = 0;
    while (input_buffer[i] != '\0')
    {
        if (input_buffer[i] == '(')
            par++;
        if (input_buffer[i] == ')')
            par--;
        if (input_buffer[i] == '[')
            squ++;
        if (input_buffer[i] == ']')
            squ--;
        if (input_buffer[i] == '{')
            bra++;
        if (input_buffer[i] == '}')
            bra--;
        i++;
    }
    return par == 0 && bra == 0 && squ == 0;
}

void getstring()
{
    printf(">>> ");
    fgets(input_buffer, 255, stdin);

    while (!balanced())
    {
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
            sl_dis_str(ctx, input_buffer);
        }
        catch
        {
            puts(ex_msg);
        }
    }

    return 0;
}