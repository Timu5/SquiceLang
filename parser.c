#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

extern char buffer[255];
extern int number;

int lasttoken;

int nexttoken()
{
	lasttoken = gettoken();
	//printf("Token: %s \n", tokenstr(lasttoken));
	return lasttoken;
}

void match(int token)
{
	if(lasttoken != token)
	{
		printf("Unexpexted token, expect %s got %s\n", tokenstr(token), tokenstr(lasttoken));
		exit(-1);
	}
}

void match2(int token, int token2)
{
	if(lasttoken != token && lasttoken != token2)
	{
		printf("Unexpexted token, expect %s or %s got %s\n", tokenstr(token), tokenstr(token2), tokenstr(lasttoken));
		exit(-1);
	}
}

node_t* expr();

// primary :=  ident | number | '(' expr ')'
node_t* primary()
{
	node_t* prim = NULL;
	if(lasttoken == T_NUMBER)
	{
		prim = node_value(value_number((float)number));
	}	
	else if(lasttoken == T_IDENT)
	{
		prim = node_ident((char*)strdup(buffer));
	}
	else if(lasttoken == T_LPAREN)
	{
		nexttoken();
		prim = expr(0);
		match(T_RPAREN);
	}
	else
	{
		printf("Unexpexted token in primary!\n");
		//error, unexpected token
	}
	nexttoken();
	return prim;
}

// expr := primary |  expr OP expr
node_t* expr(int min)
{
	int pre[] = {
			4, // +
			4, // -
			3, // /
			3, // *
			14, // =
			7, // ==
			7, // != 
			6, // <=
			6, // >=
			6, // <
			6, // >
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

node_t* statment()
{
	switch(lasttoken)
	{
	case T_LBRACE:
		//parse block
		match(T_RBRACE);
		break;
	case T_LET:
		nexttoken();
		match(T_IDENT);	
		char* name = strdup(buffer);
		
		nexttoken();
		match(T_ASSIGN);
		nexttoken();
		
		node_t* exp = expr(0);
		match(T_SEMICOLON);

		return node_decl(node_ident(name), exp);
	case T_IF:
		nexttoken();
		match(T_LPAREN);

		nexttoken();
		node_t* arg = expr(0);
		match(T_RPAREN);

		nexttoken();
		node_t* body = statment();
		
		return node_cond(arg, body);
	case T_WHILE:
		nexttoken();
		match(T_LPAREN);

		nexttoken();
		node_t* arg2 = expr(0);
		match(T_RPAREN);

		nexttoken();
		node_t* body2 = statment();
		
		return node_loop(arg2, body2);
	default:;
		node_t* e = expr(0);
		match(T_SEMICOLON);
		return e;
	}
}

node_t* parse()
{
	nexttoken();
	node_list_t* list = NULL;
	node_list_t* last = NULL;
	while(lasttoken != T_EOF)
	{
		node_t* n = statment();
		nexttoken();
		
		if(list == NULL)
		{
			list = (node_list_t*)malloc(sizeof(node_list_t));
			list->el = n;
			list->next = NULL;
			last = list;
		}
		else
		{	
			node_list_t* tmp = last;
			last = (node_list_t*)malloc(sizeof(node_list_t));
			last->el = n;
			last->next = NULL;
			tmp->next = last;
		}
	}
	return node_block(list);
}
