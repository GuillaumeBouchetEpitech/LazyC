
#pragma once

typedef struct StringBuffer {
  char *data;
  unsigned len;
  unsigned cap;
} StringBuffer;

StringBuffer* StringBuffer__create();
void StringBuffer__free(StringBuffer** self);
int StringBuffer__appendChar(StringBuffer* self, char inChar);
int StringBuffer__appendData(StringBuffer* self, unsigned int inSize, const char* inData);
int StringBuffer__appendCString(StringBuffer* self, const char* inCStr);

