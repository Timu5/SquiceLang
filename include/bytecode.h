#ifndef _BYTECODE_H_
#define _BYTECODE_H_

#include "vector.h"

enum {
    O_NOP,
    O_PUSHN, // push number
    O_PUSHS, // push string
    O_PUSHV, // push value
    O_STORE,
    O_UNARY,
    O_BINARY,
    O_CALL,
    O_RET,
    O_RETN, // return null
    O_JMP,
    O_BRZ, // brach if zero
    O_INDEX,
    O_MEMBER
};

// Structure to store bytecode in binary format
struct binary_s {
    vector(int) adresses;
    vector(char*) symbols;
    vector(int) fadresses;
    vector(char*) fsymbols;
    char* block;
    int size;
    int index; // free label index
    int loop; // hold current loop index, needed by break
};

typedef struct binary_s binary_t;

binary_t* binary_new();

int bytecode_emit(binary_t* bin, int opcode);
int bytecode_emitstr(binary_t* bin, int opcode, char* string);
int bytecode_emitint(binary_t* bin, int opcode, int number);
int bytecode_emitdouble(binary_t* bin, int opcode, double number);

int bytecode_addlabel(binary_t* bin, char* name, int adress);
int bytecode_addtofill(binary_t* bin, char* name, int adress);
int bytecode_filllabels(binary_t* bin);

#endif
