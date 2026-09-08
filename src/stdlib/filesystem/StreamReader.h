
#pragma once

typedef struct StreamReader StreamReader;

StreamReader *StreamReader__create(const char *inFilepath);
unsigned int StreamReader__read(StreamReader *self, char *outBufferData, unsigned int inBufferSize);
void StreamReader__free(StreamReader **self);
