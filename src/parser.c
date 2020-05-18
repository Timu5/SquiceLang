#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SquiceLang.h"

sl_parser_t *sl_parser_new(char *input)
{
    sl_parser_t *parser = (sl_parser_t*)malloc(sizeof(sl_parser_t));
    parser->lexer = sl_lexer_new(input);
    parser->lasttoken = 0;
    return parser;
}

void sl_parser_free(sl_parser_t *parser)
{
    sl_lexer_free(parser->lexer);
    free(parser);
}

static int nexttoken(sl_parser_t *parser)
{
    return parser->lasttoken = sl_gettoken(parser->lexer);
}

static void match(sl_parser_t *parser, int token)
{
    if (parser->lasttoken != token)
        throw("Unexpected token, expect %s got %s", sl_tokenstr(token), sl_tokenstr(parser->lasttoken));
}

sl_node_t *expr(sl_parser_t *parser, int min);

// primary :=  ident | number | string | '(' expr ')' | UNARY_OP primary |
//             array | dict |
//             primary '[' expr ']' | primary '(' expr ')'| primary '.' ident
sl_node_t *primary(sl_parser_t *parser)
{
    sl_node_t *prim = NULL;
    if (parser->lasttoken == SL_TOKEN_NUMBER)
    {
        prim = node_int(parser->lexer->number);
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_STRING)
    {
        prim = node_string(strdup(parser->lexer->buffer));
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_IDENT)
    {
        prim = node_ident(strdup(parser->lexer->buffer));
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_LPAREN) // '(' expr ')'
    {
        nexttoken(parser);
        prim = expr(parser, 0);
        match(parser, SL_TOKEN_RPAREN);
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_PLUS || parser->lasttoken == SL_TOKEN_MINUS || parser->lasttoken == SL_TOKEN_EXCLAM) // UNARY_OP primary
    {
        // UNARY_OP primary
        int op = parser->lasttoken;
        nexttoken(parser);
        return node_unary(op, primary(parser));
    }
    else if (parser->lasttoken == SL_TOKEN_LBRACK)
    {
        // new array
        nexttoken(parser);
        sl_vector(sl_node_t *) elements = NULL;
        while (parser->lasttoken != SL_TOKEN_RBRACK)
        {
            sl_node_t *e = expr(parser, 0);
            sl_vector_push(elements, e);
            if (parser->lasttoken != SL_TOKEN_COMMA)
                break;
            nexttoken(parser);
        }
        match(parser, SL_TOKEN_RBRACK);
        nexttoken(parser);

        // syntax sugar, convert [1,2,3] to list(1,2,3)
        prim = node_call(node_ident(strdup("list")), node_block(elements));
    }
    else if (parser->lasttoken == SL_TOKEN_LBRACE)
    {
        // new dictonary
        nexttoken(parser);
        sl_vector(sl_node_t *) keys = NULL;
        sl_vector(sl_node_t *) values = NULL;
        while (parser->lasttoken != SL_TOKEN_RBRACE)
        {
            if (!(parser->lasttoken == SL_TOKEN_STRING || parser->lasttoken == SL_TOKEN_IDENT))
                throw("Unexpexted token, expect SL_TOKEN_STRING or SL_TOKEN_IDENT got %s", parser->lasttoken);
            sl_vector_push(keys, node_string(strdup(parser->lexer->buffer)));

            nexttoken(parser);
            match(parser, SL_TOKEN_COLON);
            nexttoken(parser);

            sl_node_t *e = expr(parser, 0);
            sl_vector_push(values, e);

            if (parser->lasttoken != SL_TOKEN_COMMA)
                break;
            nexttoken(parser);
        }
        match(parser, SL_TOKEN_RBRACE);
        nexttoken(parser);

        // syntax sugar, convert {a:1, b:2+2} to dict(list('a', 'b'), list(1, 2+2))
        sl_vector(sl_node_t *) args = NULL;
        sl_vector_push(args, node_call(node_ident(strdup("list")), node_block(keys)));
        sl_vector_push(args, node_call(node_ident(strdup("list")), node_block(values)));
        prim = node_call(node_ident(strdup("dict")), node_block(args));
    }
    else
    {
        throw("Unexpected token in primary!");
    }

    while (parser->lasttoken == SL_TOKEN_DOT || parser->lasttoken == SL_TOKEN_LBRACK || parser->lasttoken == SL_TOKEN_LPAREN)
    {
        if (parser->lasttoken == SL_TOKEN_LPAREN) // primary '(' expr ')'
        {
            nexttoken(parser);
            sl_vector(sl_node_t *) args = NULL;
            while (parser->lasttoken != SL_TOKEN_RPAREN)
            {
                sl_node_t *e = expr(parser, 0);
                sl_vector_push(args, e);

                if (parser->lasttoken != SL_TOKEN_COMMA)
                    break;
                nexttoken(parser);
            }
            match(parser, SL_TOKEN_RPAREN);
            nexttoken(parser);
            prim = node_call(prim, node_block(args));
        }
        else if (parser->lasttoken == SL_TOKEN_LBRACK) // primary '[' expr ']'
        {
            nexttoken(parser);
            sl_node_t *e = expr(parser, 0);
            match(parser, SL_TOKEN_RBRACK);
            nexttoken(parser);
            prim = node_index(prim, e);
        }
        else if (parser->lasttoken == SL_TOKEN_DOT) // primary '.' ident
        {
            nexttoken(parser);
            match(parser, SL_TOKEN_IDENT);
            char *name = strdup(parser->lexer->buffer);
            nexttoken(parser);
            prim = node_member(prim, name);
        }
    }
    return prim;
}

