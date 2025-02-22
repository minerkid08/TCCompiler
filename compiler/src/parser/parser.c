#include "../token.h"
#include "dynList.h"
#include "parseNodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parseStatement(const Token* tokens, int* i, StatementNode* destNode);
void parseFunction(const Token* tokens, int* i, StatementNode* destNode);
void parseFunctionCall(const Token* tokens, int* i, StatementNode* destNode);
void parseVarDecl(const Token* tokens, int* i, StatementNode* destNode);

ExprNode* parseExpression(const Token* tokens, int* i, int* endTypes, int endTypesLen);

#define consumeToken() (tokens + ++(*i))
#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		exit(1);                                                                                                       \
	}

StatementNode* parse(const Token* tokens)
{
	StatementNode* outNodes = dynList_new(0, sizeof(StatementNode));
	dynList_reserve((void**)&outNodes, 5);
	int nodec = 0;
	int len = dynList_size(tokens);
	for (int i = 0; i < len; i++)
	{
		dynList_resize((void**)&outNodes, nodec + 1);
		const Token* token = tokens + i;
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "inline") == 0 || strcmp(token->data, "asm") == 0 ||
				strcmp(token->data, "function") == 0)
			{
				parseFunction(tokens, &i, outNodes + nodec);
				nodec++;
				continue;
			}
		}
		if (token->type == TOKEN_EOF)
			break;
		nodec += parseStatement(tokens, &i, outNodes + nodec);
	}
	dynList_resize((void**)&outNodes, nodec);
	return outNodes;
}

#define Func_Inline 1
#define Func_Asm 2
#define Func_ForwardDecl 4

char newLineOrEnd(const Token* token)
{
	if (token->type == TOKEN_NEWLINE)
		return 1;
	if (token->type != TOKEN_KEYWORD)
		return 0;
	return strcmp(token->data, "end") == 0;
}

void parseFunction(const Token* tokens, int* i, StatementNode* destNode)
{
	destNode->type = StatementTypeFunc;
	StatementNodeFunc* fun = &destNode->func;
	fun->type = 0;
	const Token* token = tokens + *i;
	if (strcmp(token->data, "inline") == 0)
	{
		fun->type |= Func_Inline;
		token = consumeToken();
	}
	if (strcmp(token->data, "asm") == 0)
	{
		fun->type |= Func_Asm;
		token = consumeToken();
	}
	if (strcmp(token->data, "function"))
		err("expected 'function' after modifiers\n");
	token = consumeToken();
	if (token->type != TOKEN_LITERAL)
		err("expected literal after function keyword\n");

	fun->name = token->data;
	token = consumeToken();

	if (token->type != TOKEN_OPENPARAN)
		err("expected '(' after function name\n");

	token = consumeToken();

	fun->argc = 0;
	if (token->type != TOKEN_CLOSEPARAN)
	{
		int i2 = *i;
		while (1)
		{
			if (token->type != TOKEN_LITERAL)
				err("expected ')' after function argument list\n");
			fun->argc++;
			token = consumeToken();
			if (token->type != TOKEN_COMMA && token->type != TOKEN_CLOSEPARAN)
				err("expected ',' or ')' after function argument\n");
			if (token->type == TOKEN_CLOSEPARAN)
				break;
			token = consumeToken();
		}
		*i = i2;
		i2 = 0;
		token = tokens + *i;
		fun->argNames = malloc(sizeof(char*) * fun->argc);
		while (1)
		{
			if (token->type != TOKEN_LITERAL)
				err("expected ')' after function argument list\n");
			fun->argNames[i2++] = token->data;
			token = consumeToken();
			if (token->type != TOKEN_COMMA && token->type != TOKEN_CLOSEPARAN)
				err("expected ',' or ')' after function argument\n");
			if (token->type == TOKEN_CLOSEPARAN)
				break;
			token = consumeToken();
		}
	}

	token = consumeToken();
	if (!newLineOrEnd(token))
		err("expected newline or end keyword after function definition\n");
	if (token->type == TOKEN_KEYWORD)
	{
		fun->type |= Func_ForwardDecl;
		return;
	}
	else
	{
		fun->statements = dynList_new(0, sizeof(StatementNode));
		dynList_reserve((void**)&fun->statements, 5);
		int nodec = 0;
		while (1)
		{
			dynList_resize((void**)&fun->statements, nodec + 1);
			StatementNode* node = fun->statements + nodec;
			token = consumeToken();
			if (token->type == TOKEN_KEYWORD)
			{
				if (strcmp(token->data, "end") == 0)
					return;
			}
			nodec += parseStatement(tokens, i, node);
		}
	}
}

