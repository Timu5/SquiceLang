#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

sl_lexer_t *sl_lexer_new(char *input)
{
    sl_lexer_t *lexer = (sl_lexer_t*)malloc(sizeof(sl_lexer_t));
    lexer->input = input;
    lexer->index = 0;
    lexer->buffer = (char*)malloc(255);
    lexer->number = 0;
    lexer->line = 0;
    lexer->col = 0;
    lexer->lastchar = ' ';
}

void sl_lexer_free(sl_lexer_t *lexer)
{
    free(lexer->buffer);
    free(lexer);
}

static int nextchar(sl_lexer_t *lexer)
{
    //lastchar = fgetc(input);
    if(lexer->input[lexer->index] == 0)
    {
        lexer->lastchar = EOF;
    }
    else
    {
        lexer->lastchar = lexer->input[lexer->index];
        lexer->index++;
    }
    
    if (lexer->lastchar == '\n')
    {
        lexer->line++;
        lexer->col = 0;
    }
    else
    {
        lexer->col++;
    }
    return lexer->lastchar;
}

int sl_gettoken(sl_lexer_t *lexer)
{
    while (isspace(lexer->lastchar))
        nextchar(lexer); // eaat white space

    if (lexer->lastchar <= 0)
    {
        return SL_TOKEN_EOF;
    }
    else if (isalpha(lexer->lastchar))
    {
        int ptr = 0;
        do
        {
            lexer->buffer[ptr++] = (char)tolower(lexer->lastchar);
            nextchar(lexer);
        } while (isalpha(lexer->lastchar) || isdigit(lexer->lastchar));

        lexer->buffer[ptr] = 0;

        if (strcmp(lexer->buffer, "if") == 0)
            return SL_TOKEN_IF;
        else if (strcmp(lexer->buffer, "else") == 0)
            return SL_TOKEN_ELSE;
        else if (strcmp(lexer->buffer, "while") == 0)
            return SL_TOKEN_WHILE;
        else if (strcmp(lexer->buffer, "let") == 0)
            return SL_TOKEN_LET;
        else if (strcmp(lexer->buffer, "fn") == 0)
            return SL_TOKEN__FN;
        else if (strcmp(lexer->buffer, "return") == 0)
            return SL_TOKEN_RETURN;
        else if (strcmp(lexer->buffer, "break") == 0)
            return SL_TOKEN_BREAK;

        return SL_TOKEN_IDENT;
    }
    else if (isdigit(lexer->lastchar))
    {
        int ptr = 0;
        do
        {
            lexer->buffer[ptr++] = (char)lexer->lastchar;
            nextchar(lexer);
        } while (isalnum(lexer->lastchar));
        lexer->buffer[ptr] = 0;

        lexer->number = (int)strtol(lexer->buffer, NULL, 0);

        return SL_TOKEN_NUMBER;
    }
    else if (lexer->lastchar == '"')
    {
        nextchar(lexer);
        int ptr = 0;
        while (lexer->lastchar != '"' && lexer->lastchar > 0)
        {
            lexer->buffer[ptr++] = (char)lexer->lastchar;
            nextchar(lexer);
        }
        if (lexer->lastchar < 0)
            throw("Unexpected end of file");

        lexer->buffer[ptr] = 0;
        nextchar(lexer);
        return SL_TOKEN_STRING;
    }

    int tmp = SL_TOKEN_UNKOWN;
    switch (lexer->lastchar)
    {
    case ',':
        tmp = SL_TOKEN_COMMA;
        break;
    case ':':
        tmp = SL_TOKEN_COLON;
        break;
    case ';':
        tmp = SL_TOKEN_SEMICOLON;
        break;
    case '.':
        tmp = SL_TOKEN_DOT;
        break;
    case '+':
        tmp = SL_TOKEN_PLUS;
        break;
    case '-':
        tmp = SL_TOKEN_MINUS;
        break;
    case '/':
        if (nextchar(lexer) == '/')
        {
            int l = lexer->line;
            while (nextchar(lexer) >= 0 && l == lexer->line)
                ;
            return sl_gettoken(lexer);
        }
        else if (lexer->lastchar == '*')
        {
            nextchar(lexer);
            while (!(lexer->lastchar == '*' && nextchar(lexer) == '/'))
            {
                if (lexer->lastchar < 0)
                    throw("Unexpected end of file");
                nextchar(lexer);
            }
            nextchar(lexer);
            return sl_gettoken(lexer);
        }
        else
            return SL_TOKEN_SLASH;
        break;
    case '*':
        tmp = SL_TOKEN_ASTERISK;
        break;
    case '!':
    {
        if (nextchar(lexer) == '=')
            tmp = SL_TOKEN_NOTEQUAL;
        else
            return SL_TOKEN_EXCLAM;
        break;
    }
    case '=':
    {
        if (nextchar(lexer) == '=')
            tmp = SL_TOKEN_EQUAL;
        else
            return SL_TOKEN_ASSIGN;
        break;
    }
    case '(':
        tmp = SL_TOKEN_LPAREN;
        break;
    case ')':
        tmp = SL_TOKEN_RPAREN;
        break;
    case '{':
        tmp = SL_TOKEN_LBRACE;
        break;
    case '}':
        tmp = SL_TOKEN_RBRACE;
        break;
    case '[':
        tmp = SL_TOKEN_LBRACK;
        break;
    case ']':
        tmp = SL_TOKEN_RBRACK;
        break;
    case '<':
    {
        if (nextchar(lexer) == '=')
            tmp = SL_TOKEN_LESSEQUAL;
        else
            return SL_TOKEN_LCHEVR;
    }
    case '>':
    {
        if (nextchar(lexer) == '=')
            tmp = SL_TOKEN_MOREEQUAL;
        else
            return SL_TOKEN_RCHEVR;
    }
    }

    nextchar(lexer);
    return tmp;
}

