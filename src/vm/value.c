#include <stdlib.h>
#include <string.h>

#include "SquiceLang.h"

sl_value_t *sl_value_null()
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_NULL;
    v->constant = 0;
    v->refs = 0;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_number(double val)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_NUMBER;
    v->constant = 0;
    v->refs = 0;
    v->number = val;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_string(char *val)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_STRING;
    v->constant = 0;
    v->refs = 0;
    v->string = val;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_array(sl_vector(sl_value_t *) arr)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_ARRAY;
    v->constant = 0;
    v->refs = 0;
    v->array = arr;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_dict(sl_vector(char *) names, sl_vector(sl_value_t *) values)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_DICT;
    v->constant = 0;
    v->refs = 0;
    v->dict.names = names;
    v->dict.values = values;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_fn(sl_fn_t *fn)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_FN;
    v->constant = 0;
    v->refs = 0;
    v->fn = fn;
    v->markbit = 0;
    return v;
}

sl_value_t *sl_value_ref(sl_value_t *val)
{
    sl_value_t *v = sl_gc_alloc_value();
    v->type = SL_VALUE_REF;
    v->refs = 0;
    v->ref = val;
    val->refs++;
    v->markbit = 0;
    return v;
}

void sl_value_free_members(sl_value_t *val)
{
    if (val->type == SL_VALUE_STRING)
    {
        free(val->string);
    }
    else if (val->type == SL_VALUE_ARRAY)
    {
        //for (int i = 0; i < sl_vector_size(val->array); i++)
        //    sl_value_free(val->array[i]);
        sl_vector_free(val->array);
    }
    else if (val->type == SL_VALUE_DICT)
    {
        for (int i = 0; i < sl_vector_size(val->dict.names); i++)
            free(val->dict.names[i]);
        sl_vector_free(val->dict.names);
        sl_vector_free(val->dict.values);
    }
    else if (val->type == SL_VALUE_FN)
    {
        free(val->fn);
    }
}

void sl_value_free(sl_value_t *val)
{
    sl_value_free_members(val);
    free(val);
}

void sl_value_assign(sl_value_t *a, sl_value_t *b)
{
    sl_value_t *olda = a;

    //while (a->type == SL_VALUE_REF)
    //    a = a->ref;
    while (b->type == SL_VALUE_REF)
        b = b->ref;

    if (a->constant)
        throw("Cannot assign to const value");

    if (a->type == SL_VALUE_STRING)
        free(a->string);

    if (b->type == SL_VALUE_ARRAY || b->type == SL_VALUE_DICT || b->type == SL_VALUE_FN)
    {
        sl_value_free_members(a);
        a->ref = b;
        a->type = SL_VALUE_REF;
        b->refs++;
    }
    else
    {
        memcpy(a, b, sizeof(sl_value_t));
    }

    if (a->type == SL_VALUE_STRING)
        a->string = strdup(b->string);
}

sl_value_t *sl_value_unary(int op, sl_value_t *a)
{
    while (a->type == SL_VALUE_REF)
        a = a->ref;

    if (a->type != SL_VALUE_NUMBER)
        throw("Cannot perform unary operation on non numbers");

    switch (op)
    {
    case SL_TOKEN_PLUS:
        return sl_value_number(a->number);
    case SL_TOKEN_MINUS:
        return sl_value_number(-a->number);
    case SL_TOKEN_EXCLAM:
        return sl_value_number(!a->number);
    }
    throw("Unkown unary operation %d", op);
    return sl_value_null();
}

static sl_value_t *binary_number(int op, sl_value_t *a, sl_value_t *b)
{
    switch (op)
    {
    case SL_TOKEN_PLUS:
        return sl_value_number(a->number + b->number);
    case SL_TOKEN_MINUS:
        return sl_value_number(a->number - b->number);
    case SL_TOKEN_ASTERISK:
        return sl_value_number(a->number * b->number);
    case SL_TOKEN_SLASH:
        return sl_value_number(a->number / b->number);
    case SL_TOKEN_EQUAL:
        return sl_value_number(a->number == b->number);
    case SL_TOKEN_NOTEQUAL:
        return sl_value_number(a->number != b->number);
    case SL_TOKEN_LESSEQUAL:
        return sl_value_number(a->number <= b->number);
    case SL_TOKEN_MOREEQUAL:
        return sl_value_number(a->number >= b->number);
    case SL_TOKEN_LCHEVR:
        return sl_value_number(a->number < b->number);
    case SL_TOKEN_RCHEVR:
        return sl_value_number(a->number > b->number);
    }
    throw("Unkown binary operation %d", op);
    return sl_value_null();
}

