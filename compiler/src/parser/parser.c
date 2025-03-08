#include "dynList.h"
#include "parseNodes.h"
#include "token.h"
#include "utils.h"
#include <string.h>

StatementNode* parseBlock(const Token* tokens, int* i);
int parseStatement(const Token* tokens, int* i, StatementNode* destNode);
void parseFunction(const Token* tokens, int* i, StatementNode* destNode);
void parseFunctionCall(const Token* tokens, int* i, StatementNode* destNode);
void parseVarDecl(const Token* tokens, int* i, StatementNode* destNode);
void parseVarAssign(const Token* tokens, int* i, StatementNode* destNode);
void parseIf(const Token* tokens, int* i, StatementNode* destNode);
void parseWhile(const Token* tokens, int* i, StatementNode* destNode);
void parseReturn(const Token* tokens, int* i, StatementNode* destNode);

ExprNode* parseExpression(const Token* tokens, int* i, int* endTypes, int endTypesLen);

#define consumeToken() (tokens + ++(*i))

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
			StatementNode* node = dynList_get(fun->statements, nodec);
			token = consumeToken();
			if (token->type == TOKEN_KEYWORD)
			{
				if (strcmp(token->data, "end") == 0)
				{
					dynList_resize((void**)&fun->statements, nodec);
					return;
				}
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
		if (strcmp(token->data, "local") == 0)
			parseVarDecl(tokens, i, destNode);
		else if (strcmp(token->data, "if") == 0)
			parseIf(tokens, i, destNode);
		else if (strcmp(token->data, "return") == 0)
			parseReturn(tokens, i, destNode);
		else if (strcmp(token->data, "while") == 0)
			parseWhile(tokens, i, destNode);
		else
			err("unexpected keyword '%s'\n", token->data);
		return 1;
	}
	token = tokens + (*i + 1);
	if (token->type == TOKEN_OPENPARAN)
		parseFunctionCall(tokens, i, destNode);
	else
		parseVarAssign(tokens, i, destNode);
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
	if (token->type == TOKEN_OPERATOR && (token->data)[0] == OPERATOR_ASSIGN)
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

void parseVarAssign(const Token* tokens, int* i, StatementNode* destNode)
{
	const char* name = (tokens + *i)->data;
	const Token* token = consumeToken();
	destNode->type = StatementTypeVarAssign;
	if (token->type != TOKEN_OPERATOR || token->data[0] != OPERATOR_ASSIGN)
		err("expected '=' after var name\n");
	StatementNodeVarAssign* var = &destNode->varAssing;
	var->name = name;
	int endTypes[] = {TOKEN_SEMICOLON};
	var->expr = parseExpression(tokens, i, endTypes, 1);
	token = tokens + *i;
	if (token->type != TOKEN_SEMICOLON)
		err("expected semicolon after var assign\n");
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
	funCall->rtnVar = 0;
	token = consumeToken();
	if (token->type == TOKEN_OPERATOR)
	{
		if (token->data[0] != OPERATOR_RTN)
			err("invalid operator after function call\n");
		token = consumeToken();
		if (token->type != TOKEN_LITERAL)
			err("expected literal after return operator\n");
		funCall->rtnVar = token->data;
		token = consumeToken();
	}
	if (token->type != TOKEN_SEMICOLON)
		err("expected semicolon after function call\n");
}

ExprNode* parseExpression(const Token* tokens, int* i, int* endTypes, int endTypesLen)
{
	const Token* token;
	int size = 0;
	ExprNode* outNodes = dynList_new(0, sizeof(ExprNode*));
	dynList_reserve((void**)&outNodes, 10);
	while (1)
	{
		token = consumeToken();
		for (int j = 0; j < endTypesLen; j++)
		{
			if (token->type == endTypes[j])
				return outNodes;
		}
		dynList_resize((void**)&outNodes, size + 1);
		ExprNode* exprNode = outNodes + size;
		if (token->type == TOKEN_NUMBER)
		{
			exprNode->type = ExprTypeNum;
			exprNode->num.val = atoi(token->data);
		}
		if (token->type == TOKEN_LITERAL)
		{
			exprNode->type = ExprTypeVar;
			exprNode->var.name = token->data;
		}
		if (token->type == TOKEN_OPERATOR)
		{
			exprNode->type = ExprTypeOpr;
			exprNode->opr.opr = token->data[0];
		}
		size++;
	}
	return 0;
}

