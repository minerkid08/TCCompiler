#pragma once

#include "buffer.h"
#include "../parser/parseNodes.h"

Buffer* genCode(const StatementNode* statements);
void loadVar(Buffer* buf, int reg, const char* name);
