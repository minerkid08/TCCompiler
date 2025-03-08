#include "dynList.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tokenNumber 1
#define tokenString 2
#define tokenLabel 3

typedef struct
{
	int type;
	const char* data;
} Token;

typedef struct
{
	int addr;
	const char* name;
} Label;

int getRegId(const char* reg)
{
	if (strcmp(reg, "sp") == 0)
		return 14;
	if (strcmp(reg, "flags") == 0)
		return 15;
	return atoi(reg + 1);
}

int main(int argc, const char** argv)
{
	const char* filename;
	if (argc == 1)
		filename = "out2.asm";
	else
		filename = argv[1];

	Token* tokens = dynList_new(0, sizeof(Token));
	dynList_reserve((void**)&tokens, 50);

	Label* labels = dynList_new(0, sizeof(Label));
	dynList_reserve((void**)&tokens, 5);

	FILE* file = fopen(filename, "rb");
	if (file == 0)
		err("cant open file\n");
	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* data = malloc(len + 1);
	fread(data, 1, len, file);
	data[len] = 0;
	fclose(file);

	char buf[64];
	int buflen = 0;
	for (int i = 0; i < len; i++)
	{
		char c = data[i];
		if (isalnum(c))
		{
			buf[buflen++] = c;
		}
		else if (buflen > 0)
		{
			buf[buflen] = 0;
			char* tokenStr = malloc(buflen + 1);
			strncpy(tokenStr, buf, buflen + 1);
			// printf("%s\n", tokenStr);
			if (c == ':')
			{
				int len = dynList_size(labels);
				dynList_resize((void**)&labels, len + 1);
				Label* label = labels + len;
				label->addr = dynList_size(tokens);
				label->name = tokenStr;
			}
			else
			{
				int len = dynList_size(tokens);
				dynList_resize((void**)&tokens, len + 1);
				Token* token = tokens + len;
				token->data = tokenStr;
				if (isNums(tokenStr))
					token->type = tokenNumber;
				else
					token->type = tokenString;
			}
			buflen = 0;
		}
		if (c == ';')
		{
			while (c != '\n')
				c = data[++i];
		}
	}

	int regs[16];
	memset(regs, 0, sizeof(int) * 16);

	regs[14] = 0xff + 1;

	int stack[0xff];
	memset(stack, 0, sizeof(stack));

	len = dynList_size(tokens);
	for (int i = 0; i < len; i++)
	{
		Token* token = tokens + i;
		if (strcmp(token->data, "mov") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				regs[r1] = val;
			}
			else
			{
				int r2 = getRegId(token->data);
				regs[r1] = regs[r2];
			}
		}
		else if (strcmp(token->data, "add") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			token = tokens + (++i);
			int r2 = getRegId(token->data);
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				regs[r1] = regs[r2] + val;
			}
			else
			{
				int r3 = getRegId(token->data);
				regs[r1] = regs[r2] + regs[r3];
			}
		}
		else if (strcmp(token->data, "sub") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			token = tokens + (++i);
			int r2 = getRegId(token->data);
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				regs[r1] = regs[r2] - val;
			}
			else
			{
				int r3 = getRegId(token->data);
				regs[r1] = regs[r2] - regs[r3];
			}
		}
		else if (strcmp(token->data, "cmp") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				regs[15] = regs[r1] == val;
			}
			else
			{
				int r2 = getRegId(token->data);
				regs[15] = regs[r1] == regs[r2];
			}
		}
		else if (strcmp(token->data, "jmp") == 0)
		{
			token = tokens + (++i);
			int l2 = dynList_size(labels);
			char found = 0;
			for (int j = 0; j < l2; j++)
			{
				if (strcmp(labels[j].name, token->data) == 0)
				{
					found = 1;
					i = labels[j].addr - 1;
					break;
				}
			}
			if (found == 0)
				err("undefined label %s\n", token->data);
		}
		else if (strcmp(token->data, "je") == 0)
		{
			token = tokens + (++i);
			if (regs[15])
			{
				int l2 = dynList_size(labels);
				char found = 0;
				for (int j = 0; j < l2; j++)
				{
					if (strcmp(labels[j].name, token->data) == 0)
					{
						found = 1;
						i = labels[j].addr - 1;
						break;
					}
				}
				if (found == 0)
					err("undefined label %s\n", token->data);
			}
		}
		else if (strcmp(token->data, "jne") == 0)
		{
			token = tokens + (++i);
			if (!regs[15])
			{
				int l2 = dynList_size(labels);
				char found = 0;
				for (int j = 0; j < l2; j++)
				{
					if (strcmp(labels[j].name, token->data) == 0)
					{
						found = 1;
						i = labels[j].addr - 1;
						break;
					}
				}
				if (found == 0)
					err("undefined label %s\n", token->data);
			}
		}
		else if (strcmp(token->data, "push") == 0)
		{
			regs[14] = regs[14] - 2;
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				stack[regs[14]] = val;
			}
			else
			{
				int r1 = getRegId(token->data);
				stack[regs[14]] = regs[r1];
			}
		}
		else if (strcmp(token->data, "pop") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			regs[r1] = stack[regs[14]];
			regs[14] = regs[14] + 2;
		}
		else if (strcmp(token->data, "call") == 0)
		{
			token = tokens + (++i);
			regs[14] = regs[14] - 2;
			stack[regs[14]] = i;
			int l2 = dynList_size(labels);
			char found = 0;
			for (int j = 0; j < l2; j++)
			{
				if (strcmp(labels[j].name, token->data) == 0)
				{
					found = 1;
					i = labels[j].addr - 1;
					break;
				}
			}
			if (found == 0)
				err("undefined label %s\n", token->data);
		}
		else if (strcmp(token->data, "ret") == 0)
		{
			i = stack[regs[14]];
			regs[14] = regs[14] + 2;
		}
		else if (strcmp(token->data, "out") == 0)
		{
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				printf("%d\n", val);
			}
			else
			{
				int r1 = getRegId(token->data);
				printf("%d\n", regs[r1]);
			}
		}
		else if (strcmp(token->data, "in") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			int val;
			printf("> ");
			scanf("%d", &val);
			regs[r1] = val;
		}
		else if (strcmp(token->data, "store") == 0)
		{
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				token = tokens + (++i);
				int r1 = getRegId(token->data);
				stack[val] = regs[r1];
			}
			else
			{
				int r2 = getRegId(token->data);
				token = tokens + (++i);
				if (token->type == tokenNumber)
				{
					int val = atoi(token->data);
					token = tokens + (++i);
					int r1 = getRegId(token->data);
					stack[regs[r2] + val] = regs[r1];
				}
				else
				{
					i--;
					token = tokens + (++i);
					int r1 = getRegId(token->data);
					stack[regs[r2]] = regs[r1];
				}
			}
		}
		else if (strcmp(token->data, "load") == 0)
		{
			token = tokens + (++i);
			int r1 = getRegId(token->data);
			token = tokens + (++i);
			if (token->type == tokenNumber)
			{
				int val = atoi(token->data);
				regs[r1] = stack[val];
			}
			else
			{
				int r2 = getRegId(token->data);
				token = tokens + (++i);
				if (token->type == tokenNumber)
				{
					int val = atoi(token->data);
					regs[r1] = stack[regs[r2] + val];
				}
				else
				{
					regs[r1] = stack[regs[r2]];
					i--;
				}
			}
		}
		else if (strcmp(token->data, "exit") == 0)
			return 0;
		else
			err("unknown instruction %s\n", token->data);
	}
}
