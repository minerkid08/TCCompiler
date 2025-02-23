#include "dynList.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
	int size;
	int capacity;
	int elemSize;
} DynamicListHeader;

#define dynList_header(list) list - sizeof(DynamicListHeader)

void* dynList_new(int size, int elemSize)
{
	int sizeBytes = sizeof(DynamicListHeader) + elemSize * size;
	DynamicListHeader* header = malloc(sizeBytes);
	assert(header);
	header->elemSize = elemSize;
	header->size = size;
	header->capacity = size;
	return header + 1;
}

void* dynList_newX(int size, int capacity, int elemSize)
{
	int sizeBytes = sizeof(DynamicListHeader) + elemSize * capacity;
	DynamicListHeader* header = malloc(sizeBytes);
	assert(header);
	header->elemSize = elemSize;
	header->size = size;
	header->capacity = capacity;
	return header + 1;
}

void dynList_resize(void** list2, int newSize)
{
	void* list = *list2;
	assert(list);
	DynamicListHeader* header = dynList_header(list);
	if (header->capacity >= newSize)
	{
		header->size = newSize;
		return;
	}
	int sizeBytes = sizeof(DynamicListHeader) + header->elemSize * newSize;
	DynamicListHeader* newList = realloc(header, sizeBytes);
	if (newList == 0)
	{
    int oldSize = sizeof(DynamicListHeader) + header->elemSize * header->capacity;
		printf("failed reallocating %d(%d) to %d(%d) bytes\n", oldSize, header->capacity, sizeBytes, newSize);
		fflush(stdout);
		assert(0);
	}
	newList->capacity = newSize;
	newList->size = newSize;
	*list2 = newList + 1;
}

void dynList_reserve(void** list2, int newCapacity)
{
	void* list = *list2;
	assert(list);
	DynamicListHeader* header = dynList_header(list);
	if (header->capacity >= newCapacity)
		return;
	int sizeBytes = sizeof(DynamicListHeader) + header->elemSize * newCapacity;
	DynamicListHeader* newList = realloc(header, sizeBytes);
	assert(newList);
	newList->capacity = newCapacity;
	*list2 = newList + 1;
}

void dynList_free(void* list)
{
	assert(list);
	DynamicListHeader* header = dynList_header(list);
	free(header);
}

int dynList_size(const void* list)
{
	const DynamicListHeader* header = dynList_header(list);
	return header->size;
}

int dynList_capacity(const void* list)
{
	const DynamicListHeader* header = dynList_header(list);
	return header->capacity;
}
