#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "value.h"
#include "ast.h"
#include "parser.h"
#include "stack.h"

extern FILE* input;

void quit()
{
	fclose(input);
}


int main(int argc, char ** argv)
{
	if(argc < 2)
	{
		printf("Usage: lang input\n");
		return -1;
	}

	input = fopen(argv[1], "r");

	if(!input)
	{
		printf("Cannot open file.\n");
		return -2;
	}
	atexit(quit);
	
	node_t* tree = parse();
	tree->eval(tree, NULL);
	node_list_t* first = tree->block;
	
	while(first != NULL)
	{
		printf("%d ", first->el->type);
		first = first->next;
	}
	return 0;
}
