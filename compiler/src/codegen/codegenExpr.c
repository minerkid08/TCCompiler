#include "codegenExpr.h"
#include "buffer.h"
#include "codegen.h"
#include "codegen/vars.h"
#include "parser/parseNodes.h"
#include "utils.h"

#include "dynList.h"
#include "token.h"

#include <stdio.h>

#define ExprTypeTmpVar 4

extern const char* funName;
extern int ifc;

void printStack(const ExprNode** expr, const char* indent)
{
	int len = dynList_size(expr);
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = expr[i];
		switch (node->type)
		{
		case ExprTypeExpr:
			break;
		case ExprTypeVar:
			printf("%s  var: %s\n", indent, node->var.name);
			break;
		case ExprTypeNum:
			printf("%s  num: %d\n", indent, node->num.val);
			break;
		case ExprTypeOpr:
			printf("%s  opr: %c\n", indent, node->opr.opr);
			break;
		}
	}
}

int getPrec(char c)
{
	switch (c)
	{
	case OPERATOR_ADD:
		return 3;
	case OPERATOR_SUB:
		return 2;
	case OPERATOR_MUL:
		return 5;
	case OPERATOR_DIV:
		return 4;
	case OPERATOR_SHL:
		return 1;
	case OPERATOR_SHR:
		return 1;
	case OPERATOR_AND:
		return 0;
	case OPERATOR_OR:
		return 0;
	case OPERATOR_XOR:
		return 0;

	case OPERATOR_EQ:
		return -1;
	case OPERATOR_NEQ:
		return -1;
	case OPERATOR_LT:
		return -1;
	case OPERATOR_GT:
		return -1;
	case OPERATOR_LTE:
		return -1;
	case OPERATOR_GTE:
		return -1;
	}
	printf("invalid operator '%c'\n", c);
	fflush(stdout);
	exit(1);
	return -1;
}

