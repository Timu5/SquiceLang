#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "utils.h"

FILE* input;

char buffer[255] = { 0 };
int number = 0;
int line = 1;
int col = 0;

int lastchar = ' ';

int nextchar()
{
    lastchar = fgetc(input);
    if(lastchar == '\n')
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

int gettoken()
{
    while(isspace(lastchar))
        nextchar(); // eaat white space
    
    if(lastchar < 0)
    {
        return T_EOF;
    }
    else if(isalpha(lastchar))
    {
        int ptr = 0;
        do
        {
            buffer[ptr++] = (char)tolower(lastchar);
            nextchar();
        } while(isalpha(lastchar) || isdigit(lastchar));

        buffer[ptr] = 0;
        
        if(strcmp(buffer, "if") == 0)
            return T_IF;
        else if(strcmp(buffer, "else") == 0)
            return T_ELSE;
        else if(strcmp(buffer, "while") == 0)
            return T_WHILE;
        else if(strcmp(buffer, "let") == 0)
            return T_LET;
        else if(strcmp(buffer, "fn") == 0)
            return T_FN;
        else if (strcmp(buffer, "return") == 0)
            return T_RETURN;
        else if (strcmp(buffer, "break") == 0)
            return T_BREAK;

        return T_IDENT;
    }
    else if (isdigit(lastchar))
    {
        int ptr = 0;
        do
        {
            buffer[ptr++] = (char)lastchar;
            nextchar();
        } while(isalnum(lastchar));
        buffer[ptr] = 0;
        
        number = (int)strtol(buffer, NULL, 0);
           
        return T_NUMBER;
    }
    else if(lastchar == '"')
    {
        nextchar();
        int ptr = 0;
        while(lastchar != '"' && lastchar > 0)
        {
            buffer[ptr++] = (char)lastchar;
            nextchar();
        }
        if(lastchar < 0)
            throw("Unexpected end of file");

        buffer[ptr] = 0;
        nextchar();
        return T_STRING;
    }

    int tmp = T_UNKOWN;
    switch(lastchar)
    {
    case ',':
        tmp =  T_COMMA;
        break;
    case ':':
        tmp = T_COLON;
        break;
    case ';':
        tmp = T_SEMICOLON;
        break;
    case '.':
        tmp = T_DOT;
        break;
    case  '+':
        tmp = T_PLUS;
        break;
    case '-':
        tmp = T_MINUS;
        break;
    case '/':
        if(nextchar() == '/')
        {
            int l = line;
            while(nextchar() >= 0 && l == line)
               ;
            return gettoken();
        }
        else if(lastchar == '*')
        {
            nextchar();
            while(!(lastchar == '*' && nextchar() == '/'))
            {
                if(lastchar < 0)
                    throw("Unexpected end of file");
                nextchar();
            }
            nextchar();
            return gettoken();
        }
        else
            return T_SLASH;
        break;
    case '*':
        tmp = T_ASTERISK;
        break;
    case '!':
    {   
        if(nextchar() == '=')
            tmp = T_NOTEQUAL;
        else
            return  T_EXCLAM;
        break;
    }
    case '=':
    {
        if(nextchar() == '=')
            tmp =  T_EQUAL;
        else
            return T_ASSIGN;
        break;
    }
    case '(':
        tmp = T_LPAREN;
        break;
    case  ')':
        tmp = T_RPAREN;
        break;
    case '{':
        tmp = T_LBRACE;
        break;
    case '}':
        tmp = T_RBRACE;
        break;
    case '[':
        tmp = T_LBRACK;
        break;
    case ']':
        tmp = T_RBRACK;
        break;
    case '<':
    {
        if(nextchar() == '=')
            tmp = T_LESSEQUAL;
        else
            return T_LCHEVR;
    }
    case '>':
    {
        if(nextchar() == '=')
            tmp = T_MOREEQUAL;
        else
            return T_RCHEVR;
    }
    }

    nextchar();
    return tmp;
}


char* tokenstr(int token)
{
    if(token < T_IDENT || token > T_UNKOWN)
        return "WRONG TOKEN!";
    char* names[] = {   
    "T_IDENT", 
    "T_NUMBER",
    "T_STRING",
    
    "T_FN",
    "T_RETURN",
    "T_LET",
    "T_IF",
    "T_ELSE",
    "T_WHILE",
    "T_BREAK",

    "T_SEMICOLON", // ;
    "T_COMMA",     // ,
    "T_DOT",       // .
    
    "T_PLUS",      // +
    "T_MINUS",     // -
    "T_SLASH",     // /
    "T_ASTERISK",  // *
    
    "T_ASSIGN",    // =
    "T_EQUAL",     // ==
    "T_NOTEQUAL",  // !=
    "T_LESSEQUAL", // <=
    "T_MOREEQUAL", // >=
    "T_LCHEVR",    // <
    "T_RCHEVR",    // >

    "T_LPAREN",    // (
    "T_RPAREN",    // )
    "T_LBRACE",    // {
    "T_RBRACE",    // }
    "T_LBRACK",    // [
    "T_RBRACK",    // ]
    "T_EXCLAM",    // !


    "T_EOF",
    "T_UNKOWN"
    };
    return names[token - T_IDENT];
}