void parseWhile(const Token* tokens, int* i, StatementNode* destNode)
{
	destNode->type = StatementTypeWhile;
	const Token* token = consumeToken();
	if (token->type != TOKEN_OPENPARAN)
		err("expected '(' after 'while' keyword\n");
	int endTypes[] = {TOKEN_CLOSEPARAN};
	destNode->loopWhile.expr = parseExpression(tokens, i, endTypes, 1);
	token = consumeToken();
	if (strcmp(token->data, "do"))
		err("expected then after while condition\n");

	StatementNode* statements = dynList_new(0, sizeof(StatementNode));
	dynList_reserve((void**)&statements, 5);
	int nodec = 0;
	while (1)
	{
		token = consumeToken();
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "end") == 0)
			{
				dynList_resize((void**)&statements, nodec);
				destNode->loopWhile.statments = statements;
				return;
			}
		}
		dynList_resize((void**)&statements, nodec + 1);
		StatementNode* node = statements + nodec;
		nodec += parseStatement(tokens, i, node);
	}
}

void parseIf(const Token* tokens, int* i, StatementNode* destNode)
{
	destNode->type = StatementTypeIf;
	const Token* token = consumeToken();
	if (token->type != TOKEN_OPENPARAN)
		err("expected '(' after 'if' keyword\n");
	int endTypes[] = {TOKEN_CLOSEPARAN};
	destNode->condIf.expr = parseExpression(tokens, i, endTypes, 1);
	token = consumeToken();
	if (strcmp(token->data, "then"))
		err("expected then after if condition\n");

	StatementNode* statements = dynList_new(0, sizeof(StatementNode));
	dynList_reserve((void**)&statements, 5);
	int nodec = 0;
	while (1)
	{
		token = consumeToken();
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "end") == 0)
			{
				dynList_resize((void**)&statements, nodec);
				destNode->condIf.statments = statements;
        destNode->condIf.next.type = 0;
				return;
			}
			if (strcmp(token->data, "else") == 0)
			{
				dynList_resize((void**)&statements, nodec);
				destNode->condIf.statments = statements;
        destNode->condIf.next.type = StatementTypeElse;
        destNode->condIf.next.elsev.statments = parseBlock(tokens, i);
				return;
			}
		}
		dynList_resize((void**)&statements, nodec + 1);
		StatementNode* node = statements + nodec;
		nodec += parseStatement(tokens, i, node);
	}
}

void parseReturn(const Token* tokens, int* i, StatementNode* destNode)
{
	destNode->type = StatementTypeReturn;
	destNode->ret.expr = 0;
	const Token* token = consumeToken();
	if (token->type == TOKEN_SEMICOLON)
		return;
	int endTypes[] = {TOKEN_SEMICOLON};
	(*i)--;
	destNode->ret.expr = parseExpression(tokens, i, endTypes, 1);
}

StatementNode* parseBlock(const Token* tokens, int* i)
{
	StatementNode* statements = dynList_new(0, sizeof(StatementNode));
	dynList_reserve((void**)&statements, 5);
  const Token* token;
	int nodec = 0;
	while (1)
	{
		token = consumeToken();
		if (token->type == TOKEN_KEYWORD)
		{
			if (strcmp(token->data, "end") == 0)
			{
				dynList_resize((void**)&statements, nodec);
				return statements;
			}
		}
		dynList_resize((void**)&statements, nodec + 1);
		StatementNode* node = statements + nodec;
		nodec += parseStatement(tokens, i, node);
	}
}
