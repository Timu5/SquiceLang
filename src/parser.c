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
    return lasttoken = gettoken();
}

void match(int token)
{
    if(lasttoken != token)
        throw("Unexpexted token, expect %s got %s", tokenstr(token), tokenstr(lasttoken));
}

node_t* expr(int min);

// primary :=  ident | number | string | '(' expr ')' | UNARY_OP primary |
//             array | dict |
//             primary '[' expr ']' | primary '(' expr ')'| primary '.' ident
node_t* primary()
{
    node_t* prim = NULL;
    if (lasttoken == T_NUMBER)
    {
        prim = node_int(number);
        nexttoken();
    }
    else if (lasttoken == T_STRING)
    {
        prim = node_string(strdup(buffer));
        nexttoken();
    }
    else if (lasttoken == T_IDENT)
    {
        prim = node_ident(strdup(buffer));
        nexttoken();
    }
    else if (lasttoken == T_LPAREN) // '(' expr ')'
    {
        nexttoken();
        prim = expr(0);
        match(T_RPAREN);
        nexttoken();
    }
    else if (lasttoken == T_PLUS || lasttoken == T_MINUS || lasttoken == T_EXCLAM) // UNARY_OP primary
    {
        // UNARY_OP primary
        int op = lasttoken;
        nexttoken();
        return node_unary(op, primary());
    }
    else if (lasttoken == T_LBRACK)
    {
        // new array
        nexttoken();
        vector(node_t*) elements = NULL;
        while (lasttoken != T_RBRACK)
        {
            node_t* e = expr(0);
            vector_push(elements, e);
            if (lasttoken != T_COMMA)
                break;
            nexttoken();
        }
        match(T_RBRACK);
        nexttoken();

        // syntax sugar, convert [1,2,3] to list(1,2,3)
        prim = node_call(node_ident(strdup("list")), node_block(elements)); 
    }
    else if (lasttoken == T_LBRACE)
    {
        // new dictonary
        nexttoken();
        vector(node_t*) keys = NULL;
        vector(node_t*) values = NULL;
        while (lasttoken != T_RBRACE)
        {
            if (!(lasttoken == T_STRING || lasttoken == T_IDENT))
                throw("Unexpexted token, expect T_STRING or T_IDENT got %s", lasttoken);
            vector_push(keys, node_string(strdup(buffer)));

            nexttoken();
            match(T_COLON);
            nexttoken();

            node_t* e = expr(0);
            vector_push(values, e);

            if (lasttoken != T_COMMA)
                break;
            nexttoken();
        }
        match(T_RBRACE);
        nexttoken();

        // syntax sugar, convert {a:1, b:2+2} to dict(list('a', 'b'), list(1, 2+2))
        vector(node_t*) args = NULL;
        vector_push(args, node_call(node_ident(strdup("list")), node_block(keys)));
        vector_push(args, node_call(node_ident(strdup("list")), node_block(values)));
        prim = node_call(node_ident(strdup("dict")), node_block(args));
    }
    else
    {
        throw("Unexpexted token in primary!");
    }

    while (lasttoken == T_DOT || lasttoken == T_LBRACK || lasttoken == T_LPAREN)
    {     
        if (lasttoken == T_LPAREN) // primary '(' expr ')'
        {
            nexttoken();
            vector(node_t*) args = NULL;
            while (lasttoken != T_RPAREN)
            {
                node_t* e = expr(0);
                vector_push(args, e);

                if (lasttoken != T_COMMA)
                    break;
                nexttoken();
            }
            match(T_RPAREN);
            nexttoken();
            prim = node_call(prim, node_block(args));
        }
        else if (lasttoken == T_LBRACK) // primary '[' expr ']'
        {
            nexttoken();
            node_t* e = expr(0);
            match(T_RBRACK);
            nexttoken();
            prim = node_index(prim, e);
        }
        else if (lasttoken == T_DOT) // primary '.' ident
        {
            nexttoken();
            match(T_IDENT);
            char* name = strdup(buffer);
            nexttoken();
            prim = node_member(prim, name);
        }
    }
    return prim;
}

// expr := primary |  expr OP expr
node_t* expr(int min)
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
    node_t* lhs = primary();
    while(1)
    {
        if(lasttoken < T_PLUS || lasttoken > T_RCHEVR || pre[lasttoken - T_PLUS] < min)
            break;

        int op = lasttoken;
        int prec = pre[lasttoken - T_PLUS];
        int assoc = 0; // 0 left, 1 right
        int nextmin = assoc ? prec : prec + 1;
        nexttoken();
        node_t* rhs = expr(nextmin);
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
node_t* statment()
{
    switch(lasttoken)
    {
    case T_LBRACE:
        nexttoken();
        
        vector(node_t*) list = NULL;
        while(lasttoken != T_RBRACE)
        {
            node_t* node = statment();
            vector_push(list, node);
        }
        match(T_RBRACE);
        
        nexttoken();
        return node_block(list);
    case T_LET:
        nexttoken();
        match(T_IDENT); 
        char* name = strdup(buffer);
        
        nexttoken();
        match(T_ASSIGN);
        nexttoken();
        
        node_t* exp = expr(0);
        match(T_SEMICOLON);

        nexttoken();
        return node_decl(node_ident(name), exp);
    case T_IF:
        nexttoken();
        match(T_LPAREN);

        nexttoken();
        node_t* arg = expr(0);
        match(T_RPAREN);

        nexttoken();
        node_t* body = statment();
        node_t* elsebody = NULL;

        if(lasttoken == T_ELSE)
        {
            nexttoken();
            elsebody = statment();
        }

        return node_cond(arg, body, elsebody);
    case T_WHILE:
        nexttoken();
        match(T_LPAREN);

        nexttoken();
        node_t* arg2 = expr(0);
        match(T_RPAREN);

        nexttoken();
        node_t* body2 = statment();
        
        return node_loop(arg2, body2);
    case T_FN:
        nexttoken();
        match(T_IDENT);
        
        char* fnname = strdup(buffer);
        
        nexttoken();
        match(T_LPAREN);
    
        vector(char*) args = NULL;
        
        while(nexttoken() != T_RPAREN)
        {
            match(T_IDENT);
            vector_push(args, strdup(buffer));
            if(nexttoken() != T_COMMA)
                break;
        }
        
        match(T_RPAREN);
        nexttoken();

        node_t* fnbody = statment();

        return node_func(fnname, args, fnbody);
    case T_RETURN:
        nexttoken();

        node_t* retnode = NULL;
        if (lasttoken != T_SEMICOLON)
            retnode = expr(0);

        match(T_SEMICOLON);
        nexttoken();

        return node_return(retnode);
    case T_BREAK:
        nexttoken();
        match(T_SEMICOLON);
        nexttoken();

        return node_break();
    default:;
        node_t* e = expr(0);
        match(T_SEMICOLON);
        nexttoken();
        return e;
    }
}

node_t* parse()
{
    nexttoken();
    vector(node_t*) funcs = NULL;
    vector(node_t*) stmts = NULL;
    while(lasttoken != T_EOF)
    {
        node_t* n = statment();

        if(n->type == N_FUNC)
            vector_push(funcs, n);
        else
            vector_push(stmts, n); 
    }
    return node_root(node_block(funcs), node_block(stmts));
}
