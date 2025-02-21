#pragma once

#define Func_Inline 1
#define Func_Asm 2
#define Func_ForwardDecl 4

typedef struct StatementNode StatementNode;
typedef struct ExprNode ExprNode;

#define StatementTypeFunc 0
#define StatementTypeFunCall 1
#define StatementTypeVarDef 2
#define StatementTypeVarAssign 3

typedef struct 
{
  const char* name;
  int type;
  int argc;
  const char** argNames;
  StatementNode* statements;
} StatementNodeFunc;

typedef struct
{
  const char* name;
  int argc;
  ExprNode** argExprs;
} StatementNodeFunCall;

typedef struct
{
  const char* name;
  ExprNode* expr;
} StatementNodeVarDef;

typedef struct
{
  const char* name;
  ExprNode* expr;
} StatementNodeVarAssign;

struct StatementNode
{
  int type;
  union
  {
    StatementNodeFunc func;
    StatementNodeFunCall funCall;
    StatementNodeVarDef varDef;
    StatementNodeVarAssign varAssing;
  };
};

#define ExprTypeExpr 0
#define ExprTypeVar 1
#define ExprTypeNum 2
#define ExprTypeOpr 3

typedef struct
{
  ExprNode* expr;
} ExprNodeExpr;

typedef struct
{
  const char* name;
} ExprNodeVar;

typedef struct
{
  int val;
} ExprNodeNum;

typedef struct
{
  char opr;
} ExprNodeOpr;

struct ExprNode
{
  int type;
  union
  {
    ExprNodeExpr expr;
    ExprNodeOpr opr;
    ExprNodeNum num;
    ExprNodeVar var;
  };
};
