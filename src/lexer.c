#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "SquiceLang.h"

sl_lexer_t *sl_lexer_new(char *input)
{
    sl_lexer_t *lexer = (sl_lexer_t *)malloc(sizeof(sl_lexer_t));
    lexer->input = input;
    lexer->buffer = (char *)malloc(255);
    lexer->number = 0;
    lexer->marker.index = 0;
    lexer->marker.line = 1;
    lexer->marker.column = 1;
    lexer->lastchar = ' ';
    return lexer;
}

void sl_lexer_free(sl_lexer_t *lexer)
{
    free(lexer->buffer);
    free(lexer);
}

static int nextchar(sl_lexer_t *lexer)
{
    if (lexer->input[lexer->marker.index] == 0)
    {
        lexer->lastchar = EOF;
    }
    else
    {
        lexer->lastchar = lexer->input[lexer->marker.index];
        lexer->marker.index++;
    }

    if (lexer->lastchar == '\n')
    {
        lexer->marker.line++;
        lexer->marker.column = 1;
    }
    else
    {
        lexer->marker.column++;
    }
    if (lexer->lastchar < -1)
        lexer->lastchar = -1;
    return lexer->lastchar;
}

int sl_gettoken(sl_lexer_t *lexer)
{
    while (isspace(lexer->lastchar))
        nextchar(lexer); // eaat white space

    lexer->startmarker = lexer->marker;

    if (lexer->lastchar <= 0)
    {
        return SL_TOKEN_EOF;
    }
    else if (isalpha(lexer->lastchar) || lexer->lastchar == '_')
    {
        int ptr = 0;
        do
        {
            lexer->buffer[ptr++] = (char)tolower(lexer->lastchar);
            nextchar(lexer);
        } while (isalpha(lexer->lastchar) || isdigit(lexer->lastchar) || lexer->lastchar == '_');

        lexer->buffer[ptr] = 0;

        if (strcmp(lexer->buffer, "f") == 0 && lexer->lastchar == '"')
        {
            sl_gettoken(lexer);
            return SL_TOKEN_FSTRING;
        }

        if (strcmp(lexer->buffer, "if") == 0)
            return SL_TOKEN_IF;
        else if (strcmp(lexer->buffer, "else") == 0)
            return SL_TOKEN_ELSE;
        else if (strcmp(lexer->buffer, "while") == 0)
            return SL_TOKEN_WHILE;
        else if (strcmp(lexer->buffer, "let") == 0)
            return SL_TOKEN_LET;
        else if (strcmp(lexer->buffer, "fn") == 0)
            return SL_TOKEN_FN;
        else if (strcmp(lexer->buffer, "return") == 0)
            return SL_TOKEN_RETURN;
        else if (strcmp(lexer->buffer, "break") == 0)
            return SL_TOKEN_BREAK;
        else if (strcmp(lexer->buffer, "import") == 0)
            return SL_TOKEN_IMPORT;
        else if (strcmp(lexer->buffer, "class") == 0)
            return SL_TOKEN_CLASS;
        else if (strcmp(lexer->buffer, "try") == 0)
            return SL_TOKEN_TRY;
        else if (strcmp(lexer->buffer, "catch") == 0)
            return SL_TOKEN_CATCH;
        else if (strcmp(lexer->buffer, "throw") == 0)
            return SL_TOKEN_THROW;

        return SL_TOKEN_IDENT;
    }
    else if (isdigit(lexer->lastchar))
    {
        double value = 0.0;
        int exponent = 0;

        if (lexer->lastchar == '0')
        {
            nextchar(lexer);
            if (lexer->lastchar == 'x' || lexer->lastchar == 'X')
            {
                nextchar(lexer);
                while (isdigit(lexer->lastchar) ||
                       (lexer->lastchar >= 'A' && lexer->lastchar <= 'F') ||
                       (lexer->lastchar >= 'a' && lexer->lastchar <= 'f'))
                {
                    if (lexer->lastchar >= 'a')
                        value = value * 16 + (lexer->lastchar - 'a' + 10);
                    else if (lexer->lastchar >= 'A')
                        value = value * 16 + (lexer->lastchar - 'A' + 10);
                    else
                        value = value * 16 + (lexer->lastchar - '0');
                    nextchar(lexer);
                }
                lexer->number = value;

                return SL_TOKEN_NUMBER;
            }
        }

        while (isdigit(lexer->lastchar))
        {
            value = value * 10 + (lexer->lastchar - '0');
            nextchar(lexer);
        }

        if (lexer->lastchar == '.')
        {
            nextchar(lexer);
            while (isdigit(lexer->lastchar))
            {
                value = value * 10 + (lexer->lastchar - '0');
                exponent--;
                nextchar(lexer);
            }
        }

        if (lexer->lastchar == 'e' || lexer->lastchar == 'E')
        {
            int sign = 1;
            int i = 0;
            nextchar(lexer);
            if (lexer->lastchar == '-')
            {
                sign = -1;
                nextchar(lexer);
            }
            else if (lexer->lastchar == '+')
            {
                /* do nothing when positive :) */
                nextchar(lexer);
            }
            while (isdigit(lexer->lastchar))
            {
                i = i * 10 + (lexer->lastchar - '0');
                nextchar(lexer);
            }
            exponent += sign * i;
        }

        while (exponent > 0)
        {
            value *= 10;
            exponent--;
        }
        while (exponent < 0)
        {
            value *= 0.1;
            exponent++;
        }
        lexer->number = value;

        return SL_TOKEN_NUMBER;
    }
    else if (lexer->lastchar == '"')
    {
        nextchar(lexer);
        int ptr = 0;
        while (lexer->lastchar != '"' && lexer->lastchar > 0)
        {
            if (lexer->lastchar == '\\')
            {
                nextchar(lexer);
                switch (lexer->lastchar)
                {
                case 'a':
                    lexer->buffer[ptr++] = '\a';
                    break;
                case 'b':
                    lexer->buffer[ptr++] = '\b';
                    break;
                case 'e':
                    lexer->buffer[ptr++] = '\e';
                    break;
                case 'f':
                    lexer->buffer[ptr++] = '\f';
                    break;
                case 'n':
                    lexer->buffer[ptr++] = '\n';
                    break;
                case 'r':
                    lexer->buffer[ptr++] = '\r';
                    break;
                case 't':
                    lexer->buffer[ptr++] = '\t';
                    break;
                case 'v':
                    lexer->buffer[ptr++] = '\v';
                    break;
                case '\\':
                    lexer->buffer[ptr++] = '\\';
                    break;
                case '\'':
                    lexer->buffer[ptr++] = '\'';
                    break;
                case '\"':
                    lexer->buffer[ptr++] = '\"';
                    break;
                default:
                    throw("Unexpected character after \\ in string on line %d column %d",
                          lexer->marker.line, lexer->marker.column);
                    break;
                }
            }
            else
            {
                lexer->buffer[ptr++] = (char)lexer->lastchar;
            }
            nextchar(lexer);
        }
        if (lexer->lastchar < 0)
            throw("Missing ending \" in string on line %d column %d",
                  lexer->startmarker.line, lexer->startmarker.column);

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
            size_t l = lexer->marker.line;
            while (nextchar(lexer) >= 0 && l == lexer->marker.line)
                ;
            return sl_gettoken(lexer);
        }
        else if (lexer->lastchar == '*')
        {
            nextchar(lexer);
            while (!(lexer->lastchar == '*' && nextchar(lexer) == '/'))
            {
                if (lexer->lastchar < 0)
                    throw("Missing \"*/\" in muliline comment starting at line %d colum %d",
                          lexer->startmarker.line, lexer->startmarker.column);
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
        break;
    }
    case '>':
    {
        if (nextchar(lexer) == '=')
            tmp = SL_TOKEN_MOREEQUAL;
        else
            return SL_TOKEN_RCHEVR;
        break;
    }
    case '&':
    {
        if (nextchar(lexer) == '&')
            tmp = SL_TOKEN_AND;
        else
            return SL_TOKEN_UNKOWN;
        break;
    }
    case '|':
    {
        if (nextchar(lexer) == '|')
            tmp = SL_TOKEN_OR;
        else
            return SL_TOKEN_UNKOWN;
        break;
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
        "SL_TOKEN_FSTRING",

        "SL_TOKEN_FN",
        "SL_TOKEN_RETURN",
        "SL_TOKEN_LET",
        "SL_TOKEN_IF",
        "SL_TOKEN_ELSE",
        "SL_TOKEN_WHILE",
        "SL_TOKEN_BREAK",
        "SL_TOKEN_IMPORT",
        "SL_TOKEN_CLASS",
        "SL_TOKEN_TRY",
        "SL_TOKEN_CATCH",
        "SL_TOKEN_THROW",

        "SL_TOKEN_COLON",     // :
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
        "SL_TOKEN_AND",       // &&
        "SL_TOKEN_OR",        // ||

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
