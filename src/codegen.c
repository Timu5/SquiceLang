#include <stdio.h>
#include <string.h>

#include "SquiceLang.h"

void sl_codegen_root(sl_node_t *node, sl_binary_t *binary)
{
    for (int i = 0; i < sl_vector_size(node->root.funcs->block); i++)
    {
        sl_bytecode_emitint(binary, SL_OPCODE_PUSHI, (int)sl_vector_size(node->root.funcs->block[i]->func.args));
        int adr = sl_bytecode_emitint(binary, SL_OPCODE_PUSHI, 0x22222222);
        sl_bytecode_emitstr(binary, SL_OPCODE_STOREFN, node->root.funcs->block[i]->func.name);

        char *name = sl_mprintf("func_%s", node->root.funcs->block[i]->func.name);
        sl_bytecode_addtofill(binary, name, adr + 1);
    }

    for (int i = 0; i < sl_vector_size(node->root.stmts->block); i++)
    {
        sl_node_t *n = node->root.stmts->block[i];
        n->codegen(n, binary);
    }

    sl_bytecode_emit(binary, SL_OPCODE_RETN);

    for (int i = 0; i < sl_vector_size(node->root.funcs->block); i++)
    {
        sl_node_t *f = node->root.funcs->block[i];
        f->codegen(f, binary);
    }
}
void sl_codegen_ident(sl_node_t *node, sl_binary_t *binary)
{
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitstr(binary, SL_OPCODE_PUSHV, node->ident);
}

void sl_codegen_unary(sl_node_t *node, sl_binary_t *binary)
{
    node->unary.val->codegen(node->unary.val, binary);
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitint(binary, SL_OPCODE_UNARY, node->unary.op);
}

void sl_codegen_binary(sl_node_t *node, sl_binary_t *binary)
{
    node->binary.a->codegen(node->binary.a, binary);
    node->binary.b->codegen(node->binary.b, binary);
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitint(binary, SL_OPCODE_BINARY, node->binary.op);
}

void sl_codegen_double(sl_node_t *node, sl_binary_t *binary)
{
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitdouble(binary, SL_OPCODE_PUSHN, node->number);
}

void sl_codegen_string(sl_node_t *node, sl_binary_t *binary)
{
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitstr(binary, SL_OPCODE_PUSHS, node->string);
}

void sl_codegen_call(sl_node_t *node, sl_binary_t *binary)
{
    for (int i = (int)sl_vector_size(node->call.args->block) - 1; i >= 0; i--)
    {
        sl_node_t *n = node->call.args->block[i];
        n->codegen(n, binary);
    }
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitint(binary, SL_OPCODE_PUSHI, (int)sl_vector_size(node->call.args->block));

    node->call.func->codegen(node->call.func, binary);

    if (node->call.func->type == SL_NODETYPE_MEMBER)
    {
        int adr = binary->size - (int)strlen(node->call.func->member.name) - 1 - 1;
        binary->block[adr] = SL_OPCODE_MEMBERD;
        sl_bytecode_emit(binary, SL_OPCODE_CALLM);
    }
    else
    {
        sl_bytecode_emit(binary, SL_OPCODE_CALL);
    }
}

void sl_codegen_func(sl_node_t *node, sl_binary_t *binary)
{
    char *name = sl_mprintf("func_%s", node->func.name);
    sl_bytecode_addlabel(binary, name, binary->size);

    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emit(binary, SL_OPCODE_SCOPE);
    sl_bytecode_emitstr(binary, SL_OPCODE_STORE, "argc");
    for (int i = 0; i < (int)sl_vector_size(node->func.args); i++)
    {
        sl_bytecode_emitstr(binary, SL_OPCODE_STORE, node->func.args[i]);
    }

    node->func.body->codegen(node->func.body, binary);

    sl_bytecode_emit(binary, SL_OPCODE_ENDSCOPE);

    sl_bytecode_emit(binary, SL_OPCODE_RETN);
}

