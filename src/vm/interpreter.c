#include <stdio.h>
#include "bytecode.h"
#include "value.h"
#include "builtin.h"
#include "contex.h"

int ip = 0;
char *opcodes = NULL;
vector(int) call_stack = NULL;
ctx_t *global;

static char *getstr()
{
    char buffer[512];
    int i = 0;
    while(opcodes[ip + i] != NULL)
    {
        buffer[i] = opcodes[ip + i];
        i++;
    }
    buffer[i] = 0;
    ip += i;
    return strdup(buffer);
}

static int getint()
{
    ip += 4;
    return *(int*)&opcodes[ip];
}

static double getdouble()
{
    ip += 8;
    return *(double*)&opcodes[ip];
}

int main()
{
    global = ctx_new(NULL);
    builtin_install(global);

    FILE *file = fopen("test.bin", "rb");
    fseek(file, 0, SEEK_END);
    long fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    opcodes = (char*)malloc(fsize);
    fread(opcodes, 1, fsize, file);
    fclose(file);

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
            vector_push(call_stack, ip);
            ip = getint();
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
            if (v->number == 0)
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

    return 0;
}