#ifndef _VALUE_H_
#define _VALUE_H_

enum {
    V_NULL,
    V_NUMBER,
    V_STRING,
    V_ARRAY,
    //V_OBJECT,
    V_REF
};


typedef struct value_s {
    int type;
    int constant;
    int refs;
    union {
        double number;
        char* string;
        struct { int count; struct value_s** ptr; } array;
        struct value_s* ref;
    };
} value_t;

value_t* value_null();
value_t* value_number(double val);
value_t* value_string(char* val);
value_t* value_array(int count, value_t** arr);
value_t* value_ref(value_t* val);

void value_assign(value_t* a, value_t* b);

value_t* value_unary(int op, value_t* a);
value_t* value_binary(int op, value_t* a, value_t* b);

value_t* value_get(int i, value_t* a);

void value_free(value_t* val);

#endif
