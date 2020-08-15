#include <stdio.h>

#include "SquiceLang.h"

static char *getstr(char *opcodes, int *ip)
{
    char buffer[512];
    int i = 0;
    while (opcodes[*ip + i] != '\0')
    {
        buffer[i] = opcodes[*ip + i];
        i++;
    }
    buffer[i] = 0;
    *ip += i + 1;
    return strdup(buffer);
}

static int getint(char *opcodes, int *ip)
{
    int value = *(int *)&opcodes[*ip];
    *ip += 4;
    return value;
}

static double getdouble(char *opcodes, int *ip)
{
    double value = *(double *)&opcodes[*ip];
    *ip += 8;
    return value;
}

void dis(char *opcodes, long fsize)
{
    int ip = 0;
    while (1)
    {
        if (ip >= fsize)
            break;
        int byte = opcodes[ip];
        printf("%d %x:\t\t\t\t", ip, byte);
        ip += 1;

        switch (byte)
        {
        case SL_OPCODE_NOP:
            printf("NOP");
            break;
        case SL_OPCODE_PUSHI:
            printf("pushi %d", getint(opcodes, &ip));
            break;
        case SL_OPCODE_PUSHN:
            printf("pushn %f", getdouble(opcodes, &ip));
            break;
        case SL_OPCODE_PUSHS:
            printf("pushs \"%s\"", getstr(opcodes, &ip));
            break;
        case SL_OPCODE_PUSHV:
            printf("pushv \"%s\"", getstr(opcodes, &ip));
            break;
        case SL_OPCODE_POP:
            printf("pop");
            break;
        case SL_OPCODE_STOREFN:
            printf("storefn \"%s\"", getstr(opcodes, &ip));
            break;
        case SL_OPCODE_STORE:
            printf("store \"%s\"", getstr(opcodes, &ip));
            break;
        case SL_OPCODE_UNARY:
            printf("unary %d", getint(opcodes, &ip));
            break;
        case SL_OPCODE_BINARY:
            printf("binary %d", getint(opcodes, &ip));
            break;
        case SL_OPCODE_CALL:
            printf("call");
            break;
        case SL_OPCODE_CALLM:
            printf("callm");
            break;
        case SL_OPCODE_RETN:
            printf("retn");
            break;
        case SL_OPCODE_RET:
            printf("ret");
            break;
        case SL_OPCODE_JMP:
            printf("jmp %d", getint(opcodes, &ip));
            break;
        case SL_OPCODE_BRZ:
            printf("brz %d", getint(opcodes, &ip));
            break;
        case SL_OPCODE_INDEX:
            printf("index");
            break;
        case SL_OPCODE_MEMBER:
            printf("member \"%s\"", getstr(opcodes, &ip));
            break;
        case SL_OPCODE_MEMBERD:
            printf("memberd \"%s\"", getstr(opcodes, &ip));
            break;
        default:
            printf("data %x", byte);
            break;
        }

        putchar('\n');
    }

    ip = 0;
}

