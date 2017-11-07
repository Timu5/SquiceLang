#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "stack.h"

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

// primary :=  ident | number | string | function | '(' expr ')'
node_t* primary()
{
	node_t* prim = NULL;
	if(lasttoken == T_NUMBER)
	{
		prim = node_value(value_number((float)number));
	}
	else if(lasttoken == T_STRING)
	{
		prim = node_value(value_string(strdup(buffer)));
	}
	else if(lasttoken == T_IDENT)
	{
		char* name = strdup(buffer);
		if(nexttoken() == T_LPAREN)
		{
			// parser fn call
			node_list_t* args = NULL;
			nexttoken();
			while(lasttoken != T_RPAREN)
			{
				node_t* e = expr(0);
				node_list_t* tmp = (node_list_t*)malloc(sizeof(node_list_t));
				tmp->el = e;
				tmp->next  = NULL;
				if(args == NULL)
				{
					args = tmp;
				}
				else
				{
					node_list_t* last = args;
					while(last->next) last = last->next;
					last->next = tmp;
				}
				if(lasttoken != T_COMMA)
					break;
				nexttoken();	
			}
			match(T_RPAREN);
			nexttoken();
			return node_call(name, args);
		}
		else
		{
			// ident
			return node_ident(name);
		}
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
		exit(-7);
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

node_t* statment()
{
	switch(lasttoken)
	{
	case T_LBRACE:
		nexttoken();
		
		node_list_t* list = NULL;
		while(lasttoken != T_RBRACE)
		{
			node_t* node = statment();
			//nexttoken();
			node_list_t* tmp = (node_list_t*)malloc(sizeof(node_list_t));
			tmp->el = node;
			tmp->next = NULL;
			if(!list)
			{
				list = tmp;
			}
			else
			{
				node_list_t* last = list;
				while(last->next) last = last->next;
				last->next = tmp;
			}
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
	
		stack_t* args = stack_new();
		
		while(nexttoken() != T_RPAREN)
		{
			match(T_IDENT);
			stack_push(args, strdup(buffer));
			if(nexttoken() != T_COMMA)
				break;
		}
		
		char** argv = args->data;
		int argc = args->used;
		free(args);
		
		match(T_RPAREN);
		nexttoken();

		node_t* fnbody = statment();

		return node_func(fnname, argc, argv, fnbody);
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
	node_list_t* funcs = NULL;
	node_list_t* stmts = NULL;
	while(lasttoken != T_EOF)
	{
		node_t* n = statment();
	
		node_list_t* tmp = (node_list_t*)malloc(sizeof(node_list_t));
		tmp->el = n;
		tmp->next = NULL;

		if(n->type == N_FUNC)
		{
			node_list_t* old = funcs;
			funcs = tmp;
			tmp->next = old;
		}
		else
		{
			if(!stmts)
			{
				stmts = tmp;
			}
			else
			{
				node_list_t* s = stmts;
				while(s->next) s = s->next;
				s->next = tmp;
			} 
		}
	}
	return node_root(funcs, stmts);
}
