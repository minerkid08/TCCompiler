#include "dynList.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define tokenNone 0
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
	const char* name;
	Token a1;
	Token a2;
	Token a3;
} Instruction;

typedef struct
{
	int addr;
	const char* name;
} Label;

char getRegId(const char* reg)
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

	Instruction* instructions = dynList_new(0, sizeof(Instruction));
	dynList_reserve((void**)&instructions, 50);
	dynList_resize((void**)&instructions, 1);

	Label* labels = dynList_new(0, sizeof(Label));
	dynList_reserve((void**)&labels, 5);

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
	int state = 0;
	Instruction* ins = instructions;
	for (int i = 0; i < len; i++)
	{
		char c = data[i];
		if (isalnum(c) || c == '_')
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
				label->addr = dynList_size(instructions) - 1;
				label->name = tokenStr;
			}
			else
			{
				if (state == 0)
				{
					ins->name = tokenStr;
				}
				else
				{
					Token* token = 0;
					if (state == 1)
						token = &ins->a1;
					else if (state == 2)
						token = &ins->a2;
					else
						token = &ins->a3;
					token->data = tokenStr;
					if (isNums(tokenStr))
						token->type = tokenNumber;
					else
						token->type = tokenString;
				}
				state++;
			}
			buflen = 0;
		}
		if (c == ';')
		{
			while (c != '\n')
				c = data[++i];
		}
		if (c == '\n')
		{
			if (state != 0)
			{
				if (state == 1)
				{
					ins->a1.type = 0;
					ins->a2.type = 0;
					ins->a3.type = 0;
				}
				if (state == 2)
				{
					ins->a2.type = 0;
					ins->a3.type = 0;
				}
				if (state == 3)
				{
					ins->a3.type = 0;
				}
				state = 0;
				int len = dynList_size(instructions);
				dynList_resize((void**)&instructions, len + 1);
				ins = instructions + len;
			}
		}
	}
	len = dynList_size(instructions);
	dynList_resize((void**)&instructions, len);

	unsigned short regs[16];
	memset(regs, 0, sizeof(unsigned short) * 16);

	len = dynList_size(instructions);
	for (int i = 0; i < len; i++)
	{
		Instruction* ins = instructions + i;
		if (strcmp(ins->name, "mov") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			if (ins->a2.type == tokenNumber)
			{
				int val = atoi(ins->a2.data);
				regs[r1] = val;
			}
			else
			{
				int r2 = getRegId(ins->a2.data);
				regs[r1] = regs[r2];
			}
		}
		else if (strcmp(ins->name, "add") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			int r2 = getRegId(ins->a2.data);
			if (ins->a3.type == tokenNumber)
			{
				int val = atoi(ins->a3.data);
				regs[r1] = regs[r2] + val;
			}
			else
			{
				int r3 = getRegId(ins->a3.data);
				regs[r1] = regs[r2] + regs[r3];
			}
		}
		else if (strcmp(ins->name, "sub") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			int r2 = getRegId(ins->a2.data);
			if (ins->a3.type == tokenNumber)
			{
				int val = atoi(ins->a3.data);
				regs[r1] = regs[r2] - val;
			}
			else
			{
				int r3 = getRegId(ins->a3.data);
				regs[r1] = regs[r2] - regs[r3];
			}
		}

		else if (strcmp(ins->name, "cmp") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			if (ins->a2.type == tokenNumber)
			{
				int val = atoi(ins->a2.data);
				regs[15] = regs[r1] == val;
			}
			else
			{
				int r2 = getRegId(ins->a2.data);
				regs[15] = regs[r1] == regs[r2];
			}
		}
		else if (strcmp(ins->name, "jmp") == 0)
		{
			int l2 = dynList_size(labels);
			char found = 0;
			for (int j = 0; j < l2; j++)
			{
				if (strcmp(labels[j].name, ins->a1.data) == 0)
				{
					found = 1;
					i = labels[j].addr - 1;
					break;
				}
			}
			if (found == 0)
				err("undefined label %s\n", ins->a1.data);
		}
		else if (strcmp(ins->name, "je") == 0)
		{
			if (regs[15])
			{
				int l2 = dynList_size(labels);
				char found = 0;
				for (int j = 0; j < l2; j++)
				{
					if (strcmp(labels[j].name, ins->a1.data) == 0)
					{
						found = 1;
						i = labels[j].addr - 1;
						break;
					}
				}
				if (found == 0)
					err("undefined label %s\n", ins->a1.data);
			}
		}
		else if (strcmp(ins->name, "jne") == 0)
		{
			if (!regs[15])
			{
				int l2 = dynList_size(labels);
				char found = 0;
				for (int j = 0; j < l2; j++)
				{
					if (strcmp(labels[j].name, ins->a1.data) == 0)
					{
						found = 1;
						i = labels[j].addr - 1;
						break;
					}
				}
				if (found == 0)
					err("undefined label %s\n", ins->a1.data);
			}
		}
		else if (strcmp(ins->name, "push") == 0)
		{
			regs[14] = regs[14] - 2;
			if (ins->a1.type == tokenNumber)
			{
				int val = atoi(ins->a1.data);
				writeHeap16(regs[14], val);
			}
			else
			{
				int r1 = getRegId(ins->a1.data);
				writeHeap16(regs[14], regs[r1]);
			}
		}
		else if (strcmp(ins->name, "pop") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			regs[r1] = readHeap16(regs[14]);
			regs[14] = regs[14] + 2;
		}
		else if (strcmp(ins->name, "call") == 0)
		{
			regs[14] = regs[14] - 2;
			writeHeap16(regs[14], i);
			int l2 = dynList_size(labels);
			char found = 0;
			for (int j = 0; j < l2; j++)
			{
				if (strcmp(labels[j].name, ins->a1.data) == 0)
				{
					found = 1;
					i = labels[j].addr - 1;
					break;
				}
			}
			if (found == 0)
				err("undefined label %s\n", ins->a1.data);
		}
		else if (strcmp(ins->name, "ret") == 0)
		{
			i = readHeap16(regs[14]);
			regs[14] = regs[14] + 2;
		}
		else if (strcmp(ins->name, "out") == 0)
		{
			if (ins->a1.type == tokenNumber)
			{
				int val = atoi(ins->a1.data);
				printf("%d\n", val);
			}
			else
			{
				int r1 = getRegId(ins->a1.data);
				printf("%hu\n", regs[r1]);
			}
		}
		else if (strcmp(ins->name, "in") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			int val;
			printf("> ");
			scanf("%d", &val);
			regs[r1] = val;
		}
		else if (strcmp(ins->name, "store_16") == 0)
		{
			if (ins->a1.type == tokenNumber)
			{
				int val = atoi(ins->a1.data);
				int r1 = getRegId(ins->a2.data);
				writeHeap16(val, regs[r1]);
			}
			else
			{
				int r2 = getRegId(ins->a1.data);
				if (ins->a2.type == tokenNumber)
				{
					int val = atoi(ins->a2.data);
					int r1 = getRegId(ins->a3.data);
					writeHeap16(regs[r2] + val, regs[r1]);
				}
				else
				{
					int r1 = getRegId(ins->a2.data);
					writeHeap16(regs[r2], regs[r1]);
				}
			}
		}
		else if (strcmp(ins->name, "store_8") == 0)
		{
			if (ins->a1.type == tokenNumber)
			{
				int val = atoi(ins->a1.data);
				int r1 = getRegId(ins->a2.data);
				writeHeap8(val, regs[r1]);
			}
			else
			{
				int r2 = getRegId(ins->a1.data);
				if (ins->a2.type == tokenNumber)
				{
					int val = atoi(ins->a2.data);
					int r1 = getRegId(ins->a3.data);
					writeHeap8(regs[r2] + val, regs[r1]);
				}
				else
				{
					int r1 = getRegId(ins->a2.data);
					writeHeap8(regs[r2], regs[r1]);
				}
			}
		}
		else if (strcmp(ins->name, "load_16") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			if (ins->a2.type == tokenNumber)
			{
				int val = atoi(ins->a2.data);
				regs[r1] = readHeap16(val);
			}
			else
			{
				int r2 = getRegId(ins->a2.data);
				if (ins->a3.type == tokenNumber)
				{
					int val = atoi(ins->a3.data);
					regs[r1] = readHeap16(regs[r2] + val);
				}
				else
				{
					regs[r1] = readHeap16(regs[r2]);
				}
			}
		}
		else if (strcmp(ins->name, "load_8") == 0)
		{
			int r1 = getRegId(ins->a1.data);
			if (ins->a2.type == tokenNumber)
			{
				int val = atoi(ins->a2.data);
				regs[r1] = readHeap8(val);
			}
			else
			{
				int r2 = getRegId(ins->a2.data);
				if (ins->a3.type == tokenNumber)
				{
					int val = atoi(ins->a3.data);
					regs[r1] = readHeap8(regs[r2] + val);
				}
				else
				{
					regs[r1] = readHeap8(regs[r2]);
				}
			}
		}
		else if (strcmp(ins->name, "exit") == 0)
			return 0;
		else
			err("unknown instruction %s, %d\n", ins->name, i);
	}
}
