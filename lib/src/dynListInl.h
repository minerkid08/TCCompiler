#pragma once

typedef struct
{
  int size;
  int capacity;
  int elemSize;
  void* data;
} DynListInl;

void DynListInl_alloc(DynListInl* list);
void DynListInl_free(DynListInl* list);
void* DynListInl_get(DynListInl* list, int i);