int parseStatement(const Token* tokens, int* i, StatementNode* destNode)
{
	const Token* token = tokens + *i;
	if (token->type == TOKEN_NEWLINE)
		return 0;
	if (token->type != TOKEN_KEYWORD && token->type != TOKEN_LITERAL)
		err("unexpected token '%s'\n", tokenTypeToStr(token->type));
	if (token->type == TOKEN_KEYWORD)
	{
		if (strcmp(token->data, "local"))
			err("unexpected keyword '%s'\n", token->data);
		parseVarDecl(tokens, i, destNode);
		return 1;
	}
	token = tokens + (*i + 1);
	if (token->type != TOKEN_OPENPARAN)
		err("expected ')' after literal\n");
	parseFunctionCall(tokens, i, destNode);
	return 1;
}

void parseVarDecl(const Token* tokens, int* i, StatementNode* destNode)
{
	const Token* token = consumeToken();
	destNode->type = StatementTypeVarDef;
	if (token->type != TOKEN_LITERAL)
		err("expected literal after local\n");
	StatementNodeVarDef* var = &destNode->varDef;
	var->name = token->data;
	token = consumeToken();
	if (token->type == TOKEN_OPERATOR && ((const char*)token->data)[0] == '=')
	{
		int endTypes[] = {TOKEN_SEMICOLON};
		var->expr = parseExpression(tokens, i, endTypes, 1);
		token = tokens + *i;
	}
	else
	{
		var->expr = 0;
		if (token->type != TOKEN_SEMICOLON)
			err("expected semicolon after var decl\n");
	}
}

void parseFunctionCall(const Token* tokens, int* i, StatementNode* destNode)
{
	const Token* token = tokens + *i;
	destNode->type = StatementTypeFunCall;
	StatementNodeFunCall* funCall = &destNode->funCall;
	funCall->name = (const char*)token->data;
	token = consumeToken();
	token = consumeToken();
	funCall->argc = 0;
	funCall->argExprs = 0;
	if (token->type != TOKEN_CLOSEPARAN)
	{
		int i2 = *i;
		while (1)
		{
			token = consumeToken();
			if (token->type == TOKEN_COMMA)
				funCall->argc++;
			if (token->type == TOKEN_CLOSEPARAN)
				break;
		}
		funCall->argc++;
		*i = i2;
		i2 = 0;
		funCall->argExprs = malloc(sizeof(ExprNode) * funCall->argc);
		int endTypes[] = {TOKEN_COMMA, TOKEN_CLOSEPARAN};
		(*i)--;
		while (1)
		{
			funCall->argExprs[i2++] = parseExpression(tokens, i, endTypes, 2);
			token = tokens + *i;
			if (token->type == TOKEN_CLOSEPARAN)
				break;
		}
	}
	token = consumeToken();
	if (token->type != TOKEN_SEMICOLON)
		err("expected semicolon after function call\n");
}

ExprNode* parseExpression(const Token* tokens, int* i, int* endTypes, int endTypesLen)
{
	const Token* token;
	int size = 0;
	ExprNode* outNodes = dynList_new(0, sizeof(ExprNode*));
	while (1)
	{
		token = consumeToken();
		for (int j = 0; j < endTypesLen; j++)
		{
			if (token->type == endTypes[j])
				return outNodes;
		}
		size++;
		dynList_resize((void**)&outNodes, size);
		ExprNode* exprNode = outNodes + (size - 1);
		if (token->type == TOKEN_NUMBER)
		{
			exprNode->type = ExprTypeNum;
			exprNode->num.val = atoi(token->data);
		}
		else if (token->type == TOKEN_LITERAL)
		{
			exprNode->type = ExprTypeVar;
			exprNode->var.name = token->data;
		}
		else if (token->type == TOKEN_OPERATOR)
		{
			exprNode->type = ExprTypeOpr;
			exprNode->opr.opr = ((char*)token->data)[0];
		}
		else
			err("invalid token in expression\n");
	}
	return 0;
}
