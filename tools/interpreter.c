#define SL_IMPLEMENTATION

#include <stdio.h>
#include "SquiceLang.h"
#include "main.c"

char input_buffer[255];
sl_ctx_t *global;

int balanced()
{
    int i = 0;
    int par = 0;
    int bra = 0;
    int squ = 0;
    while (input_buffer[i] != '\0')
    {
        switch (input_buffer[i])
        {
            case '(': par++; break;
            case ')': par--; break;
            case '[': squ++; break;
            case ']': squ--; break;
            case '{': bra++; break;
            case '}': bra--; break;
        }
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

sl_binary_t *load_module(char *name)
{
    FILE *fd;
    char fullname[255] = { 0 };
    strcpy(fullname, name);
    strcat(fullname, ".sqlang");
    if((fd = fopen(fullname, "r")) != NULL)
    {
        fclose(fd);
        return sl_compile_file(fullname);
    }
    return NULL;
}

int main(int argc, char **argv)
{
    sl_ctx_t *ctx = sl_ctx_new(NULL);
    sl_builtin_install(ctx);
    global = ctx;

    while (1)
    {
        try
        {
            getstring();
            sl_dis_str(ctx, input_buffer, load_module, NULL);
            sl_eval_str(ctx, input_buffer, load_module, NULL);
        }
        catch
        {
            puts(ex_msg);
        }
    }

    return 0;
}