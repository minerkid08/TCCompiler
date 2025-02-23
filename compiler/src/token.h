#pragma once

#define TOKEN_LITERAL 0
#define TOKEN_NUMBER 1
#define TOKEN_OPENPARAN 2
#define TOKEN_CLOSEPARAN 3
#define TOKEN_KEYWORD 4
#define TOKEN_COMMA 5
#define TOKEN_SEMICOLON 6
#define TOKEN_NEWLINE 7
#define TOKEN_STRING 8
#define TOKEN_OPERATOR 9
#define TOKEN_EOF 10
#define TOKEN_SUBSTUTION 11

typedef struct {
  int type;
  char *data;
} Token;

const char* tokenTypeToStr(int type);
