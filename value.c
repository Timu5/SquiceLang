#include <stdlib.h>

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
	return value_number(0);
}

static value_t* binary_number(int op, value_t* a, value_t* b)
{
	switch(op)
	{
	case '+':
		return value_number(a->number + b->number);
	case '-':
		return value_number(a->number - b->number);	
	case '*':
		return value_number(a->number * b->number);
	case '/':
		return value_number(a->number / b->number);
	//TODO
	}
	// exception unkown operator
}

static value_t* binary_string(int op, value_t* a, value_t* b)
{
	//TODO
}

static value_t* binary_array(int op, value_t* a, value_t* b)
{
	//TODO
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

