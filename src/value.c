#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "value.h"
#include "ex.h"
#include "gc.h"
#include "contex.h"

value_t* value_null()
{
    value_t* v = gc_alloc_value();
    v->type = V_NULL;
    v->constant = 0;
    v->refs = 0;
    v->markbit = 0;
    return v;
}

value_t* value_number(double val)
{
    value_t* v = gc_alloc_value();
    v->type = V_NUMBER;
    v->constant = 0;
    v->refs = 0;
    v->number = val;
    v->markbit = 0;
    return v;
}

value_t* value_string(char* val)
{
    value_t* v = gc_alloc_value();
    v->type = V_STRING;
    v->constant = 0;
    v->refs = 0;
    v->string = val;
    v->markbit = 0;
    return v;
}

value_t* value_array(vector(value_t*) arr)
{
    value_t* v = gc_alloc_value();
    v->type = V_ARRAY;
    v->constant = 0;
    v->refs = 0;
    v->array = arr;
    v->markbit = 0;
    return v;
}

value_t* value_dict(vector(char*) names, vector(value_t*) values)
{
    value_t* v = gc_alloc_value();
    v->type = V_DICT;
    v->constant = 0;
    v->refs = 0;
    v->dict.names = names;
    v->dict.values = values;
    v->markbit = 0;
    return v;
}

value_t* value_fn(fn_t* fn)
{
    value_t* v = gc_alloc_value();
    v->type = V_FN;
    v->constant = 0;
    v->refs = 0;
    v->fn = fn;
    v->markbit = 0;
    return v;
}

value_t* value_ref(value_t* val)
{
    value_t* v = gc_alloc_value();
    v->type = V_REF;
    v->refs = 0;
    v->ref = val;
    val->refs++;
    v->markbit = 0;
    return v;
}

void value_free(value_t* val)
{
    if(val->type == V_STRING)
    {
        free(val->string);
    }
    else if(val->type == V_ARRAY)
    {
        vector_free(val->string);
    }
    else if (val->type == V_DICT)
    {
        for (int i = 0; i < vector_size(val->dict.names); i++)
            free(val->dict.names[i]);
        vector_free(val->dict.names);
        vector_free(val->dict.values);
    }
    else if(val->type == V_FN)
    {
        free(val->fn);
    }

    free(val);
}

void value_assign(value_t* a, value_t* b)
{  
    value_t* olda = a;

    while(a->type == V_REF)
        a = a->ref;
    while(b->type == V_REF)
        b = b->ref;

    if(a->constant)
        throw("cannot assign to const value");
    
    if(a->type == V_STRING)
        free(a->string);

    if(b->type == V_ARRAY || b->type == V_DICT || b->type == V_FN)
    {
        a->ref = b;
        a->type = V_REF;
        b->refs++;
    }
    else
    {
        memcpy(a, b, sizeof(value_t));
    }

    if(a->type == V_STRING)
        a->string = strdup(b->string);
}

value_t* value_unary(int op, value_t* a)
{
    while(a->type == V_REF)
        a = a->ref;

    if(a->type != V_NUMBER)
        throw("Cannot perform unary operation on non numbers");
    
    switch(op)
    {
        case T_PLUS:
            return value_number(a->number);
        case T_MINUS:
            return value_number(-a->number);
        case T_EXCLAM:
            return value_number(!a->number);
    }
}

static value_t* binary_number(int op, value_t* a, value_t* b)
{
    switch(op)
    {
    case T_PLUS:
        return value_number(a->number + b->number);
    case T_MINUS:
        return value_number(a->number - b->number); 
    case T_ASTERISK:
        return value_number(a->number * b->number);
    case T_SLASH:
        return value_number(a->number / b->number);
    case T_EQUAL:
        return value_number(a->number == b->number);
    case T_NOTEQUAL:
        return value_number(a->number != b->number);
    case T_LESSEQUAL:
        return value_number(a->number <= b->number);
    case T_MOREEQUAL:
        return value_number(a->number >= b->number);
    case T_LCHEVR:
        return value_number(a->number < b->number);
    case T_RCHEVR:
        return value_number(a->number > b->number); 
    }
    throw("Unkown binary operation");
}

static value_t* binary_string(int op, value_t* a, value_t* b)
{
    switch(op)
    {
    case T_PLUS:;
        int len1 = strlen(a->string);
        int len2 = strlen(b->string);
        char* str = (char*)malloc(sizeof(char) * (len1 + len2 + 1));
        str[0] = 0;
        strcat(str, a->string);
        strcat(str, b->string);
        return value_string(str);
    case T_EQUAL:
        return value_number(strcmp(a->string, b->string) == 0);
    case T_NOTEQUAL:
        return value_number(strcmp(a->string, b->string) != 0);
    }
    throw("Unkown binary operation");
}

static value_t* binary_array(int op, value_t* a, value_t* b)
{
    throw("Cannot perform any binary operation on type array");
}

static value_t* binary_dict(int op, value_t* a, value_t* b)
{
    throw("Cannot perform any binary operation on type dict");
}

value_t* value_binary(int op, value_t* a, value_t* b)
{
    if(a->type == V_REF)
        a = a->ref;
    if(b->type == V_REF)
        b = b->ref;

    if(a->type != b->type)
        throw("Type mismatch");

    switch(a->type)
    {
    case V_NULL:
        throw("Cannot perfom operation on null");
    case V_NUMBER:
        return binary_number(op, a, b);
    case V_STRING:
        return binary_string(op, a, b);
    case V_ARRAY:
        return binary_string(op, a, b);
    case V_DICT:
        return binary_dict(op, a, b);
    }
    throw("Unkown value type");
}

value_t* value_get(int i, value_t* a)
{
    if(a->type == V_REF)
        a = a->ref;

    if(a->type == V_STRING)
    {
        int len = strlen(a->string);
        if(i >= len)
            throw("Index out of range");
     
        char buf[2];
        buf[0] = a->string[i];
        buf[1] = 0;
        return value_string(strdup(buf));
    }
    else if(a->type == V_ARRAY)
    {
        if(i >= vector_size(a->array))
            throw("Index aut of range");
        return a->array[i];
    }

    throw("Cannot index value of this type");
}

value_t * value_member(char * name, value_t * a)
{
    if (a->type == V_REF)
        a = a->ref;

    if (a->type == V_DICT)
    {
        for (int i = 0; i < vector_size(a->dict.names); i++)
        {
            if (strcmp(a->dict.names[i], name) == 0)
                return a->dict.values[i];
        }

        vector_push(a->dict.names, strdup(name));
        vector_push(a->dict.values, value_null());

        return a->dict.values[vector_size(a->dict.values)-1];
    }

    throw("Cannot get member for this type");
}

