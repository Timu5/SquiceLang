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
        printf("%d %02hhx:\t\t\t\t", ip, byte);
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
        case SL_OPCODE_IMPORT:
            printf("import \"%s\"", getstr(opcodes, &ip));
            break;
        default:
            printf("data %x", byte);
            break;
        }

        putchar('\n');
    }

    ip = 0;
}
void sl_exec(sl_ctx_t *global, sl_ctx_t *context, sl_binary_t *binary, int ip, sl_binary_t *(*load_module)(char *name))
{
    char *opcodes = binary->block;
    int size = binary->size;
    sl_vector(int) call_stack = NULL;

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
            char* name = getstr(opcodes, &ip);
            sl_value_t *val = sl_ctx_getvar(context, name);
            if(val == NULL)
            {
                throw("No variable named '%s'", name);
            }
            sl_vector_push(global->stack, val);
            free(name);
            break;
        }
        case SL_OPCODE_POP:
            sl_vector_pop(global->stack);
            break;
        case SL_OPCODE_STOREFN:
        {
            int adr = (int)sl_vector_pop(global->stack)->number;
            int argc = (int)sl_vector_pop(global->stack)->number;
            sl_ctx_addfn(context, binary, getstr(opcodes, &ip), argc, adr, NULL);
            break;
        }
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
                throw("Can only call functions");
            }
            sl_fn_t *fn = fn_value->fn;
            sl_value_t *parent = NULL;
            if (byte == SL_OPCODE_CALLM)
            {
                parent = sl_vector_pop(global->stack);
            }
            sl_value_t *argc = sl_vector_pop(global->stack);
            
            if((int)(argc->number) < fn->argc)
            {
                throw("To little arguments for function, expect %d got %d", fn->argc, (int)(argc->number));
            }
            sl_vector_push(global->stack, argc);
            if (fn->native != NULL)
            {
                fn->native(global);
            }
            else
            {
                sl_binary_t *fn_binary = fn->binary;
                if(binary == fn_binary)
                {
                    // same module :)
                    context = sl_ctx_new(context);
                    if (byte == SL_OPCODE_CALLM)
                    {
                        sl_ctx_addvar(context, strdup("this"), parent); // add "this" variable
                    }
                    sl_vector_push(call_stack, ip);
                    //sl_vector_pop(global->stack);
                    ip = fn->address;
                }
                else
                {
                    // diffrent module :(
                    sl_ctx_t *n_context = sl_ctx_new(fn->ctx);
                    if (byte == SL_OPCODE_CALLM)
                    {
                        sl_ctx_addvar(n_context, strdup("this"), parent); // add "this" variable
                    }
                    sl_exec(global, n_context, fn->binary, fn->address, load_module);
                    // what with return value ???? :(
                }
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
            free(name);
            break;
        }
        case SL_OPCODE_MEMBERD:
        {
            sl_value_t *var = sl_vector_pop(global->stack);
            char *name = getstr(opcodes, &ip);
            sl_vector_push(global->stack, var);
            sl_vector_push(global->stack, sl_value_member(name, var));
            free(name);
            break;
        }
        case SL_OPCODE_IMPORT:
        {
            // eval module
            char *name = getstr(opcodes, &ip);
            if(load_module == NULL)
                throw("Module loading not supported");

            sl_binary_t *module = load_module(name);
            if(module == NULL)
                throw("Cannot find %s module", name);

            sl_ctx_t *module_ctx = sl_ctx_new(NULL);
            sl_builtin_install(module_ctx);
            module_ctx = sl_ctx_new(module_ctx);
            sl_exec(module_ctx, module_ctx, module, 0, load_module);
            
            // load it's context into dictonary value
            sl_vector(char*) names = NULL;
            sl_vector(sl_value_t*) values = NULL;
            for(int i = 0; i < sl_vector_size(module_ctx->vars); i++)
            {
                char* vname = module_ctx->vars[i]->name;
                sl_value_t* value = module_ctx->vars[i]->val;
                sl_vector_push(names, vname);
                sl_vector_push(values, value);
            }
            sl_value_t *module_dict = sl_value_dict(names, values);

            // assign this value to variable withing current context
            sl_ctx_addvar(context, name, module_dict);
            break;
        }
        }
    }
}