static sl_value_t *binary_string(int op, sl_value_t *a, sl_value_t *b)
{
    switch (op)
    {
    case SL_TOKEN_PLUS:;
        int len1 = (int)strlen(a->string);
        int len2 = (int)strlen(b->string);
        char *str = (char *)malloc(sizeof(char) * (len1 + len2 + 1));
        str[0] = 0;
        strcat(str, a->string);
        strcat(str, b->string);
        return sl_value_string(str);
    case SL_TOKEN_EQUAL:
        return sl_value_number(strcmp(a->string, b->string) == 0);
    case SL_TOKEN_NOTEQUAL:
        return sl_value_number(strcmp(a->string, b->string) != 0);
    }
    throw("Unkown binary operation %d", op);
    return sl_value_null();
}

static sl_value_t *binary_array(int op, sl_value_t *a, sl_value_t *b)
{
    throw("Cannot perform any binary operation on type array");
    return sl_value_null();
}

static sl_value_t *binary_dict(int op, sl_value_t *a, sl_value_t *b)
{
    throw("Cannot perform any binary operation on type dict");
    return sl_value_null();
}

static char *sl_valuetypestr(int token)
{
    if (token < 0 || token > SL_VALUE_REF)
        return "WRONG VALUE!";
    char *names[] = {
        "SL_VALUE_NULL",
        "SL_VALUE_NUMBER",
        "SL_VALUE_STRING",
        "SL_VALUE_ARRAY",
        "SL_VALUE_DICT",
        "SL_VALUE_FN",
        "SL_VALUE_REF"};
    return names[token];
}

sl_value_t *sl_value_binary(int op, sl_value_t *a, sl_value_t *b)
{
    if (op == SL_TOKEN_ASSIGN)
    {
        sl_value_assign(a, b);
        return a;
    }

    while (a->type == SL_VALUE_REF)
        a = a->ref;
    while (b->type == SL_VALUE_REF)
        b = b->ref;

    if (a->type != b->type)
        throw("Type mismatch %s %s %s", sl_tokenstr(op), sl_valuetypestr(a->type), sl_valuetypestr(b->type));

    switch (a->type)
    {
    case SL_VALUE_NULL:
        throw("Cannot perform operation on null");
    case SL_VALUE_NUMBER:
        return binary_number(op, a, b);
    case SL_VALUE_STRING:
        return binary_string(op, a, b);
    case SL_VALUE_ARRAY:
        return binary_string(op, a, b);
    case SL_VALUE_DICT:
        return binary_dict(op, a, b);
    }
    throw("Unkown value type");
    return sl_value_null();
}

sl_value_t *sl_value_get(int i, sl_value_t *a)
{
    while (a->type == SL_VALUE_REF)
        a = a->ref;

    if (a->type == SL_VALUE_STRING)
    {
        int len = (int)strlen(a->string);
        if (i >= len)
            throw("Index out of range");

        char buf[2];
        buf[0] = a->string[i];
        buf[1] = 0;
        return sl_value_string(strdup(buf));
    }
    else if (a->type == SL_VALUE_ARRAY)
    {
        if (i >= sl_vector_size(a->array))
            throw("Index out of range");
        return a->array[i];
    }

    throw("Cannot index value of this type");
    return sl_value_null();
}

sl_value_t *sl_value_member(char *name, sl_value_t *a)
{
    if (a->type == SL_VALUE_REF)
        a = a->ref;

    if (a->type == SL_VALUE_DICT)
    {
        for (int i = 0; i < sl_vector_size(a->dict.names); i++)
        {
            if (strcmp(a->dict.names[i], name) == 0)
                return a->dict.values[i];
        }

        sl_vector_push(a->dict.names, strdup(name));
        sl_vector_push(a->dict.values, sl_value_null());

        return a->dict.values[sl_vector_size(a->dict.values) - 1];
    }

    throw("Cannot get member for this type");
    return sl_value_null();
}
