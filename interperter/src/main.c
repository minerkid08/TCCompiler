#include "dynList.h"
#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define tokenNumber 1
#define tokenString 2
#define tokenLabel 3

typedef struct
{
  char type;
  const char* data;
} Token;

int main(int argc, const char** argv)
{
	Token* tokens = dynList_new(0, sizeof(char*));
	dynList_reserve((void**)&tokens, 50);

	FILE* file = fopen(argv[1], "rb");
	if (file == 0)
		err("cant open file\n");
	fseek(file, 0, SEEK_END);
	unsigned long long len = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* data = malloc(len + 1);
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
      int len = dynList_size(tokens);
      dynList_resize((void**)&tokens, len + 1);
      Token* token = tokens + len;
      token->data = tokenStr;
      if(c == ':')
		}
	}
}
