#include "dynListInl.h"
#include <assert.h>
#include <stdlib.h>

void DynListInl_alloc(DynListInl* list)
{
	if (list->size > list->capacity)
		list->capacity = list->size;
	int sizeBytes = list->size * list->elemSize;
	if (list->data)
	{
		void* data = realloc(list->data, sizeBytes);
		assert(data);
		list->data = data;
	}
	else
	{
		list->data = malloc(sizeBytes);
		assert(list->data);
	}
}

void DynListInl_free(DynListInl* list)
{
	free(list->data);
}

void* DynListInl_get(DynListInl* list, int i)
{
	assert(i >= 0);
	assert(i < list->size);
	return list->data + (i * list->elemSize);
}
