#ifndef _BYTECODE_H_
#define _BYTECODE_H_

#include "vector.h"

enum SL_OPCODE
{
    SL_OPCODE_NOP,
    SL_OPCODE_PUSHN, // push number
    SL_OPCODE_PUSHS, // push string
    SL_OPCODE_PUSHV, // push value
    SL_OPCODE_POP,   // remove element from stack
    SL_OPCODE_STORE,
    SL_OPCODE_STOREFN, // store function
    SL_OPCODE_UNARY,
    SL_OPCODE_BINARY,
    SL_OPCODE_CALL,
    SL_OPCODE_CALLM, // call member
    SL_OPCODE_RET,
    SL_OPCODE_RETN, // return null
    SL_OPCODE_JMP,
    SL_OPCODE_BRZ, // brach if zero
    SL_OPCODE_INDEX,
    SL_OPCODE_MEMBER,
    SL_OPCODE_MEMBERD // member with parent duplicate
};

// Structure to store bytecode in binary format
struct sl_binary_s
{
    sl_vector(int) adresses;
    sl_vector(char *) symbols;
    sl_vector(int) fadresses;
    sl_vector(char *) fsymbols;
    char *block;
    int size;
    int index; // free label index
    int loop;  // hold current loop index, needed by break
};

typedef struct sl_binary_s sl_binary_t;

sl_binary_t *sl_binary_new();
void sl_binary_save(sl_binary_t *bin, char *filename);

int sl_bytecode_emit(sl_binary_t *bin, int opcode);
int sl_bytecode_emitstr(sl_binary_t *bin, int opcode, char *string);
int sl_bytecode_emitint(sl_binary_t *bin, int opcode, int number);
int sl_bytecode_emitdouble(sl_binary_t *bin, int opcode, double number);

int sl_bytecode_addlabel(sl_binary_t *bin, char *name, int adress);
int sl_bytecode_addtofill(sl_binary_t *bin, char *name, int adress);
int sl_bytecode_fill(sl_binary_t *bin);

#endif
