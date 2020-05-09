#ifndef _LEXER_H_
#define _LEXER_H_

enum SL_TOKEN
{
    SL_TOKEN_IDENT,
    SL_TOKEN_NUMBER,
    SL_TOKEN_STRING,

    //keyword
    SL_TOKEN__FN,
    SL_TOKEN_RETURN,
    SL_TOKEN_LET,
    SL_TOKEN_IF,
    SL_TOKEN_ELSE,
    SL_TOKEN_WHILE,
    SL_TOKEN_BREAK,

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

int sl_gettoken();
char *sl_tokenstr(int token);
#endif
