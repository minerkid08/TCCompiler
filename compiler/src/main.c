#include "codegen/codegen.h"
#include "dynList.h"
#include "parser/parseNodes.h"
#include "parser/parser.h"
#include "token.h"
#include "tokenizer/tokenizer.h"
#include <stdio.h>
#include <stdlib.h>

void printExpr(const ExprNode* expr, const char* indent)
{
	int len = dynList_size(expr);
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = expr + i;
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

void printExpr2(const ExprNode** expr, const char* indent)
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

void printNodes(StatementNode* nodes, const char* indent)
{
	int len = dynList_size(nodes);
	for (int i = 0; i < len; i++)
	{
		StatementNode* node = nodes + i;
		switch (node->type)
		{
		case StatementTypeFunc:
			printf("%sfunction decl, %s, %d, %d\n", indent, node->func.name, node->func.type, node->func.argc);
			if ((node->func.type & 4) == 0)
				printNodes(node->func.statements, "  ");
			break;
		case StatementTypeFunCall:
			printf("%sfunction call, %s, %d\n", indent, node->funCall.name, node->funCall.argc);
			break;
		case StatementTypeVarDef:
			printf("%svar decl, %s\n", indent, node->varDef.name);
			if (node->varDef.expr)
				printExpr(node->varDef.expr, indent);
			break;
		}
	}
}

int getPrec(char c)
{
	switch (c)
	{
	case '+':
		return 2;
	case '-':
		return 1;
	case '*':
		return 4;
	case '/':
		return 3;
	}
	printf("invalid operator '%c'\n", c);
	exit(1);
	return -1;
}

int main(int argc, const char** argv)
{
	const char* filename = argv[1];
	if (argc == 1)
	{
		printf("more args pls\n");
		filename = "test2.lua";
	}

	Token* tokens = tokenize(filename);

	StatementNode* nodes = parse(tokens);
	//Buffer* buf = genCode(nodes);
	//printf("%s", buf->data);
	printNodes(nodes, "");
	int len = dynList_size(nodes->varDef.expr);
	printf("processing expression\n");
	fflush(stdout);
	const ExprNode** stack1 = dynList_new(0, sizeof(ExprNode*));
	printf("processing expression\n");
	fflush(stdout);
	const ExprNode** stack2 = dynList_new(0, sizeof(ExprNode*));
	printf("processing expression\n");
	fflush(stdout);

	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = nodes->varDef.expr + i;
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
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = stack1[i];
		int l = dynList_size(stack2);
		dynList_resize((void**)&stack2, l + 1);
		stack2[l] = node;
	}

	printExpr2(stack2, "  ");
}
