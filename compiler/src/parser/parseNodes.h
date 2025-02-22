#pragma once

#define Func_Inline 1
#define Func_Asm 2
#define Func_ForwardDecl 4

typedef struct StatementNode StatementNode;
typedef struct ExprNode ExprNode;

typedef struct StatementNodeFunc StatementNodeFunc;
typedef struct StatementNodeFunCall StatementNodeFunCall;
typedef struct StatementNodeVarDef StatementNodeVarDef;
typedef struct StatementNodeVarAssign StatementNodeVarAssign;

typedef struct ExprNodeOpr ExprNodeOpr;
typedef struct ExprNodeVar ExprNodeVar;
typedef struct ExprNodeNum ExprNodeNum;
typedef struct ExprNodeExpr ExprNodeExpr;

#define StatementTypeFunc 0
#define StatementTypeFunCall 1
#define StatementTypeVarDef 2
#define StatementTypeVarAssign 3

struct StatementNodeFunc
{
  const char* name;
  int type;
  int argc;
  const char** argNames;
  StatementNode* statements;
};

struct StatementNodeFunCall
{
  const char* name;
  int argc;
  ExprNode** argExprs;
};

struct StatementNodeVarDef
{
  const char* name;
  ExprNode* expr;
};

struct StatementNodeVarAssign
{
  const char* name;
  ExprNode* expr;
};

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

struct ExprNodeExpr
{
  ExprNode* expr;
};

struct ExprNodeVar
{
  const char* name;
};

struct ExprNodeNum
{
  int val;
};

struct ExprNodeOpr
{
  char opr;
};

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
