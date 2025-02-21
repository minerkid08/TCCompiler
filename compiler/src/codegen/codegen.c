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

void genStatements(const StatementNode* statements, Buffer* buf)
{
	int len = dynList_size(statements);
	for (int i = 0; i < len; i++)
	{
		const StatementNode* statement = statements + i;
		switch (statement->type)
		{
		case StatementTypeVarDef: {
			const StatementNodeVarDef* var = &statement->varDef;
			pushVar(var->name);
			break;
		}
		}
	}
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

void loadVar(Buffer* buf, int i, const char* name)
{
	int idx = getVar(name);
	int len = dynList_size(vars);
	if (idx == len - 1)
		bufferWrite(buf, "load r%d, [sp]\n", i);
	else
		bufferWrite(buf, "sub r%d, sp, %d\nload r%d, [r%d]\n", i, (len - idx - 1) * 2, i, i);
}

Buffer* genCode(const StatementNode* statements)
{
	Buffer* buf = malloc(sizeof(Buffer));
	bufferInit(buf);
	int len = dynList_size(statements);

	vars = dynList_new(0, sizeof(const char*));
	scopeVarCounts = dynList_new(0, sizeof(int));
	functions = dynList_new(0, sizeof(StatementNodeFunc));

	pushScope(buf);

	for (int i = 0; i < len; i++)
	{
		const StatementNode* node = statements + i;
		switch (node->type)
		{
		case StatementTypeVarDef: {
			const StatementNodeVarDef* varDecl = &node->varDef;
			pushVar(varDecl->name);
			if (varDecl->expr)
			{
				if (varDecl->expr->type != ExprTypeNum)
					err("only number exprs are currently supported\n");
				bufferWrite(buf, "mov r1, %d\npush r1 ; %s\n", varDecl->expr->num.val, varDecl->name);
			}
			else
				bufferWrite(buf, "sub sp, sp, 2 ; %s\n", varDecl->name);
			break;
		}
		case StatementTypeFunCall: {
			const StatementNodeFunCall* funCall = &node->funCall;
			for (int j = 0; j < funCall->argc; j++)
			{
				const ExprNode* expr = funCall->argExprs[j];
				if (expr->type == ExprTypeNum)
					bufferWrite(buf, "mov r%d, %d\n", j + 1, expr->num.val);
				else if (expr->type == ExprTypeVar)
					loadVar(buf, j + 1, expr->var.name);
				else
					err("only var an num exprs are supported\n");
			}
      bufferWrite(buf, "call %s\n", funCall->name); 
			break;
		}
		}
	}
	popScope(buf);

	return buf;
}
