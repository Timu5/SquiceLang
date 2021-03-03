#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "SquiceLang.h"

sl_parser_t *sl_parser_new(char *input)
{
    sl_parser_t *parser = (sl_parser_t *)malloc(sizeof(sl_parser_t));
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
        throw("Unexpected token, expect %s got %s on line %d column %d",
              sl_tokenstr(token),
              sl_tokenstr(parser->lasttoken),
              parser->lexer->startmarker.line,
              parser->lexer->startmarker.column);
}

sl_node_t *expr(sl_parser_t *parser, int min);

// primary :=  ident | number | string | '(' expr ')' | UNARY_OP primary |
//             array | dict | fstring
//             primary '[' expr ']' | primary '(' expr ')'| primary '.' ident
sl_node_t *primary(sl_parser_t *parser)
{
    sl_node_t *prim = NULL;
    sl_marker_t marker = parser->lexer->startmarker;
    if (parser->lasttoken == SL_TOKEN_NUMBER)
    {
        prim = node_number(marker, parser->lexer->number);
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_STRING)
    {
        prim = node_string(marker, strdup(parser->lexer->buffer));
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_IDENT)
    {
        prim = node_ident(marker, strdup(parser->lexer->buffer));
        nexttoken(parser);
    }
    else if (parser->lasttoken == SL_TOKEN_FSTRING)
    {
        // split string into parts
        sl_vector(char *) parts = NULL;
        int ptr = 0;
        char *buffer = (char *)malloc(255);
        size_t len = strlen(parser->lexer->buffer);
        char *fstring = parser->lexer->buffer;
        int inexpr = 0;
        for (int i = 0; i < len; i++)
        {
            if (inexpr)
            {
                if (fstring[i] == '}')
                {
                    // create buffer copy and add to parts
                    buffer[ptr] = 0;
                    char *cpy = strdup(buffer);
                    ptr = 0;
                    sl_vector_push(parts, cpy);
                    inexpr = 0;
                    continue;
                }
            }
            else
            {
                // TODO: Add backslash support!
                if (fstring[i] == '$' && fstring[i + 1] == '{')
                {
                    if (i != 0)
                    {
                        // create bufffer copy and add to parts
                        buffer[ptr] = 0;
                        char *cpy = strdup(buffer);
                        ptr = 0;
                        sl_vector_push(parts, cpy);
                    }
                    inexpr = 1;
                }
            }
            buffer[ptr] = fstring[i];
            ptr++;
        }
        if (ptr != 0)
        {
            buffer[ptr] = 0;
            char *cpy = strdup(buffer);
            ptr = 0;
            sl_vector_push(parts, cpy);
        }
        if (inexpr)
            throw("Missing '}' in string interpolation on line %d column %d",
                  parser->lexer->startmarker.line,
                  parser->lexer->startmarker.column);

        sl_vector(sl_node_t *) nodes = NULL;
        for (int i = 0; i < sl_vector_size(parts); i++)
        {
            char *p = parts[i];
            if (parts[i][0] == '$' && parts[i][1] == '{')
            {
                // create parser and parse this string!!!
                sl_parser_t *pars = sl_parser_new(parts[i] + 2);
                nexttoken(pars);
                sl_node_t *ex = expr(pars, 0);
                sl_parser_free(pars);

                sl_vector(sl_node_t *) node = NULL;
                sl_vector_push(node, ex);
                sl_vector_push(nodes, node_call(marker, node_ident(marker, strdup("str")), node_block(marker, node)));
                free(parts[i]);
            }
            else
            {
                // check memory management!!!
                sl_vector_push(nodes, node_string(marker, parts[i]));
            }
        }
        sl_vector_free(parts);

        size_t nodescnt = sl_vector_size(nodes);

        // generate binary additions for all nodes
        while (sl_vector_size(nodes) > 1)
        {
            sl_node_t *a = nodes[nodescnt - 2];
            sl_node_t *b = nodes[nodescnt - 1];

            nodes[nodescnt - 2] = node_binary(marker, SL_TOKEN_PLUS, a, b);

            sl_vector_pop(nodes);
            nodescnt--;
        }
        nexttoken(parser);
        free(buffer);
        prim = nodes[0];
        sl_vector_free(nodes);
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
        return node_unary(marker, op, primary(parser));
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
        prim = node_call(marker, node_ident(marker, strdup("list")), node_block(marker, elements));
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
                throw("Unexpected token, expect SL_TOKEN_STRING or SL_TOKEN_IDENT got %s on line %s column %s",
                      sl_tokenstr(parser->lasttoken),
                      parser->lexer->startmarker.line,
                      parser->lexer->startmarker.column);
            sl_vector_push(keys, node_string(marker, strdup(parser->lexer->buffer)));

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
        sl_vector_push(args, node_call(marker, node_ident(marker, strdup("list")), node_block(marker, keys)));
        sl_vector_push(args, node_call(marker, node_ident(marker, strdup("list")), node_block(marker, values)));
        prim = node_call(marker, node_ident(marker, strdup("dict")), node_block(marker, args));
    }
    else
    {
        throw("Unexpected token in primary line %d column %d", parser->lexer->startmarker.line, parser->lexer->startmarker.column);
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
            prim = node_call(marker, prim, node_block(marker, args));
        }
        else if (parser->lasttoken == SL_TOKEN_LBRACK) // primary '[' expr ']'
        {
            nexttoken(parser);
            sl_node_t *e = expr(parser, 0);
            match(parser, SL_TOKEN_RBRACK);
            nexttoken(parser);
            prim = node_index(marker, prim, e);
        }
        else if (parser->lasttoken == SL_TOKEN_DOT) // primary '.' ident
        {
            nexttoken(parser);
            match(parser, SL_TOKEN_IDENT);
            char *name = strdup(parser->lexer->buffer);
            nexttoken(parser);
            prim = node_member(marker, prim, name);
        }
    }
    return prim;
}

