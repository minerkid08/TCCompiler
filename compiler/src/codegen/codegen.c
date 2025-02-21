#include "codegen.h"
#include "buffer.h"
#include "dynList.h"
#include <stdio.h>
#include <stdlib.h>

#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		exit(1);                                                                                                       \
	}

const char** vars;
int* scopeVarCounts;
StatementNodeFunc* functions;

void pushScope()
{
	int scopec = dynList_size(scopeVarCounts);
	dynList_resize((void**)&scopec, scopec + 1);
	scopeVarCounts[scopec] = 0;
}

void pushVar(const char* name)
{
	int scopec = dynList_size(scopeVarCounts);
	scopeVarCounts[scopec - 1]++;
	int varc = dynList_size(vars);
	dynList_resize((void**)&vars, varc + 1);
	vars[varc] = name;
}

void popScope()
{
	int varc = dynList_size(vars);
	int scopec = dynList_size(scopeVarCounts);
	int newLen = varc - scopeVarCounts[scopec - 1];
	dynList_resize((void**)&vars, newLen);
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
		if (node->type != StatementTypeFunc)
			err("only function definitions can be top level\n");
		const StatementNodeFunc* func = &node->func;
		bufferWrite(buf, "%s:\n", func->name);
		pushScope();
		genStatements(func->statements, buf);
		popScope();
	}

	return buf;
}