void sl_codegen_return(sl_node_t *node, sl_binary_t *binary)
{
    if (node->ret != NULL)
    {
        node->ret->codegen(node->ret, binary);
        sl_bytecode_adddebug(binary, node->marker);
        sl_bytecode_emit(binary, SL_OPCODE_RET);
    }
    else
    {
        sl_bytecode_adddebug(binary, node->marker);
        sl_bytecode_emit(binary, SL_OPCODE_RETN);
    }
}

void sl_codegen_cond(sl_node_t *node, sl_binary_t *binary)
{
    node->cond.arg->codegen(node->cond.arg, binary);
    sl_bytecode_adddebug(binary, node->marker);
    int adr = sl_bytecode_emitint(binary, SL_OPCODE_BRZ, 0x22222222); // fill it later with adress!!!
    char *name = sl_mprintf("cond_%d", binary->index);
    char *ename = sl_mprintf("condend_%d", binary->index++);
    sl_bytecode_addtofill(binary, strdup(name), adr + 1);
    node->cond.body->codegen(node->cond.body, binary);

    if (node->cond.elsebody != NULL)
    {
        adr = sl_bytecode_emitint(binary, SL_OPCODE_JMP, 0x22222222);
        sl_bytecode_addlabel(binary, strdup(name), binary->size);
        sl_bytecode_addtofill(binary, strdup(ename), adr + 1);
        node->cond.elsebody->codegen(node->cond.elsebody, binary);
        sl_bytecode_addlabel(binary, strdup(ename), binary->size);
    }
    else
    {
        sl_bytecode_addlabel(binary, strdup(name), binary->size);
    }
    free(ename);
    free(name);
}

void sl_codegen_loop(sl_node_t *node, sl_binary_t *binary)
{
    int old = binary->loop;
    int i = binary->index++;
    binary->loop = i;

    char *lname = sl_mprintf("loops_%d", i);
    char *lename = sl_mprintf("loopend_%d", i);

    sl_bytecode_addlabel(binary, strdup(lname), binary->size);
    node->loop.arg->codegen(node->loop.arg, binary);
    sl_bytecode_adddebug(binary, node->marker);
    int adr = sl_bytecode_emitint(binary, SL_OPCODE_BRZ, 0x22222222);
    sl_bytecode_addtofill(binary, strdup(lename), adr + 1);
    node->loop.body->codegen(node->loop.body, binary);
    adr = sl_bytecode_emitint(binary, SL_OPCODE_JMP, 0x22222222);
    sl_bytecode_addtofill(binary, strdup(lname), adr + 1);
    sl_bytecode_addlabel(binary, strdup(lename), binary->size);

    binary->loop = old;
    free(lname);
    free(lename);
}

void sl_codegen_break(sl_node_t *node, sl_binary_t *binary)
{
    if (binary->loop < 0)
        sl_throw("No loop to break from at line %d column %d",
              node->marker.line,
              node->marker.column);

    char *lename = sl_mprintf("loopend_%d", binary->loop);

    sl_bytecode_adddebug(binary, node->marker);
    int adr = sl_bytecode_emitint(binary, SL_OPCODE_JMP, 0x22222222);
    sl_bytecode_addtofill(binary, lename, adr + 1);
}

