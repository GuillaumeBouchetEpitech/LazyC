
#pragma once

typedef struct PointerHeapArray
{
  void **data;
  unsigned int cap;
  unsigned int len;
} PointerHeapArray;

PointerHeapArray *PointerHeapArray__preAllocate(unsigned int inCapacity);
void PointerHeapArray__free(PointerHeapArray **self);
int PointerHeapArray__pushBack(PointerHeapArray *self, void *inData);
int PointerHeapArray__popBack(PointerHeapArray *self);
int PointerHeapArray__eraseAtIndex(PointerHeapArray *self, unsigned int inIndex);
int PointerHeapArray__insertAtIndex(PointerHeapArray *self, unsigned int inIndex, void *inData);
int PointerHeapArray__pushFront(PointerHeapArray *self, void *inData);
int PointerHeapArray__popFront(PointerHeapArray *self);
void PointerHeapArray__clear(PointerHeapArray *self);

typedef int (*SortCallbackPtr)(void*, void*, void*);

int PointerHeapArray__sortFromCallback(PointerHeapArray *self, SortCallbackPtr inCallback, void* inUserData);
