#ifndef _VALUE_H_
#define _VALUE_H_

#include "vector.h"

enum
{
    V_NULL,
    V_NUMBER,
    V_STRING,
    V_ARRAY,
    V_DICT,
    V_FN,
    V_REF
};

struct fn_s;

typedef struct value_s
{
    int type;
    int constant;
    int refs;
    int markbit;
    union {
        double number;
        char *string;
        vector(struct value_s *) array;
        struct
        {
            vector(char *) names;
            vector(struct value_s *) values;
        } dict;
        struct fn_s *fn;
        struct value_s *ref;
    };
} value_t;

value_t *value_null();
value_t *value_number(double val);
value_t *value_string(char *val);
value_t *value_array(vector(value_t *) arr);
value_t *value_dict(vector(char *) names, vector(value_t *) values);
value_t *value_fn(struct fn_s *fn);
value_t *value_ref(value_t *val);

void value_assign(value_t *a, value_t *b);

value_t *value_unary(int op, value_t *a);
value_t *value_binary(int op, value_t *a, value_t *b);

value_t *value_get(int i, value_t *a);
value_t *value_member(char *name, value_t *a);

void value_free(value_t *val);

#endif
