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
    while (opcodes[ip + i] != '\0')
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
        case O_POP:
            printf("pop");
            break;
        case O_STOREFN:
            printf("storefn \"%s\"", getstr());
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
        case O_CALLM:
            printf("callm");
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
        case O_MEMBERD:
            printf("memberd \"%s\"", getstr());
            break;
        default:
            printf("data %x", byte);
            break;
        }

        putchar('\n');
    }

    ip = 0;
}

void exec(ctx_t *global, char * opcodes, int size)
{
    ctx_t *context = global;
    while (1)
    {
        if (ip >= size)
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
            vector_push(global->stack, ctx_getvar(context, getstr()));
            break;
        case O_POP:
            vector_pop(global->stack);
            break;
        case O_STOREFN:
            ctx_addfn(context, getstr(), (int)vector_pop(global->stack)->number, NULL);
            break;
        case O_STORE:
            ctx_addvar(context, getstr(), vector_pop(global->stack));
            break;
        case O_UNARY:
        {
            value_t *a = vector_pop(global->stack);
            vector_push(global->stack, value_unary(getint(), a));
            break;
        }
        case O_BINARY:
        {
            value_t *b = vector_pop(global->stack);
            value_t *a = vector_pop(global->stack);
            vector_push(global->stack, value_binary(getint(), a, b));
            break;
        }
        case O_CALL:
        case O_CALLM:
            value_t *fn_value = vector_pop(global->stack);
            while (fn_value->type == V_REF)
                fn_value = fn_value->ref;
            if (fn_value->type != V_FN)
            {
                throw("Can only call functions!");
            }
            if (fn_value->fn->native != NULL)
            {
                fn_value->fn->native(global);
            }
            else
            {
                context = ctx_new(context);
                if (byte == O_CALLM)
                {
                    ctx_addvar(context, "this", vector_pop(global->stack)); // add "this" variable
                }
                vector_push(call_stack, ip);
                //vector_pop(global->stack);
                ip = fn_value->fn->address;
            }
            break;
        case O_RETN:
            vector_push(global->stack, value_null());
        case O_RET:
            if (vector_size(call_stack) == 0)
            {
                throw("Nothing to return from!");
                return;
            }
            int ret_adr = vector_pop(call_stack);
            ip = ret_adr;
            if (context->parent != NULL)
                context = context->parent;
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
        case O_MEMBERD:
        {
            value_t *var = vector_pop(global->stack);
            char *name = getstr();
            vector_push(global->stack, var);
            vector_push(global->stack, value_member(name, var));
            break;
        }
        }
    }
}

int main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Usage: vm input\n");
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");
    fseek(file, 0, SEEK_END);
    fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    opcodes = (char *)malloc(fsize);
    fread(opcodes, 1, fsize, file);
    fclose(file);

    global = ctx_new(NULL);
    builtin_install(global);

    try
    {
        exec(global, opcodes, fsize);
    }
    catch
    {
        puts(ex_msg);
    }

    return 0;
}