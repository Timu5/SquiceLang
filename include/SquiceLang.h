#ifndef _SQUICELANG_
#define _SQUICELANG_

#include <string.h>
#include <stdlib.h>

// Vector based on https://github.com/graphitemaster/cvec
struct sl_vector_s
{
    size_t allocated;
    size_t used;
};

typedef struct sl_vector_s sl_vector_t;

#define sl_vector(x) x *

/* Attempts to grow [VECTOR] by [MORE]*/
#define sl_vector_try_grow(VECTOR, MORE) \
    (((!(VECTOR) || sl_vector_meta(VECTOR)->used + (MORE) >= sl_vector_meta(VECTOR)->allocated)) ? (void)vec_grow(((void **)&(VECTOR)), (MORE), sizeof(*(VECTOR))) : (void)0)

/* Get the metadata block for [VECTOR] */
#define sl_vector_meta(VECTOR) \
    ((sl_vector_t *)(((unsigned char *)(VECTOR)) - sizeof(sl_vector_t)))

/* Deletes [VECTOR] and sets it to NULL */
#define sl_vector_free(VECTOR) \
    ((void)((VECTOR) ? (vec_delete((void *)(VECTOR)), (VECTOR) = NULL) : 0))

/* Pushes back [VALUE] into [VECTOR] */
#define sl_vector_push(VECTOR, VALUE) \
    (sl_vector_try_grow((VECTOR), 1), (VECTOR)[sl_vector_meta(VECTOR)->used] = (VALUE), sl_vector_meta(VECTOR)->used++)

/* Get the size of [VECTOR] */
#define sl_vector_size(VECTOR) \
    ((VECTOR) ? sl_vector_meta(VECTOR)->used : 0)

/* Get the capacity of [VECTOR] */
#define sl_vector_capacity(VECTOR) \
    ((VECTOR) ? sl_vector_meta(VECTOR)->allocated : 0)

/* Resize [VECTOR] to accomodate [SIZE] more elements */
#define sl_vector_resize(VECTOR, SIZE)                                          \
    (sl_vector_try_grow((VECTOR), (SIZE)), sl_vector_meta(VECTOR)->used += (SIZE), \
     &(VECTOR)[sl_vector_meta(VECTOR)->used - (SIZE)])

/* Get the last element in [VECTOR] */
#define sl_vector_last(VECTOR) \
    ((VECTOR)[sl_vector_meta(VECTOR)->used - 1])

#define sl_vector_lastn(VECTOR, N) \
    (&(VECTOR)[sl_vector_meta(VECTOR)->used - N])

/* Pop an element off the back of [VECTOR] */
#define sl_vector_pop(VECTOR) \
    ((VECTOR)[--sl_vector_meta(VECTOR)->used])
//((void)(sl_vector_meta(VECTOR)->used -= 1))

/* Shrink the size of [VECTOR] down to [SIZE] */
#define sl_vector_shrinkto(VECTOR, SIZE) \
    ((void)(sl_vector_meta(VECTOR)->used = (SIZE)))

/* Shrink [VECTOR] down by [AMOUNT] */
#define sl_vector_shrinkby(VECTOR, AMOUNT) \
    ((void)(sl_vector_meta(VECTOR)->used -= (AMOUNT)))

/* Append to [VECTOR], [COUNT] elements from [POINTER] */
#define sl_vector_append(VECTOR, COUNT, POINTER) \
    ((void)(memcpy(sl_vector_resize((VECTOR), (COUNT)), (POINTER), (COUNT) * sizeof(*(POINTER)))))

/* Remove from [VECTOR], [COUNT] elements starting from [INDEX] */
#define sl_vector_remove(VECTOR, INDEX, COUNT)                                               \
    ((void)(memmove((VECTOR) + (INDEX), (VECTOR) + (INDEX) + (COUNT),                     \
                    sizeof(*(VECTOR)) * (sl_vector_meta(VECTOR)->used - (INDEX) - (COUNT))), \
            sl_vector_meta(VECTOR)->used -= (COUNT)))

#define vec_delete(VECTOR) free(sl_vector_meta(VECTOR))

void vec_grow(void **vector, size_t more, size_t type_size);

