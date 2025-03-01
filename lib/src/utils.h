#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		assert(0);                                                                                                 \
	}
