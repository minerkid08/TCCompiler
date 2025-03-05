#include "codegen/codegen.h"
#include "dynList.h"
#include "parser/parseNodes.h"
#include "parser/parser.h"
#include "target.h"
#include "token.h"
#include "tokenizer/tokenizer.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

void printExpr(const ExprNode* expr, const char* indent)
{
	int len = dynList_size(expr);
	printf("expr len: %d\n", len);
	for (int i = 0; i < len; i++)
	{
		const ExprNode* node = expr + i;
		switch (node->type)
		{
		case ExprTypeExpr:
			printf("%s  expr: ?\n", indent);
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

void printNodes(const StatementNode* nodes, const char* indent)
{
	int len = dynList_size(nodes);
	for (int i = 0; i < len; i++)
	{
		const StatementNode* node = nodes + i;
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
		case StatementTypeIf:
			printf("%sif\n", indent);
			printExpr(node->condIf.expr, indent);
			printNodes(node->condIf.statments, "    ");
			break;
		}
	}
}

int target = ARCH_SYMPHONY;

int main(int argc, const char** argv)
{
	const char* filename;
	const char* outFilename;

	if (argc == 1)
	{
		filename = "test.lua";
		outFilename = "test.asm";
		target = ARCH_SYMPHONY_EXT;
	}
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-a") == 0)
		{
			if (strcmp(argv[i + 1], "symphony-ext") == 0)
				target = ARCH_SYMPHONY_EXT;
		}
		else if (strcmp(argv[i], "-o") == 0)
		{
			outFilename = argv[i + 1];
		}
		else
			filename = argv[i];
	}

	if (outFilename == 0)
		err("no output file\n");
	if (filename == 0)
		err("no input file\n");

	Token* tokens = tokenize(filename);

	// int len = dynList_size(tokens);
	// for (int i = 0; i < len; i++)
	//{
	//	printf("%s, %s\n", tokenTypeToStr(tokens[i].type), tokens[i].data);
	// }

	StatementNode* nodes = parse(tokens);

	// printNodes(nodes, "");

	Buffer* buf = genCode(nodes);

	FILE* outFile = fopen(outFilename, "wb");
	fprintf(outFile, "%s", buf->data);
	fclose(outFile);
}