enum SL_NODETYPE
{
    SL_NODETYPE_ROOT,
    SL_NODETYPE_IDENT,
    SL_NODETYPE_UNARY,
    SL_NODETYPE_BINARY,
    SL_NODETYPE_NUMBER,
    SL_NODETYPE_STRING,
    SL_NODETYPE_CALL,
    SL_NODETYPE_FUNC,
    SL_NODETYPE_RETURN,
    SL_NODETYPE_COND,
    SL_NODETYPE_LOOP,
    SL_NODETYPE_BREAK,
    SL_NODETYPE_DECL,
    SL_NODETYPE_INDEX,
    SL_NODETYPE_BLOCK,
    SL_NODETYPE_MEMBER,
    SL_NODETYPE_IMPORT,
    SL_NODETYPE_CLASS
};

struct sl_binary_s;

struct sl_node_s
{
    enum SL_NODETYPE type;
    void (*codegen)(struct sl_node_s *this, struct sl_binary_s *binary);
    void (*free)(struct sl_node_s *this);
    union {
        struct
        {
            struct sl_node_s *funcs;
            struct sl_node_s *stmts;
        } root;
        char *ident;
        struct
        {
            int op;
            struct sl_node_s *val;
        } unary;
        struct
        {
            int op;
            struct sl_node_s *a;
            struct sl_node_s *b;
        } binary;
        double number;
        char *string;
        struct
        {
            struct sl_node_s *func;
            struct sl_node_s *args;
        } call;
        struct
        {
            char *name;
            sl_vector(char *) args;
            struct sl_node_s *body;
        } func;
        struct sl_node_s *ret;
        struct
        {
            struct sl_node_s *arg;
            struct sl_node_s *body;
            struct sl_node_s *elsebody;
        } cond;
        struct
        {
            struct sl_node_s *arg;
            struct sl_node_s *body;
        } loop;
        struct
        {
            struct sl_node_s *name;
            struct sl_node_s *value;
        } decl;
        struct
        {
            struct sl_node_s *var;
            struct sl_node_s *expr;
        } index;
        sl_vector(struct sl_node_s *) block;
        struct
        {
            struct sl_node_s *parent;
            char *name;
        } member;
        char *import;
        struct
        {
            char* name;
            sl_vector(struct sl_node_s *) methods;
        } class;
    };
};

typedef struct sl_node_s sl_node_t;

struct sl_value_s;

struct sl_var_s
{
    char *name;
    struct sl_value_s *val;
};

typedef struct sl_var_s sl_var_t;

struct sl_ctx_s;

struct sl_fn_s
{
    int address;
    int argc; 
    struct sl_binary_s *binary; /* binary with funcion code */
    struct sl_ctx_s *ctx;
    void (*native)(struct sl_ctx_s *); // call if not NULL
};

typedef struct sl_fn_s sl_fn_t;

struct sl_ctx_s
{
    struct sl_ctx_s *parent;
    struct sl_ctx_s *child;
    sl_vector(struct sl_var_s *) vars;
    sl_vector(struct sl_value_s *) stack;
};

typedef struct sl_ctx_s sl_ctx_t;

#define sl_node_free(node) ((node)->free((node)))

sl_node_t *node_root(sl_node_t *funcs, sl_node_t *stmts);
sl_node_t *node_ident(char *name);
sl_node_t *node_unary(int op, sl_node_t *val);
sl_node_t *node_binary(int op, sl_node_t *a, sl_node_t *b);
sl_node_t *node_number(double number);
sl_node_t *node_string(char *string);
sl_node_t *node_call(sl_node_t *func, sl_node_t *args);
sl_node_t *node_func(char *name, sl_vector(char *) args, sl_node_t *body);
sl_node_t *node_return(sl_node_t *expr);
sl_node_t *node_cond(sl_node_t *arg, sl_node_t *body, sl_node_t *elsebody);
sl_node_t *node_loop(sl_node_t *arg, sl_node_t *body);
sl_node_t *node_break();
sl_node_t *node_decl(sl_node_t *name, sl_node_t *value);
sl_node_t *node_index(sl_node_t *var, sl_node_t *expr);
sl_node_t *node_block(sl_vector(sl_node_t *) list);
sl_node_t *node_member(sl_node_t *parent, char *name);
sl_node_t *node_import(char *name);
sl_node_t *node_class(char *name, sl_vector(sl_node_t *) list);

