#include "dynList.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

char** usages;
char** functions;
char** files;

int isLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

void parseFile(const char* filename)
{
	FILE* file = fopen(filename, "rb");
	if (file == 0)
		err("cant find file '%s'\n", filename);
	fseek(file, 0, SEEK_END);
	unsigned long long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = malloc(size);
	fread(data, 1, size, file);

	char buf[64];
	int bufLen = 0;

	char reading = 1;

	for (int i = 0; i < size; i++)
	{
		char c = data[i];
		if (reading)
		{
			if (isLetter(c))
			{
				buf[bufLen++] = c;
			}
			else
			{
				buf[bufLen] = 0;
				if (reading == 2)
				{
					char* str = malloc(bufLen + 1);
					strncpy(str, buf, bufLen + 1);
					int len = dynList_size(usages);
					dynList_resize((void**)&usages, len + 1);
					usages[len] = str;
					reading = 0;
				}
				else if (c == ':')
				{
					char* str = malloc(bufLen + 1);
					strncpy(str, buf, bufLen + 1);
					int len = dynList_size(functions);
					dynList_resize((void**)&functions, len + 1);
					functions[len] = str;
					reading = 0;
				}
				else if (strcmp(buf, "call") == 0)
					reading = 2;
				else
					reading = 0;
				bufLen = 0;
			}
		}
		if (c == '\n')
			reading = 1;
	}
	int len = dynList_size(files);
	dynList_resize((void**)&files, len + 1);
	files[len] = data;
}

int main(int argc, const char** argv)
{
	usages = dynList_new(1, sizeof(char*));
	usages[0] = "main";
	functions = dynList_new(0, sizeof(char*));
	files = dynList_new(0, sizeof(char*));

	if (argc < 3)
		err("usage: linker outFile inFiles\n");
	for (int i = 2; i < argc; i++)
	{
		parseFile(argv[i]);
	}
	int usageLen = dynList_size(usages);
	int functionLen = dynList_size(functions);
	int error = 0;
	for (int i = 0; i < usageLen; i++)
	{
		char found = 0;
		for (int j = 0; j < functionLen; j++)
		{
			if (strcmp(functions[j], usages[i]) == 0)
			{
				found = 1;
				break;
			}
		}
		if (!found)
		{
			error = 1;
			printf("undefined ref to '%s'\n", usages[i]);
		}
	}
  if(error)
    return 1;

	FILE* file = fopen(argv[1], "wb");
	fprintf(file, "call main\n");

	int filesLen = dynList_size(files);
	for (int i = 0; i < filesLen; i++)
	{
		fprintf(file, "%s", files[i]);
	}
	fclose(file);
}
