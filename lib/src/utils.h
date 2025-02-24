#pragma once

#include <stdio.h>
#include <stdlib.h>
#define err(args...)                                                                                                   \
	{                                                                                                                  \
		printf(args);                                                                                                  \
		exit(1);                                                                                                       \
	}
