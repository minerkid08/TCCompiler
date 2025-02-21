#include "token.h"

const char* tokenTypeToStr(int type)
{
	switch (type)
	{
	case TOKEN_LITERAL:
    return "literal";
		break;
	case TOKEN_NUMBER:
    return "number";
		break;
	case TOKEN_OPENPARAN:
    return "open paran";
		break;
	case TOKEN_CLOSEPARAN:
    return "close paran";
		break;
	case TOKEN_KEYWORD:
    return "keyword";
		break;
	case TOKEN_COMMA:
    return "comma";
		break;
	case TOKEN_SEMICOLON:
    return "semicolon";
		break;
	case TOKEN_NEWLINE:
    return "newline";
		break;
	case TOKEN_STRING:
    return "string";
		break;
	case TOKEN_OPERATOR:
    return "operator";
		break;
	case TOKEN_EOF:
    return "eof";
		break;
	case TOKEN_SUBSTUTION:
    return "asm substution";
		break;
	}
  return "unknown";
}
