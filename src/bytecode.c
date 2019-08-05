#include <stdlib.h>
#include <string.h>
#include "bytecode.h"

binary_t* binary_new()
{
    binary_t* bin = (binary_t*)malloc(sizeof(binary_t));
    bin->adresses = NULL;
    bin->symbols = NULL;
    bin->size = 0;
    bin->block = NULL;
    bin->index = 0;
    bin->loop = -1;
    return bin;
}

int bytecode_emit(binary_t* bin, int opcode)
{
    bin->size += 1;
    bin->block = (char*)realloc(bin->block, bin->size);
    bin->block[bin->size - 1] = (char)opcode;
    return bin->size - 1;
}

int bytecode_emitstr(binary_t* bin, int opcode, char* string)
{
    int len = strlen(string);
    bin->size += 1 + len + 1;
    bin->block = (char*)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - len - 1] = (char)opcode;
    memcpy(&bin->block[bin->size - 1 - len], string, len);
    bin->block[bin->size - 1] = 0;
    return bin->size - 1 - len - 1;
}

int bytecode_emitint(binary_t* bin, int opcode, int number)
{
    bin->size += 1 + 4;
    bin->block = (char*)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 4] = (char)opcode;
    *(int*)(&(bin->block[bin->size - 4])) = number;
    return bin->size - 1 - 4;
}

int bytecode_emitdouble(binary_t* bin, int opcode, double number)
{
    bin->size += 1 + 8;
    bin->block = (char*)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 8] = (char)opcode;
    *(double*)(&(bin->block[bin->size - 8])) = number;
    return bin->size - 1 - 8;
}