void sl_exec(sl_ctx_t *global, sl_binary_t *binary, char *opcodes, int size)
{
    int ip = 0;
    long fsize = 0;
    sl_vector(int) call_stack = NULL;
    
    sl_ctx_t *context = global;

    while (1)
    {
        if (ip >= size)
            break;
        int byte = opcodes[ip];
        ip += 1;

        switch (byte)
        {
        case SL_OPCODE_NOP:
            break;
        case SL_OPCODE_PUSHN:
            sl_vector_push(global->stack, sl_value_number(getdouble(opcodes, &ip)));
            break;
        case SL_OPCODE_PUSHI:
            sl_vector_push(global->stack, sl_value_number(getint(opcodes, &ip)));
            break;
        case SL_OPCODE_PUSHS:
            sl_vector_push(global->stack, sl_value_string(getstr(opcodes, &ip)));
            break;
        case SL_OPCODE_PUSHV:
        {
            sl_value_t *val = sl_ctx_getvar(context, getstr(opcodes, &ip));
            if(val == NULL)
            {
                throw("No such variable!");
            }
            sl_vector_push(global->stack, val);
            break;
        }
        case SL_OPCODE_POP:
            sl_vector_pop(global->stack);
            break;
        case SL_OPCODE_STOREFN:
            sl_ctx_addfn(context, binary, getstr(opcodes, &ip), (int)sl_vector_pop(global->stack)->number, NULL);
            break;
        case SL_OPCODE_STORE:
            sl_ctx_addvar(context, getstr(opcodes, &ip), sl_vector_pop(global->stack));
            break;
        case SL_OPCODE_UNARY:
        {
            sl_value_t *a = sl_vector_pop(global->stack);
            sl_vector_push(global->stack, sl_value_unary(getint(opcodes, &ip), a));
            break;
        }
        case SL_OPCODE_BINARY:
        {
            sl_value_t *b = sl_vector_pop(global->stack);
            sl_value_t *a = sl_vector_pop(global->stack);
            sl_vector_push(global->stack, sl_value_binary(getint(opcodes, &ip), a, b));
            break;
        }
        case SL_OPCODE_CALL:
        case SL_OPCODE_CALLM:
        {
            sl_value_t *fn_value = sl_vector_pop(global->stack);
            while (fn_value->type == SL_VALUE_REF)
                fn_value = fn_value->ref;
            if (fn_value->type != SL_VALUE_FN)
            {
                throw("Can only call functions!");
            }
            if (fn_value->fn->native != NULL)
            {
                fn_value->fn->native(global);
            }
            else
            {
                context = sl_ctx_new(context);
                if (byte == SL_OPCODE_CALLM)
                {
                    sl_ctx_addvar(context, "this", sl_vector_pop(global->stack)); // add "this" variable
                }
                sl_vector_push(call_stack, ip);
                //sl_vector_pop(global->stack);
                ip = fn_value->fn->address;
            }
        }
            break;
        case SL_OPCODE_RETN:
            sl_vector_push(global->stack, sl_value_null());
        case SL_OPCODE_RET:
            if (sl_vector_size(call_stack) == 0)
            {
                // nothing to return
                return;
            }
            int ret_adr = sl_vector_pop(call_stack);
            ip = ret_adr;
            if (context->parent != NULL)
                context = context->parent;
            break;
        case SL_OPCODE_JMP:
            ip = getint(opcodes, &ip);
            break;
        case SL_OPCODE_BRZ:
        {
            sl_value_t *v = sl_vector_pop(global->stack);
            int nip = getint(opcodes, &ip);
            if (v->number == 0)
                ip = nip;
        }
            break;
        case SL_OPCODE_INDEX:
        {
            sl_value_t *expr = sl_vector_pop(global->stack);
            sl_value_t *var = sl_vector_pop(global->stack);
            sl_vector_push(global->stack, sl_value_get((int)expr->number, var));
            break;
        }
        case SL_OPCODE_MEMBER:
        {
            sl_value_t *var = sl_vector_pop(global->stack);
            char *name = getstr(opcodes, &ip);
            sl_vector_push(global->stack, sl_value_member(name, var));
            break;
        }
        case SL_OPCODE_MEMBERD:
        {
            sl_value_t *var = sl_vector_pop(global->stack);
            char *name = getstr(opcodes, &ip);
            sl_vector_push(global->stack, var);
            sl_vector_push(global->stack, sl_value_member(name, var));
            break;
        }
        }
    }
}
/*
int vm_main(int argc, char ** argv)
{
    if (argc < 2)
    {
        printf("Usage: vm input\n");
        return -1;
    }

    FILE *file = fopen(argv[1], "rb");
    fseek(file, 0, SEEK_END);
    int fsize = ftell(file);
    fseek(file, 0, SEEK_SET);

    int opcodes = (char *)malloc(fsize);
    fread(opcodes, 1, fsize, file);
    fclose(file);

    try
    {
        exec(opcodes, fsize);
    }
    catch
    {
        puts(ex_msg);
    }

    return 0;
}*/