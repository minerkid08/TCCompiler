#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		*(char*)0 = 0;                                                                                                 \
	}
