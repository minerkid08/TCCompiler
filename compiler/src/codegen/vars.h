#pragma once

#include "buffer.h"

void initVars();

void pushVar(const char* name);
void pushScope(Buffer* buf);
void popScope(Buffer* buf);
void popScopeRtn(Buffer* buf);

void loadVar(Buffer* buf, int reg, const char* name);
void setVar(Buffer* buf, int reg, const char* name);

void loadVarX(Buffer* buf, int reg, int auxReg, const char* name);
void setVarX(Buffer* buf, int reg, int auxReg, const char* name);

void loadTmpVar(Buffer* buf, int reg, int name);
void setTmpVar(Buffer* buf, int reg, int name);

void loadTmpVarX(Buffer* buf, int reg, int auxReg, int name);
void setTmpVarX(Buffer* buf, int reg, int auxReg, int name);

void clearReg(int reg);
void clearRegs();
void clearTmpVars();
void setReg(int reg, const char* value);
