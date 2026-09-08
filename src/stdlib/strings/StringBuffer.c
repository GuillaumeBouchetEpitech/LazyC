
#include "./StringBuffer.h"

#include <stdlib.h>
#include <string.h>

StringBuffer* StringBuffer__create() {
  StringBuffer* newBuf = calloc(1, sizeof(StringBuffer));
  if (newBuf == NULL) {
    return NULL;
  }
  newBuf->len = 0;
  newBuf->cap = 16;
  newBuf->data = calloc(newBuf->cap, sizeof(char));
  return newBuf;
}

void StringBuffer__free(StringBuffer** self) {
  free((*self)->data);
  free(*self);
  *self = NULL;
}

static int _StringBuffer__grow(StringBuffer* self, unsigned int inNewCapacity) {
  if (self->cap >= inNewCapacity) {
    return 0;
  }
  while (self->cap < inNewCapacity) {
    self->cap *= 2;
  }
  self->data = realloc(self->data, self->cap);
  return self->data == NULL ? -1 : 0;
}

int StringBuffer__appendChar(StringBuffer* self, char inChar) {
  if (_StringBuffer__grow(self, self->len + 2) != 0) {
    return -1;
  }
  self->data[self->len++] = inChar;
  self->data[self->len] = '\0';
  return 0;
}

int StringBuffer__appendData(StringBuffer* self, unsigned int inSize, const char* inData) {
  if (_StringBuffer__grow(self, self->len + inSize + 1) != 0) {
    return -1;
  }
  memcpy(self->data + self->len, inData, inSize);
  self->len += inSize;
  self->data[self->len] = '\0';
  return 0;
}

int StringBuffer__appendCString(StringBuffer* self, const char* inCStr) {
  return StringBuffer__appendData(self, strlen(inCStr), inCStr);
}


