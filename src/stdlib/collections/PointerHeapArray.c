
#include "PointerHeapArray.h"

#include <stdlib.h>
#include <string.h>

#include <stdio.h>

PointerHeapArray *PointerHeapArray__preAllocate(unsigned int inCapacity)
{
  PointerHeapArray *self = calloc(1, sizeof(PointerHeapArray));
  if (!self)
  {
    return NULL;
  }

  self->len = 0;
  self->cap = inCapacity < 32 ? 32 : inCapacity;
  self->data = calloc(self->cap, sizeof(void *));
  if (!self->data)
  {
    free(self);
    return NULL;
  }

  return self;
}

void PointerHeapArray__free(PointerHeapArray **self)
{
  if (!self || !*self)
  {
    return;
  }
  free((*self)->data);
  free(*self);
  *self = NULL;
}

static int _PointerHeapArray__grow(PointerHeapArray *self, unsigned int inNewCapacity)
{
  if (self->cap >= inNewCapacity)
  {
    return 0;
  }
  while (self->cap < inNewCapacity)
  {
    self->cap *= 2;
  }
  self->data = realloc(self->data, self->cap * sizeof(void *));
  return self->data == NULL ? -1 : 0;
}

int PointerHeapArray__pushBack(PointerHeapArray *self, void *inData)
{
  if (_PointerHeapArray__grow(self, self->len + 1) != 0)
  {
    return -1;
  }
  self->data[self->len++] = inData;
  return 0;
}

int PointerHeapArray__popBack(PointerHeapArray *self)
{
  if (self->len > 0)
  {
    self->data[--self->len] = NULL;
    return 0;
  }
  return -1;
}

int PointerHeapArray__eraseAtIndex(PointerHeapArray *self, unsigned int inIndex)
{
  if (inIndex > self->len)
  {
    return -1;
  }

  // const unsigned int sliceSize = self->len - inIndex;
  // memmove(self->data + inIndex * sizeof(void*), self->data + (inIndex + 1) * sizeof(void*), sliceSize);

  for (unsigned int ii = inIndex; ii < self->len; ++ii) {
    self->data[ii] = self->data[ii + 1];
  }

  self->data[--self->len] = NULL;

  return 0;
}

int PointerHeapArray__insertAtIndex(PointerHeapArray *self, unsigned int inIndex, void *inData)
{
  if (inIndex == self->len)
  {
    return PointerHeapArray__pushBack(self, inData);
  }
  if (inIndex > self->len)
  {
    return -1;
  }

  if (_PointerHeapArray__grow(self, self->len + 1) != 0)
  {
    return -1;
  }

  // const unsigned int sliceSize = self->len - inIndex;
  // printf(" => sliceSize:%d\n", sliceSize);
  // memmove(self->data + inIndex + 1, self->data + inIndex, sliceSize);

  for (int ii = ((int)self->len) - 1; ii >= ((int)inIndex); --ii) {
    self->data[ii + 1] = self->data[ii];
  }

  self->data[inIndex] = inData;
  self->len += 1;

  return 0;
}

int PointerHeapArray__pushFront(PointerHeapArray *self, void *inData)
{
  return PointerHeapArray__insertAtIndex(self, 0, inData);
}

int PointerHeapArray__popFront(PointerHeapArray *self)
{
  return PointerHeapArray__eraseAtIndex(self, 0);
}

void PointerHeapArray__clear(PointerHeapArray *self)
{
  self->len = 0;
}

int PointerHeapArray__sortFromCallback(PointerHeapArray *self, SortCallbackPtr inCallback, void* inUserData)
{
  int dataWasSorted = 0;

  // TODO: this is a "bubble sort", the simplest and slowest sorting algorithm
  int mustRepeat = 0;
  do {
    mustRepeat = 0;

    for (unsigned int ii = 0; ii + 1 < self->len; ++ii)
    {
      const int result = (*inCallback)(self->data[ii], self->data[ii + 1], inUserData);

      if (result < 0) {
        // swap pointers
        void* tmpData = self->data[ii];
        self->data[ii] = self->data[ii + 1];
        self->data[ii + 1] = tmpData;
        // set vars
        mustRepeat = 1;
        dataWasSorted = 1;
      }
    }

  }
  while (mustRepeat != 0);

  return dataWasSorted;
}
