#include "../token.h"
#include "dynList.h"
#include "parseNodes.h"
#include <string.h>

void parseFunction(Token* tokens, int* i, StatementNode* destNode);

StatementNode* parse(Token* tokens)
{
  StatementNode* outNodes = dynList_new(0, sizeof(StatementNode));
  dynList_reserve((void**)&outNodes, 5);
	int nodec = 0;
  int len = dynList_size(tokens);
	for (int i = 0; i < len; i++)
	{
    nodec++;
    dynList_resize((void**)&outNodes, nodec);
		Token* token = tokens + i;
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "inline") == 0 || strcmp(token->data, "asm") == 0 ||
				strcmp(token->data, "function") == 0)
			{
				parseFunction(tokens, &i, outNodes + nodec);
			}
		}
	}
	return 0;
}

void parseFunction(Token* tokens, int* i, StatementNode* destNode)
{

}