void sl_codegen_decl(sl_node_t *node, sl_binary_t *binary)
{
    if (sl_vector_size(node->decl.names) == 1)
    {
        sl_bytecode_adddebug(binary, node->marker);
        sl_bytecode_emitint(binary, SL_OPCODE_PUSHI, 0);
        sl_bytecode_emitstr(binary, SL_OPCODE_STORE, node->decl.names[0]);

        sl_bytecode_adddebug(binary, node->marker);
        sl_bytecode_emitstr(binary, SL_OPCODE_PUSHV, node->decl.names[0]);
        node->decl.value->codegen(node->decl.value, binary);
        sl_bytecode_emitint(binary, SL_OPCODE_BINARY, SL_TOKEN_ASSIGN);
    }
    else
    {
        sl_bytecode_adddebug(binary, node->marker);
        for (int i = 0; i < sl_vector_size(node->decl.names); i++)
        {
            sl_bytecode_emitint(binary, SL_OPCODE_PUSHI, 0);
            sl_bytecode_emitstr(binary, SL_OPCODE_STORE, node->decl.names[i]);
        }

        sl_bytecode_adddebug(binary, node->marker);
        for (int i = 0; i < sl_vector_size(node->decl.names); i++)
        {
            // make tuple out of variables
            sl_bytecode_emitstr(binary, SL_OPCODE_PUSHV, node->decl.names[i]);
            if (i >= 1)
                sl_bytecode_emitint(binary, SL_OPCODE_BINARY, SL_TOKEN_COMMA);
        }
        node->decl.value->codegen(node->decl.value, binary);

        sl_bytecode_emitint(binary, SL_OPCODE_BINARY, SL_TOKEN_ASSIGN);
    }
}

void sl_codegen_index(sl_node_t *node, sl_binary_t *binary)
{
    node->index.var->codegen(node->index.var, binary);
    node->index.expr->codegen(node->index.expr, binary);
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emit(binary, SL_OPCODE_INDEX);
}

void sl_codegen_block(sl_node_t *node, sl_binary_t *binary)
{
    sl_bytecode_emit(binary, SL_OPCODE_SCOPE);
    for (int i = 0; i < sl_vector_size(node->block); i++)
        node->block[i]->codegen(node->block[i], binary);
    sl_bytecode_emit(binary, SL_OPCODE_ENDSCOPE);
}

void sl_codegen_member(sl_node_t *node, sl_binary_t *binary)
{
    node->member.parent->codegen(node->member.parent, binary);
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitstr(binary, SL_OPCODE_MEMBER, node->member.name);
}

void sl_codegen_import(sl_node_t *node, sl_binary_t *binary)
{
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emitstr(binary, SL_OPCODE_IMPORT, node->import);
}

void sl_codegen_class(sl_node_t *node, sl_binary_t *binary)
{
}

void sl_codegen_trycatch(sl_node_t *node, sl_binary_t *binary)
{
    int i = binary->index++;

    char *lname = sl_mprintf("catch_%d", i);
    char *lename = sl_mprintf("catchend_%d", i);

    sl_bytecode_adddebug(binary, node->marker);
    int adr = sl_bytecode_emitint(binary, SL_OPCODE_TRY, 0x22222222);
    sl_bytecode_addtofill(binary, strdup(lname), adr + 1);

    node->trycatch.tryblock->codegen(node->trycatch.tryblock, binary);

    sl_bytecode_emit(binary, SL_OPCODE_ENDTRY);
    adr = sl_bytecode_emitint(binary, SL_OPCODE_JMP, 0x22222222);
    sl_bytecode_addtofill(binary, strdup(lename), adr + 1);

    sl_bytecode_addlabel(binary, strdup(lname), binary->size);
    // TODO: Add support for custom exception name
    sl_bytecode_emit(binary, SL_OPCODE_SCOPE);
    sl_bytecode_emitstr(binary, SL_OPCODE_STORE, "exception");
    node->trycatch.tryblock->codegen(node->trycatch.catchblock, binary);
    sl_bytecode_emit(binary, SL_OPCODE_ENDSCOPE);

    sl_bytecode_addtofill(binary, strdup(lename), adr + 1);
    sl_bytecode_addlabel(binary, strdup(lename), binary->size);

    free(lname);
    free(lename);
}

void sl_codegen_throw(sl_node_t *node, sl_binary_t *binary)
{
    node->ret->codegen(node->throw, binary);
    sl_bytecode_adddebug(binary, node->marker);
    sl_bytecode_emit(binary, SL_OPCODE_THROW);
}
