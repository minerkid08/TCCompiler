#include "utils.h"
#include <stdio.h>

typedef struct
{
  const char* name;
  const char* bitField;
} IsaInstruction;

void parseIsa(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (file == 0)
		err("could not open %s\n", filename);
  fseek(file, 0, SEEK_END);
  unsigned long long len = ftell(file);
  fseek(file, 0, SEEK_SET);
	char* data = malloc(len + 1);
	fread(data, 1, len, file);
	data[len] = 0;
	fclose(file);
}
