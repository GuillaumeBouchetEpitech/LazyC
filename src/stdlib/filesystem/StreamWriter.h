
#pragma once

typedef struct StreamWriter StreamWriter;

StreamWriter *StreamWriter__create(const char *inFilepath);
int StreamWriter__write(StreamWriter *self, const char *inContentData, unsigned int inContentSize);
void StreamWriter__free(StreamWriter **self);
