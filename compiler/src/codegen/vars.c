#include "vars.h"
#include "dynList.h"
#include "utils.h"
#include <string.h>

#define REGCONT_SIZE 32
#define REGCOUNT 14

const char** vars;
char regContents[REGCOUNT][REGCONT_SIZE];
int* scopeVarCounts;

void initVars()
{
	memset(&regContents, 0, REGCOUNT * REGCONT_SIZE);
	vars = dynList_new(0, sizeof(const char*));
	scopeVarCounts = dynList_new(0, sizeof(int));
}

void pushVar(const char* name)
{
	int scopec = dynList_size(scopeVarCounts);
	scopeVarCounts[scopec - 1]++;
	int varc = dynList_size(vars);
	dynList_resize((void**)&vars, varc + 1);
	vars[varc] = name;
}

void pushScope(Buffer* buf)
{
	int scopec = dynList_size(scopeVarCounts);
	dynList_resize((void**)&scopeVarCounts, scopec + 1);
	scopeVarCounts[scopec] = 0;
	bufferWrite(buf, "push r13\nmov r13, sp\n");
}

void popScope(Buffer* buf)
{
	int varc = dynList_size(vars);
	int scopec = dynList_size(scopeVarCounts);
	int newLen = varc - scopeVarCounts[scopec - 1];
	dynList_resize((void**)&vars, newLen);
	bufferWrite(buf, "mov sp, r13\npop r13\n");
}

void popScopeRtn(Buffer* buf)
{
	int len = dynList_size(scopeVarCounts);
	for (int i = 0; i < len; i++)
	{
		bufferWrite(buf, "mov sp, r13\npop r13\n");
	}
}

int getVar(const char* name)
{
	int len = dynList_size(vars);
	for (int i = 1; i < len; i++)
	{
		if (vars[i] == 0)
			continue;
		if (strcmp(vars[i], name) == 0)
			return i;
	}
	err("variable %s not found\n", name);
	return 0;
}

void setVar(Buffer* buf, int reg, const char* name)
{
	clearReg(reg + 1);
	snprintf(regContents[reg], REGCONT_SIZE, "v%s", name);
	regContents[reg + 1][0] = 0;
	int idx = getVar(name);
	int len = dynList_size(vars);
	if (idx == len - 1)
		bufferWrite(buf, "store [sp], r%d ; %s\n", reg, name);
	else
		bufferWrite(buf, "add r%d, sp, %d\nstore [r%d], r%d ; %s\n", reg + 1, (len - idx - 1) * 2, reg + 1, reg, name);
}

void loadVar(Buffer* buf, int reg, const char* name)
{
	char target[REGCONT_SIZE];
	snprintf(target, REGCONT_SIZE, "v%s", name);
	int foundReg = -1;
	for (int i = 0; i < REGCOUNT; i++)
	{
		if (strcmp(regContents[i], target) == 0)
		{
			if (foundReg == -1)
				foundReg = i;
			if (i == reg)
				foundReg = reg;
		}
	}

	if (reg == foundReg)
		return;
	if (foundReg != -1)
	{
		bufferWrite(buf, "mov r%d, r%d ; %s\n", reg, foundReg);
		return;
	}

	int idx = getVar(name);
	int len = dynList_size(vars);
	if (idx == len - 1)
		bufferWrite(buf, "load r%d, [sp] ; %s\n", reg, name);
	else
		bufferWrite(buf, "add r%d, sp, %d\nload r%d, [r%d] ; %s\n", reg, (len - idx - 1) * 2, reg, reg, name);
	strncpy(target, regContents[reg], REGCONT_SIZE);
}

void setTmpVar(Buffer* buf, int reg, int name)
{
	clearReg(reg + 1);
	snprintf(regContents[reg], REGCONT_SIZE, "t%d", name);
	regContents[reg + 1][0] = 0;
	bufferWrite(buf, "add r%d, sp, %d\nstore [r%d], r%d ; %s\n", reg + 1, name * 2, reg + 1, reg, regContents[reg]);
}

void loadTmpVar(Buffer* buf, int reg, int name)
{
	char target[REGCONT_SIZE];
	snprintf(target, REGCONT_SIZE, "t%d", name);
	int foundReg = -1;
	for (int i = 0; i < REGCOUNT; i++)
	{
		if (strcmp(regContents[i], target) == 0)
		{
			if (foundReg == -1)
				foundReg = i;
			if (i == reg)
				foundReg = reg;
		}
	}

	if (reg == foundReg)
		return;
	if (foundReg != -1)
	{
		bufferWrite(buf, "mov r%d, r%d ; %s\n", reg, foundReg, target);
		return;
	}

	bufferWrite(buf, "sub r%d, sp, %d\nload r%d, [r%d] ; %s\n", reg, name * 2, reg, reg, target);
	strncpy(target, regContents[reg], REGCONT_SIZE);
}

void clearReg(int reg)
{
	regContents[reg][0] = 0;
}

void clearRegs()
{
	for (int i = 1; i < REGCOUNT; i++)
		regContents[i][0] = 0;
}

void clearTmpVars()
{
	for (int i = 1; i < REGCOUNT; i++)
	{
		if (regContents[i][0] == 't')
			regContents[i][0] = 0;
	}
}
