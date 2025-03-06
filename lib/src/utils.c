#include "utils.h"

int isNumc(char c)
{
	return c >= '0' && c <= '9';
}

int isLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int isNums(const char* str)
{
	int len = strlen(str);
	int hasSign = str[0] == '-';
	for (int i = 0; i < len; i++)
	{
		if (hasSign && i == 0)
		{
			if (len == 1)
				return 0;
			continue;
		}
		char c = str[i];
		if (!isNumc(c) && c != '.')
			return 0;
	}
	return 1;
}
