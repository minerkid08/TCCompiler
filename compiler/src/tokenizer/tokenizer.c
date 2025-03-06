#include "tokenizer.h"
#include "dynList.h"
#include "token.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* keywords[] = {"function", "return", "end",	 "if",		"else", "then", "do",
						  "local",	  "while",	"break", "require", "ref",	"asm",	"inline"};

const char operatorChars[] = {'+', '-', '*', '/', '=', '!', '<', '>', '^'};

const char* operators[] = {"+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">=", "<<", ">>", "&", "|", "^", "->"};

const char operatorConsts[] = {OPERATOR_ADD, OPERATOR_SUB, OPERATOR_MUL, OPERATOR_DIV, OPERATOR_ASSIGN, OPERATOR_EQ,
							   OPERATOR_NEQ, OPERATOR_LT,  OPERATOR_GT,	 OPERATOR_LTE, OPERATOR_GTE,	OPERATOR_SHL,
							   OPERATOR_SHR, OPERATOR_AND, OPERATOR_OR,	 OPERATOR_XOR, OPERATOR_RTN};

int isKeyword(const char* str)
{
	for (int i = 0; i < sizeof(keywords) / sizeof(char*); i++)
	{
		if (strcmp(str, keywords[i]) == 0)
			return 1;
	}
	return 0;
}

char isOperator(const char* str)
{
	for (int i = 0; i < sizeof(operators) / sizeof(char*); i++)
	{
		if (strcmp(str, operators[i]) == 0)
			return operatorConsts[i];
	}
	return 0;
}

int isOperatorc(char c)
{
	for (int i = 0; i < sizeof(operatorChars) / sizeof(char); i++)
	{
		if (c == operatorChars[i])
			return 1;
	}
	return 0;
}

void addToken(Token** tokens, const char* str)
{
	int len = strlen(str);
	int type;
	char* tokenStr = malloc(len);
	strcpy(tokenStr, str);
	if (strcmp(str, ")") == 0)
		type = TOKEN_CLOSEPARAN;
	else if (strcmp(str, "(") == 0)
		type = TOKEN_OPENPARAN;
	else if (isKeyword(str))
		type = TOKEN_KEYWORD;
	else if (isNums(str))
		type = TOKEN_NUMBER;
	else if (str[0] == '\"')
		type = TOKEN_STRING;
	else if (strcmp(str, ";") == 0)
		type = TOKEN_SEMICOLON;
	else if (strcmp(str, ",") == 0)
		type = TOKEN_COMMA;
	else if (strcmp(str, "\n") == 0)
		type = TOKEN_NEWLINE;
	else
	{
		char c = isOperator(str);
		if (c)
		{
			tokenStr[0] = c;
			tokenStr[1] = '\0';
			type = TOKEN_OPERATOR;
		}
		else if (str[0] == '\1')
			type = TOKEN_EOF;
		else
			type = TOKEN_LITERAL;
	}
	int tokenId = dynList_size(*tokens);
	dynList_resize((void**)tokens, tokenId + 1);
	Token* token = &(*tokens)[tokenId];
	token->type = type;
	token->data = tokenStr;
}

void tokenizeFile(Token** tokens, const char* filename);
Token* tokenize(const char* filename)
{
	Token* tokens = dynList_new(0, sizeof(Token));
	dynList_reserve((void**)&tokens, 20);
	tokenizeFile(&tokens, filename);

	int tokenId = dynList_size(tokens);
	dynList_resize((void**)&tokens, tokenId + 1);
	Token* token = tokens + tokenId;
	token->type = TOKEN_EOF;
	token->data = 0;

	return tokens;
}

void tokenizeFile(Token** tokens, const char* filename)
{
	FILE* file = fopen(filename, "rb");
  if(file == 0)
    err("cant find file '%s'\n", filename);
	fseek(file, 0, SEEK_END);
	unsigned long long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(size);

	fread(data, 1, size, file);

	char buf[64];
	int bufLen = 0;

	char inToken = 0;
	char inStr = 0;
	for (int i = 0; i < size; i++)
	{
		char c = data[i];
		if (c == '-')
		{
			char c2 = data[i + 1];
			if (c2 == '-')
			{
				i += 2;
				while (c2 != '\n')
				{
					c2 = data[++i];
				}
				continue;
			}
		}
		if (!inStr)
		{
			if (inToken)
			{
				if (!isNumc(c) && !isLetter(c))
				{
					buf[bufLen] = '\0';
					if (strcmp(buf, "require") == 0)
					{
						if (data[i] != '(')
							err("expected '(' after require\n");
						if (data[++i] != '"')
							err("expected string in require statement\n");
						bufLen = 0;
						inToken = 0;
						c = data[++i];
						buf[bufLen++] = c;
						while (c != '"')
						{
							c = data[++i];
							buf[bufLen++] = c;
						}
						buf[--bufLen] = '\0';
						if (data[++i] != ')')
							err("expected ')' after require statement\n");
						if (data[++i] != ';')
							err("expected ';' after require statement\n");
            tokenizeFile(tokens, buf);
						bufLen = 0;
            continue;
					}
					else
					{
						addToken(tokens, buf);
						bufLen = 0;
						inToken = 0;
					}
				}
				else
				{
					buf[bufLen++] = c;
				}
			}
			if (c != '\t' && c != '\r' && c != ' ' && !inToken)
			{
				inToken = 1;
				buf[bufLen++] = c;
				if (!isNumc(c) && !isLetter(c))
				{
					if (c == '-' && isNumc(data[i + 1]))
						continue;
					if (c == '\"')
					{
						inStr = 1;
					}
					else
					{
						if (isOperatorc(data[i + 1]))
						{
							buf[bufLen++] = data[++i];
						}
						buf[bufLen] = '\0';
						addToken(tokens, buf);
						bufLen = 0;
						inToken = 0;
					}
				}
			}
		}
		else
		{
			buf[bufLen++] = c;
			if (c == '\"')
			{
				inStr = 0;
				inToken = 0;
				buf[bufLen] = '\0';
				addToken(tokens, buf);
				bufLen = 0;
			}
		}
	}

	free(data);

	fclose(file);
}