// expr := primary |  expr OP expr
sl_node_t *expr(sl_parser_t *parser, int min)
{
    int pre[] = {
        4, // +
        4, // -
        5, // /
        5, // *
        1, // =
        2, // ==
        2, // !=
        3, // <=
        3, // >=
        3, // <
        3, // >
    };
    sl_node_t *lhs = primary(parser);
    while (1)
    {
        if (parser->lasttoken < SL_TOKEN_PLUS || parser->lasttoken > SL_TOKEN_RCHEVR || pre[parser->lasttoken - SL_TOKEN_PLUS] < min)
            break;

        int op = parser->lasttoken;
        int prec = pre[parser->lasttoken - SL_TOKEN_PLUS];
        int assoc = 0; // 0 left, 1 right
        int nextmin = assoc ? prec : prec + 1;
        nexttoken(parser);
        sl_node_t *rhs = expr(parser, nextmin);
        lhs = node_binary(op, lhs, rhs);
    }
    return lhs;
}

// block := '{' statement '}'
// let := 'let' ident '=' expr ';'
// if := 'if' '(' expr ')' statement ['else' statement]
// while := 'while' '(' expr ')' statement
// return := 'return' ';' | 'return' expr ';'
// break := 'break' ';'
// statement := block | let | if | while | funca | return | break | expr ';'
sl_node_t *statment(sl_parser_t *parser)
{
    switch (parser->lasttoken)
    {
    case SL_TOKEN_LBRACE:
        nexttoken(parser);

        sl_vector(sl_node_t *) list = NULL;
        while (parser->lasttoken != SL_TOKEN_RBRACE)
        {
            sl_node_t *node = statment(parser);
            sl_vector_push(list, node);
        }
        match(parser, SL_TOKEN_RBRACE);

        nexttoken(parser);
        return node_block(list);
    case SL_TOKEN_LET:
        nexttoken(parser);
        match(parser, SL_TOKEN_IDENT);
        char *name = strdup(parser->lexer->buffer);

        nexttoken(parser);
        match(parser, SL_TOKEN_ASSIGN);
        nexttoken(parser);

        sl_node_t *exp = expr(parser, 0);
        match(parser, SL_TOKEN_SEMICOLON);

        nexttoken(parser);
        return node_decl(node_ident(name), exp);
    case SL_TOKEN_IF:
        nexttoken(parser);
        match(parser, SL_TOKEN_LPAREN);

        nexttoken(parser);
        sl_node_t *arg = expr(parser, 0);
        match(parser, SL_TOKEN_RPAREN);

        nexttoken(parser);
        sl_node_t *body = statment(parser);
        sl_node_t *elsebody = NULL;

        if (parser->lasttoken == SL_TOKEN_ELSE)
        {
            nexttoken(parser);
            elsebody = statment(parser);
        }

        return node_cond(arg, body, elsebody);
    case SL_TOKEN_WHILE:
        nexttoken(parser);
        match(parser, SL_TOKEN_LPAREN);

        nexttoken(parser);
        sl_node_t *arg2 = expr(parser, 0);
        match(parser, SL_TOKEN_RPAREN);

        nexttoken(parser);
        sl_node_t *body2 = statment(parser);

        return node_loop(arg2, body2);
    case SL_TOKEN__FN:
        nexttoken(parser);
        match(parser, SL_TOKEN_IDENT);

        char *fnname = strdup(parser->lexer->buffer);

        nexttoken(parser);
        match(parser, SL_TOKEN_LPAREN);

        sl_vector(char *) args = NULL;

        while (nexttoken(parser) != SL_TOKEN_RPAREN)
        {
            match(parser, SL_TOKEN_IDENT);
            sl_vector_push(args, strdup(parser->lexer->buffer));
            if (nexttoken(parser) != SL_TOKEN_COMMA)
                break;
        }

        match(parser, SL_TOKEN_RPAREN);
        nexttoken(parser);

        sl_node_t *fnbody = statment(parser);

        return node_func(fnname, args, fnbody);
    case SL_TOKEN_RETURN:
        nexttoken(parser);

        sl_node_t *retnode = NULL;
        if (parser->lasttoken != SL_TOKEN_SEMICOLON)
            retnode = expr(parser, 0);

        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);

        return node_return(retnode);
    case SL_TOKEN_BREAK:
        nexttoken(parser);
        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);

        return node_break();
    default:;
        sl_node_t *e = expr(parser, 0);
        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);
        return e;
    }
}

sl_node_t *sl_parse(sl_parser_t *parser)
{
    nexttoken(parser);
    sl_vector(sl_node_t *) funcs = NULL;
    sl_vector(sl_node_t *) stmts = NULL;
    while (parser->lasttoken != SL_TOKEN_EOF)
    {
        sl_node_t *n = statment(parser);

        if (n->type == SL_NODETYPE_FUNC)
            sl_vector_push(funcs, n);
        else
            sl_vector_push(stmts, n);
    }
    return node_root(node_block(funcs), node_block(stmts));
}