// expr := primary |  expr OP expr
sl_node_t *expr(sl_parser_t *parser, int min)
{
    int pre[] = {
        6, // +
        6, // -
        7, // /
        7, // *
        1, // =
        4, // ==
        4, // !=
        5, // <=
        5, // >=
        5, // <
        5, // >
        3, // &&
        2  // ||
    };
    sl_node_t *lhs = primary(parser);
    while (1)
    {
        if (parser->lasttoken < SL_TOKEN_PLUS ||
            parser->lasttoken > SL_TOKEN_OR ||
            pre[parser->lasttoken - SL_TOKEN_PLUS] < min)
            break;

        sl_marker_t marker = parser->lexer->startmarker;
        int op = parser->lasttoken;
        int prec = pre[parser->lasttoken - SL_TOKEN_PLUS];
        int assoc = (parser->lasttoken == SL_TOKEN_ASSIGN) ? 1 : 0; // 0 left, 1 right
        int nextmin = assoc ? prec : prec + 1;
        nexttoken(parser);
        sl_node_t *rhs = expr(parser, nextmin);
        lhs = node_binary(marker, op, lhs, rhs);
    }
    return lhs;
}

// block := '{' statement '}'
// let := 'let' ident '=' expr ';'
// if := 'if' '(' expr ')' statement ['else' statement]
// while := 'while' '(' expr ')' statement
// return := 'return' ';' | 'return' expr ';'
// break := 'break' ';'
// import := 'import' ident ';'
// class := 'class' ident '{' methods '}'
// try := 'try' statement 'catch' statement
// throw := 'throw' expr ';'
// statement := block | let | if | while | funca | return | break | import | class | try | expr ';'
sl_node_t *statment(sl_parser_t *parser)
{
    sl_marker_t marker = parser->lexer->startmarker;
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
        return node_block(marker, list);
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
        return node_decl(marker, node_ident(marker, name), exp);
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

        return node_cond(marker, arg, body, elsebody);
    case SL_TOKEN_WHILE:
        nexttoken(parser);
        match(parser, SL_TOKEN_LPAREN);

        nexttoken(parser);
        sl_node_t *arg2 = expr(parser, 0);
        match(parser, SL_TOKEN_RPAREN);

        nexttoken(parser);
        sl_node_t *body2 = statment(parser);

        return node_loop(marker, arg2, body2);
    case SL_TOKEN_FN:
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

        return node_func(marker, fnname, args, fnbody);
    case SL_TOKEN_RETURN:
        nexttoken(parser);

        sl_node_t *retnode = NULL;
        if (parser->lasttoken != SL_TOKEN_SEMICOLON)
            retnode = expr(parser, 0);

        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);

        return node_return(marker, retnode);
    case SL_TOKEN_BREAK:
        nexttoken(parser);
        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);

        return node_break(marker);
    case SL_TOKEN_IMPORT:
        nexttoken(parser);
        match(parser, SL_TOKEN_IDENT);
        char *module_name = strdup(parser->lexer->buffer);

        nexttoken(parser);
        match(parser, SL_TOKEN_SEMICOLON);

        nexttoken(parser);
        return node_import(marker, module_name);
    case SL_TOKEN_CLASS:
        nexttoken(parser);
        match(parser, SL_TOKEN_IDENT);
        char *class_name = strdup(parser->lexer->buffer);

        nexttoken(parser);
        match(parser, SL_TOKEN_LBRACE);

        nexttoken(parser);

        sl_vector(sl_node_t *) constructor = NULL;

        // TODO: move most of this nonsense to codegen!

        // generated constructor code:
        // let this = dict();
        // this.old_name = __class_old_name_;
        // ...
        // return this;

        sl_vector_push(constructor, node_decl(marker,
                                              node_ident(marker, strdup("this")),
                                              node_call(marker,
                                                        node_ident(marker, strdup("dict")),
                                                        node_block(marker, NULL))));

        sl_vector(sl_node_t *) constructors = NULL;
        sl_vector(sl_node_t *) methods_list = NULL;
        while (parser->lasttoken != SL_TOKEN_RBRACE)
        {
            if (parser->lasttoken != SL_TOKEN_FN)
            {
                throw("Expect only methods inside class on line %s column %s",
                      parser->lexer->startmarker.line,
                      parser->lexer->startmarker.column);
            }
            sl_node_t *node = statment(parser);

            // change name to __class_name__
            char *old_name = node->func.name;
            char *new_name = malloc(6 + strlen(old_name) + strlen(class_name));
            strcpy(new_name, "__");
            strcat(new_name, class_name);
            strcat(new_name, "_");
            strcat(new_name, old_name);
            strcat(new_name, "__");

            if (strcmp(old_name, class_name) == 0)
            {
                sl_vector_push(constructors, node);
            }

            // add code to conctructor
            sl_vector_push(constructor,
                           node_binary(marker, SL_TOKEN_ASSIGN,
                                       node_member(marker,
                                                   node_ident(marker, strdup("this")), strdup(old_name)),
                                       node_ident(marker, strdup(new_name))));

            free(old_name);
            node->func.name = new_name;

            sl_vector_push(methods_list, node);
        }
        match(parser, SL_TOKEN_RBRACE);

        if (sl_vector_size(constructors) > 1)
        {
            throw("Class %s has more than one constructor", class_name);
        }
        else if (sl_vector_size(constructors) == 1)
        {
            if (constructors[0]->func.body->type != SL_NODETYPE_BLOCK)
            {
                throw("Constructor body need to be block on line %d colum %d",
                      constructors[0]->func.body->marker.line,
                      constructors[0]->func.body->marker.column);
            }

            sl_vector(sl_node_t *) args = NULL;
            for (int i = 0; i < sl_vector_size(constructors[0]->func.args); i++)
            {
                sl_vector_push(args,
                               node_ident(marker, strdup(constructors[0]->func.args[i])));
            }

            sl_vector_push(constructor,
                           node_call(marker,
                                     node_member(marker,
                                                 node_ident(marker, strdup("this")), strdup(class_name)),
                                     node_block(marker, args)));
            sl_vector_push(constructor,
                           node_return(marker,
                                       node_ident(marker, strdup("this"))));

            sl_vector(char *) args2 = NULL;
            for (int i = 0; i < sl_vector_size(constructors[0]->func.args); i++)
            {
                sl_vector_push(args2, strdup(constructors[0]->func.args[i]));
            }
            sl_vector_push(methods_list,
                           node_func(marker, strdup(class_name), args2,
                                     node_block(marker, constructor)));
        }
        else
        {
            sl_vector_push(constructor,
                           node_return(marker,
                                       node_ident(marker, strdup("this"))));
            sl_vector_push(methods_list,
                           node_func(marker, strdup(class_name), NULL,
                                     node_block(marker, constructor)));
        }

        sl_vector_free(constructors);
        nexttoken(parser);
        return node_class(marker, class_name, methods_list);
    case SL_TOKEN_TRY:
        nexttoken(parser);
        match(parser, SL_TOKEN_LBRACE);
        sl_node_t *tryblock = statment(parser);

        match(parser, SL_TOKEN_CATCH);
        nexttoken(parser);
        match(parser, SL_TOKEN_LBRACE);
        sl_node_t *catchblock = statment(parser);

        return node_trycatch(marker, tryblock, catchblock);
    case SL_TOKEN_THROW:
    {
        nexttoken(parser);
        sl_node_t *e = expr(parser, 0);
        match(parser, SL_TOKEN_SEMICOLON);
        nexttoken(parser);
        return node_throw(marker, e);
    }
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
    sl_marker_t marker = parser->lexer->marker;
    while (parser->lasttoken != SL_TOKEN_EOF)
    {
        sl_node_t *n = statment(parser);

        if (n->type == SL_NODETYPE_FUNC)
        {
            sl_vector_push(funcs, n);
        }
        else if (n->type == SL_NODETYPE_CLASS)
        {
            // generate constructor from class
            for (int i = 0; i < sl_vector_size(n->class.methods); i++)
                sl_vector_push(funcs, n->class.methods[i]);
            sl_node_free(n);
        }
        else
        {
            sl_vector_push(stmts, n);
        }
    }
    sl_node_t *root = node_root(marker, node_block(marker, funcs), node_block(marker, stmts));
    return root;
}
