#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "value.h"
#include "ex.h"

value_t* value_null()
{
	value_t* v = (value_t*)malloc(sizeof(value_t));
	v->type = V_NULL;
	return v;
}

value_t* value_number(double val)
{
	value_t* v = (value_t*)malloc(sizeof(value_t));
	v->type = V_NUMBER;
	v->number = val;
	return v;
}

value_t* value_string(char* val)
{
	value_t* v = (value_t*)malloc(sizeof(value_t));
	v->type = V_STRING;
	v->string = val;
	return v;
}

value_t* value_array(int count, value_t** arr)
{
	value_t* v = (value_t*)malloc(sizeof(value_t));
	v->type = V_ARRAY;
	v->array.ptr = arr;
	v->array.count = count;
	return v;
}

void value_free(value_t* val)
{
	if(val->type == V_STRING)
		free(val->string);
	else if(val->type == V_ARRAY)
		for(int n = 0; n < val->array.count; n++)
			value_free(val->array.ptr[n]);
	free(val);
}

value_t* value_unary(int op, value_t* a)
{
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

value_t* value_binary(int op, value_t* a, value_t* b)
{
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
	}
	throw("Unkown value type");
}

value_t* value_get(int i, value_t* a)
{
	if(a->type == V_STRING)
	{
		int len = strlen(a->string);
		if(i >= len)
			throw("Index out of range");
		return value_number(a->string[i]);
	}
	else if(a->type == V_ARRAY)
	{
		if(i >= a->array.count)
			throw("Index aut of range");
		return a->array.ptr[i];
	}

	throw("Cannot index value of this type");
}