void sl_node_print(sl_node_t *node, int ind);

void sl_builtin_install(sl_ctx_t *ctx);

enum SL_OPCODE
{
    SL_OPCODE_NOP,
    SL_OPCODE_PUSHN, // push number
    SL_OPCODE_PUSHI, // push integer
    SL_OPCODE_PUSHS, // push string
    SL_OPCODE_PUSHV, // push value
    SL_OPCODE_POP,   // remove element from stack
    SL_OPCODE_STORE,
    SL_OPCODE_STOREFN, // store function
    SL_OPCODE_UNARY,
    SL_OPCODE_BINARY,
    SL_OPCODE_CALL,
    SL_OPCODE_CALLM, // call member
    SL_OPCODE_RET,
    SL_OPCODE_RETN, // return null
    SL_OPCODE_JMP,
    SL_OPCODE_BRZ, // brach if zero
    SL_OPCODE_INDEX,
    SL_OPCODE_MEMBER,
    SL_OPCODE_MEMBERD, // member with parent duplicate
    SL_OPCODE_IMPORT
};

// Structure to store bytecode in binary format
struct sl_binary_s
{
    sl_vector(int) adresses;
    sl_vector(char *) symbols;
    sl_vector(int) fadresses;
    sl_vector(char *) fsymbols;
    char *block;
    int size;
    int index; // free label index
    int loop;  // hold current loop index, needed by break
};

typedef struct sl_binary_s sl_binary_t;

sl_binary_t *sl_binary_new();
void sl_binary_free(sl_binary_t *bin);
void sl_binary_save(sl_binary_t *bin, char *filename);

int sl_bytecode_emit(sl_binary_t *bin, int opcode);
int sl_bytecode_emitstr(sl_binary_t *bin, int opcode, char *string);
int sl_bytecode_emitint(sl_binary_t *bin, int opcode, int number);
int sl_bytecode_emitdouble(sl_binary_t *bin, int opcode, double number);

int sl_bytecode_addlabel(sl_binary_t *bin, char *name, int adress);
int sl_bytecode_addtofill(sl_binary_t *bin, char *name, int adress);
int sl_bytecode_fill(sl_binary_t *bin);

