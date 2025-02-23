#include "dynList.h"
#include <assert.h>
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
	char zero = 0;
	if (size == 0)
	{
		size = 5;
		zero = 1;
	}
	int sizeBytes = sizeof(DynamicListHeader) + elemSize * size;
	DynamicListHeader* header = malloc(sizeBytes);
	assert(header);
	header->elemSize = elemSize;
	if (zero)
		header->size = 0;
	else
		header->size = size;
	header->capacity = size;
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
  dynList_reserve(list2, header->capacity * 2);
  list = *list2;
  header = dynList_header(list);
	header->size = newSize;
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

void* dynList_get(void* list, int i)
{
	const DynamicListHeader* header = dynList_header(list);
  assert(i >= 0);
  assert(i < header->size);
	return list + (header->elemSize * i);
}
