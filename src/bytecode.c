#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include "SquiceLang.h"

sl_binary_t *sl_binary_new()
{
    sl_binary_t *bin = (sl_binary_t *)malloc(sizeof(sl_binary_t));
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

void sl_binary_save(sl_binary_t *bin, char *filename)
{
    FILE *file = fopen(filename, "wb");

    fwrite(bin->block, 1, bin->size, file);

    fclose(file);
}

int sl_bytecode_emit(sl_binary_t *bin, int opcode)
{
    bin->size += 1;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1] = (char)opcode;
    return bin->size - 1;
}

int sl_bytecode_emitstr(sl_binary_t *bin, int opcode, char *string)
{
    int len = (int)strlen(string);
    bin->size += 1 + len + 1;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - len - 1] = (char)opcode;
    memcpy(&bin->block[bin->size - 1 - len], string, len);
    bin->block[bin->size - 1] = 0;
    return bin->size - 1 - len - 1;
}

int sl_bytecode_emitint(sl_binary_t *bin, int opcode, int number)
{
    bin->size += 1 + 4;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 4] = (char)opcode;
    *(int *)(&(bin->block[bin->size - 4])) = number;
    return bin->size - 1 - 4;
}

int sl_bytecode_emitdouble(sl_binary_t *bin, int opcode, double number)
{
    bin->size += 1 + 8;
    bin->block = (char *)realloc(bin->block, bin->size);
    bin->block[bin->size - 1 - 8] = (char)opcode;
    *(double *)(&(bin->block[bin->size - 8])) = number;
    return bin->size - 1 - 8;
}

int sl_bytecode_addlabel(sl_binary_t *bin, char *name, int adress)
{
    sl_vector_push(bin->adresses, adress);
    sl_vector_push(bin->symbols, name);
    return 0;
}

int sl_bytecode_addtofill(sl_binary_t *bin, char *name, int adress)
{
    sl_vector_push(bin->fadresses, adress);
    sl_vector_push(bin->fsymbols, name);
    return 0;
}

int sl_bytecode_fill(sl_binary_t *bin)
{
    for (int i = 0; i < sl_vector_size(bin->fadresses); i++)
    {
        bin->fadresses[i];
        bin->fsymbols[i];
        for (int j = 0; j < sl_vector_size(bin->adresses); j++)
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