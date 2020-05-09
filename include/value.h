#ifndef _VALUE_H_
#define _VALUE_H_

#include "vector.h"

enum SL_VALUE
{
    SL_VALUE_NULL,
    SL_VALUE_NUMBER,
    SL_VALUE_STRING,
    SL_VALUE_ARRAY,
    SL_VALUE_DICT,
    SL_VALUE_FN,
    SL_VALUE_REF
};

struct fn_s;

struct sl_value_s
{
    enum SL_VALUE type;
    int constant;
    int refs;
    int markbit;
    union {
        double number;
        char *string;
        vector(struct sl_value_s *) array;
        struct
        {
            vector(char *) names;
            vector(struct sl_value_s *) values;
        } dict;
        struct fn_s *fn;
        struct sl_value_s *ref;
    };
};

typedef struct sl_value_s sl_value_t;

sl_value_t *value_null();
sl_value_t *value_number(double val);
sl_value_t *value_string(char *val);
sl_value_t *value_array(vector(sl_value_t *) arr);
sl_value_t *value_dict(vector(char *) names, vector(sl_value_t *) values);
sl_value_t *value_fn(struct fn_s *fn);
sl_value_t *value_ref(sl_value_t *val);

void value_assign(sl_value_t *a, sl_value_t *b);

sl_value_t *value_unary(int op, sl_value_t *a);
sl_value_t *value_binary(int op, sl_value_t *a, sl_value_t *b);

sl_value_t *value_get(int i, sl_value_t *a);
sl_value_t *value_member(char *name, sl_value_t *a);

void value_free(sl_value_t *val);

#endif
