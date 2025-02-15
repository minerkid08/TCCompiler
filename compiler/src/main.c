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
	char* data;
} Function;

typedef struct
{
	const char* name;
} Variable;

#define MODIFER_INLINE FUNCTION_INLINE
#define MODIFER_ASM FUNCTION_ASM

#define consumeToken() tokens + ++(*i)

Token* tokens;
Function* functions;
Variable* vars;
int* scopeVarCount;

int modifiers = 0;

char* out;
int outLen = 0;
int outMaxLen = 512;

#define writeStr(fmt, args...) *dataLen += snprintf(data + *dataLen, *maxDataLen - *dataLen, fmt, args)

int varIndex(const char* name)
{
	int len = dynList_size(vars);
	for (int i = 0; i < len; i++)
	{
		if (strcmp(vars[i].name, name) == 0)
			return i;
	}
	printf("var %s does not exist\n", name);
	exit(1);
	return 0;
}

void parseStatement(int* i, char* data, int* dataLen, int* maxDataLen);

void parseFunction(int* i)
{
	dynList_resize((void**)&vars, 0);
	dynList_resize((void**)&scopeVarCount, 1);
	scopeVarCount[0] = 0;

	int funId = dynList_size(functions);
	dynList_resize((void**)&functions, funId + 1);
	Function* fun = functions + funId;
	fun->id = funId;
	fun->type = modifiers;

	Token* token = consumeToken();
	if (token->type != TOKEN_LITERAL)
	{
		printf("expected literal\n");
		exit(1);
	}

	fun->name = token->data;

	token = consumeToken();
	if (token->type != TOKEN_OPENPARAN)
	{
		printf("expected (\n");
		exit(1);
	}

	fun->argc = 0;

	int i2 = 0;
	while (token->type != TOKEN_CLOSEPARAN)
	{
		token = tokens + *i + ++i2;
		if (token->type == TOKEN_LITERAL)
			fun->argc++;
	}

	fun->argNames = malloc(sizeof(char*) * fun->argc);

	token = tokens + *i;

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
		fun->data = malloc(512);
	}
	else if (token->type == TOKEN_KEYWORD)
	{
		if (strcmp(token->data, "end") == 0)
			fun->data = 0;
	}
	else
	{
		printf("expected end or newline after function definition\n");
		exit(1);
	}

	if (fun->data)
	{
		int dataMaxLen = 512;
		int dataLen = 0;
		char running = 1;
		while (running)
		{
			token = consumeToken();
			if (token->type == TOKEN_KEYWORD)
			{
				if (strcmp(token->data, "end") == 0)
				{
					running = 0;
					break;
				}
			}
			if (fun->type & FUNCTION_ASM)
			{
				if (strcmp(token->data, "$") == 0)
				{
					token = consumeToken();
					char found = 0;
					for (int j = 0; j < fun->argc; j++)
					{
						if (strcmp(token->data, fun->argNames[j]) == 0)
						{
							found = 1;
							dataLen += snprintf(fun->data + dataLen, dataMaxLen - dataLen, "r%d ", j + 1);
							break;
						}
					}
					if (!found)
					{
						printf("unknown asm variable '%s'\n", (char*)token->data);
						exit(1);
					}
				}
				else if (token->type == TOKEN_NEWLINE)
				{
					dataLen += snprintf(fun->data + dataLen, dataMaxLen - dataLen, "\n");
				}
				else if (token->type == TOKEN_COMMA)
				{
					dataLen -= 1;
					dataLen += snprintf(fun->data + dataLen, dataMaxLen - dataLen, ", ");
				}
				else
				{
					dataLen += snprintf(fun->data + dataLen, dataMaxLen - dataLen, "%s ", (char*)token->data);
				}
			}
			else
			{
				parseStatement(i, fun->data, &dataLen, &dataMaxLen);
			}
		}

		if (!(fun->type & FUNCTION_INLINE))
			outLen += snprintf(out + outLen, outMaxLen - outLen, "%s:\n%sret\n\n", fun->name, fun->data);
	}

	modifiers = 0;
}