char *sl_tokenstr(int token)
{
    if (token < SL_TOKEN_IDENT || token > SL_TOKEN_UNKOWN)
        return "WRONG TOKEN!";
    char *names[] = {
        "SL_TOKEN_IDENT",
        "SL_TOKEN_NUMBER",
        "SL_TOKEN_STRING",

        "SL_TOKEN__FN",
        "SL_TOKEN_RETURN",
        "SL_TOKEN_LET",
        "SL_TOKEN_IF",
        "SL_TOKEN_ELSE",
        "SL_TOKEN_WHILE",
        "SL_TOKEN_BREAK",

        "SL_TOKEN_SEMICOLON", // ;
        "SL_TOKEN_COMMA",     // ,
        "SL_TOKEN_DOT",       // .

        "SL_TOKEN_PLUS",     // +
        "SL_TOKEN_MINUS",    // -
        "SL_TOKEN_SLASH",    // /
        "SL_TOKEN_ASTERISK", // *

        "SL_TOKEN_ASSIGN",    // =
        "SL_TOKEN_EQUAL",     // ==
        "SL_TOKEN_NOTEQUAL",  // !=
        "SL_TOKEN_LESSEQUAL", // <=
        "SL_TOKEN_MOREEQUAL", // >=
        "SL_TOKEN_LCHEVR",    // <
        "SL_TOKEN_RCHEVR",    // >

        "SL_TOKEN_LPAREN", // (
        "SL_TOKEN_RPAREN", // )
        "SL_TOKEN_LBRACE", // {
        "SL_TOKEN_RBRACE", // }
        "SL_TOKEN_LBRACK", // [
        "SL_TOKEN_RBRACK", // ]
        "SL_TOKEN_EXCLAM", // !

        "SL_TOKEN_EOF",
        "SL_TOKEN_UNKOWN"};
    return names[token - SL_TOKEN_IDENT];
}
