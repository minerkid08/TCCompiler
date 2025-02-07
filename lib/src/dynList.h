#pragma once

void* dynList_new(int size, int elemSize);
void dynList_resize(void** list, int newSize);
void dynList_reserve(void** list, int newCapacity);
void dynList_free(void* list);
int dynList_size(void* list);
int dynList_capacity(void* list);
