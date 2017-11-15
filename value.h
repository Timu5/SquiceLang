#ifndef _VALUE_H_
#define _VALUE_H_

enum {
	V_NUMBER,
	V_STRING,
	V_ARRAY,
	//V_OBJECT
};


typedef struct value_s {
	int type;
	union {
		double number;
		char* string;
		struct { int count; struct value_s** ptr; } array;
	};
} value_t;

value_t* value_number(double val);
value_t* value_string(char* val);
value_t* value_array(int count, value_t** arr);

value_t* value_unary(int op, value_t* a);
value_t* value_binary(int op, value_t* a, value_t* b);

value_t* value_get(int i, value_t* a);

void value_free(value_t* val);

#endif
