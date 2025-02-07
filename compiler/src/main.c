#include "dynList.h"
#include "token.h"
#include "tokenizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FUNCTION_ASM (1 << 0)
#define FUNCTION_INLINE (1 << 1)

typedef struct
{
	int id;
	int type;
	int argc;
	const char* name;
	const char** argNames;
	Token* tokens;
} Function;

#define MODIFER_INLINE (1 << 0)
#define MODIFER_ASM (1 << 1)

#define consumeToken() tokens + ++i

int main(int argc, const char** argv)
{
	if (argc == 0)
	{
		printf("more args pls\n");
		return 1;
	}

	Token* tokens = tokenize(argv[1]);

  Function* functions = dynList_new(0, sizeof(Function));
  dynList_reserve((void**)&functions, 20);

	int tokenC = dynList_size(tokens);

	int modifers = 0;

	for (int i = 0; i < tokenC; i++)
	{
		Token* token = tokens + i;
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "inline") == 0)
				modifers |= MODIFER_INLINE;
			if (strcmp(token->data, "asm") == 0)
				modifers |= MODIFER_ASM;
			if (strcmp(token->data, "function") == 0)
			{
        int funId = dynList_size(functions);
        dynList_resize((void**)&functions, funId + 1);
				Function* fun = functions + funId;
        fun->id = funId;
				fun->type = modifers;

				token = consumeToken();
				if (token->type != TOKEN_LITERAL)
				{
					printf("expected literal\n");
					return 1;
				}

				fun->name = token->data;

				token = consumeToken();
				if (token->type != TOKEN_OPENPARAN)
				{
					printf("expected (\n");
					return 1;
				}

				fun->argc = 0;

				int i2 = 0;
				while (token->type != TOKEN_CLOSEPARAN)
				{
					token = tokens + i + ++i2;
					if (token->type == TOKEN_LITERAL)
						fun->argc++;
				}

				fun->argNames = malloc(sizeof(char*) * fun->argc);

				token = tokens + i;

				i2 = 0;
				while (token->type != TOKEN_CLOSEPARAN)
				{
					token = consumeToken();
					if (token->type == TOKEN_LITERAL)
						fun->argNames[i2++] = token->data;
				}

				token = consumeToken();
				if (token->type == TOKEN_NEWLINE)
				{
					fun->tokens = tokens + i + 1;
				}
				else if (token->type == TOKEN_KEYWORD)
				{
					if (strcmp(token->data, "end") == 0)
						fun->tokens = 0;
				}
				else
				{
					printf("expected end or newline after function definition\n");
					return 1;
				}

				printf("parsing function: %s, %d, %d\n", fun->name, fun->argc, modifers);
				modifers = 0;
			}
		}
		printf("%d, %s\n", tokens[i].type, (char*)tokens[i].data);
	}

	return 0;
}
