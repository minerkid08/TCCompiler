#include "codegen.h"
#include "buffer.h"
#include "codegenExpr.h"
#include "dynList.h"
#include "vars.h"
#include <stdlib.h>
#include <string.h>

#include "parser/parseNodes.h"
#include "utils.h"

StatementNodeFunc* functions;

const char* funName;
int ifc;

void pushFunction(const StatementNodeFunc* func)
{
	int len = dynList_size(functions);
	for (int i = 0; i < len; i++)
	{
		StatementNodeFunc* func2 = functions + i;
		if (strcmp(func2->name, func->name) == 0)
		{
			if (func2->argc != func->argc)
				err("attempted to redefine '%s' with a different signature\n", func->name);
			return;
		}
	}
	dynList_resize((void**)&functions, len + 1);
	StatementNodeFunc* func2 = functions + len;
	func2->type = func->type & 3;
	func2->name = func->name;
	func2->argc = func->argc;
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
	case StatementTypeVarAssign: {
		const StatementNodeVarAssign* varDecl = &node->varAssing;
		genExpr(buf, 1, varDecl->expr);
		setVar(buf, 1, varDecl->name);
		break;
	}
	case StatementTypeFunCall: {
		const StatementNodeFunCall* funCall = &node->funCall;
		int funLen = dynList_size(functions);
		char exists = 0;
		for (int i = 0; i < funLen; i++)
		{
			const StatementNodeFunc* func = functions + i;
			if (strcmp(func->name, funCall->name) == 0)
			{
				if (func->argc != funCall->argc)
					err("attempted to call '%s' with an incorrect number of args\n", funCall->name);
				exists = 1;
				break;
			}
		}
		if (!exists)
			err("attempted to call '%s' without it existing\n", funCall->name);
		for (int j = 0; j < funCall->argc; j++)
		{
			genExpr(buf, j + 1, funCall->argExprs[j]);
		}
		bufferWrite(buf, "call %s\n", funCall->name);
		clearRegs();
		if (funCall->rtnVar)
			setVar(buf, 1, funCall->rtnVar);
		break;
	}
	case StatementTypeIf: {
		const StatementNodeIf* condIf = &node->condIf;
		genExpr(buf, 1, condIf->expr);
		bufferWrite(buf, "cmp r1, 0\nje %sIf%d\n", funName, ifc);
		pushScope(buf);
		int len = dynList_size(condIf->statments);
		for (int i = 0; i < len; i++)
			genStatement(buf, condIf->statments + i);
		popScope(buf);
		if (condIf->next.type == StatementTypeElse)
			bufferWrite(buf, "jmp %sElse%d\n", funName, ifc);
		bufferWrite(buf, "%sIf%d:\n", funName, ifc);

		if (condIf->next.type == StatementTypeElse)
		{
			pushScope(buf);
			int len = dynList_size(condIf->next.elsev.statments);
			for (int i = 0; i < len; i++)
				genStatement(buf, condIf->next.elsev.statments + i);
			bufferWrite(buf, "%sElse%d:\n", funName, ifc);
		}
		ifc++;
		break;
	}
	case StatementTypeWhile: {
		const StatementNodeWhile* loopWhile = &node->loopWhile;
		int c = ifc++;
		bufferWrite(buf, "%sWhile%d:\n", funName, c);
		clearRegs();
		genExpr(buf, 1, loopWhile->expr);
		bufferWrite(buf, "cmp r1, 0\nje %sWhileEnd%d\n", funName, c);
		pushScope(buf);
		int len = dynList_size(loopWhile->statments);
		for (int i = 0; i < len; i++)
			genStatement(buf, loopWhile->statments + i);
		popScope(buf);
		bufferWrite(buf, "jmp %sWhile%d\n", funName, c);
		bufferWrite(buf, "%sWhileEnd%d:\n", funName, c);
		clearRegs();
		break;
	}
	case StatementTypeReturn: {
		if (node->ret.expr)
		{
			genExpr(buf, 1, node->ret.expr);
		}
		popScopeRtn(buf);
		bufferWrite(buf, "ret\n");
		break;
	}
	}
}

Buffer* genCode(const StatementNode* statements)
{
	Buffer* buf = malloc(sizeof(Buffer));
	bufferInit(buf);
	int len = dynList_size(statements);

	initVars();

	functions = dynList_new(0, sizeof(StatementNodeFunc));

	for (int i = 0; i < len; i++)
	{
		const StatementNode* node = statements + i;
		if (node->type == StatementTypeFunc)
		{
			const StatementNodeFunc* func = &node->func;
			pushFunction(func);
			if (!(func->type & Func_ForwardDecl))
			{
				funName = func->name;
				ifc = 0;
				bufferWrite(buf, "%s:\n", func->name);
				pushScope(buf);
				for (int j = 0; j < func->argc; j++)
				{
					pushVar(func->argNames[j]);
					bufferWrite(buf, "push r%d ; %s\n", j + 1, func->argNames[j]);
				}
				int len = dynList_size(func->statements);
				for (int i = 0; i < len; i++)
					genStatement(buf, func->statements + i);
				popScope(buf);
				bufferWrite(buf, "ret ; %s\n\n", func->name);
				clearRegs();
			}
		}
	}

	return buf;
}
