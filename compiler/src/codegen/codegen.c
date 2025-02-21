#include "codegen.h"
#include "buffer.h"
#include "dynList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		exit(1);                                                                                                       \
	}

const char** vars;
int* scopeVarCounts;
StatementNodeFunc* functions;

void pushScope(Buffer* buf)
{
	int scopec = dynList_size(scopeVarCounts);
	dynList_resize((void**)&scopeVarCounts, scopec + 1);
	scopeVarCounts[scopec] = 0;
	bufferWrite(buf, "push r13\nmov r13, sp\n");
}

void pushVar(const char* name)
{
	int scopec = dynList_size(scopeVarCounts);
	scopeVarCounts[scopec - 1]++;
	int varc = dynList_size(vars);
	dynList_resize((void**)&vars, varc + 1);
	vars[varc] = name;
}

void popScope(Buffer* buf)
{
	int varc = dynList_size(vars);
	int scopec = dynList_size(scopeVarCounts);
	int newLen = varc - scopeVarCounts[scopec - 1];
	dynList_resize((void**)&vars, newLen);
	bufferWrite(buf, "mov sp, r13\npop r13\n");
}

int getVar(const char* name)
{
	int len = dynList_size(vars);
	for (int i = 0; i < len; i++)
	{
		if (strcmp(vars[i], name) == 0)
			return i;
	}
	err("variable %s not found\n", name);
	return 0;
}

void loadVar(Buffer* buf, int reg, const char* name)
{
	int idx = getVar(name);
	int len = dynList_size(vars);
	if (idx == len - 1)
		bufferWrite(buf, "load r%d, [sp] ; %s\n", reg, name);
	else
		bufferWrite(buf, "sub r%d, sp, %d\nload r%d, [r%d] ; %s\n", reg, (len - idx - 1) * 2, reg, reg, name);
}

void genExpr(Buffer* buf, int reg, const ExprNode* expr)
{
	int len = dynList_size(expr);
	if (len > 1)
		err("currently expressions can only have one token\n");
	if (expr->type == ExprTypeNum)
		bufferWrite(buf, "mov r%d, %d\n", reg, expr->num.val);
	else
		loadVar(buf, reg, expr->var.name);
}

void genStatement(Buffer* buf, const StatementNode* node)
{
	switch (node->type)
	{
	case StatementTypeVarDef: {
		const StatementNodeVarDef* varDecl = &node->varDef;
		pushVar(varDecl->name);
		if (varDecl->expr)
		{
			genExpr(buf, 1, varDecl->expr);
			bufferWrite(buf, "push r1 ; %s\n", varDecl->name);
		}
		else
			bufferWrite(buf, "sub sp, sp, 2 ; %s\n", varDecl->name);
		break;
	}
	case StatementTypeFunCall: {
		const StatementNodeFunCall* funCall = &node->funCall;
		for (int j = 0; j < funCall->argc; j++)
		{
			genExpr(buf, j + 1, funCall->argExprs[j]);
		}
		bufferWrite(buf, "call %s\n", funCall->name);
		break;
	}
	}
}

Buffer* genCode(const StatementNode* statements)
{
	Buffer* buf = malloc(sizeof(Buffer));
	bufferInit(buf);
	int len = dynList_size(statements);

	vars = dynList_new(0, sizeof(const char*));
	scopeVarCounts = dynList_new(0, sizeof(int));
	functions = dynList_new(0, sizeof(StatementNodeFunc));

	for (int i = 0; i < len; i++)
	{
		const StatementNode* node = statements + i;
		if (node->type == StatementTypeFunc)
		{
			const StatementNodeFunc* func = &node->func;
			bufferWrite(buf, "%s:\n", func->name);
			pushScope(buf);
			for (int j = 0; j < func->argc; j++)
				bufferWrite(buf, "push r%d ; %s\n", j + 1, func->argNames[j]);
			int len = dynList_size(func->statements);
			for (int i = 0; i < len; i++)
				genStatement(buf, func->statements + i);
			popScope(buf);
			bufferWrite(buf, "ret ; %s\n", func->name);
		}
	}

	return buf;
}
