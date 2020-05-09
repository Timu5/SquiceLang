#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

FILE *input;

char buffer[255] = {0};
int number = 0;
int line = 1;
int col = 0;

int lastchar = ' ';

int nextchar()
{
    lastchar = fgetc(input);
    if (lastchar == '\n')
    {
        line++;
        col = 0;
    }
    else
    {
        col++;
    }
    return lastchar;
}

int sl_gettoken()
{
    while (isspace(lastchar))
        nextchar(); // eaat white space

    if (lastchar < 0)
    {
        return SL_TOKEN_EOF;
    }
    else if (isalpha(lastchar))
    {
        int ptr = 0;
        do
        {
            buffer[ptr++] = (char)tolower(lastchar);
            nextchar();
        } while (isalpha(lastchar) || isdigit(lastchar));

        buffer[ptr] = 0;

        if (strcmp(buffer, "if") == 0)
            return SL_TOKEN_IF;
        else if (strcmp(buffer, "else") == 0)
            return SL_TOKEN_ELSE;
        else if (strcmp(buffer, "while") == 0)
            return SL_TOKEN_WHILE;
        else if (strcmp(buffer, "let") == 0)
            return SL_TOKEN_LET;
        else if (strcmp(buffer, "fn") == 0)
            return SL_TOKEN__FN;
        else if (strcmp(buffer, "return") == 0)
            return SL_TOKEN_RETURN;
        else if (strcmp(buffer, "break") == 0)
            return SL_TOKEN_BREAK;

        return SL_TOKEN_IDENT;
    }
    else if (isdigit(lastchar))
    {
        int ptr = 0;
        do
        {
            buffer[ptr++] = (char)lastchar;
            nextchar();
        } while (isalnum(lastchar));
        buffer[ptr] = 0;

        number = (int)strtol(buffer, NULL, 0);

        return SL_TOKEN_NUMBER;
    }
    else if (lastchar == '"')
    {
        nextchar();
        int ptr = 0;
        while (lastchar != '"' && lastchar > 0)
        {
            buffer[ptr++] = (char)lastchar;
            nextchar();
        }
        if (lastchar < 0)
            throw("Unexpected end of file");

        buffer[ptr] = 0;
        nextchar();
        return SL_TOKEN_STRING;
    }

    int tmp = SL_TOKEN_UNKOWN;
    switch (lastchar)
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
        if (nextchar() == '/')
        {
            int l = line;
            while (nextchar() >= 0 && l == line)
                ;
            return sl_gettoken();
        }
        else if (lastchar == '*')
        {
            nextchar();
            while (!(lastchar == '*' && nextchar() == '/'))
            {
                if (lastchar < 0)
                    throw("Unexpected end of file");
                nextchar();
            }
            nextchar();
            return sl_gettoken();
        }
        else
            return SL_TOKEN_SLASH;
        break;
    case '*':
        tmp = SL_TOKEN_ASTERISK;
        break;
    case '!':
    {
        if (nextchar() == '=')
            tmp = SL_TOKEN_NOTEQUAL;
        else
            return SL_TOKEN_EXCLAM;
        break;
    }
    case '=':
    {
        if (nextchar() == '=')
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
        if (nextchar() == '=')
            tmp = SL_TOKEN_LESSEQUAL;
        else
            return SL_TOKEN_LCHEVR;
    }
    case '>':
    {
        if (nextchar() == '=')
            tmp = SL_TOKEN_MOREEQUAL;
        else
            return SL_TOKEN_RCHEVR;
    }
    }

    nextchar();
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
        "T_NOTEQUAL",  // !=
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
