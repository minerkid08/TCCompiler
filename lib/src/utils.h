#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int isNumc(char c);
int isLetter(char c);
int isNums(const char* str);

#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		exit(1);                                                                                                       \
	}