void parseFunctionCall(int* i, char* data, int* dataLen, int* maxDataLen)
{
	const char* name = tokens[*i].data;
	Function* fun = 0;
	char found = 0;
	for (int i2 = 0; i2 < dynList_size(functions); i2++)
	{
		fun = functions + i2;
		if (strcmp(fun->name, name) == 0)
		{
			found = 1;
			break;
		}
	}
	if (!found)
	{
		printf("cant find function \'%s\'\n", name);
		exit(1);
	}
	int i2 = *i;
	int argc = 0;
	Token* token = consumeToken();
	while (token->type != TOKEN_CLOSEPARAN)
	{
		if (token->type == TOKEN_LITERAL || token->type == TOKEN_NUMBER)
			argc++;
		token = consumeToken();
	}
	const Token** args = malloc(argc * sizeof(const Token**));
	*i = i2;
	i2 = 0;
	token = consumeToken();
	while (token->type != TOKEN_CLOSEPARAN)
	{
		if (token->type == TOKEN_LITERAL || token->type == TOKEN_NUMBER)
			args[i2++] = token;
		token = consumeToken();
	}
	if (argc != fun->argc)
	{
		printf("not good number of args\n");
		exit(1);
	}

	for (i2 = 0; i2 < argc; i2++)
	{
		if (args[i2]->type == TOKEN_NUMBER)
			writeStr("mov r%d, %s\n", i2 + 1, (char*)args[i2]->data);
		else if (args[i2]->type == TOKEN_LITERAL)
		{
			int ind = varIndex(args[i2]->data);
			if (ind != 0)
				writeStr("sub r13, sp, %d\nload r%d [r13]\n", ind, i2 + 1);
			else
				writeStr("load r%d [sp]\n", i2 + 1);
		}
	}
	if (fun->type & FUNCTION_INLINE)
		writeStr("%s", fun->data);
	else
		writeStr("call %s\n", name);
	*i = *i + 2;
	free(args);
}

void parseVarDecl(int* i, char* data, int* dataLen, int* maxDataLen)
{
	Token* token = consumeToken();
	const char* name = token->data;
	token = consumeToken();

	int varLen = dynList_size(vars);
	dynList_resize((void**)&vars, varLen + 1);

	Variable* var = vars + varLen;
	var->name = name;

	int scopeVarLen = dynList_size(scopeVarCount);
	scopeVarCount[scopeVarLen - 1]++;

	if (token->type == TOKEN_NEWLINE || token->type == TOKEN_SEMICOLON)
	{
		*dataLen += snprintf(data + *dataLen, *maxDataLen - *dataLen, "sub sp, sp, 2\n");
		return;
	}
	if (strcmp(token->data, "="))
	{
		printf("expected '=' after var deceleration\n");
		exit(1);
	}
	token = consumeToken();
	*dataLen += snprintf(data + *dataLen, *maxDataLen - *dataLen, "push %s\n", (char*)token->data);
}

void parseStatement(int* i, char* data, int* dataLen, int* maxDataLen)
{
	Token* token = tokens + *i;
	if (token->type == TOKEN_LITERAL)
	{
		token = tokens + *i + 1;

		if (token->type == TOKEN_OPENPARAN)
		{
			parseFunctionCall(i, data, dataLen, maxDataLen);
			return;
		}
	}
	if (token->type == TOKEN_KEYWORD)
	{
		if (strcmp(token->data, "local") == 0)
		{
			parseVarDecl(i, data, dataLen, maxDataLen);
			return;
		}
	}
}

int main(int argc, const char** argv)
{
	if (argc == 0)
	{
		printf("more args pls\n");
		return 1;
	}

	tokens = tokenize(argv[1]);

	functions = dynList_new(0, sizeof(Function));
	dynList_reserve((void**)&functions, 20);

	vars = dynList_new(0, sizeof(Variable));
	dynList_reserve((void**)&vars, 5);

	scopeVarCount = dynList_new(1, sizeof(int));
	dynList_reserve((void**)&scopeVarCount, 5);

	int tokenC = dynList_size(tokens);

	out = malloc(512);

	for (int i = 0; i < tokenC; i++)
	{
		Token* token = tokens + i;
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "inline") == 0)
			{
				modifiers |= MODIFER_INLINE;
				continue;
			}
			else if (strcmp(token->data, "asm") == 0)
			{
				modifiers |= MODIFER_ASM;
				continue;
			}
			else if (strcmp(token->data, "function") == 0)
			{
				parseFunction(&i);
				continue;
			}
		}
		parseStatement(&i, out, &outLen, &outMaxLen);
	}
	printf("%s\n", out);

	return 0;
}
