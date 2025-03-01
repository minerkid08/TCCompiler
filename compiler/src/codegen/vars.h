#pragma once

#include "buffer.h"

void initVars();

void pushVar(const char* name);
void pushScope(Buffer* buf);
void popScope(Buffer* buf);
void popScopeRtn(Buffer* buf);

void loadVar(Buffer* buf, int reg, const char* name);
void setVar(Buffer* buf, int reg, const char* name);

void loadTmpVar(Buffer* buf, int reg, int name);
void setTmpVar(Buffer* buf, int reg, int name);

void clearReg(int reg);
void clearRegs();
void clearTmpVars();
