#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "lexer.h"
#include "ast.h"
#include "vector.h"
#include "utils.h"

extern char buffer[255];
extern int number;

int lasttoken;

int nexttoken()
{
    return lasttoken = sl_gettoken();
}

void match(int token)
{
    if (lasttoken != token)
        throw("Unexpexted token, expect %s got %s", sl_tokenstr(token), sl_tokenstr(lasttoken));
}

sl_node_t *expr(int min);

// primary :=  ident | number | string | '(' expr ')' | UNARY_OP primary |
//             array | dict |
//             primary '[' expr ']' | primary '(' expr ')'| primary '.' ident
sl_node_t *primary()
{
    sl_node_t *prim = NULL;
    if (lasttoken == SL_TOKEN_NUMBER)
    {
        prim = node_int(number);
        nexttoken();
    }
    else if (lasttoken == SL_TOKEN_STRING)
    {
        prim = node_string(strdup(buffer));
        nexttoken();
    }
    else if (lasttoken == SL_TOKEN_IDENT)
    {
        prim = node_ident(strdup(buffer));
        nexttoken();
    }
    else if (lasttoken == SL_TOKEN_LPAREN) // '(' expr ')'
    {
        nexttoken();
        prim = expr(0);
        match(SL_TOKEN_RPAREN);
        nexttoken();
    }
    else if (lasttoken == SL_TOKEN_PLUS || lasttoken == SL_TOKEN_MINUS || lasttoken == SL_TOKEN_EXCLAM) // UNARY_OP primary
    {
        // UNARY_OP primary
        int op = lasttoken;
        nexttoken();
        return node_unary(op, primary());
    }
    else if (lasttoken == SL_TOKEN_LBRACK)
    {
        // new array
        nexttoken();
        vector(sl_node_t *) elements = NULL;
        while (lasttoken != SL_TOKEN_RBRACK)
        {
            sl_node_t *e = expr(0);
            vector_push(elements, e);
            if (lasttoken != SL_TOKEN_COMMA)
                break;
            nexttoken();
        }
        match(SL_TOKEN_RBRACK);
        nexttoken();

        // syntax sugar, convert [1,2,3] to list(1,2,3)
        prim = node_call(node_ident(strdup("list")), node_block(elements));
    }
    else if (lasttoken == SL_TOKEN_LBRACE)
    {
        // new dictonary
        nexttoken();
        vector(sl_node_t *) keys = NULL;
        vector(sl_node_t *) values = NULL;
        while (lasttoken != SL_TOKEN_RBRACE)
        {
            if (!(lasttoken == SL_TOKEN_STRING || lasttoken == SL_TOKEN_IDENT))
                throw("Unexpexted token, expect SL_TOKEN_STRING or SL_TOKEN_IDENT got %s", lasttoken);
            vector_push(keys, node_string(strdup(buffer)));

            nexttoken();
            match(SL_TOKEN_COLON);
            nexttoken();

            sl_node_t *e = expr(0);
            vector_push(values, e);

            if (lasttoken != SL_TOKEN_COMMA)
                break;
            nexttoken();
        }
        match(SL_TOKEN_RBRACE);
        nexttoken();

        // syntax sugar, convert {a:1, b:2+2} to dict(list('a', 'b'), list(1, 2+2))
        vector(sl_node_t *) args = NULL;
        vector_push(args, node_call(node_ident(strdup("list")), node_block(keys)));
        vector_push(args, node_call(node_ident(strdup("list")), node_block(values)));
        prim = node_call(node_ident(strdup("dict")), node_block(args));
    }
    else
    {
        throw("Unexpexted token in primary!");
    }

    while (lasttoken == SL_TOKEN_DOT || lasttoken == SL_TOKEN_LBRACK || lasttoken == SL_TOKEN_LPAREN)
    {
        if (lasttoken == SL_TOKEN_LPAREN) // primary '(' expr ')'
        {
            nexttoken();
            vector(sl_node_t *) args = NULL;
            while (lasttoken != SL_TOKEN_RPAREN)
            {
                sl_node_t *e = expr(0);
                vector_push(args, e);

                if (lasttoken != SL_TOKEN_COMMA)
                    break;
                nexttoken();
            }
            match(SL_TOKEN_RPAREN);
            nexttoken();
            prim = node_call(prim, node_block(args));
        }
        else if (lasttoken == SL_TOKEN_LBRACK) // primary '[' expr ']'
        {
            nexttoken();
            sl_node_t *e = expr(0);
            match(SL_TOKEN_RBRACK);
            nexttoken();
            prim = node_index(prim, e);
        }
        else if (lasttoken == SL_TOKEN_DOT) // primary '.' ident
        {
            nexttoken();
            match(SL_TOKEN_IDENT);
            char *name = strdup(buffer);
            nexttoken();
            prim = node_member(prim, name);
        }
    }
    return prim;
}