void sl_codegen_root(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_ident(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_unary(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_binary(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_double(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_string(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_call(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_func(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_return(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_cond(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_loop(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_break(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_decl(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_index(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_block(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_member(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_import(sl_node_t *node, sl_binary_t *binary);
void sl_codegen_class(sl_node_t *node, sl_binary_t *binary);

sl_ctx_t *sl_ctx_new(sl_ctx_t *parent);
void sl_ctx_free(sl_ctx_t *ctx);
struct sl_value_s *sl_ctx_getvar(sl_ctx_t *ctx, char *name);
void sl_ctx_addvar(sl_ctx_t *ctx, char *name, struct sl_value_s *val);
sl_fn_t *sl_ctx_getfn(sl_ctx_t *ctx, char *name);
void sl_ctx_addfn(sl_ctx_t *ctx, sl_binary_t *binary, char *name, int argc, int address, void (*fn)(sl_ctx_t *));

#define SL_ALLOC(type) (type*)sl_safe_alloc(sizeof(type))

void *sl_safe_alloc(int size);
struct sl_value_s *sl_gc_alloc_value();
void sl_gc_collect(sl_ctx_t *ctx);
void sl_gc_freeall();

enum SL_TOKEN
{
    SL_TOKEN_IDENT,
    SL_TOKEN_NUMBER,
    SL_TOKEN_STRING,

    //keyword
    SL_TOKEN_FN,
    SL_TOKEN_RETURN,
    SL_TOKEN_LET,
    SL_TOKEN_IF,
    SL_TOKEN_ELSE,
    SL_TOKEN_WHILE,
    SL_TOKEN_BREAK,
    SL_TOKEN_IMPORT,
    SL_TOKEN_CLASS,

    SL_TOKEN_COLON,     // :
    SL_TOKEN_SEMICOLON, // ;
    SL_TOKEN_COMMA,     // ,
    SL_TOKEN_DOT,       // .

    SL_TOKEN_PLUS,     // +
    SL_TOKEN_MINUS,    // -
    SL_TOKEN_SLASH,    // /
    SL_TOKEN_ASTERISK, // *

    SL_TOKEN_ASSIGN,    // =
    SL_TOKEN_EQUAL,     // ==
    SL_TOKEN_NOTEQUAL,  // !=
    SL_TOKEN_LESSEQUAL, // <=
    SL_TOKEN_MOREEQUAL, // >=
    SL_TOKEN_LCHEVR,    // <
    SL_TOKEN_RCHEVR,    // >

    SL_TOKEN_LPAREN, // (
    SL_TOKEN_RPAREN, // )
    SL_TOKEN_LBRACE,        // {
    SL_TOKEN_RBRACE,        // }
    SL_TOKEN_LBRACK,        // [
    SL_TOKEN_RBRACK,        // ]
    SL_TOKEN_EXCLAM,        // !

    SL_TOKEN_EOF,
    SL_TOKEN_UNKOWN
};

struct sl_lexer_s
{
    char *input;
    int index;
    char *buffer;
    double number;
    int line;
    int col;
    int lastchar;
};

typedef struct sl_lexer_s sl_lexer_t;

sl_lexer_t *sl_lexer_new(char *input);
void sl_lexer_free(sl_lexer_t *lexer);

int sl_gettoken(sl_lexer_t *lexer);
char *sl_tokenstr(int token);

struct sl_parser_s
{
    sl_lexer_t *lexer;
    int lasttoken;
};

typedef struct sl_parser_s sl_parser_t;

sl_parser_t *sl_parser_new(char *input);
void sl_parser_free(sl_parser_t *parser);

sl_node_t *sl_parse(sl_parser_t *parser);

#include <setjmp.h>

extern jmp_buf __ex_buf__;
extern char ex_msg[256];

#define try if (!setjmp(__ex_buf__))
#define catch else

void throw(char *msg, ...);

char *sl_mprintf(char *fmt, ...);

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#pragma warning( disable : 4100)
#pragma warning( disable : 4996)
#endif

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

struct sl_fn_s;

struct sl_value_s
{
    enum SL_VALUE type;
    int constant;
    int refs;
    int markbit;
    union {
        double number;
        char *string;
        sl_vector(struct sl_value_s *) array;
        struct
        {
            sl_vector(char *) names;
            sl_vector(struct sl_value_s *) values;
        } dict;
        struct sl_fn_s *fn;
        struct sl_value_s *ref;
    };
};

typedef struct sl_value_s sl_value_t;

sl_value_t *sl_value_null();
sl_value_t *sl_value_number(double val);
sl_value_t *sl_value_string(char *val);
sl_value_t *sl_value_array(sl_vector(sl_value_t *) arr);
sl_value_t *sl_value_dict(sl_vector(char *) names, sl_vector(sl_value_t *) values);
sl_value_t *sl_value_fn(struct sl_fn_s *fn);
sl_value_t *sl_value_ref(sl_value_t *val);

void sl_value_assign(sl_value_t *a, sl_value_t *b);

sl_value_t *sl_value_unary(int op, sl_value_t *a);
sl_value_t *sl_value_binary(int op, sl_value_t *a, sl_value_t *b);

sl_value_t *sl_value_get(int i, sl_value_t *a);
sl_value_t *sl_value_member(char *name, sl_value_t *a);

void sl_value_free(sl_value_t *val);

void sl_exec(sl_ctx_t *global, sl_ctx_t *context, sl_binary_t *binary, int ip, sl_binary_t *(*load_module)(char *name));

sl_binary_t *sl_compile_str(char *code);
sl_binary_t *sl_compile_file(char *filename);

void sl_eval_str(sl_ctx_t *ctx, char *code, sl_binary_t *(*load_module)(char *name));
void sl_eval_file(sl_ctx_t *ctx, char *filename, sl_binary_t *(*load_module)(char *name));
void sl_dis_str(sl_ctx_t *ctx, char *code, sl_binary_t *(*load_module)(char *name));

#endif