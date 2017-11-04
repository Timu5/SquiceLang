#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "value.h"

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

value_t* value_array(value_t* val)
{
	value_t* v = (value_t*)malloc(sizeof(value_t));
	v->type = V_ARRAY;
	v->array = val;
	return v;
}

value_t* value_unary(int op, value_t* a)
{
	if(a->type != V_NUMBER)
		return value_number(0);
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
	return value_number(0);
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
	return value_number(0);
}

static value_t* binary_array(int op, value_t* a, value_t* b)
{
	//TODO
	return value_number(0);
}

value_t* value_binary(int op, value_t* a, value_t* b)
{
	if(a->type != b->type)
		return value_number(-1);
	switch(a->type)
	{
	case V_NUMBER:
		return binary_number(op, a, b);
	case V_STRING:
		return binary_string(op, a, b);
	case V_ARRAY:
		return binary_string(op, a, b);
	}
}

void value_set(int i, value_t* a)
{
	//TODO
			
}

value_t* value_get(int i, value_t* a)
{
	return value_number(0);
}

