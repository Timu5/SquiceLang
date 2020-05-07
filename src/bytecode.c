#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "bytecode.h"

binary_t *binary_new()
{
    binary_t *bin = (binary_t *)malloc(sizeof(binary_t));
    bin->adresses = NULL;
    bin->symbols = NULL;
    bin->fadresses = NULL;
    bin->fsymbols = NULL;
    bin->size = 0;
    bin->block = NULL;
    bin->index = 0;
    bin->loop = -1;
    return bin;
}

void binary_save(binary_t *bin, char *filename)
{
    FILE *file = fopen(filename, "wb");

    fwrite(bin->block, 1, bin->size, file);

    fclose(file);
}

int bytecode_emit(binary_t *bin, int opcode)
{
    bin->size += 1;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1] = (char)opcode;
    return bin->size - 1;
}

int bytecode_emitstr(binary_t *bin, int opcode, char *string)
{
    int len = strlen(string);
    bin->size += 1 + len + 1;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - len - 1] = (char)opcode;
    memcpy(&bin->block[bin->size - 1 - len], string, len);
    bin->block[bin->size - 1] = 0;
    return bin->size - 1 - len - 1;
}

int bytecode_emitint(binary_t *bin, int opcode, int number)
{
    bin->size += 1 + 4;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 4] = (char)opcode;
    *(int *)(&(bin->block[bin->size - 4])) = number;
    return bin->size - 1 - 4;
}

int bytecode_emitdouble(binary_t *bin, int opcode, double number)
{
    bin->size += 1 + 8;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 8] = (char)opcode;
    *(double *)(&(bin->block[bin->size - 8])) = number;
    return bin->size - 1 - 8;
}

int bytecode_addlabel(binary_t *bin, char *name, int adress)
{
    vector_push(bin->adresses, adress);
    vector_push(bin->symbols, name);
}

int bytecode_addtofill(binary_t *bin, char *name, int adress)
{
    vector_push(bin->fadresses, adress);
    vector_push(bin->fsymbols, name);
}

int bytecode_fill(binary_t *bin)
{
    for (int i = 0; i < vector_size(bin->fadresses); i++)
    {
        bin->fadresses[i];
        bin->fsymbols[i];
        for (int j = 0; j < vector_size(bin->adresses); j++)
        {
            if (strcmp(bin->fsymbols[i], bin->symbols[j]) == 0)
            {
                *(int *)(&(bin->block[bin->fadresses[i]])) = bin->adresses[j];
                goto end;
            }
        }
        throw("Symbol not found: %s\n", bin->fsymbols[i]);
    end:
        1;
    }
    return 0;
}