// expr := primary |  expr OP expr
sl_node_t *expr(int min)
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
    sl_node_t *lhs = primary();
    while (1)
    {
        if (lasttoken < SL_TOKEN_PLUS || lasttoken > SL_TOKEN_RCHEVR || pre[lasttoken - SL_TOKEN_PLUS] < min)
            break;

        int op = lasttoken;
        int prec = pre[lasttoken - SL_TOKEN_PLUS];
        int assoc = 0; // 0 left, 1 right
        int nextmin = assoc ? prec : prec + 1;
        nexttoken();
        sl_node_t *rhs = expr(nextmin);
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
sl_node_t *statment()
{
    switch (lasttoken)
    {
    case SL_TOKEN_LBRACE:
        nexttoken();

        vector(sl_node_t *) list = NULL;
        while (lasttoken != SL_TOKEN_RBRACE)
        {
            sl_node_t *node = statment();
            vector_push(list, node);
        }
        match(SL_TOKEN_RBRACE);

        nexttoken();
        return node_block(list);
    case SL_TOKEN_LET:
        nexttoken();
        match(SL_TOKEN_IDENT);
        char *name = strdup(buffer);

        nexttoken();
        match(SL_TOKEN_ASSIGN);
        nexttoken();

        sl_node_t *exp = expr(0);
        match(SL_TOKEN_SEMICOLON);

        nexttoken();
        return node_decl(node_ident(name), exp);
    case SL_TOKEN_IF:
        nexttoken();
        match(SL_TOKEN_LPAREN);

        nexttoken();
        sl_node_t *arg = expr(0);
        match(SL_TOKEN_RPAREN);

        nexttoken();
        sl_node_t *body = statment();
        sl_node_t *elsebody = NULL;

        if (lasttoken == SL_TOKEN_ELSE)
        {
            nexttoken();
            elsebody = statment();
        }

        return node_cond(arg, body, elsebody);
    case SL_TOKEN_WHILE:
        nexttoken();
        match(SL_TOKEN_LPAREN);

        nexttoken();
        sl_node_t *arg2 = expr(0);
        match(SL_TOKEN_RPAREN);

        nexttoken();
        sl_node_t *body2 = statment();

        return node_loop(arg2, body2);
    case SL_TOKEN__FN:
        nexttoken();
        match(SL_TOKEN_IDENT);

        char *fnname = strdup(buffer);

        nexttoken();
        match(SL_TOKEN_LPAREN);

        vector(char *) args = NULL;

        while (nexttoken() != SL_TOKEN_RPAREN)
        {
            match(SL_TOKEN_IDENT);
            vector_push(args, strdup(buffer));
            if (nexttoken() != SL_TOKEN_COMMA)
                break;
        }

        match(SL_TOKEN_RPAREN);
        nexttoken();

        sl_node_t *fnbody = statment();

        return node_func(fnname, args, fnbody);
    case SL_TOKEN_RETURN:
        nexttoken();

        sl_node_t *retnode = NULL;
        if (lasttoken != SL_TOKEN_SEMICOLON)
            retnode = expr(0);

        match(SL_TOKEN_SEMICOLON);
        nexttoken();

        return node_return(retnode);
    case SL_TOKEN_BREAK:
        nexttoken();
        match(SL_TOKEN_SEMICOLON);
        nexttoken();

        return node_break();
    default:;
        sl_node_t *e = expr(0);
        match(SL_TOKEN_SEMICOLON);
        nexttoken();
        return e;
    }
}

sl_node_t *sl_parse()
{
    nexttoken();
    vector(sl_node_t *) funcs = NULL;
    vector(sl_node_t *) stmts = NULL;
    while (lasttoken != SL_TOKEN_EOF)
    {
        sl_node_t *n = statment();

        if (n->type == SL_NODETYPE_FUNC)
            vector_push(funcs, n);
        else
            vector_push(stmts, n);
    }
    return node_root(node_block(funcs), node_block(stmts));
}