void genExpr(Buffer* buf, int reg, const ExprNode* expr)
{
	int len = dynList_size(expr);
	const ExprNode** stack1 = dynList_new(0, sizeof(ExprNode*));
	const ExprNode** stack2 = dynList_new(0, sizeof(ExprNode*));

	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = expr + i;
		if (node->type == ExprTypeOpr)
		{
			int l = dynList_size(stack1);
			if (l)
			{
				int prec = getPrec(node->opr.opr);
				while (l)
				{
					const ExprNode* prev = stack1[--l];
					if (prec < getPrec(prev->opr.opr))
					{
						dynList_resize((void**)&stack1, l);
						int l2 = dynList_size(stack2);
						dynList_resize((void**)&stack2, l2 + 1);
						stack2[l2] = prev;
					}
					else
						break;
				}
			}
			l = dynList_size(stack1);
			dynList_resize((void**)&stack1, l + 1);
			stack1[l] = node;
		}
		else
		{
			int l = dynList_size(stack2);
			dynList_resize((void**)&stack2, l + 1);
			stack2[l] = node;
		}
	}

	len = dynList_size(stack1);
	int l = dynList_size(stack2);
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = stack1[len - 1 - i];
		dynList_resize((void**)&stack2, l + i + 1);
		stack2[l + i] = node;
	}

	// printStack(stack2, "  ");
	dynList_resize((void**)&stack1, 0);

	int tmpVarCount = 0;
	int prevTmpVar = -1;

	len = dynList_size(stack2);
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = stack2[i];

		if (node->type == ExprTypeNum || node->type == ExprTypeVar || node->type == ExprTypeTmpVar)
		{
			int l2 = dynList_size(stack1);
			dynList_resize((void**)&stack1, l2 + 1);
			stack1[l2] = node;
		}
		if (node->type == ExprTypeOpr)
		{
			int l2 = dynList_size(stack1);
			const ExprNode* a1 = stack1[l2 - 2];
			const ExprNode* a2 = stack1[l2 - 1];
			ExprNode* res = malloc(sizeof(ExprNode));
			if (a1->type == ExprTypeNum && a2->type == ExprTypeNum)
			{
				res->type = ExprTypeNum;
				switch (node->opr.opr)
				{
				case OPERATOR_ADD:
					res->num.val = a1->num.val + a2->num.val;
					break;
				case OPERATOR_SUB:
					res->num.val = a1->num.val - a2->num.val;
					break;
				case OPERATOR_MUL:
					res->num.val = a1->num.val * a2->num.val;
					break;
				case OPERATOR_DIV:
					res->num.val = a1->num.val / a2->num.val;
					break;
				case OPERATOR_SHL:
					res->num.val = a1->num.val << a2->num.val;
					break;
				case OPERATOR_SHR:
					res->num.val = a1->num.val >> a2->num.val;
					break;
				case OPERATOR_AND:
					res->num.val = a1->num.val & a2->num.val;
					break;
				case OPERATOR_OR:
					res->num.val = a1->num.val | a2->num.val;
					break;
				case OPERATOR_XOR:
					res->num.val = a1->num.val ^ a2->num.val;
					break;
				case OPERATOR_EQ:
					res->num.val = a1->num.val == a2->num.val;
					break;
				case OPERATOR_NEQ:
					res->num.val = a1->num.val != a2->num.val;
					break;
				case OPERATOR_LT:
					res->num.val = a1->num.val < a2->num.val;
					break;
				case OPERATOR_GT:
					res->num.val = a1->num.val > a2->num.val;
					break;
				case OPERATOR_LTE:
					res->num.val = a1->num.val <= a2->num.val;
					break;
				case OPERATOR_GTE:
					res->num.val = a1->num.val >= a2->num.val;
					break;
				}
			}
			else
			{
				char usingLastVar = 0;
				if (tmpVarCount != 0)
				{
					if (!((a1->type == ExprTypeTmpVar && a1->num.val == tmpVarCount) ||
						  (a2->type == ExprTypeTmpVar && a2->num.val == tmpVarCount)))
					{
						setTmpVar(buf, reg, tmpVarCount);
						tmpVarCount--;
					}
					if (a2->type == ExprTypeTmpVar)
					{
						if (a2->num.val == tmpVarCount)
							usingLastVar = 1;
						loadTmpVarX(buf, reg + 1, reg + 2, a2->num.val);
					}
					else if (a1->type == ExprTypeTmpVar)
					{
						if (a1->num.val == tmpVarCount)
							usingLastVar = 1;
						loadTmpVarX(buf, reg, reg + 2, a1->num.val);
					}
				}

				res->type = ExprTypeTmpVar;
				res->num.val = ++tmpVarCount;
				char* a2Str;

				a2Str = malloc(6);
				snprintf(a2Str, 6, "r%d", reg + 1);

				if (a1->type == ExprTypeNum)
					bufferWrite(buf, "mov r%d, %d\n", reg, a1->num.val);
				else if (a1->type == ExprTypeVar)
					loadVar(buf, reg, a1->var.name);

				if (a2->type == ExprTypeNum)
					snprintf(a2Str, 6, "%d", a2->num.val);
				else if (a2->type == ExprTypeVar)
					loadVar(buf, reg + 1, a2->var.name);

				prevTmpVar = tmpVarCount;
				if (getPrec(node->opr.opr) > -1)
				{
					const char* opr;

					switch (node->opr.opr)
					{
					case OPERATOR_ADD:
						opr = "add";
						break;
					case OPERATOR_SUB:
						opr = "sub";
						break;
					case OPERATOR_MUL:
						err("multiplying is not supported\n");
						break;
					case OPERATOR_DIV:
						err("division is not supported\n");
						break;
					case OPERATOR_SHL:
						opr = "lsl";
						break;
					case OPERATOR_SHR:
						opr = "lsr";
						break;
					case OPERATOR_AND:
						opr = "and";
						break;
					case OPERATOR_OR:
						opr = "or";
						break;
					case OPERATOR_XOR:
						opr = "xor";
						break;
					}
					bufferWrite(buf, "%s r%d, r%d, %s\n", opr, reg, reg, a2Str);
				}
				else
				{
					const char* opr;

					switch (node->opr.opr)
					{
					case OPERATOR_EQ:
						opr = "je";
						break;
					case OPERATOR_NEQ:
						opr = "jne";
						break;
					case OPERATOR_LT:
						opr = "jl";
						break;
					case OPERATOR_GT:
						opr = "jg";
						break;
					case OPERATOR_LTE:
						opr = "jle";
						break;
					case OPERATOR_GTE:
						opr = "jge";
						break;
					}

					bufferWrite(buf, "cmp r%d, %s\n", reg, a2Str);
					bufferWrite(buf, "mov r%d, 1\n", reg);
					bufferWrite(buf, "%s %sCmp%d\n", opr, funName, ifc);
					bufferWrite(buf, "mov r%d, 0\n", reg);
					bufferWrite(buf, "%sCmp%d:\n", funName, ifc);
				}
				char buf[5];
				snprintf(buf, 5, "t%d", tmpVarCount);
				setReg(reg, buf);
			}
			stack1[l2 - 2] = res;
			dynList_resize((void**)&stack1, l2 - 1);
		}
	}

	if (tmpVarCount == 0)
	{
		const ExprNode* node = stack1[0];
		if (node->type == ExprTypeNum)
			bufferWrite(buf, "mov r%d, %d\n", reg, node->num.val);
		else if (node->type == ExprTypeVar)
			loadVar(buf, reg, node->var.name);
	}
	clearTmpVars();
}
