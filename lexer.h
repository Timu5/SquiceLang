#ifndef _LEXER_H_
#define _LEXER_H_

enum {
	T_IDENT, 
	T_NUMBER,
	T_STRING,
	
	//keyword
	T_FN,
	T_RETURN,
	T_LET,
	T_IF,
	T_ELSE,
	T_WHILE,

	T_SEMICOLON, // ;
	T_COMMA,     // ,
	
	T_PLUS,      // +
	T_MINUS,     // -
	T_SLASH,     // /
	T_ASTERISK,  // *
	
	T_ASSIGN,    // =
	T_EQUAL,     // ==
	T_NOTEQUAL,  // !=
	T_LESSEQUAL, // <=
	T_MOREEQUAL, // >=
	T_LCHEVR,    // <
	T_RCHEVR,    // >

	T_LPAREN,    // (
	T_RPAREN,    // )
	T_LBRACE,    // {
	T_RBRACE,    // }
	T_LBRACK,    // [
	T_RBRACK,    // ]
	T_EXCLAM,    // !


	T_EOF,
	T_UNKOWN
};

int gettoken();
char* tokenstr(int token);
#endif 
