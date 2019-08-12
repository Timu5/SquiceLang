#include <stdio.h>
#include "bytecode.h"
#include "utils.h"
#include "value.h"
#include "builtin.h"
#include "contex.h"

int ip = 0;
char *opcodes = NULL;
long fsize = 0;
vector(int) call_stack = NULL;
ctx_t *global;

static char *getstr()
{
    char buffer[512];
    int i = 0;
    while (opcodes[ip + i] != NULL)
    {
        buffer[i] = opcodes[ip + i];
        i++;
    }
    buffer[i] = 0;
    ip += i + 1;
    return strdup(buffer);
}

static int getint()
{
    int value = *(int *)&opcodes[ip];
    ip += 4;
    return value;
}

static double getdouble()
{
    double value = *(double *)&opcodes[ip];
    ip += 8;
    return value;
}

void dis()
{
    ip = 0;
    while (1)
    {
        if (ip >= fsize)
            break;
        int byte = opcodes[ip];
        printf("%d:\t\t\t\t", ip);
        ip += 1;

        switch (byte)
        {
        case O_NOP:
            printf("NOP");
            break;
        case O_PUSHN:
            printf("pushn %d", getint());
            break;
        case O_PUSHS:
            printf("pushs \"%s\"", getstr());
            break;
        case O_PUSHV:
            printf("pushv \"%s\"", getstr());
            break;
        case O_STORE:
            printf("store \"%s\"", getstr());
            break;
        case O_UNARY:
            printf("unary %d", getint());
            break;
        case O_BINARY:
            printf("binary %d", getint());
            break;
        case O_CALL:
            printf("call");
            break;
        case O_RETN:
            printf("retn");
            break;
        case O_RET:
            printf("ret");
            break;
        case O_JMP:
            printf("jmp %d", getint());
            break;
        case O_BRZ:
            printf("brz %d", getint());
            break;
        case O_INDEX:
            printf("index");
            break;
        case O_MEMBER:
            printf("member \"%s\"", getstr());
            break;
        default:
            printf("data %x", byte);
            break;
        }

        putchar('\n');
    }

    ip = 0;
}

int main()
{
    global = ctx_new(NULL);
    builtin_install(global);

    FILE *file = fopen("test.bin", "rb");
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    opcodes = (char *)malloc(fsize);
    fread(opcodes, 1, fsize, file);
    fclose(file);

    //dis();
    //return 1;

    try
    {
        while (1)
        {
            if (ip >= fsize)
                break;
            int byte = opcodes[ip];
            ip += 1;

            switch (byte)
            {
            case O_NOP:
                break;
            case O_PUSHN:
                vector_push(global->stack, value_number(getint()));
                break;
            case O_PUSHS:
                vector_push(global->stack, value_string(getstr()));
                break;
            case O_PUSHV:
                vector_push(global->stack, ctx_getvar(global, getstr()));
                break;
            case O_STORE:
                ctx_addvar(global, getstr(), vector_pop(global->stack));
                break;
            case O_UNARY:
            {
                value_t *a = vector_pop(global->stack);
                vector_push(global->stack, value_unary(getint(), a));
                break;
            }
            case O_BINARY:
            {
                value_t *a = vector_pop(global->stack);
                value_t *b = vector_pop(global->stack);
                vector_push(global->stack, value_binary(getint(), a, b));
                break;
            }
            case O_CALL:
                value_t *fn = vector_pop(global->stack);
                while (fn->type == V_REF)
                    fn = fn->ref;
                if (fn->type != V_FN)
                {
                    printf("Can only call functions!");
                    return;
                }
                if (fn->fn->native != NULL)
                {
                    fn->fn->native(global);
                }
                else
                {
                    //vector_push(call_stack, ip);
                    //ip = getint();
                    printf("Calling not implemented!");
                }
                break;
            case O_RETN:
                vector_push(global->stack, value_null());
            case O_RET:
                if (vector_size(call_stack) == 0)
                {
                    printf("Nothing to return from!");
                    return;
                }
                int ret_adr = vector_pop(call_stack);
                ip = ret_adr;
                break;
            case O_JMP:
                ip = getint();
                break;
            case O_BRZ:
                value_t *v = vector_pop(global->stack);
                int nip = getint();
                if (v->number != 0)
                    ip = nip;
                break;
            case O_INDEX:
            {
                value_t *expr = vector_pop(global->stack);
                value_t *var = vector_pop(global->stack);
                vector_push(global->stack, value_get((int)expr->number, var));
                break;
            }
            case O_MEMBER:
            {
                value_t *var = vector_pop(global->stack);
                char *name = getstr();
                vector_push(global->stack, value_member(name, var));
                break;
            }
            }
        }
    }
    catch
    {
        puts(ex_msg);
    }

    return 0;
}