#ifndef _PARSER_H_
#define _PARSER_H_

#include "lexer.h"
#include "ast.h"

struct sl_parser_s
{
    sl_lexer_t *lexer;
    int lasttoken;
};

typedef struct sl_parser_s sl_parser_t;

sl_parser_t *sl_parser_new(char *input);
void sl_parser_free(sl_parser_t *parser);

sl_node_t *sl_parse(sl_parser_t *parser);

#endif
